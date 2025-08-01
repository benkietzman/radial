// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Request.cpp
// author     : Ben Kietzman
// begin      : 2022-04-21
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Request"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Request()
Request::Request(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "request", argc, argv, pCallback)
{
  m_pUtility->setReadSize(67108864);
  m_pUtility->setSslWriteSize(67108864);
}
// }}}
// {{{ ~Request()
Request::~Request()
{
}
// }}}
// {{{ accept()
void Request::accept(string strPrefix)
{
  // {{{ prep work
  SSL_CTX *ctx = NULL;
  string strError;
  stringstream ssMessage;
  strPrefix += "->Request::accept()";
  threadIncrement();
  // }}}
  if ((ctx = m_pUtility->sslInitServer(m_strData + "/server.crt", m_strData + "/server.key", strError)) != NULL)
  {
    // {{{ prep work
    addrinfo hints, *result;
    bool bBound[3] = {false, false, false};
    int fdSocket, nReturn;
    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((nReturn = getaddrinfo(NULL, "7234", &hints, &result)) == 0)
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
                thread threadSocket(&Request::socket, this, strPrefix, fdClient, ctx);
                pthread_setname_np(threadSocket.native_handle(), "socket");
                threadSocket.detach();
              }
              else
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->accept(" << errno << ") error [accept," << fds[0].fd << "]:  " << strerror(errno);
                notify(ssMessage.str());
              }
            }
            // }}}
          }
          else if (!bExit && nReturn < 0 && errno != EINTR)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
            notify(ssMessage.str());
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
    else
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->";
      if (!bBound[0])
      {
        ssMessage << "getaddrinfo(" << nReturn << ") error:  " << gai_strerror(nReturn);
      }
      else
      {
        ssMessage << ((!bBound[1])?"socket":"bind") << "(" << errno << ") error:  " << strerror(errno);
      }
      notify(ssMessage.str());
    }
    // {{{ post work
    SSL_CTX_free(ctx);
    // }}}
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->Utility::sslInitServer() error:  " << strError;
    notify(ssMessage.str());
  }
  // {{{ post work
  m_pUtility->sslDeinit();
  setShutdown();
  threadDecrement();
  // }}}
}
// }}}
// {{{ callback()
void Request::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Request::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    if (ptJson->m["Function"]->v == "ping")
    {
      bResult = true;
    }
    else
    {
      strError = "Please provide a valid Function:  ping.";
    }
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
}
// }}}
// {{{ request()
void Request::request(string strPrefix, size_t &unActive, const string strBuffer, bool &bResponse, int &fdResponse, list<string> &responses, mutex &mutexResponses)
{
  // {{{ prep work
  string strError, strJson;
  stringstream ssMessage;
  Json *ptJson = new Json(strBuffer);
  strPrefix += "->Request::request()";
  threadIncrement();
  throughput("request");
  keyRemovals(ptJson);
  // }}}
  if (!empty(ptJson, "Interface"))
  {
    if (ptJson->m["Interface"]->v == "hub")
    {
      if (!empty(ptJson, "Function"))
      {
        if (ptJson->m["Function"]->v == "list" || ptJson->m["Function"]->v == "ping")
        {
          hub(ptJson);
          mutexResponses.lock();
          responses.push_back(ptJson->j(strJson));
          if (!bResponse && fdResponse != -1)
          {
            bResponse = true;
            write(fdResponse, "\n", 1);
          }
          mutexResponses.unlock();
        }
        else
        {
          ptJson->i("Status", "error");
          ptJson->i("Error", "Please provide a valid Function:  list, ping.");
          mutexResponses.lock();
          responses.push_back(ptJson->j(strJson));
          if (!bResponse && fdResponse != -1)
          {
            bResponse = true;
            write(fdResponse, "\n", 1);
          }
          mutexResponses.unlock();
        }
      }
      else
      {
        ptJson->i("Status", "error");
        ptJson->i("Error", "Please provide the Function.");
        mutexResponses.lock();
        responses.push_back(ptJson->j(strJson));
        if (!bResponse && fdResponse != -1)
        {
          bResponse = true;
          write(fdResponse, "\n", 1);
        }
        mutexResponses.unlock();
      }
    }
    else
    {
      bool bRestricted = false;
      m_mutexShare.lock();
      if (m_i.find(ptJson->m["Interface"]->v) != m_i.end())
      {
        bRestricted = m_i[ptJson->m["Interface"]->v]->bRestricted;
      }
      else
      {
        list<radialLink *>::iterator linkIter = m_l.end();
        for (auto j = m_l.begin(); linkIter == m_l.end() && j != m_l.end(); j++)
        {
          if ((*j)->interfaces.find(ptJson->m["Interface"]->v) != (*j)->interfaces.end())
          { 
            linkIter = j;
          }
        }
        if (linkIter != m_l.end())
        {
          bRestricted = (*linkIter)->interfaces[ptJson->m["Interface"]->v]->bRestricted;
        }
      }
      m_mutexShare.unlock();
      if (!bRestricted || auth(ptJson, strError))
      {
        hub(ptJson->m["Interface"]->v, ptJson);
      }
      else
      {
        ptJson->i("Status", "error");
        ptJson->i("Error", strError);
      }
      mutexResponses.lock();
      responses.push_back(ptJson->j(strJson));
      if (!bResponse && fdResponse != -1)
      {
        bResponse = true;
        write(fdResponse, "\n", 1);
      }
      mutexResponses.unlock();
    }
  }
  else
  {
    ptJson->i("Status", "error");
    ptJson->i("Error", "Please provide the Interface.");
    mutexResponses.lock();
    responses.push_back(ptJson->j(strJson));
    if (!bResponse && fdResponse != -1)
    {
      bResponse = true;
      write(fdResponse, "\n", 1);
    }
    mutexResponses.unlock();
  }
  // {{{ post work
  delete ptJson;
  threadDecrement();
  mutexResponses.lock();
  if (unActive > 0)
  {
    unActive--;
  }
  mutexResponses.unlock();
  // }}}
}
// }}}
// {{{ socket()
void Request::socket(string strPrefix, int fdSocket, SSL_CTX *ctx)
{
  // {{{ prep work
  string strError;
  stringstream ssMessage;
  SSL *ssl;
  strPrefix += "->Request::socket()";
  threadIncrement();
  // }}}
  if ((ssl = m_pUtility->sslAccept(ctx, fdSocket, strError)) != NULL)
  {
    // {{{ prep work
    bool bResponse = false;
    int fdResponse[2] = {-1, -1}, nReturn;
    // }}}
    if ((nReturn = pipe(fdResponse)) == 0)
    {
      // {{{ prep work
      bool bActive = true, bExit = false, bNeedWrite = false, bWantWrite = false;
      char cChar;
      list<string> responses;
      mutex mutexResponses;
      size_t unActive = 0, unPosition;
      string strBuffers[2], strJson;
      time_t CActivity, CTime;
      time(&CActivity);
      // }}}
      while (!bExit)
      {
        // {{{ prep work
        pollfd fds[2];
        fds[0].fd = fdSocket;
        fds[0].events = POLLIN;
        if (!bNeedWrite)
        {
          mutexResponses.lock();
          while (!responses.empty())
          {
            strBuffers[1].append(responses.front()+"\n");
            responses.pop_front();
          }
          mutexResponses.unlock();
        }
        if (bWantWrite || !strBuffers[1].empty())
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
              time(&CActivity);
              bWantWrite = false;
              if (nReturn <= 0)
              {
                switch (SSL_get_error(ssl, nReturn))
                { 
                  case SSL_ERROR_WANT_WRITE: bWantWrite = true; break;
                }
              }
              while ((unPosition = strBuffers[0].find("\n")) != string::npos)
              {
                if (strBuffers[0].substr(0, unPosition).size() < m_unMaxPayload)
                {
                  mutexResponses.lock();
                  unActive++;
                  mutexResponses.unlock();
                  thread threadRequest(&Request::request, this, strPrefix, ref(unActive), strBuffers[0].substr(0, unPosition), ref(bResponse), ref(fdResponse[1]), ref(responses), ref(mutexResponses));
                  pthread_setname_np(threadRequest.native_handle(), "request");
                  threadRequest.detach();
                }
                strBuffers[0].erase(0, (unPosition + 1));
              }
            }
            else
            {
              bExit = true;
              if (nReturn < 0 && errno != 104)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslRead(" << SSL_get_error(ssl, nReturn) << ") error [read," << fds[0].fd << "]:  " << m_pUtility->sslstrerror(ssl, nReturn);
                log(ssMessage.str());
              }
            }
          }
          if (bWritable)
          {
            if (m_pUtility->sslWrite(ssl, strBuffers[1], nReturn))
            {
              time(&CActivity);
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
                ssMessage << strPrefix << "->Utility::sslWrite(" << SSL_get_error(ssl, nReturn) << ") error [write," << fds[0].fd << "]:  " << m_pUtility->sslstrerror(ssl, nReturn);
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
            if ((nReturn = read(fds[1].fd, &cChar, 1)) > 0)
            {
              mutexResponses.lock();
              bResponse = false;
              mutexResponses.unlock();
            }
            else
            {
              bExit = true;
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->read(" << errno << ") error:  " << strerror(errno);
                log(ssMessage.str());
              }
            }
          }
          else if (fds[1].revents & POLLERR)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->poll() error [" << fds[1].fd << "]:  Encountered a POLLERR.";
            log(ssMessage.str());
          }
          else if (fds[1].revents & POLLNVAL)
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
        time(&CTime);
        if (shutdown())
        {
          bExit = true;
        }
        else if ((CTime - CActivity) > 3600)
        {
          bExit = true;
          ssMessage.str("");
          ssMessage << strPrefix << ":  Closing due to over one hour of inactivity.";
          log(ssMessage.str());
        }
        // }}}
      }
      // {{{ post work
      close(fdResponse[0]);
      fdResponse[0] = -1;
      close(fdResponse[1]);
      fdResponse[1] = -1;
      while (bActive)
      {
        mutexResponses.lock();
        if (unActive == 0)
        {
          bActive = false;
        }
        mutexResponses.unlock();
        msleep(250);
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
}
}
