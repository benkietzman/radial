// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Link.cpp
// author     : Ben Kietzman
// begin      : 2022-04-28
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Link"
// }}}
extern "C++"
{ 
namespace radial
{
// {{{ Link()
Link::Link(string strPrefix, int argc, char **argv, function<void(string, Json *, const bool)> callback) : Interface(strPrefix, "link", argc, argv, callback)
{
  ifstream inLinks((m_strData + "/links.json");
  m_ptLinks = NULL;
  if (inLinks)
  {
    string strLine;
    stringstream ssJson;
    while (getline(inLinks, strLine))
    {
      ssJson << strLine;
    }
    m_ptLinks = new Json(ssJson.str());
  }
  m_pWarden->vaultRetrieve(["link", "Password"], m_strPassword, strError);
}
// }}}
// {{{ ~Link()
Link::~Link()
{
  delete m_ptLinks;
}
// }}}
// {{{ accept()
void Link::accept(string strPrefix)
{
  // {{{ prep work
  SSL_CTX *ctxC = NULL, *ctxS = NULL;
  string strError;
  stringstream ssMessage;
  strPrefix += "->Link::accept()";
  setlocale(LC_ALL, "");
  // }}}
  if ((ctxS = m_pUtility->sslInitServer(m_strData + "/server.crt", m_strData + "/server.key", strError)) != NULL)
  {
    SSL_CTX_set_mode(ctxS, SSL_MODE_AUTO_RETRY);
    if ((ctxC = m_pUtility->sslInitClient(strError)) != NULL)
    {
      // {{{ prep work
      addrinfo hints, *result;
      bool bBound[3] = {false, false, false};
      int fdSocket, nReturn;
      SSL_CTX_set_mode(ctxC, SSL_MODE_AUTO_RETRY);
      memset(&hints, 0, sizeof(addrinfo));
      hints.ai_family = AF_INET6;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;
      if ((nReturn = getaddrinfo(NULL, "7565", &hints, &result)) == 0)
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
        if (listen(fdSocket, 5) == 0)
        {
          // {{{ prep work
          bool bExit = false;
          list<radial_link *> links;
          pollfd *fds;
          size_t unIndex;
          ssMessage.str("");
          ssMessage << strPrefix << "->listen():  Listening to incoming socket.";
          log(ssMessage.str());
          // }}}
          while (!shutdown() && !bExit)
          {
            // {{{ prep work
            fds = new pollfd[links.size() + 1];
            unIndex = 0;
            fds[unIndex].fd = fdSocket;
            fds[unIndex].events = POLLIN;
            unIndex++;
            for (auto &link : links)
            {
              fds[unIndex].fd = link->fdSocket;
              fds[unIndex].events = POLLIN;
              if (!link->strBuffer[1].empty())
              {
                fds[unIndex].events |= POLLOUT;
              }
              unIndex++;
            }
            // }}}
            if ((nReturn = poll(fds, unIndex, 100)) > 0)
            {
              // {{{ accept
              if (fds[0].revents & POLLIN)
              {
                // {{{ prep work
                int fdClient;
                sockaddr_in cli_addr;
                socklen_t clilen = sizeof(cli_addr);
                // }}}
                if ((fdClient = ::accept(fdSocket, (sockaddr *)&cli_addr, &clilen)) >= 0)
                {
                  // {{{ prep work
                  sockaddr_storage addr;
                  socklen_t len = sizeof(addr);
                  ssMessage.str("");
                  ssMessage << strPrefix << "->accept():  Accepted incoming socket.";
                  log(ssMessage.str());
                  // }}}
                  if (getpeername(fdClient, (sockaddr *)&addr, &len) == 0)
                  {
                    // {{{ prep work
                    char szIP[INET6_ADDRSTRLEN];
                    SSL *ssl;
                    if (addr.ss_family == AF_INET)
                    {
                      sockaddr_in *s = (sockaddr_in *)&addr;
                      inet_ntop(AF_INET, &s->sin_addr, szIP, sizeof(szIP));
                    }
                    else if (addr.ss_family == AF_INET6)
                    {
                      sockaddr_in6 *s = (sockaddr_in6 *)&addr;
                      inet_ntop(AF_INET6, &s->sin6_addr, szIP, sizeof(szIP));
                    }
                    ssMessage.str("");
                    ssMessage << strPrefix << "->getpeername() [" << szIP << "]:  Retrieved peer information.";
                    log(ssMessage.str());
                    ERR_clear_error();
                    // }}}
                    if ((ssl = m_pUtility->sslAccept(ctxS, fdClient, strError)) != NULL)
                    {
                      radial_link *ptLink = new radial_link;
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Utility::sslAccept() [" << szIP << "]:  Accepted incoming socket.";
                      log(ssMessage.str());
                      ptLink->fdSocket = fdClient;
                      ptLink->ssl = ssl;
                      links.push_back(ptLink);
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Utility::sslAccept() error:  " << strError;
                      notify(ssMessage.str());
                      close(fdClient);
                    }
                  }
                  else
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->getpeername(" << errno << ") error:  " << strerror(errno);
                    notify(ssMessage.str());
                    close(fdClient);
                  }
                }
                else
                {
                  bExit = true;
                  ssMessage.str("");
                  ssMessage << strPrefix << "->accept(" << errno << ") error:  " << strerror(errno);
                  notify(ssMessage.str());
                }
              }
              // }}}
            }
            else if (nReturn < 0 && errno != EINTR)
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
      SSL_CTX_free(ctxC);
      // }}}
    }
    else
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->Utility::sslInitClient() error:  " << strError;
      notify(ssMessage.str());
    }
    // {{{ post work
    SSL_CTX_free(ctxS);
    EVP_cleanup();
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
  // }}}
}
// }}}
// {{{ callback()
void Link::callback(string strPrefix, Json *ptJson, const bool bResponse = true)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Link::callback()";
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
void Link::request(Json *ptJson)
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
void Link::socket(string strPrefix, SSL_CTX *ctxS, int fdSocket)
{
  bool bExit = false;
  int nReturn;
  size_t unPosition;
  string strBuffers[2], strError;
  stringstream ssMessage;
  SSL *ssl;
  common_socket_type eSocketType = COMMON_SOCKET_UNKNOWN;
  Json *ptJson;

  strPrefix += "->Link::socket()";
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
          if (m_pUtility->socketType(fdSocket, eSocketType, strError))
          {
            if (eSocketType == COMMON_SOCKET_ENCRYPTED)
            {
              ERR_clear_error();
              if ((ssl = m_pUtility->sslAccept(ctxS, fdSocket, strError)) == NULL)
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
        if (!bExit && ((eSocketType == COMMON_SOCKET_ENCRYPTED && m_pUtility->sslRead(ssl, strBuffers[0], nReturn)) || (eSocketType == COMMON_SOCKET_UNENCRYPTED && m_pUtility->fdRead(fdSocket, strBuffers[0], nReturn))))
        {
          if ((unPosition = strBuffers[0].find("\n")) != string::npos)
          {
            ptJson = new Json(strBuffers[0].substr(0, unPosition));
            strBuffers[0].erase(0, (unPosition + 1));
            request(ptJson);
            ptJson->json(strBuffers[1]);
            strBuffers[1] += "\n";
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
        if ((eSocketType == COMMON_SOCKET_ENCRYPTED && m_pUtility->sslWrite(ssl, strBuffers[1], nReturn)) || (eSocketType == COMMON_SOCKET_UNENCRYPTED && m_pUtility->fdWrite(fdSocket, strBuffers[1], nReturn)))
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
