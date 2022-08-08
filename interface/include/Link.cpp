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
Link::Link(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "link", argc, argv, pCallback)
{
  ifstream inLink((m_strData + "/link.json").c_str());
  string strError;

  m_bUpdate = true;
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
  inLink.close();
  if (m_pWarden != NULL)
  {
    m_pWarden->vaultRetrieve({"link", "Password"}, m_strPassword, strError);
  }
  m_unLink = RADIAL_LINK_UNKNOWN;
}
// }}}
// {{{ ~Link()
Link::~Link()
{
  if (m_ptLink != NULL)
  {
    delete m_ptLink;
  }
}
// }}}
// {{{ add()
size_t Link::add(list<radialLink *> &links, radialLink *ptLink)
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
    for (auto linkIter = links.begin(); !bFound && linkIter != links.end(); linkIter++)
    {
      if ((bHasNode && (*linkIter)->strNode == ptLink->strNode) || (bHasServer && (*linkIter)->strServer == ptLink->strServer && (*linkIter)->strPort == ptLink->strPort))
      {
        bool bClose = false;
        bFound = true;
        if (ptLink->bAuthenticated)
        {
          (*linkIter)->bAuthenticated = ptLink->bAuthenticated;
        }
        if (ptLink->fdSocket != -1)
        {
          if ((*linkIter)->fdSocket == -1)
          {
            (*linkIter)->fdSocket = ptLink->fdSocket;
            (*linkIter)->ssl = ptLink->ssl;
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
        if (!bClose)
        {
          unResult = 2;
        }
      }
    }
    if (!bFound)
    {
      radialLink *ptAdd = new radialLink;
      ptAdd->bAuthenticated = ptLink->bAuthenticated;
      ptAdd->bSslAcceptRetry = ptLink->bSslAcceptRetry;
      ptAdd->bSslConnectRetry = ptLink->bSslConnectRetry;
      ptAdd->fdConnecting = ptLink->fdConnecting;
      ptAdd->fdSocket = ptLink->fdSocket;
      ptAdd->rp = ptLink->rp;
      ptAdd->ssl = ptLink->ssl;
      ptAdd->unUnique = ptLink->unUnique;
      ptAdd->responses.clear();
      while (!ptLink->responses.empty())
      {
        ptAdd->responses.push_back(ptLink->responses.front());
        ptLink->responses.pop_front();
      }
      ptAdd->strBuffers[0] = ptLink->strBuffers[0];
      ptAdd->strBuffers[1] = ptLink->strBuffers[1];
      ptAdd->strNode = ptLink->strNode;
      ptAdd->strPort = ptLink->strPort;
      ptAdd->strServer = ptLink->strServer;
      links.push_back(ptAdd);
      unResult = 1;
    }
  }

  return unResult;
}
// }}}
// {{{ process()
void Link::process(string strPrefix)
{
  // {{{ prep work
  long lArg;
  SSL_CTX *ctxC = NULL, *ctxS = NULL;
  string strError, strJson;
  stringstream ssMessage;
  strPrefix += "->Link::socket()";
  if ((lArg = fcntl(0, F_GETFL, NULL)) >= 0)
  {   
    lArg |= O_NONBLOCK;
    fcntl(0, F_SETFL, lArg);
  }
  if ((lArg = fcntl(1, F_GETFL, NULL)) >= 0)
  {
    lArg |= O_NONBLOCK;
    fcntl(1, F_SETFL, lArg);
  }
  // }}}
  if ((ctxS = m_pUtility->sslInitServer(m_strData + "/server.crt", m_strData + "/server.key", strError)) != NULL && (ctxC = m_pUtility->sslInitClient(strError)) != NULL)
  {
    // {{{ prep work
    addrinfo hints, *result;
    bool bBound[3] = {false, false, false};
    int fdSocket, nReturn;
    SSL_CTX_set_mode(ctxS, SSL_MODE_AUTO_RETRY);
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
      if (listen(fdSocket, 5) == 0)
      {
        // {{{ prep work
        bool bExit = false;
        list<radialLink *> links;
        list<int> removals;
        pollfd *fds;
        size_t unIndex, unLink = m_unLink, unPosition, unUnique = 0;
        string strLine;
        time_t CBootstrap, CBroadcast, CTime, unBootstrapSleep = 0, unBroadcastSleep = 5;
        Json *ptBoot = new Json;
        ssMessage.str("");
        ssMessage << strPrefix << "->listen():  Listening to incoming socket.";
        log(ssMessage.str());
        time(&CTime);
        CBroadcast = CBootstrap = CTime;
        // }}}
        while (!bExit)
        {
          // {{{ prep work
          fds = new pollfd[links.size() + 3];
          unIndex = 0;
          // {{{ stdin
          fds[unIndex].fd = 0;
          fds[unIndex].events = POLLIN;
          unIndex++;
          // }}}
          // {{{ stdout
          fds[unIndex].fd = -1;
          fds[unIndex].events = POLLOUT;
          if (m_strBuffers[1].empty())
          {
            m_mutexShare.lock();
            while (!m_responses.empty())
            {
              m_strBuffers[1].append(m_responses.front() + "\n");
              m_responses.pop_front();
            }
            m_mutexShare.unlock(); 
          }
          if (!m_strBuffers[1].empty())
          {
            fds[unIndex].fd = 1;
          }
          unIndex++;
          // }}}
          // {{{ accept
          fds[unIndex].fd = fdSocket;
          fds[unIndex].events = POLLIN;
          unIndex++;
          // }}}
          // {{{ links
          for (auto &link : links)
          {
            fds[unIndex].events = POLLIN;
            if (link->fdSocket == -1)
            {
              // {{{ getaddrinfo
              if (link->rp == NULL)
              {
                memset(&(link->hints), 0, sizeof(addrinfo));
                link->hints.ai_family = AF_UNSPEC;
                link->hints.ai_socktype = SOCK_STREAM;
                if ((nReturn = getaddrinfo(link->strServer.c_str(), link->strPort.c_str(), &(link->hints), &(link->result))) == 0)
                {
                  link->rp = link->result;
                }
                else
                {
                  removals.push_back(-1);
                }
              }
              // }}}
              // {{{ socket
              else if (link->fdConnecting == -1)
              {
                if ((link->fdConnecting = socket(link->rp->ai_family, link->rp->ai_socktype, link->rp->ai_protocol)) >= 0)
                {
                  long lArg;
                  if ((lArg = fcntl(link->fdConnecting, F_GETFL, NULL)) >= 0)
                  {
                    lArg |= O_NONBLOCK;
                    fcntl(link->fdConnecting, F_SETFL, lArg);
                  }
                }
                else
                {
                  link->rp = link->rp->ai_next;
                  if (link->rp == NULL)
                  {
                    freeaddrinfo(link->result);
                    removals.push_back(-1);
                  }
                }
              }
              // }}}
              // {{{ connect
              else if (connect(link->fdConnecting, link->rp->ai_addr, link->rp->ai_addrlen) == 0)
              {
                link->fdSocket = link->fdConnecting;
                link->fdConnecting = -1;
                if ((link->ssl = m_pUtility->sslConnect(ctxC, link->fdSocket, link->bSslConnectRetry, strError)) == NULL)
                {
                  removals.push_back(link->fdSocket);
                }
              }
              // }}}
              // {{{ error
              else if (errno != EAGAIN && errno != EINPROGRESS)
              {
                close(link->fdConnecting);
                link->fdConnecting = -1;
                link->rp = link->rp->ai_next;
              }
              // }}}
              // {{{ payload
              if (link->fdSocket != -1)
              {
                Json *ptWrite = new Json;
                freeaddrinfo(link->result);
                ptWrite->insert("_function", "handshake");
                ptWrite->insert("Password", m_strPassword);
                ptWrite->m["Links"] = new Json;
                for (auto &subLink : links)
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
                ptWrite->m["You"] = new Json;
                ptWrite->m["You"]->insert("Server", link->strServer);
                ptWrite->m["Me"] = new Json;
                ptWrite->m["Me"]->insert("Node", m_ptLink->m["Node"]->v);
                ptWrite->m["Me"]->insert("Port", m_ptLink->m["Port"]->v, 'n');
                if (!m_strServer.empty())
                {
                  ptWrite->m["Me"]->insert("Server", m_strServer);
                }
                link->responses.push_back(ptWrite->json(strJson));
                delete ptWrite;
                if (!m_strMaster.empty())
                {
                  ptWrite = new Json;
                  ptWrite->insert("_function", "master");
                  ptWrite->insert("Node", m_strMaster);
                  link->responses.push_back(ptWrite->json(strJson));
                  delete ptWrite;
                }
                if (m_unLink == RADIAL_LINK_MASTER)
                {
                  stringstream ssUnique;
                  Json *ptStorage = new Json;
                  ptStorage->insert("_function", "storageTransmit");
                  ptStorage->insert("_source", m_strName);
                  ptStorage->insert("_target", "storage");
                  ssUnique << link->fdSocket << " " << link->unUnique;
                  ptStorage->insert("_unique", ssUnique.str());
                  ptStorage->insert("Function", "retrieve");
                  hub(ptStorage, false);
                  delete ptStorage;
                }
              }
              // }}}
            }
            // {{{ SSL_accept
            if (link->bSslAcceptRetry)
            {
              if ((nReturn = SSL_accept(link->ssl)) > 0)
              {
                link->bSslAcceptRetry = false;
              }
              else
              {
                strError = m_pUtility->sslstrerror(link->ssl, nReturn, link->bSslAcceptRetry);
                if (!link->bSslAcceptRetry)
                {
                  link->bSslAcceptRetry = true;
                  removals.push_back(link->fdSocket);
                }
              }
            }
            // }}}
            // {{{ SSL_connect
            if (link->bSslConnectRetry)
            {
              if ((nReturn = SSL_connect(link->ssl)) == 1)
              {
                link->bSslConnectRetry = false;
              }
              else
              {
                strError = m_pUtility->sslstrerror(link->ssl, nReturn, link->bSslConnectRetry);
                if (!link->bSslConnectRetry)
                {
                  link->bSslConnectRetry = true;
                  removals.push_back(link->fdSocket);
                }
              }
            }
            // }}}
            // {{{ fds
            if (!link->bSslAcceptRetry && !link->bSslConnectRetry)
            {
              fds[unIndex].fd = link->fdSocket;
              if (link->strBuffers[1].empty())
              {
                while (!link->responses.empty())
                {
                  link->strBuffers[1].append(link->responses.front() + "\n");
                  link->responses.pop_front();
                }
              }
              if (!link->strBuffers[1].empty())
              {
                fds[unIndex].events |= POLLOUT;
              }
            }
            else
            {
              fds[unIndex].fd = -1;
            }
            unIndex++;
            // }}}
          }
          // }}}
          // }}}
          if ((nReturn = poll(fds, unIndex, 10)) > 0)
          {
            // {{{ stdin
            if (fds[0].revents & (POLLHUP | POLLIN))
            {
              if (m_pUtility->fdRead(fds[0].fd, m_strBuffers[0], nReturn))
              {
                while ((unPosition = m_strBuffers[0].find("\n")) != string::npos)
                {
                  Json *ptJson;
                  strLine = m_strBuffers[0].substr(0, unPosition);
                  m_strBuffers[0].erase(0, (unPosition + 1));
                  ptJson = new Json(strLine);
                  if (ptJson->m.find("_request") != ptJson->m.end())
                  {
                    Json *ptSubJson = ptJson->m["_request"];
                    if (ptSubJson->m.find("_source") != ptSubJson->m.end() && ptSubJson->m["_source"]->v == m_strName && ptSubJson->m.find("_unique") != ptSubJson->m.end() && !ptSubJson->m["_unique"]->v.empty())
                    {
                      int fdLink;
                      list<radialLink *>::iterator linkIter = links.end();
                      size_t unUnique;
                      stringstream ssUnique(ptSubJson->m["_unique"]->v);
                      ssUnique >> fdLink >> unUnique;
                      for (auto i = links.begin(); linkIter == links.end() && i != links.end(); i++)
                      {
                        if ((*i)->fdSocket == fdLink && (*i)->unUnique == unUnique)
                        {
                          linkIter = i;
                        }
                      }
                      if (linkIter != links.end())
                      {
                        if (ptJson->m.find("Function") != ptJson->m.end() && ptJson->m["Function"]->v == "list")
                        {
                          if (ptJson->m.find("Response") != ptJson->m.end())
                          {
                            if (ptSubJson->m.find("Interface") != ptSubJson->m.end() && !ptSubJson->m["Interface"]->v.empty())
                            {
                              if (ptJson->m["Response"]->m.find(ptSubJson->m["Interface"]->v) != ptJson->m["Response"]->m.end())
                              {
                                hub(ptSubJson, false);
                              }
                              else
                              {
                                keyRemovals(ptSubJson);
                                ptSubJson->insert("Status", "error");
                                ptSubJson->insert("Error", "Interface does not exist.");
                                (*linkIter)->responses.push_back(ptSubJson->json(strJson));
                              }
                            }
                            else
                            {
                              keyRemovals(ptSubJson);
                              ptSubJson->insert("Status", "error");
                              ptSubJson->insert("Error", "Please provide the Interface.");
                              (*linkIter)->responses.push_back(ptSubJson->json(strJson));
                            }
                          }
                          else
                          {
                            keyRemovals(ptSubJson);
                            ptSubJson->insert("Status", "error");
                            ptSubJson->insert("Error", "Failed to retrieve interfaces.");
                            (*linkIter)->responses.push_back(ptSubJson->json(strJson));
                          }
                        }
                        else
                        {
                          keyRemovals(ptSubJson);
                          ptSubJson->insert("Status", "error");
                          ptSubJson->insert("Error", "Invalid path of logic.");
                          (*linkIter)->responses.push_back(ptSubJson->json(strJson));
                        }
                      }
                      else
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " error [stdin," << fdLink << "," << unUnique << "]:  Link no longer exists.";
                        log(ssMessage.str());
                      }
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << " error [stdin]:  ";
                      if (ptSubJson->m.find("_source") == ptSubJson->m.end())
                      {
                        ssMessage << "Internal source does not exist.";
                      }
                      else if (ptSubJson->m["_source"]->v != m_strName)
                      {
                        ssMessage << "Internal source does not match.";
                      }
                      else
                      {
                        ssMessage << "Internal unique does not exist.";
                      }
                      log(ssMessage.str());
                    }
                  }
                  else if (ptJson->m.find("_source") != ptJson->m.end() && ptJson->m["_source"]->v == m_strName && ptJson->m.find("_unique") != ptJson->m.end() && !ptJson->m["_unique"]->v.empty())
                  {
                    int fdLink;
                    list<radialLink *>::iterator linkIter = links.end();
                    size_t unUnique;
                    stringstream ssUnique(ptJson->m["_unique"]->v);
                    ssUnique >> fdLink >> unUnique;
                    for (auto i = links.begin(); linkIter == links.end() && i != links.end(); i++)
                    {
                      if ((*i)->fdSocket == fdLink && (*i)->unUnique == unUnique)
                      {
                        linkIter = i;
                      }
                    }
                    if (linkIter != links.end())
                    {
                      if (ptJson->m.find("_function") != ptJson->m.end())
                      {
                        if (ptJson->m["_function"]->v == "storageTransmit")
                        {
                          if (ptJson->m.find("Response") != ptJson->m.end())
                          {
                            Json *ptWrite = new Json;
                            ptWrite->insert("Interface", "storage");
                            ptWrite->insert("Function", "update");
                            ptWrite->insert("Request", ptJson->m["Response"]);
                            (*linkIter)->responses.push_back(ptWrite->json(strJson));
                            delete ptWrite;
                          }
                        }
                        else
                        {
                          ssMessage.str("");
                          ssMessage << strPrefix << " error [stdin," << fdLink << "," << unUnique << "," << ptJson->m["_function"]->v << "]:  Please provide valid _function:  storageTransmit.";
                          log(ssMessage.str());
                        }
                      }
                      else
                      {
                        keyRemovals(ptJson);
                        (*linkIter)->responses.push_back(ptJson->json(strJson));
                      }
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << " error [stdin," << fdLink << "," << unUnique << "]:  Link no longer exists.";
                      log(ssMessage.str());
                    }
                  }
                  else if (ptJson->m.find("_source") != ptJson->m.end() && ptJson->m["_source"]->v == "hub")
                  {
                    if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
                    {
                      if (ptJson->m["Function"]->v == "shutdown")
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " [stdin,hub," << ptJson->m["Function"]->v << "]:  Shutting down.";
                        log(ssMessage.str());
                        setShutdown();
                      }
                      else
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " error [stdin,hub," << ptJson->m["Function"]->v << "]:  Please provide a valid Function:  shutdown.";
                        log(ssMessage.str());
                      }
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << " error [stdin,hub]:  Please provide a Function.";
                      log(ssMessage.str());
                    }
                  }
                  else
                  {
                    bool bProcessed = false;
                    strError.clear();
                    if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty() && ptJson->m["Interface"]->v != "link")
                    {
                      Json *ptSubJson = new Json(ptJson);
                      bProcessed = true;
                      keyRemovals(ptSubJson);
                      ptSubJson->json(strJson);
                      delete ptSubJson;
                      for (auto &link : links)
                      {
                        if (link->bAuthenticated)
                        {
                          link->responses.push_back(strJson);
                        }
                      }
                    }
                    else if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
                    {
                      if (ptJson->m["Function"]->v == "ping")
                      {
                        bProcessed = true;
                      }
                      else if (ptJson->m["Function"]->v == "status")
                      {
                        list<string> subLinks;
                        Json *ptStatus = new Json;
                        bProcessed = true;
                        if (!m_strMaster.empty())
                        {
                          ptStatus->insert("Master", m_strMaster);
                        }
                        if (m_ptLink->m.find("Node") != m_ptLink->m.end() && !m_ptLink->m["Node"]->v.empty())
                        {
                          ptStatus->insert("Node", m_ptLink->m["Node"]->v);
                        }
                        for (auto &link : links)
                        {
                          if (!link->strNode.empty())
                          {
                            subLinks.push_back(link->strNode);
                          }
                        }
                        subLinks.sort();
                        subLinks.unique();
                        if (!subLinks.empty())
                        {
                          ptStatus->insert("Links", subLinks);
                        }
                        ptJson->insert("Response", ptStatus);
                        delete ptStatus;
                      }
                      else
                      {
                        strError = "Please provide a valid Function:  ping, status.";
                      }
                    }
                    else
                    {
                      strError = "Please provide the Function.";
                    }
                    ptJson->insert("Status", ((bProcessed)?"okay":"error"));
                    if (!strError.empty())
                    {
                      ptJson->insert("Error", strError);
                    }
                    hub(ptJson, false);
                  }
                  delete ptJson;
                }
              }
              else
              {
                bExit = true;
                if (nReturn < 0 && errno != 104)
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Utility::fdRead(" << errno << ") [stdin," << fds[0].fd << "]:  " << strerror(errno);
                  log(ssMessage.str());
                }
              }
            }
            // }}}
            // {{{ stdout
            if (fds[1].revents & POLLOUT)
            {
              if (!m_pUtility->fdWrite(fds[1].fd, m_strBuffers[1], nReturn))
              {
                bExit = true;
                if (nReturn < 0)
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Utility::fdWrite(" << errno << ") [stdout," << fds[1].fd << "]:  " << strerror(errno);
                  log(ssMessage.str());
                }
              }
            }
            // }}}
            // {{{ accept
            if (fds[2].revents & POLLIN)
            {
              // {{{ prep work
              int fdLink;
              sockaddr_in cli_addr;
              socklen_t clilen = sizeof(cli_addr);
              // }}}
              if ((fdLink = accept(fdSocket, (sockaddr *)&cli_addr, &clilen)) >= 0)
              {
                // {{{ prep work
                sockaddr_storage addr;
                socklen_t len = sizeof(addr);
                if ((lArg = fcntl(fdLink, F_GETFL, NULL)) >= 0)
                {
                  lArg |= O_NONBLOCK;
                  fcntl(fdLink, F_SETFL, lArg);
                }
                // }}}
                if (getpeername(fdLink, (sockaddr *)&addr, &len) == 0)
                {
                  // {{{ prep work
                  bool bSslAcceptRetry;
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
                  // }}}
                  if ((ssl = m_pUtility->sslAccept(ctxS, fdLink, bSslAcceptRetry, strError)) != NULL)
                  {
                    size_t unReturn;
                    Json *ptWrite = new Json;
                    radialLink *ptLink = new radialLink;
                    ptLink->bAuthenticated = false;
                    ptLink->bSslAcceptRetry = bSslAcceptRetry;
                    ptLink->bSslConnectRetry = false;
                    ptLink->fdConnecting = -1;
                    ptLink->fdSocket = fdLink;
                    ptLink->rp = NULL;
                    ptLink->ssl = ssl;
                    ptLink->unUnique = unUnique++;
                    ptWrite->insert("_function", "handshake");
                    if (!links.empty())
                    {
                      ptWrite->m["Links"] = new Json;
                      for (auto &link : links)
                      {
                        if (!link->strNode.empty() && !link->strServer.empty() && !link->strPort.empty())
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
                    ptWrite->m["You"] = new Json;
                    ptWrite->m["You"]->insert("Server", szIP);
                    ptWrite->m["Me"] = new Json;
                    ptWrite->m["Me"]->insert("Node", m_ptLink->m["Node"]->v);
                    ptWrite->m["Me"]->insert("Port", m_ptLink->m["Port"]->v, 'n');
                    if (!m_strServer.empty())
                    {
                      ptWrite->m["Me"]->insert("Server", m_strServer);
                    }
                    ptLink->responses.push_back(ptWrite->json(strJson));
                    delete ptWrite;
                    if (!m_strMaster.empty())
                    {
                      ptWrite = new Json;
                      ptWrite->insert("_function", "master");
                      ptWrite->insert("Node", m_strMaster);
                      ptLink->responses.push_back(ptWrite->json(strJson));
                      delete ptWrite;
                    }
                    if ((unReturn = add(links, ptLink)) > 0)
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Link::add() [sslAccept," << fdLink << "]:  " << ((unReturn == 1)?"Added":"Updated") << " link.";
                      log(ssMessage.str());
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Link::add() error [sslAccept," << fdLink << "]:  Failed to add link.";
                      log(ssMessage.str());
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
                    log(ssMessage.str());
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
            // {{{ links
            for (size_t i = 3; i < unIndex; i++)
            {
              // {{{ prep work
              list<radialLink *>::iterator linkIter = links.end();
              for (auto j = links.begin(); linkIter == links.end() && j != links.end(); j++)
              {
                if ((*j)->fdSocket == fds[i].fd)
                {
                  linkIter = j;
                }
              }
              // }}}
              if (linkIter != links.end())
              {
                // {{{ read
                if (fds[i].revents & POLLIN)
                {
                  if (m_pUtility->sslRead((*linkIter)->ssl, (*linkIter)->strBuffers[0], nReturn))
                  {
                    while ((unPosition = (*linkIter)->strBuffers[0].find("\n")) != string::npos)
                    {
                      Json *ptJson = new Json((*linkIter)->strBuffers[0].substr(0, unPosition));
                      (*linkIter)->strBuffers[0].erase(0, (unPosition + 1));
                      // {{{ _function
                      if (ptJson->m.find("_function") != ptJson->m.end() && !ptJson->m["_function"]->v.empty())
                      {
                        bool bStorageTransmit = false;
                        // {{{ handshake
                        if (ptJson->m["_function"]->v == "handshake")
                        {
                          // {{{ Links
                          if (ptJson->m.find("Links") != ptJson->m.end())
                          {
                            for (auto &ptLink : ptJson->m["Links"]->l)
                            {
                              if (ptLink->m.find("Node") != ptLink->m.end() && !ptLink->m["Node"]->v.empty() && ptLink->m.find("Server") != ptLink->m.end() && !ptLink->m["Server"]->v.empty() && ptLink->m.find("Port") != ptLink->m.end() && !ptLink->m["Port"]->v.empty())
                              {
                                bool bFound = false;
                                for (auto j = links.begin(); !bFound && j != links.end(); j++)
                                {
                                  if (ptLink->m["Node"]->v == (*j)->strNode)
                                  {
                                    bFound = true;
                                  }
                                }
                                if (!bFound && ptLink->m["Node"]->v != m_ptLink->m["Node"]->v)
                                {
                                  if (ptLink->m.find("Server") != ptLink->m.end() && !ptLink->m["Server"]->v.empty())
                                  {
                                    if (ptLink->m.find("Port") != ptLink->m.end() && !ptLink->m["Port"]->v.empty())
                                    {
                                      size_t unReturn;
                                      radialLink *ptSubLink = new radialLink;
                                      ptSubLink->bAuthenticated = true;
                                      ptSubLink->bSslAcceptRetry = false;
                                      ptSubLink->bSslConnectRetry = false;
                                      ptSubLink->strNode = ptLink->m["Node"]->v;
                                      ptSubLink->strServer = ptLink->m["Server"]->v;
                                      ptSubLink->strPort = ptLink->m["Port"]->v;
                                      ptSubLink->fdConnecting = -1;
                                      ptSubLink->fdSocket = -1;
                                      ptSubLink->rp = NULL;
                                      ptSubLink->ssl = NULL;
                                      ptSubLink->unUnique = unUnique++;
                                      if ((unReturn = add(links, ptSubLink)) > 0)
                                      {
                                        ssMessage.str("");
                                        ssMessage << strPrefix << "->Link::add() [" << ptJson->m["_function"]->v << "," << ptLink->m["Node"]->v << "]:  " << ((unReturn == 1)?"Added":"Updated") << " link.";
                                        log(ssMessage.str());
                                      }
                                      else
                                      {
                                        ssMessage.str("");
                                        ssMessage << strPrefix << "->Link::add() error [" << ptJson->m["_function"]->v << "," << ptLink->m["Node"]->v << "]:  Failed to add link.";
                                        log(ssMessage.str());
                                      }
                                      delete ptSubLink;
                                    }
                                  }
                                }
                              }
                            }
                          }
                          // }}}
                          // {{{ Me
                          if (ptJson->m.find("Me") != ptJson->m.end())
                          {
                            if (ptJson->m["Me"]->m.find("Node") != ptJson->m["Me"]->m.end() && !ptJson->m["Me"]->m["Node"]->v.empty())
                            {
                              (*linkIter)->strNode = ptJson->m["Me"]->m["Node"]->v;
                            }
                            if (ptJson->m["Me"]->m.find("Server") != ptJson->m["Me"]->m.end() && !ptJson->m["Me"]->m["Server"]->v.empty())
                            {
                              (*linkIter)->strServer = ptJson->m["Me"]->m["Server"]->v;
                            }
                            if (ptJson->m["Me"]->m.find("Port") != ptJson->m["Me"]->m.end() && !ptJson->m["Me"]->m["Port"]->v.empty())
                            {
                              (*linkIter)->strPort = ptJson->m["Me"]->m["Port"]->v;
                            }
                          }
                          // }}}
                          // {{{ Password
                          if (ptJson->m.find("Password") != ptJson->m.end())
                          {
                            if (ptJson->m["Password"]->v == m_strPassword)
                            {
                              (*linkIter)->bAuthenticated = true;
                              ssMessage.str("");
                              ssMessage << strPrefix << " [" << ptJson->m["_function"]->v << "," << (*linkIter)->strNode << "]:  Authenticated link.";
                              log(ssMessage.str());
                              if (m_unLink == RADIAL_LINK_MASTER)
                              {
                                bStorageTransmit = true;
                              }
                            }
                          }
                          // }}}
                          // {{{ You
                          if (ptJson->m.find("You") != ptJson->m.end() && ptJson->m["You"]->m.find("Server") != ptJson->m["You"]->m.end() && !ptJson->m["You"]->m["Server"]->v.empty() && m_strServer.empty())
                          {
                            m_strServer = ptJson->m["You"]->m["Server"]->v;
                          }
                          // }}}
                        }
                        // }}}
                        // {{{ master
                        else if (ptJson->m["_function"]->v == "master" && ptJson->m.find("Node") != ptJson->m.end() && !ptJson->m["Node"]->v.empty() && ptJson->m["Node"]->v != m_strMaster)
                        {
                          if (m_strMaster == m_ptLink->m["Node"]->v || ptJson->m["Node"]->v == m_ptLink->m["Node"]->v)
                          {
                            m_bUpdate = true;
                          }
                          m_strMaster = ptJson->m["Node"]->v;
                        }
                        // }}}
                        // {{{ invalid
                        else
                        {
                          ptJson->insert("Status", "error");
                          ptJson->insert("Error", "Please provide a valid _function:  handshake, master.");
                          ptJson->json(strJson);
                          (*linkIter)->responses.push_back(strJson);
                        }
                        // }}}
                        if (bStorageTransmit)
                        {
                          stringstream ssUnique;
                          Json *ptStorage = new Json;
                          ptStorage->insert("_function", "storageTransmit");
                          ptStorage->insert("_source", m_strName);
                          ptStorage->insert("_target", "storage");
                          ssUnique << (*linkIter)->fdSocket << " " << (*linkIter)->unUnique;
                          ptJson->insert("_unique", ssUnique.str());
                          ptStorage->insert("Function", "retrieve");
                          hub(ptStorage, false);
                          delete ptStorage;
                        }
                      }
                      // }}}
                      // {{{ Interface
                      else if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
                      {
                        if ((*linkIter)->bAuthenticated)
                        {
                          stringstream ssUnique;
                          Json *ptInterfaces = new Json;
                          ptJson->insert("_source", m_strName);
                          ptJson->insert("_target", ptJson->m["Interface"]->v);
                          ssUnique << fds[i].fd << " " << (*linkIter)->unUnique;
                          ptJson->insert("_unique", ssUnique.str());
                          ptInterfaces->insert("Function", "list");
                          ptInterfaces->m["_request"] = new Json(ptJson);
                          ptInterfaces->insert("_source", m_strName);
                          hub(ptInterfaces, false);
                          delete ptInterfaces;
                        }
                        else
                        {
                          ptJson->insert("Status", "error");
                          ptJson->insert("Error", "Failed authentication.");
                          ptJson->json(strJson);
                          (*linkIter)->responses.push_back(strJson);
                        }
                      }
                      // }}}
                      // {{{ invalid
                      else
                      {
                        ptJson->insert("Status", "error");
                        ptJson->insert("Error", "Please provide the _function or Interface.");
                        ptJson->json(strJson);
                        (*linkIter)->responses.push_back(strJson);
                      }
                      // }}}
                      delete ptJson;
                    }
                  }
                  else
                  {
                    removals.push_back((*linkIter)->fdSocket);
                    if (nReturn < 0)
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Utility::sslRead(" << SSL_get_error((*linkIter)->ssl, nReturn) << ") error [" << (*linkIter)->strNode << "]:  " << m_pUtility->sslstrerror((*linkIter)->ssl, nReturn);
                      log(ssMessage.str());
                    }
                  }
                }
                // }}}
                // {{{ write
                if (fds[i].revents & POLLOUT)
                {
                  if (!m_pUtility->sslWrite((*linkIter)->ssl, (*linkIter)->strBuffers[1], nReturn))
                  {
                    removals.push_back((*linkIter)->fdSocket);
                    if (nReturn < 0)
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Utility::sslWrite(" << SSL_get_error((*linkIter)->ssl, nReturn) << ") error [" << (*linkIter)->strNode << "]:  " << m_pUtility->sslstrerror((*linkIter)->ssl, nReturn);
                      log(ssMessage.str());
                    }
                  }
                }
                // }}}
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
          // {{{ post work
          delete[] fds;
          time(&CTime);
          // {{{ removals
          if (!links.empty())
          {
            list<string> nodes;
            for (auto &link : links)
            {
              if (!link->strNode.empty())
              {
                nodes.push_back(link->strNode);
              }
            }
            nodes.sort();
            nodes.unique();
            for (auto &node : nodes)
            {
              list<list<radialLink *>::iterator> duplicates;
              for (auto linkIter : links)
              {
                if ((*linkIter)->strNode == node && (*linkIter)->fdSocket != -1 && (*linkIter)->bAuthenticated)
                {
                  duplicates.push_back(linkIter);
                }
              }
              if (duplicates.size() > 1)
              {
                ssMessage.str("");
                if (m_ptLink->m.find("Node") != m_ptLink->m.end() && m_ptLink->m["Node"]->v < (*duplicates.front())->strNode)
                {
                  duplicates.pop_front();
                  ssMessage << strPrefix << " [removals," << (*duplicates.front())->strNode << "," << (*duplicates.front())->fdSocket << "]:  Saved front link prior to removal of duplicates.";
                }
                else
                {
                  duplicates.pop_back();
                  ssMessage << strPrefix << " [removals," << (*duplicates.front())->strNode << "," << (*duplicates.front())->fdSocket << "]:  Saved back link prior to removal of duplicates.";
                }
                log(ssMessage.str());
                for (auto &duplicate : duplicates)
                {
                  removals.push_back((*duplicate)->fdSocket);
                }
              }
            }
          }
          if (!removals.empty())
          {
            removals.sort();
            removals.unique();
            for (auto &i : removals)
            {
              list<radialLink *>::iterator removeIter = links.end();
              for (auto j = links.begin(); removeIter == links.end() && j != links.end(); j++)
              {
                if ((*j)->fdSocket == i)
                {
                  removeIter = j;
                }
              }
              if (removeIter != links.end())
              {
                ssMessage.str("");
                ssMessage << strPrefix << " [removals," << (*removeIter)->strNode << "," << (*removeIter)->fdSocket << "]:  Removed link.";
                log(ssMessage.str());
                if ((*removeIter)->ssl != NULL)
                {
                  SSL_shutdown((*removeIter)->ssl);
                  SSL_free((*removeIter)->ssl);
                }
                if ((*removeIter)->fdSocket != -1)
                {
                  close((*removeIter)->fdSocket);
                }
                if (!m_strMaster.empty() && (*removeIter)->strNode == m_strMaster)
                {
                  m_strMaster.clear();
                }
                delete (*removeIter);
                links.erase(removeIter);
              }
            }
            removals.clear();
          }
          // }}}
          // {{{ broadcast master
          if ((CTime - CBroadcast) > unBroadcastSleep)
          {
            unsigned int unSeed = CTime;
            srand(unSeed);
            unBroadcastSleep = (rand_r(&unSeed) % 5) + 1;
            if (m_strMaster.empty())
            {
              m_bUpdate = true;
              m_strMaster = m_ptLink->m["Node"]->v;
            }
            if (!m_strMaster.empty())
            {
              Json *ptWrite = new Json;
              ptWrite->insert("_function", "master");
              ptWrite->insert("Node", m_strMaster);
              ptWrite->json(strJson);
              delete ptWrite;
              for (auto &link : links)
              {
                link->responses.push_back(strJson);
              }
            }
            CBroadcast = CTime;
          }
          // }}}
          // {{{ bootstrap links
          if (m_ptLink->m.find("Links") != m_ptLink->m.end())
          {
            if ((CTime - CBootstrap) > unBootstrapSleep)
            {
              bool bReady = true;
              unsigned int unSeed = CTime;
              srand(unSeed);
              unBootstrapSleep = (rand_r(&unSeed) % 30) + 1;
              for (auto i = links.begin(); bReady && i != links.end(); i++)
              {
                if (!(*i)->bAuthenticated || (*i)->bSslAcceptRetry || (*i)->bSslConnectRetry || (*i)->fdSocket == -1 || (*i)->strNode.empty() || (*i)->strPort.empty() || (*i)->strServer.empty())
                {
                  bReady = false;
                }
              }
              if (bReady)
              {
                if (ptBoot->l.empty())
                {
                  for (auto &ptLink : m_ptLink->m["Links"]->l)
                  {
                    if (ptLink->m.find("Server") != ptLink->m.end() && !ptLink->m["Server"]->v.empty() && ptLink->m.find("Port") != ptLink->m.end() && !ptLink->m["Port"]->v.empty())
                    {
                      bool bFound = false;
                      for (auto i = links.begin(); !bFound && i != links.end(); i++)
                      {
                        if ((*i)->strServer == ptLink->m["Server"]->v && (*i)->strPort == ptLink->m["Port"]->v)
                        {
                          bFound = true;
                        }
                      }
                      if (!bFound)
                      {
                        Json *ptSubLink = new Json;
                        ptSubLink->insert("Server", ptLink->m["Server"]->v);
                        ptSubLink->insert("Port", ptLink->m["Port"]->v, 'n');
                        ptBoot->push_back(ptSubLink);
                        delete ptSubLink;
                      }
                    }
                  }
                }
              }
              CBootstrap = CTime;
            }
            while (!ptBoot->l.empty())
            {
              size_t unReturn;
              radialLink *ptLink = new radialLink;
              ptLink->bAuthenticated = true;
              ptLink->bSslAcceptRetry = false;
              ptLink->bSslConnectRetry = false;
              ptLink->strServer = ptBoot->l.front()->m["Server"]->v;
              ptLink->strPort = ptBoot->l.front()->m["Port"]->v;
              ptLink->fdConnecting = -1;
              ptLink->fdSocket = -1;
              ptLink->rp = NULL;
              ptLink->ssl = NULL;
              ptLink->unUnique = unUnique++;
              if ((unReturn = add(links, ptLink)) > 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Link::add() [bootstrap," << ptLink->strServer << "]:  " << ((unReturn == 1)?"Added":"Update") << " link.";
                log(ssMessage.str());
              }
              else
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Link::add() error [bootstrap," << ptLink->strServer << "]:  Failed to add link.";
                log(ssMessage.str());
              }
              delete ptLink;
              delete ptBoot->l.front();
              ptBoot->l.pop_front();
            }
          }
          // }}}
          // {{{ update
          if (m_bUpdate)
          {
            m_bUpdate = false;
            unLink = ((m_strMaster == m_ptLink->m["Node"]->v)?RADIAL_LINK_MASTER:RADIAL_LINK_SLAVE);
          }
          if (m_unLink != unLink)
          {
            m_unLink = ((m_strMaster == m_ptLink->m["Node"]->v)?RADIAL_LINK_MASTER:RADIAL_LINK_SLAVE);
          }
          // }}}
          monitor(strPrefix);
          if (shutdown())
          {
            bExit = true;
          }
          // }}}
        }
        // {{{ post work
        for (auto &link : links)
        {
          if (link->ssl != NULL)
          {
            SSL_shutdown(link->ssl);
            SSL_free(link->ssl);
          }
          if (link->fdSocket != -1)
          {
            close(link->fdSocket);
          }
          delete link;
        }
        links.clear();
        delete ptBoot;
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
    SSL_CTX_free(ctxS);
    // }}}
  }
  else if (ctxS == NULL)
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->Utility::sslInitServer() error:  " << strError;
    notify(ssMessage.str());
  }
  else
  {
    SSL_CTX_free(ctxS);
    ssMessage.str("");
    ssMessage << strPrefix << "->Utility::sslInitClient() error:  " << strError;
    notify(ssMessage.str());
  }
  // {{{ post work
  m_pUtility->sslDeinit();
  setShutdown();
  // }}}
}
// }}}
}
}
