// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Application.cpp
// author     : Ben Kietzman
// begin      : 2025-05-27
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Application"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Application()
Application::Application(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), void (*pCallbackInotify)(string, const string, const string)) : Interface(strPrefix, "application", argc, argv, pCallback)
{
  map<string, list<string> > watches;

  m_unUniqueID = 0;
  m_pUtility->setReadSize(4096);
  m_pUtility->setSslWriteSize(67108864);
  // {{{ functions
  m_functions["connect"] = &Application::connect;
  m_functions["request"] = &Application::request;
  m_functions["status"] = &Application::status;
  // }}}
  m_pUtility->sslInit();
  load(strPrefix, true);
  m_pThreadApplicationAccept = new thread(&Application::applicationAccept, this, strPrefix);
  pthread_setname_np(m_pThreadApplicationAccept->native_handle(), "applicationAccept");
  watches[m_strData] = {".cred"};
  m_pThreadInotify = new thread(&Application::inotify, this, strPrefix, watches, pCallbackInotify);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
  m_pThreadSchedule = new thread(&Application::schedule, this, strPrefix);
  pthread_setname_np(m_pThreadSchedule->native_handle(), "schedule");
}
// }}}
// {{{ ~Application()
Application::~Application()
{
  m_pThreadSchedule->join();
  delete m_pThreadSchedule;
  m_pThreadApplicationAccept->join();
  delete m_pThreadApplicationAccept;
  m_pUtility->sslDeinit();
  for (auto &client : m_clients)
  {
    close(client.second);
  }
}
// }}}
// {{{ application
// {{{ applicationAccept()
void Application::applicationAccept(string strPrefix)
{
  // {{{ prep work
  SSL_CTX *ctx = NULL;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Application::applicationAccept()";
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
    if ((nReturn = getaddrinfo(NULL, "7277", &hints, &result)) == 0)
    {
      addrinfo *rp;
      bBound[0] = true;
      for (rp = result; !bBound[2] && rp != NULL; rp = rp->ai_next)
      {
        bBound[1] = false;
        if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
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
              if ((fdClient = accept(fds[0].fd, (sockaddr *)&cli_addr, &clilen)) >= 0)
              {
                thread threadApplicationSocket(&Application::applicationSocket, this, strPrefix, fdClient, ctx);
                pthread_setname_np(threadApplicationSocket.native_handle(), "applicationSocket");
                threadApplicationSocket.detach();
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
// {{{ applicationSocket()
void Application::applicationSocket(string strPrefix, int fdSocket, SSL_CTX *ctx)
{
  // {{{ prep work
  string strError;
  stringstream ssMessage;
  SSL *ssl;
  strPrefix += "->Application::applicationSocket()";
  threadIncrement();
  // }}}
  if ((ssl = m_pUtility->sslAccept(ctx, fdSocket, strError)) != NULL)
  {
    // {{{ prep work
    bool bClose = false, bExit = false, bNeedWrite = false, bToken = false, bWantWrite = false;
    char cChar = '\n';
    int nReturn;
    size_t unPosition;
    string strApplication, strBuffers[2], strMessage;
    Json *ptJson;
    // }}}
    while (!bExit)
    {
      // {{{ prep work
      pollfd fds[1];
      fds[0].fd = fdSocket;
      fds[0].events = POLLIN;
      if (!bNeedWrite)
      {
        m_mutex.lock();
        while (!m_req[strApplication][fdSocket].empty())
        {
          strMessage = m_req[strApplication][fdSocket].front() + "\n";
          m_req[strApplication][fdSocket].pop();
          strBuffers[1].append(strMessage);
        }
        m_mutex.unlock();
      }
      if (bWantWrite || !strBuffers[1].empty())
      {
        fds[0].events |= POLLOUT;
      }
      // }}}
      if ((nReturn = poll(fds, 1, 100)) > 0)
      {
        bool bReadable = (fds[0].revents & (POLLHUP | POLLIN)), bWritable = (fds[0].revents & POLLOUT);
        // {{{ read
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
              if ((unPosition = strBuffers[0].find("\n")) != string::npos)
              {
                int fdClient = -1;
                ptJson = new Json(strBuffers[0].substr(0, unPosition));
                strBuffers[0].erase(0, (unPosition + 1));
                m_mutex.lock();
                if (!empty(ptJson, "_key"))
                {
                  size_t unKey = atoi(ptJson->m["_key"]->v.c_str());
                  if (m_clients.find(unKey) != m_clients.end() && m_clientTimeouts.find(unKey) != m_clientTimeouts.end())
                  {
                    if (m_res.find(strApplication) == m_res.end())
                    {
                      m_res[strApplication] = {};
                    }
                    if (m_res[strApplication].find(fdSocket) == m_res[strApplication].end())
                    {
                      m_res[strApplication][fdSocket] = {};
                    }
                    m_res[strApplication][fdSocket][unKey] = ptJson;
                    fdClient = m_clients[unKey];
                    m_clients.erase(unKey);
                    m_clientTimeouts.erase(unKey);
                  }
                  else
                  {
                    delete ptJson;
                  }
                }
                else
                {
                  delete ptJson;
                }
                m_mutex.unlock();
                if (fdClient != -1)
                {
                  write(fdClient, &cChar, 1);
                  close(fdClient);
                }
              }
            }
            else if ((unPosition = strBuffers[0].find("\n")) != string::npos)
            {
              string strToken = strBuffers[0].substr(0, unPosition);
              Json *j, *ptToken = new Json;
              strBuffers[0].erase(0, (unPosition + 1));
              bToken = true;
              if (storageRetrieve({"application", "tokens", strToken}, ptToken, strError))
              {
                Json *ptData = new Json;
                if (!empty(ptToken, "application"))
                {
                  if (connectorAdd(ptToken->m["application"]->v, fdSocket, strError))
                  {
                    strApplication = ptToken->m["application"]->v;
                  }
                  else
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Application::connectorAdd() error [" << ptToken->m["application"]->v << "," << fdSocket << "]:  " << strError;
                    log(ssMessage.str());
                  }
                }
                if (storageRemove({"application", "tokens", strToken}, strError) && storageRetrieve({"application", "tokens"}, ptData, strError) && ptData->m.empty() && storageRemove({"application", "tokens"}, strError) && storageRetrieve({"application"}, ptData, strError) && ptData->m.empty() && storageRemove({"application"}, strError))
                {
                }
                delete ptData;
              }
              delete ptToken;
              if (!strApplication.empty())
              {
                j = new Json;
                j->i("Status", "okay");
                j->j(strBuffers[1]);
                delete j;
                strBuffers[1].append("\n");
              }
              else
              {
                j = new Json;
                j->i("Status", "error");
                if (strError.empty())
                {
                  strError = "Request was not allocated from token.";
                }
                j->i("Error", strError);
                j->j(strBuffers[1]);
                delete j;
                strBuffers[1].append("\n");
              }
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
            bClose = true;
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
          if (!m_pUtility->sslWrite(ssl, strBuffers[1], nReturn))
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
                bClose = true;
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
      }
      else if (nReturn < 0 && errno != EINTR)
      {
        bExit = true;
        ssMessage.str("");
        ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
        log(ssMessage.str());
      }
      // {{{ post work
      if (bClose || shutdown())
      {
        bExit = true;
      }
      // }}}
    }
    // {{{ post work
    if (SSL_shutdown(ssl) == 0)
    {
      SSL_shutdown(ssl);
    }
    SSL_free(ssl);
    if (!strApplication.empty())
    {
      if (!connectorRemove(strApplication, fdSocket, strError))
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->Application::connectorRemove() error [" << strApplication << "," << fdSocket << "]:  " << strError;
        log(ssMessage.str());
      }
    }
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
// {{{ autoMode()
void Application::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->Application::autoMode()";
  if (strOldMaster != strNewMaster)
  {
    stringstream ssMessage;
    ssMessage << strPrefix << " [" << strNewMaster << "]:  Updated master.";
    log(ssMessage.str());
  }
  threadDecrement();
}
// }}}
// {{{ callback()
void Application::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Application::callback()";
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
        if (exist(ptJson, "Response"))
        {
          delete ptJson->m["Response"];
        }
        ptJson->m["Response"] = d.p->m["o"];
        d.p->m.erase("o");
      }
    }
    else
    {
      strError = "Please provide a valid Function.";
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
void Application::callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Application::callbackInotify()";
  if (strPath == m_strData && strFile == ".cred")
  {
    load(strPrefix);
  }
}
// }}}
// {{{ connect()
bool Application::connect(radialUser &d, string &e)
{
  bool b = false;
  Json *o = d.p->m["o"];

  if (dep({"User"}, d.r, e))
  {
    string strApplication, strUser = d.r->m["User"]->v;
    m_mutex.lock();
    if (m_applications.find(strUser) != m_applications.end())
    {
      strApplication = m_applications[strUser];
    }
    m_mutex.unlock();
    if (!strApplication.empty())
    {
      char md5string[33];
      EVP_MD_CTX *ctx = EVP_MD_CTX_create();
      string strIdent, t;
      stringstream ssIdent;
      timespec start;
      unsigned char digest[16];
      Json *ptToken = new Json;
      clock_gettime(CLOCK_REALTIME, &start);
      ssIdent << m_strNode << "," << strUser << "," << start.tv_sec << "," << start.tv_nsec;
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
      ptToken->i("_time", to_string(start.tv_sec), 'n');
      ptToken->i("application", strApplication);
      if (storageAdd({"application", "tokens", t}, ptToken, e))
      {
        b = true;
        o->i("Token", t);
      }
      delete ptToken;
    }
    else
    {
      e = "Unable to retrieve Application based on User.";
    }
  }

  return b;
}
// }}}
// {{{ connector
// {{{ connectorAdd()
bool Application::connectorAdd(const string strApplication, int fdSocket, string &strError)
{
  bool bResult = false;
  stringstream ssMessage;
  Json *ptConnector = new Json;

  m_mutex.lock();
  if (m_req.find(strApplication) == m_req.end())
  {
    m_req[strApplication] = {};
  }
  if (m_req[strApplication].find(fdSocket) == m_req[strApplication].end())
  {
    m_req[strApplication][fdSocket] = {};
  }
  if (m_res.find(strApplication) == m_res.end())
  {
    m_res[strApplication] = {};
  }
  if (m_res[strApplication].find(fdSocket) == m_res[strApplication].end())
  {
    m_res[strApplication][fdSocket] = {};
  }
  m_mutex.unlock();
  ptConnector->i(to_string(fdSocket), "1", '1');
  if (storageAdd({"application", "connectors", strApplication, m_strNode}, ptConnector, strError))
  {
    bResult = true;
    ssMessage.str("");
    ssMessage << char(3) << "13,06 " << strApplication << " | " << m_strNode << " | " << fdSocket << " " << char(3) << " Added connector.";
    chat("#application", ssMessage.str());
  }
  else
  {
    connectorRemove(strApplication, fdSocket, strError);
  }
  delete ptConnector;

  return bResult;
}
// }}}
// {{{ connectorRemove()
bool Application::connectorRemove(const string strApplication, int fdSocket, string &strError)
{
  bool bResult = false;
  stringstream ssMessage;

  if (storageRemove({"application", "connectors", strApplication, m_strNode, to_string(fdSocket)}, strError))
  {
    Json *ptData = new Json;
    bResult = true;
    if (storageRetrieve({"application", "connectors", strApplication, m_strNode}, ptData, strError) && ptData->m.empty() && storageRemove({"application", "connectors", strApplication, m_strNode}, strError) && storageRetrieve({"application", "connectors", strApplication}, ptData, strError) && ptData->m.empty() && storageRemove({"application", "connectors", strApplication}, strError) && storageRetrieve({"application", "connectors"}, ptData, strError) && ptData->m.empty() && storageRemove({"application", "connectors"}, strError) && storageRetrieve({"application"}, ptData, strError) && ptData->m.empty() && storageRemove({"application"}, strError))
    {
    }
    delete ptData;
    ssMessage.str("");
    ssMessage << char(3) << "13,06 " << strApplication << " | " << m_strNode << " | " << fdSocket << " " << char(3) << " Removed connector.";
    chat("#application", ssMessage.str());
  }
  m_mutex.lock();
  if (m_req.find(strApplication) != m_req.end())
  {
    if (m_req[strApplication].find(fdSocket) != m_req[strApplication].end())
    {
      m_req[strApplication].erase(fdSocket);
    }
    if (m_req[strApplication].empty())
    {
      m_req.erase(strApplication);
    }
  }
  if (m_res.find(strApplication) != m_res.end())
  {
    if (m_res[strApplication].find(fdSocket) != m_res[strApplication].end())
    {
      for (auto &client : m_res[strApplication][fdSocket])
      {
        if (client.second != NULL)
        {
          delete client.second;
          client.second = NULL;
        }
        if (m_clients.find(client.first) != m_clients.end())
        {
          close(m_clients[client.first]);
          m_clients.erase(client.first);
        }
        if (m_clientTimeouts.find(client.first) != m_clientTimeouts.end())
        {
          m_clientTimeouts.erase(client.first);
        }
      }
      m_res[strApplication].erase(fdSocket);
    }
    if (m_res[strApplication].empty())
    {
      m_res.erase(strApplication);
    }
  }
  m_mutex.unlock();

  return bResult;
}
// }}}
// }}}
// {{{ load()
void Application::load(string strPrefix, const bool bSilent)
{
  map<string, string> applications;
  string strError;
  stringstream ssMessage;
  Json *ptCred = new Json;

  strPrefix += "->Application::load()";
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"radial"}, ptCred, strError))
  {
    for (auto &cred : ptCred->m)
    {
      if (!empty(cred.second, "Application"))
      {
        applications[cred.first] = cred.second->m["Application"]->v;
      }
    }
    m_mutex.lock();
    m_applications = applications;
    m_mutex.unlock();
  }
  else if (!bSilent)
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->Warden::vaultRetrieve() error [radial]:  " << strError;
    log(ssMessage.str());
  }
}
// }}}
// {{{ request()
bool Application::request(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssMessage;
  Json *i = d.p->m["i"];

  if (dep({"Application"}, d.r, e))
  {
    bool bFound = false, bPipe = false;
    int fdPipe[2] = {-1, -1}, fdSocket = -1, nReturn;
    size_t unKey = 0;
    string strApplication = d.r->m["Application"]->v;
    m_mutex.lock();
    if (m_req.find(strApplication) != m_req.end() && !m_req[strApplication].empty())
    {
      bFound = true;
      if ((nReturn = pipe(fdPipe)) == 0)
      {
        auto connIter = m_req[strApplication].begin();
        int unPick = 0;
        string strMessage;
        time_t CTime;
        unsigned int unSeed = time(&CTime);
        bPipe = true;
        unKey = m_unUniqueID++;
        m_clients[unKey] = fdPipe[1];
        m_clientTimeouts[unKey] = CTime;
        i->i("_key", to_string(unKey), 'n');
        i->j(strMessage);
        srand(unSeed);
        unPick = rand_r(&unSeed) % m_req[strApplication].size();
        for (int i = 0; i < unPick; i++)
        {
          connIter++;
        }
        fdSocket = connIter->first;
        m_req[strApplication][fdSocket].push(strMessage);
      }
      else
      {
        ssMessage.str("");
        ssMessage << "pipe(" << errno << ") " << strerror(errno);
        e = ssMessage.str();
      }
    }
    m_mutex.unlock();
    if (bPipe)
    {
      bool bExit = false;
      char cChar;
      Json *ptJson = NULL;
      while (!bExit)
      {
        pollfd fds[1];
        fds[0].fd = fdPipe[0];
        fds[0].events = POLLIN;
        if ((nReturn = poll(fds, 1, 2000)) > 0)
        {
          if (fds[0].revents & (POLLHUP | POLLIN))
          {
            if (read(fds[0].fd, &cChar, 1) > 0)
            {
              bExit = true;
              m_mutex.lock();
              if (m_res.find(strApplication) != m_res.end() && m_res[strApplication].find(fdSocket) != m_res[strApplication].end())
              {
                if (m_res[strApplication][fdSocket].find(unKey) != m_res[strApplication][fdSocket].end() && m_res[strApplication][fdSocket][unKey] != NULL)
                {
                  ptJson = m_res[strApplication][fdSocket][unKey];
                  m_res[strApplication][fdSocket].erase(unKey);
                }
                else
                {
                  e = "Failed to find response.";
                }
              }
              else
              {
                e = "Failed to find Application.";
              }
              m_mutex.unlock();
            }
            else
            {
              bExit = true;
              ssMessage.str("");
              ssMessage << "read(" << errno << ") " << strerror(errno);
              e = ssMessage.str();
            }
          }
          if (fds[0].revents & POLLERR)
          {
            bExit = true;
            e = "poll() Encountered a POLLERR.";
          }
          if (fds[0].revents & POLLNVAL)
          {
            bExit = true;
            e = "poll() Encountered a POLLNVAL.";
          }
        }
        else if (nReturn < 0 && errno != EINTR)
        {
          bExit = true;
          ssMessage.str("");
          ssMessage << "poll(" << errno << ") " << strerror(errno);
          e = ssMessage.str();
        }
        if (shutdown())
        {
          bExit = true;
        }
      }
      close(fdPipe[0]);
      if (ptJson != NULL)
      {
        b = true;
        if (exist(ptJson, "_key"))
        {
          delete ptJson->m["_key"];
          ptJson->m.erase("_key");
        }
        if (d.p->m.find("o") != d.p->m.end())
        {
          delete d.p->m["o"];
        }
        d.p->m["o"] = ptJson;
      }
    }
    else if (!bFound)
    {
      Json *ptData = new Json;
      if (storageRetrieve({"application", "connectors", strApplication}, ptData, e))
      {
        string strNode;
        for (auto nodeIter = ptData->m.begin(); strNode.empty() && nodeIter != ptData->m.end(); nodeIter++)
        {
          if (!nodeIter->second->m.empty())
          {
            strNode = nodeIter->first;
          }
        }
        if (!strNode.empty())
        {
          Json *ptLink = new Json;
          ptLink->i("Interface", "application");
          ptLink->i("Application", strApplication);
          ptLink->i("Function", "request");
          ptLink->i("Node", strNode);
          ptLink->m["Request"] = new Json(i);
          if (hub("link", ptLink, e))
          {
            b = true;
            if (exist(ptLink, "Response"))
            {
              if (d.p->m.find("o") != d.p->m.end())
              {
                delete d.p->m["o"];
              }
              d.p->m["o"] = ptLink->m["Response"];
              ptLink->m.erase("Response");
            }
          }
          delete ptLink;
        }
        else
        {
          e = "Please provide a valid Application.";
        }
      }
      else if (e == "Failed to find key.")
      {
        e = "Please provide a valid Application.";
      }
      delete ptData;
    }
  }

  return b;
}
// }}}
// {{{ schedule()
void Application::schedule(string strPrefix)
{
  // {{{ prep work
  string strError;
  stringstream ssMessage, ssQuery;
  time_t CTime[2] = {0, 0};

  threadIncrement();
  strPrefix += "->Application::schedule()";
  time(&(CTime[1]));
  // }}}
  while (!shutdown())
  {
    time(&(CTime[0]));
    if ((CTime[0] - CTime[1]) >= 60)
    {
      list<string> removals;
      Json *ptMessage = new Json;
      CTime[1] = CTime[0];
      // {{{ status
      ptMessage->i("Source", m_strNode);
      status(ptMessage);
      ptMessage->i("Action", "status");
      live("Data", "", ptMessage);
      delete ptMessage;
      // }}}
      // {{{ storage
      if (isMasterSettled() && isMaster())
      {
        list<size_t> removals;
        Json *ptData = new Json;
        if (storageRetrieve({"application", "tokens"}, ptData, strError))
        {
          list<string> removals;
          for (auto &token : ptData->m)
          {
            if (!empty(token.second, "_time"))
            {
              if (CTime[0] > atoi(token.second->m["_time"]->v.c_str()) && (CTime[0] - atoi(token.second->m["_time"]->v.c_str())) > 60)
              {
                removals.push_back(token.first);
              }
            }
            else
            {
              removals.push_back(token.first);
            }
          }
          if (!removals.empty())
          {
            while (!removals.empty())
            {
              ssMessage.str("");
              ssMessage << char(3) << "13,06 " << ((!empty(ptData->m[removals.front()], "application"))?ptData->m[removals.front()]->m["application"]->v:"") << " " << char(3) << " Removed expired token.";
              chat("#application", ssMessage.str());
              delete ptData->m[removals.front()];
              ptData->m.erase(removals.front());
              removals.pop_front();
            }
            if (!ptData->m.empty())
            {
              if (!storageAdd({"application", "tokens"}, ptData, strError))
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->storageAdd() error [application,tokens]:  " << strError;
                log(ssMessage.str());
              }
            }
            else if (storageRemove({"application", "tokens"}, strError) && storageRetrieve({"application"}, ptData, strError) && ptData->m.empty() && storageRemove({"application"}, strError))
            {
            }
          }
        }
        else if (strError != "Failed to find key.")
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->storageRetrieve() error [application,tokens]:  " << strError;
          log(ssMessage.str());
        }
        delete ptData;
        m_mutex.lock();
        for (auto &client : m_clientTimeouts)
        {
          if ((CTime[0] > client.second) && (CTime[0] - client.second) > 300)
          {
            removals.push_back(client.first);
          }
        }
        while (!removals.empty())
        {
          string strApplication;
          m_clientTimeouts.erase(removals.front());
          if (m_clients.find(removals.front()) != m_clients.end())
          {
            close(m_clients[removals.front()]);
            m_clients.erase(removals.front());
          }
          for (auto &app : m_res)
          {
            int fdSocket = -1;
            for (auto &sock : app.second)
            {
              if (sock.second.find(removals.front()) != sock.second.end())
              {
                if (sock.second[removals.front()] != NULL)
                {
                  delete sock.second[removals.front()];
                }
                sock.second.erase(removals.front());
                if (sock.second.empty())
                {
                  fdSocket = sock.first;
                }
              }
            }
            if (fdSocket != -1)
            {
              app.second.erase(fdSocket);
              if (app.second.empty())
              {
                strApplication = app.first;
              }
            }
          }
          if (!strApplication.empty())
          {
            m_res.erase(strApplication);
          }
          removals.pop_front();
        }
        m_mutex.unlock();
      }
      // }}}
    }
    msleep(1000);
  }
  // {{{ post work
  setShutdown();
  threadDecrement();
  // }}}
}
// }}}
}
}
