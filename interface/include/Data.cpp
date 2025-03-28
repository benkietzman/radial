// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Data.cpp
// author     : Ben Kietzman
// begin      : 2025-03-11
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Data"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Data()
Data::Data(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), void (*pCallbackInotify)(string, const string, const string)) : Interface(strPrefix, "data", argc, argv, pCallback)
{
  map<string, list<string> > watches;
  size_t unBuffer = 16 * 1024 * 1024;
  string strBuffer;

  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-b" || (strArg.size() > 9 && strArg.substr(0, 9) == "--buffer="))
    {
      if (strArg == "-b" && i + 1 < argc && argv[i+1][0] != '-')
      {
        strBuffer = argv[++i];
      }
      else
      {
        strBuffer = strArg.substr(0, strArg.size() - 9);
      }
      m_manip.purgeChar(strBuffer, strBuffer, "'");
      m_manip.purgeChar(strBuffer, strBuffer, "\"");
    }
  }
  // }}}
  if (!strBuffer.empty())
  {
    unBuffer = atoi(strBuffer.c_str()) * 1024 * 1024;
  }
  m_pszBuffer = new char[unBuffer];
  for (size_t i = 0; i < unBuffer; i++)
  {
    m_buffers.push_back(-1);
  }
  sem_init(&m_semBuffer, 0, unBuffer);
  m_pUtility->setReadSize(4096);
  m_pUtility->setSslWriteSize(67108864);
  // {{{ functions
  m_functions["status"] = &Data::status;
  m_functions["token"] = &Data::token;
  // }}}
  m_c = NULL;
  m_pUtility->sslInit();
  load(strPrefix, true);
  m_pThreadDataAccept = new thread(&Data::dataAccept, this, strPrefix);
  pthread_setname_np(m_pThreadDataAccept->native_handle(), "dataAccept");
  watches[m_strData + "/data"] = {"config.json"};
  m_pThreadInotify = new thread(&Data::inotify, this, strPrefix, watches, pCallbackInotify);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
  m_pThreadSchedule = new thread(&Data::schedule, this, strPrefix);
  pthread_setname_np(m_pThreadSchedule->native_handle(), "schedule");
}
// }}}
// {{{ ~Data()
Data::~Data()
{
  m_pThreadSchedule->join();
  delete m_pThreadSchedule;
  m_pThreadInotify->join();
  delete m_pThreadInotify;
  m_pThreadDataAccept->join();
  delete m_pThreadDataAccept;
  if (m_c != NULL)
  {
    delete m_c;
  }
  m_pUtility->sslDeinit();
  sem_destroy(&m_semBuffer);
  delete[] m_pszBuffer;
}
// }}}
// {{{ callback()
void Data::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Data::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    string strFunction = ptJson->m["Function"]->v;
    radialUser d;
    userInit(ptJson, d);
    if (m_functions.find(strFunction) != m_functions.end())
    {
      if ((this->*m_functions[strFunction])(d, strError))
      {
        bResult = true;
      }
    }
    else
    {
      strError = "Please provide a valid Function.";
    }
    if (bResult)
    {
      if (exist(ptJson, "Response"))
      {
        delete ptJson->m["Response"];
      }
      ptJson->m["Response"] = d.p->m["o"];
      d.p->m.erase("o");
    }
    userDeinit(d);
  }
  else
  {
    strError = "Please provide the Function.";
  }
  ptJson->i("Status", ((bResult)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->i("Error", strError);
  }
  if (bResponse)
  {
    ptJson->j(p.p);
    hub(p, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
// {{{ callbackInotify()
void Data::callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Data::callbackInotify()";
  if (strPath == m_strData && strFile == "config.json")
  {
    load(strPrefix);
  }
}
// }}}
// {{{ data
// {{{ dataAccept()
void Data::dataAccept(string strPrefix)
{
  // {{{ prep work
  SSL_CTX *ctx = NULL;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Data::dataAccept()";
  SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
  // }}}
  if ((ctx = m_pUtility->sslInitServer(m_strData + "/server.crt", m_strData + "/server.key", strError)) != NULL)
  {
    // {{{ prep work
    addrinfo hints, *result;
    bool bBound[3] = {false, false, false};
    int fdSocket, nReturn;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((nReturn = getaddrinfo(NULL, "3282", &hints, &result)) == 0)
    {
      addrinfo *rp;
      bBound[0] = true;
      for (rp = result; !bBound[2] && rp != NULL; rp = rp->ai_next)
      {
        bBound[1] = false;
        if ((fdSocket = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
        {
          int nOn = 1;
          bBound[1] = true;
          setsockopt(fdSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&nOn, sizeof(nOn));
          if (bind(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
          {
            bBound[2] = true;
          }
          else
          {
            close(fdSocket);
          }
        }
      }
      freeaddrinfo(result);
    }
    // }}}
    if (bBound[2])
    {
      // {{{ prep work
      ssMessage.str("");
      ssMessage << strPrefix << "->bind():  Bound incoming socket.";
      log(ssMessage.str());
      // }}}
      if (listen(fdSocket, SOMAXCONN) == 0)
      {
        // {{{ prep work
        bool bExit = false;
        ssMessage.str("");
        ssMessage << strPrefix << "->listen():  Listening to incoming socket.";
        log(ssMessage.str());
        // }}}
        while (!bExit)
        {
          // {{{ prep work
          pollfd fds[1];
          fds[0].fd = fdSocket;
          fds[0].events = POLLIN;
          // }}}
          if ((nReturn = poll(fds, 1, 2000)) > 0)
          {
            // {{{ accept
            if (fds[0].revents & POLLIN)
            {
              int fdClient;
              sockaddr_in cli_addr;
              socklen_t clilen = sizeof(cli_addr);
              if ((fdClient = ::accept(fds[0].fd, (sockaddr *)&cli_addr, &clilen)) >= 0)
              {
                thread threadDataSocket(&Data::dataSocket, this, strPrefix, fdClient, ctx);
                pthread_setname_np(threadDataSocket.native_handle(), "dataSocket");
                threadDataSocket.detach();
              }
              else
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->accept(" << errno << ") error [accept," << fds[0].fd << "]:  " << strerror(errno);
                log(ssMessage.str());
              }
            }
            // }}}
          }
          else if (!bExit && nReturn < 0 && errno != EINTR)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
            log(ssMessage.str());
          }
          // {{{ post work
          if (shutdown())
          {
            bExit = true;
          }
          // }}}
        }
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->listen(" << errno << ") error:  " << strerror(errno);
        notify(ssMessage.str());
      }
      // {{{ post work
      close(fdSocket);
      // }}}
    }
    else if (!bBound[0])
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->getaddrinfo(" << nReturn << ") error:  "  << gai_strerror(nReturn);
      log(ssMessage.str());
    }
    else
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->" << ((!bBound[1])?"socket":"bind") << "(" << errno << ") error:  "  << strerror(errno);
      log(ssMessage.str());
    }
    // {{{ post work
    SSL_CTX_free(ctx);
    // }}}
  }
  // {{{ post work
  setShutdown();
  threadDecrement();
  // }}}
}
// }}}
// {{{ dataError()
void Data::dataError(int &fd, const string e)
{
  string b;
  Json *j = new Json;

  j->i("Status", "error");
  if (!e.empty())
  {
    j->i("Error", e);
  }
  j->j(b);
  delete j;
  b += "\n";
  if (fd != -1 && !b.empty())
  {
    bool bClose = false, bExit = false;
    int nReturn;
    while (!bExit)
    {
      pollfd fds[1];
      fds[0].fd = fd;
      fds[0].events = POLLOUT;
      if ((nReturn = poll(fds, 1, 2000)) > 0)
      {
        if (fds[0].revents & POLLOUT)
        {
          if (m_pUtility->fdWrite(fds[0].fd, b, nReturn))
          {
            if (b.empty())
            {
              bExit = true;
            }
          }
          else
          {
            bClose = bExit = true;
          }
        }
        if (fds[0].revents & (POLLERR | POLLNVAL))
        {
          bClose = bExit = true;
        }
      }
      else if (!bExit && nReturn < 0 && errno != EINTR)
      {
        bClose = bExit = true;
      }
    }
    if (bClose)
    {
      close(fd);
      fd = -1;
    }
  }
}
// }}}
// {{{ dataResponse()
void Data::dataResponse(const string t, int &fd)
{
  // {{{ prep work
  stringstream ssMessage;
  Json *i = NULL;
  threadIncrement();
  m_mutex.lock();
  if (m_dataRequests.find(t) != m_dataRequests.end())
  {
    i = m_dataRequests[t];
    m_dataRequests.erase(t);
  }
  m_mutex.unlock();
  // }}}
  if (i != NULL)
  {
    if (!empty(i, "_path"))
    {
      bool bExit = false;
      int fdData, nReturn;
      string b, p, t;
      stringstream sp;
      Json *j;
      sp << i->m["_path"]->v;
      delete i->m["_path"];
      i->m.erase("_path");
      if (exist(i, "path"))
      {
        for (auto &item : i->m["path"]->l)
        {
          if (!item->v.empty())
          {
            sp << "/" << item->v;
          }
        }
      }
      p = sp.str();
      // {{{ directory
      if (m_file.directoryExist(p))
      {
        DIR *pDir;
        if ((pDir = opendir(p.c_str())) != NULL)
        {
          string n, strType;
          struct dirent *ptEntry;
          j = new Json;
          j->i("Status", "okay");
          j->i("Type", "directory");
          j->j(b);
          delete j;
          b.append("\n");
          j = new Json;
          while ((ptEntry = readdir(pDir)) != NULL)
          {
            n = ptEntry->d_name;
            if (n != "." && n != "..")
            {
              j->m[n] = new Json;
              switch (ptEntry->d_type)
              {
                case DT_BLK     : strType = "block device";           break;
                case DT_CHR     : strType = "character device";       break;
                case DT_DIR     : strType = "directory";              break;
                case DT_FIFO    : strType = "named pipe";             break;
                case DT_LNK     : strType = "symbolic link";          break;
                case DT_REG     : strType = "regular file";           break;
                case DT_SOCK    : strType = "UNIX domain socket";     break;
                case DT_UNKNOWN : strType = "unknown";                break;
                default         : strType = "undefined";
              }
              j->m[n]->i("Type", strType);
              if (ptEntry->d_type == DT_REG)
              {
                struct stat tStat;
                if (stat((p + (string)"/" + n).c_str(), &tStat) == 0)
                {
                  j->m[n]->i("Size", to_string(tStat.st_size), 'n');
                }
              }
            }
          }
          closedir(pDir);
          j->j(t);
          delete j;
          t.append("\n");
          b.append(t);
          while (!bExit)
          {
            pollfd fds[1];
            fds[0].fd = fd;
            fds[0].events = POLLIN;
            if (!b.empty())
            {
              fds[0].events |= POLLOUT;
            }
            if ((nReturn = poll(fds, 1, 2000)) > 0)
            {
              if ((fds[0].revents & (POLLIN | POLLHUP)) && !m_pUtility->fdRead(fds[0].fd, t, nReturn))
              {
                bExit = true;
              }
              if ((fds[0].revents & POLLOUT) && (!m_pUtility->fdWrite(fds[0].fd, b, nReturn) || b.empty()))
              {
                bExit = true;
              }
              if (fds[0].revents & (POLLERR | POLLNVAL))
              {
                bExit = true;
              }
            }
            else if (nReturn < 0 && errno != EINTR)
            {
              bExit = true;
            }
          }
        }
        else
        {
          ssMessage.str("");
          ssMessage << "opendir(" << errno << ") " << strerror(errno);
          dataError(fd, ssMessage.str());
        }
      }
      // }}}
      // {{{ file
      else
      {
        sem_wait(&m_semBuffer);
        if ((fdData = open(p.c_str(), O_RDONLY)) >= 0)
        {
          bool bClose = false;
          char *pszBuffer = NULL;
          size_t unBuffer;
          m_mutex.lock();
          for (size_t i = 0; pszBuffer == NULL && i < m_buffers.size(); i++)
          {
            if (m_buffers[i] == -1)
            {
              unBuffer = i;
              m_buffers[i] = fdData;
              pszBuffer = m_pszBuffer + i * 1024 * 512;
            }
          }
          m_mutex.unlock();
          if (pszBuffer != NULL)
          {
            size_t unLength = 0;
            j = new Json;
            j->i("Status", "okay");
            j->i("Type", "file");
            j->j(b);
            delete j;
            b.append("\n");
            memcpy(pszBuffer, b.c_str(), b.size());
            unLength = b.size();
            b.clear();
            while (!bExit)
            {
              pollfd fds[2];
              fds[0].fd = ((unLength < 1024 * 512)?fdData:-1);
              fds[0].events = POLLIN;
              fds[1].fd = fd;
              fds[1].events = POLLIN;
              if (unLength > 0)
              {
                fds[1].events |= POLLOUT;
              }
              if ((nReturn = poll(fds, 2, 2000)) > 0)
              {
                if (fds[0].revents & POLLIN)
                {
                  if ((nReturn = read(fds[0].fd, (pszBuffer + unLength), (1024 * 512 - unLength))) >= 0)
                  {
                    unLength += nReturn;
                  }
                  else if (nReturn == 0)
                  {
                    bClose = true;
                  }
                  else
                  {
                    bExit = true;
                  }
                }
                if (fds[0].revents & (POLLERR | POLLNVAL))
                {
                  bExit = true;
                }
                if ((fds[1].revents & (POLLHUP | POLLIN)) && !m_pUtility->fdRead(fds[1].fd, t, nReturn))
                {
                  bExit = true;
                }
                if (fds[1].revents & POLLOUT)
                {
                  if ((nReturn = write(fds[1].fd, pszBuffer, unLength)) >= 0)
                  {
                    unLength -= nReturn;
                    if (unLength > 0)
                    {
                      memcpy((pszBuffer + 1024 * 512), (pszBuffer + nReturn), unLength);
                      memcpy(pszBuffer, (pszBuffer + 1024 * 512), unLength);
                    }
                  }
                  else
                  {
                    bExit = true;
                  }
                }
                if (fds[1].revents & (POLLERR | POLLNVAL))
                {
                  bExit = true;
                }
              }
              else if (nReturn < 0 && errno != EINTR)
              {
                bExit = true;
              }
              if (bClose)
              {
                if (fdData != -1)
                {
                  close(fdData);
                  fdData = -1;
                }
                if (unLength == 0)
                {
                  bExit = true;
                }
              }
            }
            mutex.lock();
            m_buffers[unBuffer] = -1;
            mutex.unlock();
          }
          else
          {
            dataError(fd, "Buffer unavailable.");
          }
          if (!bClose)
          {
            close(fdData);
          }
        }
        else
        {
          ssMessage.str("");
          ssMessage << "open(" << errno << ") " << strerror(errno);
          dataError(fd, ssMessage.str());
        }
        sem_post(&m_semBuffer);
      }
      // }}}
    }
    else
    {
      dataError(fd, "Please provide the _path.");
    }
    delete i;
  }
  else
  {
    dataError(fd, "Please provide a valid token.");
  }
  // {{{ post work
  close(fd);
  threadDecrement();
  // }}}
}
// }}}
// {{{ dataSocket()
void Data::dataSocket(string strPrefix, int fdSocket, SSL_CTX *ctx)
{
  // {{{ prep work
  string strError;
  stringstream ssMessage;
  SSL *ssl;
  strPrefix += "->Data::dataSocket()";
  threadIncrement();
  // }}}
  if ((ssl = m_pUtility->sslAccept(ctx, fdSocket, strError)) != NULL)
  {
    int fdResponse[2] = {-1, -1}, nReturn;
    if ((nReturn = pipe(fdResponse)) == 0)
    {
      // {{{ prep work
      bool bExit = false, bNeedWrite = false, bToken = false, bWantWrite = false;
      size_t unPosition;
      string strBuffers[3];
      // }}}
      while (!bExit)
      {
        // {{{ prep work
        pollfd fds[2];
        fds[0].fd = fdSocket;
        fds[0].events = POLLIN;
        if (!bNeedWrite && !strBuffers[1].empty())
        {
          strBuffers[2].append(strBuffers[1]);
          strBuffers[1].clear();
        }
        if (bWantWrite || !strBuffers[2].empty())
        {
          fds[0].events |= POLLOUT;
        }
        fds[1].fd = fdResponse[0];
        fds[1].events = POLLIN;
        // }}}
        if ((nReturn = poll(fds, 2, 2000)) > 0)
        {
          bool bReadable = (fds[0].revents & (POLLHUP | POLLIN)), bWritable = (fds[0].revents & POLLOUT);
          if (bReadable)
          {
            if (m_pUtility->sslRead(ssl, strBuffers[0], nReturn))
            {
              bWantWrite = false;
              if (nReturn <= 0)
              {
                switch (SSL_get_error(ssl, nReturn))
                {
                  case SSL_ERROR_WANT_WRITE: bWantWrite = true; break;
                }
              }
              if (bToken)
              {
                strBuffers[0].clear();
              }
              else if ((unPosition = strBuffers[0].find("\n")) != string::npos)
              {
                string strToken = strBuffers[0].substr(0, unPosition);
                strBuffers[0].erase(0, (unPosition + 1));
                bToken = true;
                m_mutex.lock();
                if (m_dataTokens.find(strToken) != m_dataTokens.end())
                {
                  m_dataTokens.erase(strToken);
                  thread threadDataRequest(&Data::dataResponse, this, strToken, ref(fdResponse[1]));
                  pthread_setname_np(threadDataRequest.native_handle(), "dataResponse");
                  threadDataRequest.detach();
                }
                else
                {
                  Json *ptJson = new Json;
                  close(fdResponse[0]);
                  fdResponse[0] = -1;
                  close(fdResponse[1]);
                  fdResponse[1] = -1;
                  ptJson->i("Status", "error");
                  ptJson->i("Error", "Please provide a valid Token.");
                  ptJson->j(strBuffers[1]);
                  delete ptJson;
                  strBuffers[1] += "\n";
                }
                m_mutex.unlock();
              }
              else if (strBuffers[0].size() > 33)
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslRead() error:  Incoming data larger than token.";
                log(ssMessage.str());
              }
            }
            else
            {
              bExit = true;
              if (nReturn < 0 && errno != 104)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslRead(" << SSL_get_error(ssl, nReturn) << ") error:  " << m_pUtility->sslstrerror(ssl, nReturn);
                log(ssMessage.str());
              }
            }
          }
          if (bWritable)
          {
            if (m_pUtility->sslWrite(ssl, strBuffers[2], nReturn))
            {
              bNeedWrite = bWantWrite = false;
              if (nReturn <= 0)
              {
                switch (SSL_get_error(ssl, nReturn))
                {
                  case SSL_ERROR_WANT_READ: bNeedWrite = true; break;
                  case SSL_ERROR_WANT_WRITE: bNeedWrite = bWantWrite = true; break;
                }
              }
            }
            else
            {
              bExit = true;
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslWrite(" << SSL_get_error(ssl, nReturn) << ") error:  " << m_pUtility->sslstrerror(ssl, nReturn);
                log(ssMessage.str());
              }
            }
          }
          if (fds[0].revents & POLLERR)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->poll() error [" << fds[0].fd << "]:  Encountered a POLLERR.";
            log(ssMessage.str());
          }
          if (fds[0].revents & POLLNVAL)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->poll() error [" << fds[0].fd << "]:  Encountered a POLLNVAL.";
            log(ssMessage.str());
          }
          if (fds[1].revents & (POLLHUP | POLLIN))
          {
            if (!m_pUtility->fdRead(fds[1].fd, strBuffers[1], nReturn))
            {
              close(fdResponse[0]);
              fdResponse[0] = -1;
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::fdRead(" << errno << ") error:  " << strerror(errno);
                log(ssMessage.str());
              }
            }
          }
          if (fds[1].revents & POLLERR)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->poll() error [" << fds[1].fd << "]:  Encountered a POLLERR.";
            log(ssMessage.str());
          }
          if (fds[1].revents & POLLNVAL)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->poll() error [" << fds[1].fd << "]:  Encountered a POLLNVAL.";
            log(ssMessage.str());
          }
        }
        else if (nReturn < 0 && errno != EINTR)
        {
          bExit = true;
          ssMessage.str("");
          ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
          log(ssMessage.str());
        }
        // {{{ post work
        if ((fdResponse[0] == -1 && strBuffers[1].empty() && strBuffers[2].empty()) || shutdown())
        {
          bExit = true;
        }
        // }}}
      }
      // {{{ post work
      if (fdResponse[0] != -1)
      {
        close(fdResponse[0]);
        fdResponse[0] = -1;
      }
      if (fdResponse[1] != -1)
      {
        close(fdResponse[1]);
        fdResponse[1] = -1;
      }
      // }}}
    }
    else
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->pipe(" << errno << ") error:  " << strerror(errno);
      log(ssMessage.str());
    }
    // {{{ post work
    if (SSL_shutdown(ssl) == 0)
    {
      SSL_shutdown(ssl);
    }
    SSL_free(ssl);
    // }}}
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->Utility::sslAccept() error [accept," << fdSocket << "]:  " << strError;
    log(ssMessage.str());
  }
  // {{{ post work
  close(fdSocket);
  threadDecrement();
  // }}}
}
// }}}
// }}}
// {{{ load()
void Data::load(string strPrefix, const bool bSilent)
{
  ifstream inConf;
  stringstream ssConf, ssMessage;

  strPrefix += "->Data::load()";
  ssConf << m_strData << "/data/config.json";
  inConf.open(ssConf.str());
  if (inConf)
  {
    string strLine;
    stringstream ssJson;
    while (getline(inConf, strLine))
    {
      ssJson << strLine;
    }
    m_mutex.lock();
    if (m_c != NULL)
    {
      delete m_c;
    }
    m_c = new Json(ssJson.str());
    m_mutex.unlock();
  }
  else if (!bSilent)
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->ifstream::open(" << errno << ") error [" << m_strData << "/data/config.json]:  " << strerror(errno);
    log(ssMessage.str());
  }
  inConf.close();
}
// }}}
// {{{ schedule()
void Data::schedule(string strPrefix)
{
  // {{{ prep work
  string strError;
  stringstream ssMessage, ssQuery;
  time_t CTime[2] = {0, 0};

  threadIncrement();
  strPrefix += "->Data::schedule()";
  time(&(CTime[1]));
  // }}}
  while (!shutdown())
  {
    time(&(CTime[0]));
    // {{{ status
    if ((CTime[0] - CTime[1]) >= 60)
    {
      list<string> removals;
      Json *ptMessage = new Json;
      CTime[1] = CTime[0];
      ptMessage->i("Source", m_strNode);
      status(ptMessage);
      ptMessage->i("Action", "status");
      live("Data", "", ptMessage);
      delete ptMessage;
      m_mutex.lock();
      for (auto &hash : m_dataTokens)
      {
        if ((CTime[0] - hash.second) > 60)
        {
          removals.push_back(hash.first);
        }
      }
      while (!removals.empty())
      {
        m_dataTokens.erase(removals.front());
        if (m_dataRequests.find(removals.front()) != m_dataRequests.end())
        {
          delete m_dataRequests[removals.front()];
          m_dataRequests.erase(removals.front());
        }
        removals.pop_front();
      }
      m_mutex.unlock();
    }
    // }}}
    msleep(1000);
  }
  // {{{ post work
  setShutdown();
  threadDecrement();
  // }}}
}
// }}}
// {{{ token()
bool Data::token(radialUser &d, string &e)
{
  bool b = false;
  Json *c, *i = d.p->m["i"], *o = d.p->m["o"];

  m_mutex.lock();
  c = new Json(m_c);
  m_mutex.unlock();
  if (c != NULL)
  {
    if (!empty(i, "handle"))
    {
      bool bValidPath = true;
      if (exist(i, "path"))
      {
        for (auto p = i->m["path"]->l.begin(); bValidPath && p != i->m["path"]->l.end(); p++)
        {
          if ((*p)->v == "." || (*p)->v == ".." || (*p)->v.find("/") != string::npos)
          {
            bValidPath = false;
          }
        }
      }
      if (bValidPath)
      {
        if (exist(c, i->m["handle"]->v))
        {
          auto nodeIter = c->m[i->m["handle"]->v]->m.end();
          for (auto n = c->m[i->m["handle"]->v]->m.begin(); nodeIter == c->m[i->m["handle"]->v]->m.end() && n != c->m[i->m["handle"]->v]->m.end(); n++)
          {
            if (n->first == m_strNode)
            {
              nodeIter = n;
            }
          }
          if (nodeIter != c->m[i->m["handle"]->v]->m.end())
          {
            if (!nodeIter->second->v.empty())
            {
              char md5string[33];
              EVP_MD_CTX *ctx = EVP_MD_CTX_create();
              string strIdent, t;
              stringstream ssIdent;
              timespec start;
              unsigned char digest[16];
              b = true;
              clock_gettime(CLOCK_REALTIME, &start);
              ssIdent << m_strNode << "," << i->m["handle"]->v << "," << start.tv_sec << "," << start.tv_nsec;
              strIdent = ssIdent.str();
              EVP_DigestInit(ctx, EVP_md5());
              EVP_DigestUpdate(ctx, strIdent.c_str(), strIdent.size());
              EVP_DigestFinal(ctx, digest, NULL);
              EVP_MD_CTX_destroy(ctx);
              for (int j = 0; j < 16; j++)
              {
                sprintf(&md5string[j*2], "%02x", (unsigned int)digest[j]);
              }
              t = md5string;
              o->i("node", m_strNode);
              o->i("token", t);
              i->i("_path", nodeIter->second->v);
              delete i->m["handle"];
              i->m.erase("handle");
              m_mutex.lock();
              m_dataTokens[t] = start.tv_sec;
              m_dataRequests[t] = new Json(i);
              m_mutex.unlock();
            }
            else
            {
              e = "Path not defined for handle.";
            }
          }
          else
          {
            vector<string> nodes;
            for (auto &node : c->m[i->m["handle"]->v]->m)
            {
              if (node.first != m_strNode)
              {
                nodes.push_back(node.first);
              }
            }
            if (!nodes.empty())
            {
              unsigned int unSeed = time(NULL);
              Json *ptLink = new Json(d.r);
              ptLink->i("Interface", "data");
              ptLink->i("Node", nodes[rand_r(&unSeed) % nodes.size()]);
              if (hub("link", ptLink, e))
              {
                if (exist(ptLink, "Response"))
                {
                  o->merge(ptLink->m["Response"], true, false);
                }
                else
                {
                  e = "Failed to receive the Response.";
                }
              }
              delete ptLink;
            }
            else
            {
              e = "No available nodes for this handle.";
            }
          }
        }
        else
        {
          e = "Plesae provide a valid handle.";
        }
      }
      else
      {
        e = "Please provide a valid path.";
      }
    }
    else
    {
      e = "Please provide the handle.";
    }
  }
  else
  {
    e = "Config not loaded.";
  }
  delete c;

  return b;
}
// }}}
}
}
