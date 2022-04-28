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
  ifstream inLink((m_strData + "/link.json");
  m_bOnline = false;
  m_ptLink = NULL;
  if (inLink)
  {
    string strLine;
    stringstream ssJson;
    while (getline(inLink, strLine))
    {
      ssJson << strLine;
    }
    m_ptLink = new Json(ssJson.str());
  }
  m_pWarden->vaultRetrieve(["link", "Password"], m_strPassword, strError);
  m_unLink = 0;
}
// }}}
// {{{ ~Link()
Link::~Link()
{
  delete m_ptLink;
}
// }}}
// {{{ accept()
void Link::accept(string strPrefix)
{
  // {{{ prep work
  SSL_CTX *ctxC = NULL, *ctxS = NULL;
  string strError, strJson;
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
          pollfd *fds;
          list<int> removals;
          size_t unIndex, unPosition, unReturn;
          ssMessage.str("");
          ssMessage << strPrefix << "->listen():  Listening to incoming socket.";
          log(ssMessage.str());
          // }}}
          while (!shutdown() && !bExit)
          {
            // {{{ prep work
            fds = new pollfd[m_links.size() + 1];
            unIndex = 0;
            fds[unIndex].fd = fdSocket;
            fds[unIndex].events = POLLIN;
            unIndex++;
            for (auto &link : m_links)
            {
              fds[unIndex].fd = link->fdSocket;
              fds[unIndex].events = POLLIN;
              if (link->fdSocket == -1)
              {
                memset(&hints, 0, sizeof(addrinfo));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                if ((nReturn = getaddrinfo(link->strServer.c_str(), linnk->strPort.c_str(), &hints, &result)) == 0)
                {
                  bool bConnected[3] = {false, false, false};
                  int fdClient;
                  addrinfo *rp;
                  SSL *ssl;
                  for (rp = result; !bConnected[2] && rp != NULL; rp = rp->ai_next)
                  {
                    bConnected[0] = bConnected[1] = false;
                    if ((fdClient = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
                    {
                      bConnected[0] = true;
                      if (connect(fdClient, rp->ai_addr, rp->ai_addrlen) == 0)
                      {
                        bConnected[1] = true;
                        if ((ssl = gpCentral->utility()->sslConnect(ctxC, fdClient, strError)) != NULL)
                        {
                          bConnected[2] = true;
                        }
                        else
                        {
                          close(fdClient);
                          fdClient = -1;
                        }
                      }
                      else
                      {
                        close(fdClient);
                        fdClient = -1;
                      }
                    }
                  }
                  freeaddrinfo(result);
                  if (bConnected[2])
                  {
                    Json *ptWrite = new Json;
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Utility::sslConnect() [" << link->strServer << "]:  Connected to link.";
                    gpCentral->log(ssMessage.str());
                    link->fdSocket = fdClient;
                    link->ssl = ssl;
                    ptWrite->insert("Action", "identity");
                    ptWrite->m["Clients"] = new Json;
                    for (auto &subLink : m_links)
                    {
                      if (!subLink->strNode.empty() && !subLink->strServer.empty() && !subLink->strPort.empty())
                      {
                        Json *ptClient = new Json;
                        ptClient->insert("Node", subLink->strNode);
                        ptClient->insert("Server", subLink->strServer);
                        ptClient->insert("Port", subLink->strPort, 'n');
                        ptWrite->m["Clients"]->push_back(ptClient);
                        delete ptClient;
                      }
                    }
                    if (!m_strMaster.empty())
                    {
                      ptWrite->insert("Master", m_strMaster);
                    }
                    ptWrite->m["You"] = new Json;
                    ptWrite->m["You"]->insert("Server", link->strServer);
                    ptWrite->m["Me"] = new Json;
                    ptWrite->m["Me"]->insert("Node", m_ptLink->m["Node"]->v);
                    ptWrite->m["Me"]->insert("Port", m_ptLink->m["Port"]->v, 'n');
                    if (!strServer.empty())
                    {
                      ptWrite->m["Me"]->insert("Server", strServer);
                    }
                    link->strBuffers[1].append(ptWrite->json(strJson) + "\n");
                    delete ptWrite;
                    if (m_bOnline)
                    {
                      ptWrite = new Json;
                      ptWrite->insert("Action", "status");
                      ptWrite->insert("Status", "online");
                      link->strBuffers[1].append(ptWrite->json(strJson) + "\n");
                      delete ptWrite;
                    }
                  }
                  else if (!bConnected[1])
                  {
                    removals.push_back(-1);
                    ssMessage.str("");
                    ssMessage << strPrefix << "->" << ((!bConnected[0])?"socket":"connect") << "(" << errno << ") error [" << link->strServer << "," << link->strPort << "," << link->strNode << "]:  " << strerror(errno);
                    log(ssMessage.str());
                  }
                  else
                  {
                    removals.push_back(-1);
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Utility::sslConnect(" << strError << ") error [" << link->strServer << "," << link->strPort << "," << link->strNode << "]:  " << strError;
                    log(ssMessage.str());
                  }
                }
                else
                {
                  removals.push_back(-1);
                  ssMessage.str("");
                  ssMessage << strPrefix << "->getaddrinfo(" << nReturn << ") error [" << link->strServer << "," << link->strPort << "," << link->strNode << "]:  " << gai_strerror(nReturn);
                  log(ssMessage.str());
                }
              }
              if (link->fdSocket != -1 && !link->strBuffers[1].empty())
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
                      Json *ptWrite = new Json;
                      radial_link *ptLink = new radial_link;
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Utility::sslAccept() [" << szIP << "]:  Accepted incoming socket.";
                      log(ssMessage.str());
                      ptLink->fdSocket = fdClient;
                      ptLink->ssl = ssl;
                      ptWrite->insert("Action", "identity");
                      if (!m_links.empty())
                      {
                        ptWrite->m["Clients"] = new Json;
                        for (auto &link : m_links)
                        {
                          if (!link->strNode && !link->strServer.empty() && !link->strPort.empty())
                          {
                            Json *ptClient = new Json;
                            ptClient->insert("Node", link->strNode);
                            ptClient->insert("Server", link->strServer);
                            ptClient->insert("Port", link->strPort);
                            ptWrite->m["Clients"]->push_back(ptClient);
                            delete ptClient;
                          }
                        }
                      }
                      if (!m_strMaster.empty())
                      {
                        ptWrite->insert("Master", m_strMaster);
                      }
                      ptWrite->m["You"] = new Json;
                      ptWrite->m["You"]->insert("Server", szIP);
                      ptWrite->m["Me"] = new Json;
                      ptWrite->m["Me"]->insert("Node", m_ptLink->m["Node"]->v);
                      ptWrite->m["Me"]->insert("Port", m_ptLink->m["Port"]->v, 'n');
                      if (!strServer.empty())
                      {
                        ptWrite->m["Me"]->insert("Server", strServer);
                      }
                      ptLink->strBuffers[1].append(ptWrite->json(strJson) + "\n");
                      delete ptWrite;
                      if (m_bOnline)
                      {
                        ptWrite = new Json;
                        ptWrite->insert("Action", "status");
                        ptWrite->insert("Status", "online");
                        ptLink->strBuffers[1].append(ptWrite->json(strJson) + "\n");
                        delete ptWrite;
                      }
                      if (m_unLink == RADIAL_LINK_MASTER)
                      {
                        Json *ptStorage = new Json;
                        if (storageRetrieve(ptStorage, strError))
                        {
                          stringstream ssWrite;
                          ptWrite = new Json;
                          ptWrite->insert("Action", "storage");
                          ptWrite->insert("SubAction", "update");
                          ptWrite->insert("Data", ptStorage);
                          ssMessage.str("");
                          ssMessage << strPrefix << ":  Allocated copy of storage for transmission.";
                          log(ssMessage.str());
                          ssWrite << ptWrite << endl;
                          delete ptWrite;
                          ssMessage.str("");
                          ssMessage << strPrefix << ":  Encoded " << ssWrite.str().size() << " bytes of storage for transmission.";
                          log(ssMessage.str());
                          ptLink->strBuffers[1].append(ssWrite.str());
                          ssMessage.str("");
                          ssMessage << strPrefix << ":  Appended storage to client output buffer.";
                          log(ssMessage.str());
                        }
                        delete ptStorage;
                      }
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Link::add()";
                      if ((unResult = add(ptLink)) > 0)
                      {
                        ssMessage << ":  " << ((unResult == 1)?"Added":"Updated") << " link.";
                        log(ssMessage.str());
                      }
                      else
                      {
                        ssMessage << " error:  Failed to add link.";
                        SSL_shutdown(ssl);
                        SSL_free(ssl);
                        close(fdClient);
                      }
                      delete ptLink;
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
            delete[] fds;
            // {{{ removals
            removals.sort();
            removals.unique();
            for (auto &i : removeList)
            {
              list<radial_link *>::iterator removeIter = m_links.end();
              for (auto j = m_links.begin(); removeIter == m_links.end() && j != m_links.end(); j++)
              {
                if ((*j)->fdSocket == i)
                {
                  removeIter = j;
                }
              }
              if (removeIter != m_links.end())
              { 
                if ((*removeIter)->fdSocket != -1)
                { 
                  SSL_shutdown((*removeIter)->ssl);
                  SSL_free((*removeIter)->ssl);
                  close((*removeIter)->fdSocket);
                  ssMessage.str("");
                  ssMessage << strPrefix << "->close() [" << (*removeIter)->strNode << "]:  Closed client socket.";
                  log(ssMessage.str());
                }
                if (!m_strMaster.empty() && (*removeIter)->strNode == m_strMaster)
                { 
                  m_strMaster.clear();
                  ssMessage.str("");
                  ssMessage << strPrefix << " [" << (*removeIter)->strNode << "]:  Unset as master.";    
                  log(ssMessage.str());
                }
                ssMessage.str("");
                ssMessage << strPrefix << " [" << (*removeIter)->strNode << "]:  Removed client.";
                log(ssMessage.str());
                delete (*removeIter);
                m_links.erase(removeIter);
              }
            }
            removals.clear();
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
// {{{ add()
size_t Link::add(Json *ptLink)
{
  bool bHasNode = false, bHasServer = false, bHasSocket = false;
  size_t unResult = 0;

  if (!ptLink->strNode.empty())
  {
    bHasNode = true;
  }
  if (ptLink->fdSocket != -1)
  {
    bHasSocket = true;
  }
  if (!ptLink->strServer.empty() && !ptLink->strPort.empty())
  {
    bHasServer = true;
  }
  if (bHasNode || bHasSocket || bHasServer)
  {
    bool bFound = false;
    for (auto linkIter = m_links.begin(); !bFound && linkIter != m_links.end(); linkIter++)
    {
      if ((bHasNode && (*linkIter)->strNode == ptLink->strNode) || (bHasSocket && (*linkIter)->fdSocket == ptLink->fdSocket) || (bHasServer && (*linkIter)->strServer == ptLink->strServer && (*linkIter)->strPort == ptLink->strPort))
      {
        bFound = true;
        if (!ptLink->strNode.empty())
        {
          (*linkIter)->strNode = ptLink->strNode;
        }
        if (ptLink->fdSocket != -1)
        {
          (*linkIter)->fdSocket = ptLink->fdSocket;
        }
        if (!ptLink->strServer.empty())
        {
          (*linkIter)->strServer = ptLink->strServer;
        }
        if (!ptLink->strPort.empty())
        {
          (*linkIter)->strPort = ptLink->strPort;
        }
        unResult = 2;
      }
    }
    if (!bFound)
    {
      radial_link *ptAdd = new radial_link;
      ptAdd->fdSocket = ptLink->fdSocket;
      ptAdd->ssl = ptLink->ssl;
      ptAdd->strBuffers[0] = ptAdd->strBuffers[0];
      ptAdd->strBuffers[1] = ptAdd->strBuffers[1];
      ptAdd->strNode = ptAdd->strNode;
      ptAdd->strPort = ptAdd->strPort;
      ptAdd->strServer = ptAdd->strServer;
      m_links.push_back(ptAdd);
      unResult = 1;
    }
  }

  return unResult;
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
