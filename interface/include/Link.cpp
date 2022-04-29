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
          Json *ptBoot = new Json;
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
                  int fdLink;
                  addrinfo *rp;
                  SSL *ssl;
                  for (rp = result; !bConnected[2] && rp != NULL; rp = rp->ai_next)
                  {
                    bConnected[0] = bConnected[1] = false;
                    if ((fdLink = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
                    {
                      bConnected[0] = true;
                      if (connect(fdLink, rp->ai_addr, rp->ai_addrlen) == 0)
                      {
                        bConnected[1] = true;
                        if ((ssl = m_pUtility->sslConnect(ctxC, fdLink, strError)) != NULL)
                        {
                          bConnected[2] = true;
                        }
                        else
                        {
                          close(fdLink);
                          fdLink = -1;
                        }
                      }
                      else
                      {
                        close(fdLink);
                        fdLink = -1;
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
                    link->fdSocket = fdLink;
                    link->ssl = ssl;
                    ptWrite->insert("_function", "handshake");
                    ptWrite->insert("Password", m_strPassword);
                    ptWrite->m["Links"] = new Json;
                    for (auto &subLink : m_links)
                    {
                      if (!subLink->strNode.empty() && !subLink->strServer.empty() && !subLink->strPort.empty())
                      {
                        Json *ptLink = new Json;
                        ptLink->insert("Node", subLink->strNode);
                        ptLink->insert("Server", subLink->strServer);
                        ptLink->insert("Port", subLink->strPort, 'n');
                        ptWrite->m["Links"]->push_back(ptLink);
                        delete ptLink;
                      }
                    }
                    if (!m_strMaster.empty())
                    {
                      ptWrite->->insert("Master", m_strMaster);
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
                    if (m_bOnline)
                    {
                      ptWrite->insert("Status", "online");
                    }
                    link->strBuffers[1].append(ptWrite->json(strJson) + "\n");
                    delete ptWrite;
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
                int fdLink;
                sockaddr_in cli_addr;
                socklen_t clilen = sizeof(cli_addr);
                // }}}
                if ((fdLink = ::accept(fdSocket, (sockaddr *)&cli_addr, &clilen)) >= 0)
                {
                  // {{{ prep work
                  sockaddr_storage addr;
                  socklen_t len = sizeof(addr);
                  ssMessage.str("");
                  ssMessage << strPrefix << "->accept():  Accepted incoming socket.";
                  log(ssMessage.str());
                  // }}}
                  if (getpeername(fdLink, (sockaddr *)&addr, &len) == 0)
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
                    if ((ssl = m_pUtility->sslAccept(ctxS, fdLink, strError)) != NULL)
                    {
                      Json *ptWrite = new Json;
                      radial_link *ptLink = new radial_link;
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Utility::sslAccept() [" << szIP << "]:  Accepted incoming socket.";
                      log(ssMessage.str());
                      ptLink->bAuthenticated = false;
                      ptLink->fdSocket = fdLink;
                      ptLink->ssl = ssl;
                      ptWrite->insert("_function", "handshake");
                      if (!m_links.empty())
                      {
                        ptWrite->m["Links"] = new Json;
                        for (auto &link : m_links)
                        {
                          if (!link->strNode && !link->strServer.empty() && !link->strPort.empty())
                          {
                            Json *ptSubLink = new Json;
                            ptSubLink->insert("Node", link->strNode);
                            ptSubLink->insert("Server", link->strServer);
                            ptSubLink->insert("Port", link->strPort);
                            ptWrite->m["Links"]->push_back(ptSubLink);
                            delete ptSubLink;
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
                      if (m_bOnline)
                      {
                        ptWrite->insert("Status", "online");
                      }
                      ptLink->strBuffers[1].append(ptWrite->json(strJson) + "\n");
                      delete ptWrite;
                      /*
                      if (m_unLink == RADIAL_LINK_MASTER)
                      {
                        Json *ptStorage = new Json;
                        if (storageRetrieve(ptStorage, strError))
                        {
                          stringstream ssWrite;
                          ptWrite = new Json;
                          ptWrite->insert("_function", "storage");
                          ptWrite->insert("Function", "update");
                          ptWrite->insert("Request", ptStorage);
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
                          ssMessage << strPrefix << ":  Appended storage to link output buffer.";
                          log(ssMessage.str());
                        }
                        delete ptStorage;
                      }
                      */
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
                        close(fdLink);
                      }
                      delete ptLink;
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Utility::sslAccept() error:  " << strError;
                      notify(ssMessage.str());
                      close(fdLink);
                    }
                  }
                  else
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->getpeername(" << errno << ") error:  " << strerror(errno);
                    notify(ssMessage.str());
                    close(fdLink);
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
              for (auto &link : m_links)
              {
                for (size_t i = 1; i < unIndex; i++)
                {
                  // {{{ link
                  if (link->fdSocket == fds[i].fd)
                  {
                    // {{{ read
                    if (fds[i].revents & POLLIN)
                    {
                      nReturn = -1;
                      ERR_clear_error();
                      if (m_pUtility->sslRead(link->ssl, link->strBuffers[0], nReturn))
                      {
                        while ((unPosition = link->strBuffers[0].find("\n")) != string::npos)
                        {
                          Json *ptJson = new Json(link->strBuffers[0].substr(0, unPosition));
                          link->strBuffers[0].erase(0, (unPosition + 1));
                          thread threadLinkRequest(&Link::request, this, strPrefix, link->strNode, ptJson);
                          threadLinkRequest.detach();
                        }
                      }
                      else
                      {
                        removals.push_back(link->fdSocket);
                        if (nReturn < 0)
                        {
                          ssMessage.str("");
                          ssMessage << strPrefix << "->Utility::sslRead(" << SSL_get_error(link->ssl, nReturn) << ") error [" << link->strNode << "]:  " << m_pUtility->sslstrerror();
                          log(ssMessage.str());
                        }
                      }
                    }
                    // }}}
                    // {{{ write
                    if (fds[i].revents & POLLOUT)
                    {
                      ERR_clear_error();
                      if (!m_pUtility->sslWrite(link->ssl, link->strBuffers[1], nReturn))
                      {
                        removals.push_back(link->fdSocket);
                        if (nReturn < 0)
                        {
                          ssMessage.str("");
                          ssMessage << strPrefix << "->Utility::sslWrite(" << SSL_get_error(link->ssl, nReturn) << ") error [" << link->strNode << "]:  " << m_pUtility->sslstrerror();
                          log(ssMessage.str());
                        }
                      }
                    }
                    // }}}
                  }
                  // }}}
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
            // {{{ post work
            delete[] fds;
            // {{{ removals
            if (!removals.empty())
            {
              removals.sort();
              removals.unique();
              m_mutex.lock();
              for (auto &i : removals)
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
                    ssMessage << strPrefix << "->close() [" << (*removeIter)->strNode << "]:  Closed link socket.";
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
                  ssMessage << strPrefix << " [" << (*removeIter)->strNode << "]:  Removed link.";
                  log(ssMessage.str());
                  delete (*removeIter);
                  m_links.erase(removeIter);
                }
              }
              m_mutex.unlock();
              removals.clear();
            }
            // }}}
            // {{{ bootstrap links
            if (m_links.empty() && m_ptLink->m.find("Links") != m_ptLink->m.end())
            {
              if (ptBoot->l.empty() && m_ptLink->m.find("Links") != m_ptLink->m.end())
              {
                for (auto &i : m_ptLink->m["Links"]->l)
                {
                  if (i->m.find("Server") != i->m.end() && !i->m["Server"]->v.empty() && i->m.find("Port") != i->m.end() && !i->m["Port"]->v.empty())
                  {
                    Json *ptLink = new Json;
                    ptLink->insert("Server", i->m["Server"]->v);
                    ptLink->insert("Port", i->m["Port"]->v, 'n');
                    ptBoot->push_back(ptLink);
                    delete ptLink;
                  }
                }
              }
              if (!ptBoot->l.empty())
              {
                if (ptBoot->l.front()->m.find("Server") != ptBoot->l.front()->m.end() && !ptBoot->l.front()->m["Server"]->v.empty() && ptBoot->l.front()->m.find("Port") != ptBoot->l.front()->m.end() && !ptBoot->l.front()->m["Port"]->v.empty())
                {
                  radial_link *ptLink = new radial_link;
                  ptLink->bAuthenticated = true;
                  ptLink->strServer = ptBoot->l.front()->m["Server"]->v;
                  ptLink->strPort = ptBoot->l.front()->m["Port"]->v;
                  ptLink->fdSocket = -1;
                  ptLink->ssl = NULL;
                  if ((unResult = add(ptLink)) > 0)
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Link::add() [" << ptBoot->l.front()->m["Server"]->v << "]:  " << ((unResult == 1)?"Added":"Updated") << " link.";
                    log(ssMessage.str());
                  }
                  else
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Link::add() error [" << ptBoot->l.front()->m["Server"]->v << "]:  Failed to add link.";
                    notify(ssMessage.str());
                  }
                  delete ptLink;
                  delete ptBoot->l.front();
                  ptBoot->l.pop_front();
                }
              }
            }
            // }}}
            // }}}
          }
          // {{{ post work
          delete ptBoot;
          m_mutex.lock();
          for (auto &link : links)
          {
            if (link->fdSocket != -1)
            { 
              SSL_shutdown(link->ssl);
              SSL_free(link->ssl);
              close(link->fdSocket);
              ssMessage.str("");
              ssMessage << strPrefix << "->close() [" << link->strNode << "]:  Closed link socket.";
              log(ssMessage.str());
            }
            ssMessage.str("");
            ssMessage << strPrefix << " [" << link->strNode << "]:  Removed link.";
            log(ssMessage.str());
            delete link;
          }
          m_links.clear();
          m_mutex.unlock();
          // }}}
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
        bool bClose = false;
        bFound = true;
        if (ptLink->bAuthenticated)
        {
          (*linkIter)->bAuthenticated = ptLink->bAuthenticated;
        }
        if (ptLink->fdSocket != -1 && )
        {
          if ((*linkIter)->fdSocket == -1)
          {
            (*linkIter)->fdSocket = ptLink->fdSocket;
          }
          else
          {
            bClose = true;
          }
        }
        if (!ptLink->strNode.empty())
        {
          (*linkIter)->strNode = ptLink->strNode;
        }
        if (!ptLink->strServer.empty())
        {
          (*linkIter)->strServer = ptLink->strServer;
        }
        if (!ptLink->strPort.empty())
        {
          (*linkIter)->strPort = ptLink->strPort;
        }
        unResult = ((bClose)?0:2);
      }
    }
    if (!bFound)
    {
      radial_link *ptAdd = new radial_link;
      ptAdd->bAuthenticated = ptLink->bAuthenticated;
      ptAdd->fdSocket = ptLink->fdSocket;
      ptAdd->ssl = ptLink->ssl;
      ptAdd->strBuffers[0] = ptAdd->strBuffers[0];
      ptAdd->strBuffers[1] = ptAdd->strBuffers[1];
      ptAdd->strNode = ptAdd->strNode;
      ptAdd->strPort = ptAdd->strPort;
      ptAdd->strServer = ptAdd->strServer;
      m_mutex.lock();
      m_links.push_back(ptAdd);
      m_mutex.unlock();
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
void Link::request(string strPrefix, const string strNode, Json *ptJson)
{
  string strError, strJson;
  stringstream ssMessage;

  strPrefix += "->Link::request()";
  if (ptJson->m.find("_function") != ptJson->m.end() && !ptJson->m["_function"]->v.empty())
  {
    if (ptJson->m["_function"]->v == "handshake")
    {
    }
  }
  else if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
  {
    Json *ptInterfaces = new Json;
    ptInterfaces->insert("Function", "list");
    target(ptInterfaces);
    if (ptInterfaces->m.find("Response") != ptInterfaces->m.end())
    {
      if (ptInterfaces->m["Response"]->m.find(ptJson->m["Interface"]->v) != ptInterfaces->m["Response"]->m.end())
      {
        target(ptJson->m["Interface"]->v, ptJson);
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
    m_mutex.lock();
    for (auto &link : m_links)
    {
      if (link->strNode == strNode)
      {
        link->strBuffers[1].append(ptJson->json(strJson) + "\n");
      }
    }
    m_mutex.unlock();
  }
  delete ptJson;
}
// }}}
}
}
