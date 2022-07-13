// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Request.cpp
// author     : Ben Kietzman
// begin      : 2022-04-21
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Request"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Request()
Request::Request(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "request", argc, argv, pCallback)
{
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
  SSL_CTX *ctx = NULL;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Request::accept()";
  if ((ctx = m_pUtility->sslInitServer(m_strData + "/server.crt", m_strData + "/server.key", strError)) != NULL)
  {
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
    if (bBound[2])
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->bind():  Bound incoming socket.";
      log(ssMessage.str());
      if (listen(fdSocket, SOMAXCONN) == 0)
      {
        bool bExit = false;
        ssMessage.str("");
        ssMessage << strPrefix << "->listen():  Listening to incoming socket.";
        log(ssMessage.str());
        while (!bExit)
        {
          pollfd fds[1];
          fds[0].fd = fdSocket;
          fds[0].events = POLLIN;
          if ((nReturn = poll(fds, 1, 100)) > 0)
          {
            if (fds[0].revents & POLLIN)
            {
              int fdClient;
              sockaddr_in cli_addr;
              socklen_t clilen = sizeof(cli_addr);
              if ((fdClient = ::accept(fds[0].fd, (sockaddr *)&cli_addr, &clilen)) >= 0)
              {
                thread threadSocket(&Request::socket, this, strPrefix, ctx, fdClient);
                pthread_setname_np(threadSocket.native_handle(), "socket");
                threadSocket.detach();
              }
              else
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->accept(" << errno << ") error [" << fds[0].fd << "]:  " << strerror(errno);
                notify(ssMessage.str());
              }
            }
          }
          else if (nReturn < 0 && errno != EINTR)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
            notify(ssMessage.str());
          }
          if (shutdown())
          {
            bExit = true;
          }
        }
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->listen(" << errno << ") error:  " << strerror(errno);
        notify(ssMessage.str());
      }
      close(fdSocket);
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
    SSL_CTX_free(ctx);
    EVP_cleanup();
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->Utility::sslInitServer() error:  " << strError;
    notify(ssMessage.str());
  }
  m_pUtility->sslDeinit();
  setShutdown();
}
// }}}
// {{{ callback()
void Request::callback(string strPrefix, Json *ptJson, const bool bResponse = true)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Request::callback()";
  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
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
  ptJson->insert("Status", ((bResult)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->insert("Error", strError);
  }
  if (bResponse)
  {
    response(ptJson);
  }
  delete ptJson;
}
// }}}
// {{{ request()
void Request::request(Json *ptJson)
{
  string strError;

  if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
  {
    list<string> removals;
    if (ptJson->m["Interface"]->v == "hub")
    {
      if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
      {
        if (ptJson->m["Function"]->v == "list" || ptJson->m["Function"]->v == "ping")
        {
          target(ptJson);
        }
        else
        {
          ptJson->insert("Status", "error");
          ptJson->insert("Error", "Please provide a valid Function:  list, ping.");
        }
      }
      else
      {
        ptJson->insert("Status", "error");
        ptJson->insert("Error", "Please provide the Function.");
      }
    }
    else
    {
      Json *ptInterfaces = new Json;
      ptInterfaces->insert("Function", "list");
      target(ptInterfaces);
      if (ptInterfaces->m.find("Response") != ptInterfaces->m.end())
      {
        if (ptInterfaces->m["Response"]->m.find(ptJson->m["Interface"]->v) != ptInterfaces->m["Response"]->m.end())
        {
          if (ptInterfaces->m["Response"]->m[ptJson->m["Interface"]->v]->m.find("Restricted") == ptInterfaces->m["Response"]->m[ptJson->m["Interface"]->v]->m.end() || ptInterfaces->m["Response"]->m[ptJson->m["Interface"]->v]->m["Restricted"]->v == "0" || auth(ptJson, strError))
          {
            target(ptJson->m["Interface"]->v, ptJson);
          }
          else
          {
            ptJson->insert("Status", "error");
            ptJson->insert("Error", strError);
          }
        }
        else
        {
          ptJson->insert("Status", "error");
          ptJson->insert("Error", "Interface does not exist.");
        }
      }
      else
      {
        ptJson->insert("Status", "error");
        ptJson->insert("Error", "Failed to retrieve interfaces.");
      }
      delete ptInterfaces;
    }
    for (auto &i : ptJson->m)
    {
      if (!i.first.empty() && i.first[0] == '_')
      {
        removals.push_back(i.first);
      }
    }
    for (auto &removal : removals)
    {
      if (ptJson->m.find(removal) != ptJson->m.end())
      {
        delete ptJson->m[removal];
        ptJson->m.erase(removal);
      }
    }
  }
  else
  {
    ptJson->insert("Status", "error");
    ptJson->insert("Error", "Please provide the Interface.");
  }
}
// }}}
// {{{ socket()
void Request::socket(string strPrefix, SSL_CTX *ctx, const int fdSocket)
{
  bool bExit = false;
  int nReturn;
  long lArg;
  size_t unPosition;
  string strBuffers[2], strError;
  stringstream ssMessage;
  SSL *ssl;
  common_socket_type eSocketType = COMMON_SOCKET_UNKNOWN;
  Json *ptJson;

  strPrefix += "->Request::socket()";
  while (!bExit)
  {
    pollfd fds[1];
    fds[0].fd = fdSocket;
    fds[0].events = POLLIN;
    if (!strBuffers[1].empty())
    {
      fds[0].events |= POLLOUT;
    }
    if ((nReturn = poll(fds, 1, 100)) > 0)
    {
      if (fds[0].revents & POLLIN)
      {
        if (eSocketType == COMMON_SOCKET_UNKNOWN)
        {
          if (m_pUtility->socketType(fds[0].fd, eSocketType, strError))
          {
            if (eSocketType == COMMON_SOCKET_ENCRYPTED)
            {
              if ((ssl = m_pUtility->sslAccept(ctx, fds[0].fd, strError)) == NULL)
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslAccept() error [" << fds[0].fd << "]:  " << strError;
                log(ssMessage.str());
              }
            }
            if ((lArg = fcntl(fds[0].fd, F_GETFL, NULL)) >= 0)
            {
              lArg |= O_NONBLOCK;
              fcntl(fds[0].fd, F_SETFL, lArg);
            }
          }
          else
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->Utility::socketType() error [" << fds[0].fd << "]:  " << strError;
            log(ssMessage.str());
          }
        }
        if (!bExit && ((eSocketType == COMMON_SOCKET_ENCRYPTED && m_pUtility->sslRead(ssl, strBuffers[0], nReturn)) || (eSocketType == COMMON_SOCKET_UNENCRYPTED && m_pUtility->fdRead(fds[0].fd, strBuffers[0], nReturn))))
        {
          if ((unPosition = strBuffers[0].find("\n")) != string::npos)
          {
            ptJson = new Json(strBuffers[0].substr(0, unPosition));
            strBuffers[0].erase(0, (unPosition + 1));
            request(ptJson);
            ptJson->insert("Node", m_strNode);
            ptJson->json(strBuffers[1]);
            strBuffers[1] += "\n";
            delete ptJson;
          }
        }
        else if (!bExit)
        {
          bExit = true;
          if (nReturn < 0)
          {
            ssMessage.str("");
            ssMessage << strPrefix << "->Utility::";
            if (eSocketType == COMMON_SOCKET_ENCRYPTED)
            {
              ssMessage << "sslRead(" << SSL_get_error(ssl, nReturn) << ") error [" << fds[0].fd << "]:  " << m_pUtility->sslstrerror(ssl, nReturn);
            }
            else
            {
              ssMessage << "fdRead(" << errno << ") error [" << fds[0].fd << "]:  " << strerror(errno);
            }
            log(ssMessage.str());
          }
        }
      }
      if (fds[0].revents & POLLOUT)
      {
        if ((eSocketType == COMMON_SOCKET_ENCRYPTED && m_pUtility->sslWrite(ssl, strBuffers[1], nReturn)) || (eSocketType == COMMON_SOCKET_UNENCRYPTED && m_pUtility->fdWrite(fds[0].fd, strBuffers[1], nReturn)))
        {
          if (strBuffers[1].empty())
          {
            bExit = true;
          }
        }
        else
        {
          bExit = true;
          if (nReturn < 0)
          {
            ssMessage.str("");
            ssMessage << strPrefix << "->Utility::";
            if (eSocketType == COMMON_SOCKET_ENCRYPTED)
            {
              ssMessage << "sslWrite(" << SSL_get_error(ssl, nReturn) << ") error [" << fds[0].fd << "]:  " << m_pUtility->sslstrerror(ssl, nReturn);
            }
            else
            {
              ssMessage << "fdWrite(" << errno << ") error [" << fds[0].fd << "]:  " << strerror(errno);
            }
            log(ssMessage.str());
          }
        }
      }
    }
    else if (nReturn < 0 && errno != EINTR)
    {
      bExit = true;
      ssMessage.str("");
      ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
      log(ssMessage.str());
    }
    if (shutdown() && strBuffers[0].empty() && strBuffers[1].empty())
    {
      bExit = true;
    }
  }
  if (eSocketType == COMMON_SOCKET_ENCRYPTED)
  {
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }
  close(fdSocket);
}
// }}}
}
}
