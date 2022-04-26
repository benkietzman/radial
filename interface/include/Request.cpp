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
Request::Request(string strPrefix, int argc, char **argv, function<void(string, Json *, const bool)> callback) : Interface(strPrefix, "request", argc, argv, callback)
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
  setlocale(LC_ALL, "");
  if ((ctx = m_pUtility->sslInitServer(m_strData + "/server.crt", m_strData + "/server.key", strError)) != NULL)
  {
    addrinfo hints, *result;
    bool bBound[3] = {false, false, false};
    int fdSocket, nReturn;
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
      if (listen(fdSocket, 5) == 0)
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
              if ((fdClient = ::accept(fdSocket, (sockaddr *)&cli_addr, &clilen)) >= 0)
              {
                thread threadRequestSocket(&Request::socket, this, strPrefix, ctx, fdClient);
                threadRequestSocket.detach();
              }
              else
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->accept(" << errno << ") error:  " << strerror(errno);
                notify(ssMessage.str());
              }
            }
          }
          else if (nReturn < 0)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
            notify(ssMessage.str());
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
}
// }}}
// {{{ request()
void Request::request(Json *ptJson)
{
  if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
  {
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
          if (ptInterfaces->m["Response"]->m[ptJson->m["Interface"]->v]->m.find("Restricted") == ptInterfaces->m["Response"]->m[ptJson->m["Interface"]->v]->m.end() || ptInterfaces->m["Response"]->m[ptJson->m["Interface"]->v]->m["Restricted"]->v == "0")
          {
            target(ptJson->m["Interface"]->v, ptJson);
          }
          else
          {
            ptJson->insert("Status", "error");
            ptJson->insert("Error", "Access to interface is restricted.");
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
        delete i.second;
        ptJson->m.erase(i.first);
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
void Request::socket(string strPrefix, SSL_CTX *ctx, int fdSocket)
{
  bool bExit = false;
  int nReturn;
  size_t unPosition;
  string strBuffer[2], strError;
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
    if (!strBuffer[1].empty())
    {
      fds[0].events |= POLLOUT;
    }
    if ((nReturn = poll(fds, 1, 100)) > 0)
    {
      if (fds[0].revents & POLLIN)
      {
        if (eSocketType == COMMON_SOCKET_UNKNOWN)
        {
          if (m_pUtility->socketType(fdSocket, eSocketType, strError))
          {
            if (eSocketType == COMMON_SOCKET_ENCRYPTED)
            {
              ERR_clear_error();
              if ((ssl = m_pUtility->sslAccept(ctx, fdSocket, strError)) == NULL)
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslAccept() error:  " << strError;
                log(ssMessage.str());
              }
            }
          }
          else
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->Utility::socketType() error:  " << strError;
            log(ssMessage.str());
          }
        }
        if (!bExit && ((eSocketType == COMMON_SOCKET_ENCRYPTED && m_pUtility->sslRead(ssl, strBuffer[0], nReturn)) || (eSocketType == COMMON_SOCKET_UNENCRYPTED && m_pUtility->fdRead(fdSocket, strBuffer[0], nReturn))))
        {
          if ((unPosition = strBuffer[0].find("\n")) != string::npos)
          {
            ptJson = new Json(strBuffer[0].substr(0, unPosition));
            strBuffer[0].erase(0, (unPosition + 1));
            request(ptJson);
            ptJson->json(strBuffer[1]);
            strBuffer[1] += "\n";
            delete ptJson;
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
              ssMessage << "sslRead(" << SSL_get_error(ssl, nReturn) << ") error:  " << m_pUtility->sslstrerror();
            }
            else
            {
              ssMessage << "fdRead(" << errno << ") error:  " << strerror(errno);
            }
            log(ssMessage.str());
          }
        }
      }
      if (fds[0].revents & POLLOUT)
      {
        if ((eSocketType == COMMON_SOCKET_ENCRYPTED && m_pUtility->sslWrite(ssl, strBuffer[1], nReturn)) || (eSocketType == COMMON_SOCKET_UNENCRYPTED && m_pUtility->fdWrite(fdSocket, strBuffer[1], nReturn)))
        {
          if (strBuffer[1].empty())
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
              ssMessage << "sslWrite(" << SSL_get_error(ssl, nReturn) << ") error:  " << m_pUtility->sslstrerror();
            }
            else
            {
              ssMessage << "fdWrite(" << errno << ") error:  " << strerror(errno);
            }
            log(ssMessage.str());
          }
        }
      }
    }
    else if (nReturn < 0)
    {
      bExit = true;
      ssMessage.str("");
      ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
      log(ssMessage.str());
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
