// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Data.cpp
// author     : Ben Kietzman
// begin      : 2025-03-11
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
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
  size_t unSlots = 16; // Number of 1 MB slots within the buffer.
  string strSlots;

  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-s" || (strArg.size() > 8 && strArg.substr(0, 8) == "--slots="))
    {
      if (strArg == "-b" && i + 1 < argc && argv[i+1][0] != '-')
      {
        strSlots = argv[++i];
      }
      else
      {
        strSlots = strArg.substr(8, strArg.size() - 8);
      }
      m_manip.purgeChar(strSlots, strSlots, "'");
      m_manip.purgeChar(strSlots, strSlots, "\"");
    }
  }
  // }}}
  // {{{ initialize buffer
  // [<-------------buffer------------->]
  // [<--slot-->][<--slot-->][<--slot-->]
  //
  // [<------------------------------------------------slot------------------------------------------------->]
  // [<---------------------socket---------------------->|<----------------------file----------------------->]
  // [<---------read---------->|<---------write--------->|<---------read---------->|<---------write--------->]
  // [<--buffer-->|<---temp--->|<--buffer-->|<---temp--->|<--buffer-->|<---temp--->|<--buffer-->|<---temp--->]
  if (!strSlots.empty())
  {
    unSlots = atoi(strSlots.c_str());
  }
  m_pszBuffer = new char[unSlots * 1024 * 1024];
  for (size_t i = 0; i < unSlots; i++)
  {
    m_buffers.push_back(-1);
  }
  sem_init(&m_semBuffer, 0, unSlots);
  // }}}
  m_pUtility->setReadSize(4096);
  m_pUtility->setSslWriteSize(67108864);
  // {{{ functions
  m_functions["dirAdd"] = &Data::dirAdd;
  m_functions["dirList"] = &Data::dirList;
  m_functions["fileAppend"] = &Data::fileAppend;
  m_functions["fileRead"] = &Data::fileRead;
  m_functions["fileRemove"] = &Data::fileRemove;
  m_functions["fileWrite"] = &Data::fileWrite;
  m_functions["status"] = &Data::status;
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
      d.p->m["i"]->insert("_function", strFunction);
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
    char *pszBuffer = NULL;
    size_t unBuffer;
    sem_wait(&m_semBuffer);
    m_mutex.lock();
    for (size_t i = 0; pszBuffer == NULL && i < m_buffers.size(); i++)
    {
      if (m_buffers[i] == -1)
      {
        unBuffer = i;
        m_buffers[i] = fdSocket;
        pszBuffer = m_pszBuffer + (i * 1048576);
      }
    }
    m_mutex.unlock();
    if (pszBuffer != NULL)
    {
      // {{{ prep work
      bool bFileClose = false, bExit = false, bNeedWrite = false, bSocketClose = false, bToken = false, bWantWrite = false;
      char *pszFileReadBuffer, *pszFileReadTemp, *pszFileWriteBuffer, *pszFileWriteTemp, *pszSocketReadBuffer, *pszSocketReadTemp, *pszSocketWriteBuffer, *pszSocketWriteTemp;
      int fdFile = -1, nReturn;
      size_t unFileReadLength = 0, unFileWriteLength = 0, unLength, unPosition, unSize = 131072, unSocketReadLength = 0, unSocketWriteLength = 0;
      string strFileWriteBuffer, strFunction, strSocketReadBuffer, strSocketWriteBuffer, strPath;
      pszSocketReadBuffer = pszBuffer;
      pszSocketReadTemp = pszSocketReadBuffer + unSize;
      pszSocketWriteBuffer = pszSocketReadTemp + unSize;
      pszSocketWriteTemp = pszSocketWriteBuffer + unSize;
      pszFileReadBuffer = pszSocketWriteTemp + unSize;
      pszFileReadTemp = pszFileReadBuffer + unSize;
      pszFileWriteBuffer = pszFileReadTemp + unSize;
      pszFileWriteTemp = pszFileWriteBuffer + unSize;
      // }}}
      while (!bExit)
      {
        // {{{ prep work
        pollfd fds[2];
        fds[0].fd = -1;
        fds[0].events = 0;
        fds[1].fd = -1;
        fds[1].events = 0;
        // {{{ socket read --> socket read buffer
        if (bToken && !strSocketReadBuffer.empty() && unSocketReadLength < unSize)
        {
          unLength = ((strSocketReadBuffer.size() > (unSize - unSocketReadLength))?(unSize - unSocketReadLength):strSocketReadBuffer.size());
          memcpy((pszSocketReadBuffer + unSocketReadLength), strSocketReadBuffer.c_str(), unLength);
          strSocketReadBuffer.erase(0, unLength);
          unSocketReadLength += unLength;
        }
        // }}}
        if (!bSocketClose && strSocketReadBuffer.size() < 2048)
        {
          fds[0].fd = fdSocket;
          fds[0].events = POLLIN;
        }
        // {{{ socket read buffer --> file write buffer
        if (unSocketReadLength > 0 && unFileWriteLength < unSize)
        {
          unLength = ((unSocketReadLength > (unSize - unFileWriteLength))?(unSize - unFileWriteLength):unSocketReadLength);
          memcpy((pszFileWriteBuffer + unFileWriteLength), pszSocketReadBuffer, unLength);
          unFileWriteLength += unLength;
          if (unLength < unSocketReadLength)
          {
            memcpy(pszSocketReadTemp, (pszSocketReadBuffer + unLength), (unSocketReadLength - unLength));
            memcpy(pszSocketReadBuffer, pszSocketReadTemp, (unSocketReadLength - unLength));
            unSocketReadLength -= unLength;
          }
          else
          {
            unSocketReadLength = 0;
          }
        }
        // }}}
        if (!bFileClose && unFileWriteLength > 0)
        {
          fds[1].fd = fdFile;
          fds[1].events = POLLOUT;
        }
        if (!bNeedWrite && unSocketWriteLength < unSize)
        {
          // {{{ socket write --> socket write buffer
          if (!strSocketWriteBuffer.empty())
          {
            unLength = ((strSocketWriteBuffer.size() > (unSize - unSocketWriteLength))?(unSize - unSocketWriteLength):strSocketWriteBuffer.size());
            memcpy((pszSocketWriteBuffer + unSocketWriteLength), strSocketWriteBuffer.c_str(), unLength);
            strSocketWriteBuffer.erase(0, unLength);
            unSocketWriteLength += unLength;
          }
          // }}}
          // {{{ file read buffer --> socket write buffer
          else if (unFileReadLength > 0)
          {
            unLength = ((unFileReadLength > (unSize - unSocketWriteLength))?(unSize - unSocketWriteLength):unFileReadLength);
            memcpy((pszSocketWriteBuffer + unSocketWriteLength), pszFileReadBuffer, unLength);
            unSocketWriteLength += unLength;
            if (unLength < unFileReadLength)
            {
              memcpy(pszFileReadTemp, (pszFileReadBuffer + unLength), (unFileReadLength - unLength));
              memcpy(pszFileReadBuffer, pszFileReadTemp, (unFileReadLength - unLength));
              unFileReadLength -= unLength;
            }
            else
            {
              unFileReadLength = 0;
            }
          }
          // }}}
        }
        if (!bSocketClose && (bWantWrite || unSocketWriteLength > 0))
        {
          fds[0].fd = fdSocket;
          fds[0].events |= POLLOUT;
        }
        if (!bFileClose && unFileReadLength < unSize)
        {
          fds[1].fd = fdFile;
          fds[1].events |= POLLIN;
        }
        // }}}
        if ((nReturn = poll(fds, 2, 2000)) > 0)
        {
          // {{{ socket
          bool bReadable = (fds[0].revents & (POLLHUP | POLLIN)), bWritable = (fds[0].revents & POLLOUT);
          // {{{ read
          if (bReadable)
          {
            if (m_pUtility->sslRead(ssl, strSocketReadBuffer, nReturn))
            {
              bWantWrite = false;
              if (nReturn <= 0)
              {
                switch (SSL_get_error(ssl, nReturn))
                {
                  case SSL_ERROR_WANT_WRITE: bWantWrite = true; break;
                }
              }
              // {{{ need token
              if (!bToken)
              {
                if ((unPosition = strSocketReadBuffer.find("\n")) != string::npos)
                {
                  string strToken = strSocketReadBuffer.substr(0, unPosition);
                  Json *i = NULL, *j;
                  strSocketReadBuffer.erase(0, (unPosition + 1));
                  bToken = true;
                  m_mutex.lock();
                  if (m_dataTokens.find(strToken) != m_dataTokens.end())
                  {
                    m_dataTokens.erase(strToken);
                    if (m_dataRequests.find(strToken) != m_dataRequests.end())
                    {
                      i = m_dataRequests[strToken];
                      m_dataRequests.erase(strToken);
                    }
                    else
                    {
                      strError = "Failed to locate request from token.";
                    }
                  }
                  else
                  {
                    strError = "Please provide a valid token.";
                  }
                  m_mutex.unlock();
                  if (i != NULL)
                  {
                    if (!empty(i, "_function"))
                    {
                      strFunction = i->m["_function"]->v; 
                      if (!empty(i, "_path"))
                      {
                        stringstream ssPath;
                        ssPath << i->m["_path"]->v;
                        if (exist(i, "path"))
                        {
                          for (auto &item : i->m["path"]->l)
                          {
                            if (!item->v.empty())
                            {
                              ssPath << "/" << item->v;
                            }
                          }
                        }
                        strPath = ssPath.str();
                        // {{{ dirAdd
                        if (strFunction == "dirAdd")
                        {
                          mode_t mode = 00775;
                          bFileClose = true;
                          if (mkdir(strPath.c_str(), mode) == 0)
                          {
                            j = new Json;
                            j->i("Status", "okay");
                            j->j(strSocketWriteBuffer);
                            delete j;
                            strSocketWriteBuffer.append("\n");
                          }
                          else
                          {
                            j = new Json;
                            j->i("Status", "error");
                            ssMessage.str("");
                            ssMessage << "mkdir(" << errno << ") " << strerror(errno);
                            j->i("Error", ssMessage.str());
                            j->j(strSocketWriteBuffer);
                            delete j;
                            strSocketWriteBuffer.append("\n");
                          }
                        }
                        // }}}
                        // {{{ dirList
                        else if (strFunction == "dirList")
                        {
                          struct stat tStat;
                          bFileClose = true;
                          if (stat(strPath.c_str(), &tStat) == 0)
                          {
                            if (S_ISDIR(tStat.st_mode))
                            {
                              DIR *pDir;
                              if ((pDir = opendir(strPath.c_str())) != NULL)
                              {
                                string n, strType, v;
                                struct dirent *ptEntry;
                                j = new Json;
                                j->i("Status", "okay");
                                j->j(strSocketWriteBuffer);
                                delete j;
                                strSocketWriteBuffer.append("\n");
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
                                      if (stat((strPath + (string)"/" + n).c_str(), &tStat) == 0)
                                      {
                                        j->m[n]->i("Size", to_string(tStat.st_size), 'n');
                                      }
                                    }
                                  }
                                }
                                closedir(pDir);
                                j->j(v);
                                delete j;
                                v.append("\n");
                                strSocketWriteBuffer.append(v);
                              }
                              else
                              {
                                j = new Json;
                                j->i("Status", "error");
                                ssMessage.str("");
                                ssMessage << "opendir(" << errno << ") " << strerror(errno);
                                j->i("Error", ssMessage.str());
                                j->j(strSocketWriteBuffer);
                                delete j;
                                strSocketWriteBuffer.append("\n");
                              }
                            }
                            else
                            {
                              j = new Json;
                              j->i("Status", "error");
                              ssMessage.str("");
                              ssMessage << "S_ISDIR() Not a directory.";
                              j->i("Error", ssMessage.str());
                              j->j(strSocketWriteBuffer);
                              delete j;
                              strSocketWriteBuffer.append("\n");
                            }
                          }
                          else
                          {
                            j = new Json;
                            j->i("Status", "error");
                            ssMessage.str("");
                            ssMessage << "stat(" << errno << ") " << strerror(errno);
                            j->i("Error", ssMessage.str());
                            j->j(strSocketWriteBuffer);
                            delete j;
                            strSocketWriteBuffer.append("\n");
                          }
                        }
                        // }}}
                        // {{{ dirRemove
                        else if (strFunction == "dirRemove")
                        {
                          struct stat tStat;
                          bFileClose = true;
                          if (stat(strPath.c_str(), &tStat) == 0)
                          {
                            if (S_ISDIR(tStat.st_mode))
                            {
                              if (remove(strPath.c_str()) == 0)
                              {
                                j = new Json;
                                j->i("Status", "okay");
                                j->j(strSocketWriteBuffer);
                                delete j;
                                strSocketWriteBuffer.append("\n");
                              }
                              else
                              {
                                j = new Json;
                                j->i("Status", "error");
                                ssMessage.str("");
                                ssMessage << "remove(" << errno << ") " << strerror(errno);
                                j->i("Error", ssMessage.str());
                                j->j(strSocketWriteBuffer);
                                delete j;
                                strSocketWriteBuffer.append("\n");
                              }
                            }
                            else
                            {
                              j = new Json;
                              j->i("Status", "error");
                              ssMessage.str("");
                              ssMessage << "S_ISDIR() Not a directory.";
                              j->i("Error", ssMessage.str());
                              j->j(strSocketWriteBuffer);
                              delete j;
                              strSocketWriteBuffer.append("\n");
                            }
                          }
                          else
                          {
                            j = new Json;
                            j->i("Status", "error");
                            ssMessage.str("");
                            ssMessage << "stat(" << errno << ") " << strerror(errno);
                            j->i("Error", ssMessage.str());
                            j->j(strSocketWriteBuffer);
                            delete j;
                            strSocketWriteBuffer.append("\n");
                          }
                        }
                        // }}}
                        // {{{ fileAppend | fileWrite
                        else if (strFunction == "fileAppend" || strFunction == "fileWrite")
                        {
                          int nFlags = O_WRONLY | O_CREAT;
                          mode_t mode = 00664;
                          if (strFunction == "fileAppend")
                          {
                            nFlags |= O_APPEND;
                          }
                          if ((fdFile = open(strPath.c_str(), nFlags, mode)) >= 0)
                          {
                            j = new Json;
                            j->i("Status", "okay");
                            j->j(strSocketWriteBuffer);
                            delete j;
                            strSocketWriteBuffer.append("\n");
                          }
                          else
                          {
                            j = new Json;
                            j->i("Status", "error");
                            ssMessage.str("");
                            ssMessage << "open(" << errno << ") " << strerror(errno);
                            j->i("Error", ssMessage.str());
                            j->j(strSocketWriteBuffer);
                            delete j;
                            strSocketWriteBuffer.append("\n");
                          }
                        }
                        // }}}
                        // {{{ fileRead
                        else if (strFunction == "fileRead")
                        {
                          if ((fdFile = open(strPath.c_str(), O_RDONLY)) >= 0)
                          {
                            j = new Json;
                            j->i("Status", "okay");
                            j->j(strSocketWriteBuffer);
                            delete j;
                            strSocketWriteBuffer.append("\n");
                          }
                          else
                          {
                            j = new Json;
                            j->i("Status", "error");
                            ssMessage.str("");
                            ssMessage << "open(" << errno << ") " << strerror(errno);
                            j->i("Error", ssMessage.str());
                            j->j(strSocketWriteBuffer);
                            delete j;
                            strSocketWriteBuffer.append("\n");
                          }
                        }
                        // }}}
                        // {{{ fileRemove
                        else if (strFunction == "fileRemove")
                        {
                          struct stat tStat;
                          bFileClose = true;
                          if (stat(strPath.c_str(), &tStat) == 0)
                          {
                            if (S_ISREG(tStat.st_mode))
                            {
                              if (remove(strPath.c_str()) == 0)
                              {
                                j = new Json;
                                j->i("Status", "okay");
                                j->j(strSocketWriteBuffer);
                                delete j;
                                strSocketWriteBuffer.append("\n");
                              }
                              else
                              {
                                j = new Json;
                                j->i("Status", "error");
                                ssMessage.str("");
                                ssMessage << "remove(" << errno << ") " << strerror(errno);
                                j->i("Error", ssMessage.str());
                                j->j(strSocketWriteBuffer);
                                delete j;
                                strSocketWriteBuffer.append("\n");
                              }
                            }
                            else
                            {
                              j = new Json;
                              j->i("Status", "error");
                              ssMessage.str("");
                              ssMessage << "S_ISREG() Not a regular file.";
                              j->i("Error", ssMessage.str());
                              j->j(strSocketWriteBuffer);
                              delete j;
                              strSocketWriteBuffer.append("\n");
                            }
                          }
                          else
                          {
                            j = new Json;
                            j->i("Status", "error");
                            ssMessage.str("");
                            ssMessage << "stat(" << errno << ") " << strerror(errno);
                            j->i("Error", ssMessage.str());
                            j->j(strSocketWriteBuffer);
                            delete j;
                            strSocketWriteBuffer.append("\n");
                          }
                        }
                        // }}}
                        // {{{ invalid
                        else
                        {
                          bFileClose = true;
                          j = new Json;
                          j->i("Status", "error");
                          strError = "Please provide a valid Function.";
                          j->i("Error", strError);
                          j->j(strSocketWriteBuffer);
                          delete j;
                          strSocketWriteBuffer.append("\n");
                        }
                        // }}}
                      }
                      else
                      {
                        bFileClose = true;
                        j = new Json;
                        j->i("Status", "error");
                        strError = "Please provide the _path.";
                        j->i("Error", strError);
                        j->j(strSocketWriteBuffer);
                        delete j;
                        strSocketWriteBuffer.append("\n");
                      }
                    }
                    else
                    {
                      bFileClose = true;
                      j = new Json;
                      j->i("Status", "error");
                      strError = "Please provide the Function.";
                      j->i("Error", strError);
                      j->j(strSocketWriteBuffer);
                      delete j;
                      strSocketWriteBuffer.append("\n");
                    }
                    delete i;
                  }
                  else
                  {
                    bFileClose = true;
                    j = new Json;
                    j->i("Status", "error");
                    strError = "Request was not allocated from token.";
                    j->i("Error", strError);
                    j->j(strSocketWriteBuffer);
                    delete j;
                    strSocketWriteBuffer.append("\n");
                  }
                }
                else if (strSocketReadBuffer.size() > 33)
                {
                  bExit = true;
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Utility::sslRead() error:  Incoming data larger than token.";
                  log(ssMessage.str());
                }
              }
              // }}}
            }
            else
            {
              bSocketClose = true;
              if (nReturn < 0 && errno != 104)
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslRead(" << SSL_get_error(ssl, nReturn) << ") error:  " << m_pUtility->sslstrerror(ssl, nReturn);
                log(ssMessage.str());
              }
            }
          }
          // }}}
          // {{{ write
          if (bWritable)
          {
            bool bBlocking = false;
            long lArg, lArgOrig;
            if ((lArg = lArgOrig = fcntl(SSL_get_fd(ssl), F_GETFL, NULL)) >= 0 && !(lArg & O_NONBLOCK))
            {
              bBlocking = true;
              lArg |= O_NONBLOCK;
              fcntl(SSL_get_fd(ssl), F_SETFL, lArg);
            }
            if ((nReturn = SSL_write(ssl, pszSocketWriteBuffer, unSocketWriteLength)) > 0)
            {
              if ((size_t)nReturn < unSocketWriteLength)
              {
                memcpy(pszSocketWriteTemp, (pszSocketWriteBuffer + nReturn), (unSocketWriteLength - nReturn));
                memcpy(pszSocketWriteBuffer, pszSocketWriteTemp, (unSocketWriteLength - nReturn));
                unSocketWriteLength -= nReturn;
              }
              else
              {
                unSocketWriteLength = 0;
              }
            }
            else
            {
              bNeedWrite = bWantWrite = false;
              switch (SSL_get_error(ssl, nReturn))
              {
                case SSL_ERROR_WANT_READ: bNeedWrite = true; break;
                case SSL_ERROR_WANT_WRITE: bNeedWrite = bWantWrite = true; break;
                case SSL_ERROR_ZERO_RETURN:
                case SSL_ERROR_SYSCALL:
                case SSL_ERROR_SSL:
                {
                  bSocketClose = true;
                  if (nReturn < 0)
                  {
                    bExit = true;
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Utility::sslWrite(" << SSL_get_error(ssl, nReturn) << ") error:  " << m_pUtility->sslstrerror(ssl, nReturn);
                    log(ssMessage.str());
                  }
                  break;
                }
              }
            }
            if (bBlocking)
            {
              fcntl(SSL_get_fd(ssl), F_SETFL, lArgOrig);
            }
          }
          // }}}
          // {{{ error
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
          // }}}
          // }}}
          // {{{ file
          // {{{ read
          if (fds[1].revents & POLLIN)
          {
            if ((nReturn = read(fds[1].fd, (pszFileReadBuffer + unFileReadLength), (unSize - unFileReadLength))) > 0)
            {
              unFileReadLength += nReturn;
            }
            else
            {
              bFileClose = true;
              if (nReturn < 0)
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->read(" << errno << ") error:  " << strerror(errno);
                log(ssMessage.str());
              }
            }
          }
          // }}}
          // {{{ write
          if (fds[1].revents & POLLOUT)
          {
            if ((nReturn = write(fds[1].fd, pszFileWriteBuffer, unFileWriteLength)) > 0)
            {
              if ((size_t)nReturn < unFileWriteLength)
              {
                memcpy(pszFileWriteTemp, (pszFileWriteBuffer + nReturn), (unFileWriteLength - nReturn));
                memcpy(pszFileWriteBuffer, pszSocketReadTemp, (unFileWriteLength - nReturn));
                unFileWriteLength -= nReturn;
              }
              else
              {
                unFileWriteLength = 0;
              }
            }
            else
            {
              bFileClose = true;
              if (nReturn < 0)
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->write(" << errno << ") error:  " << strerror(errno);
                log(ssMessage.str());
              }
            }
          }
          // }}}
          // {{{ error
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
          // }}}
          // }}}
        }
        else if (nReturn < 0 && errno != EINTR)
        {
          bExit = true;
          ssMessage.str("");
          ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
          log(ssMessage.str());
        }
        // {{{ post work
        if (bFileClose && strSocketWriteBuffer.empty() && unFileReadLength == 0 && unSocketWriteLength == 0)
        {
          bSocketClose = true;
        }
        if (bSocketClose && strSocketReadBuffer.empty() && unSocketReadLength == 0 && unFileWriteLength == 0)
        {
          bFileClose = true;
        }
        if (bFileClose && bSocketClose)
        {
          bExit = true;
        }
        // }}}
      }
      // {{{ post work
      if (fdFile != -1)
      {
        close(fdFile);
      }
      // }}}
      m_mutex.lock();
      m_buffers[unBuffer] = -1;
      m_mutex.unlock();
    }
    else
    {
      ssMessage.str("");
      ssMessage << strPrefix << " error:  Buffer unavailable.";
      log(ssMessage.str());
    }
    sem_post(&m_semBuffer);
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
              ssIdent << m_strNode << "," << i->m["handle"]->v << "," << d.p << "," << start.tv_sec << "," << start.tv_nsec;
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
