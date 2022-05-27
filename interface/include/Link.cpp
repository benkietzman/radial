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
  m_pWarden->vaultRetrieve({"link", "Password"}, m_strPassword, strError);
  m_unLink = RADIAL_LINK_UNKNOWN;
}
// }}}
// {{{ ~Link()
Link::~Link()
{
  delete m_ptLink;
}
// }}}
// {{{ add()
size_t Link::add(radial_link *ptLink)
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
      radial_link *ptAdd = new radial_link;
      ptAdd->bAuthenticated = ptLink->bAuthenticated;
      ptAdd->fdSocket = ptLink->fdSocket;
      ptAdd->ssl = ptLink->ssl;
      ptAdd->strBuffers[0] = ptLink->strBuffers[0];
      ptAdd->strBuffers[1] = ptLink->strBuffers[1];
      ptAdd->strNode = ptLink->strNode;
      ptAdd->strPort = ptLink->strPort;
      ptAdd->strServer = ptLink->strServer;
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
  string strError, strJson;
  stringstream ssMessage;

  strPrefix += "->Link::callback()";
  if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
  {
    list<string> removals;
    Json *ptSubJson = new Json(ptJson);
    bResult = true;
    for (auto &i : ptSubJson->m)
    {
      if (!i.first.empty() && i.first[0] == '_')
      {
        removals.push_back(i.first);
      }
    }
    while (!removals.empty())
    {
      delete ptSubJson->m[removals.front()];
      ptSubJson->m.erase(removals.front());
      removals.pop_front();
    }
    ptSubJson->json(strJson);
    strJson += "\n";
    m_mutex.lock();
    for (auto &link : m_links)
    {
      if (link->bAuthenticated)
      {
        link->strBuffers[1].append(strJson);
      }
    }
    m_mutex.unlock();
  }
  else if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
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
void Link::request(string strPrefix, const int fdSocket, Json *ptJson)
{
  string strError, strJson;
  stringstream ssMessage;

  strPrefix += "->Link::request()";
  // {{{ _function
  if (ptJson->m.find("_function") != ptJson->m.end() && !ptJson->m["_function"]->v.empty())
  {
    m_mutex.lock();
    for (auto &link : m_links)
    {
      if (link->fdSocket == fdSocket)
      {
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
                for (auto &subLink : m_links)
                {
                  if (ptLink->m["Node"]->v == subLink->strNode)
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
                      size_t unResult;
                      radial_link *ptSubLink = new radial_link;
                      ptSubLink->bAuthenticated = true;
                      ptSubLink->strNode = ptLink->m["Node"]->v;
                      ptSubLink->strServer = ptLink->m["Server"]->v;
                      ptSubLink->strPort = ptLink->m["Port"]->v;
                      ptSubLink->fdSocket = -1;
                      ptSubLink->ssl = NULL;
                      if ((unResult = add(ptSubLink)) <= 0)
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << "->Link::add() error [" << ptJson->m["_funtion"]->v << "," << ptLink->m["Node"]->v << "]:  Failed to add link.";
                        notify(ssMessage.str());
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
              link->strNode = ptJson->m["Me"]->m["Node"]->v;
            }
            if (ptJson->m["Me"]->m.find("Server") != ptJson->m["Me"]->m.end() && !ptJson->m["Me"]->m["Server"]->v.empty())
            {
              link->strServer = ptJson->m["Me"]->m["Server"]->v;
            }
            if (ptJson->m["Me"]->m.find("Port") != ptJson->m["Me"]->m.end() && !ptJson->m["Me"]->m["Port"]->v.empty())
            {
              link->strPort = ptJson->m["Me"]->m["Port"]->v;
            }
          }
          // }}}
          // {{{ Password
          if (ptJson->m.find("Password") != ptJson->m.end())
          {
            if (ptJson->m["Password"]->v == m_strPassword)
            {
              link->bAuthenticated = true;
              if (m_unLink == RADIAL_LINK_MASTER)
              {
                storageTransmit(strPrefix, link);
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
      }
    }
    m_mutex.unlock();
  }
  // }}}
  // {{{ Interface
  else if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
  {
    bool bAuthenticated = false;
    m_mutex.lock();
    for (auto &link : m_links)
    {
      if (link->fdSocket == fdSocket && link->bAuthenticated)
      {
        bAuthenticated = true;
      }
    }
    m_mutex.unlock();
    if (bAuthenticated)
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
    }
    else
    {
      ptJson->insert("Status", "error");
      ptJson->insert("Error", "Failed authentication.");
    }
    ptJson->json(strJson);
    strJson += "\n";
    m_mutex.lock();
    for (auto &link : m_links)
    {
      if (link->fdSocket == fdSocket)
      {
        link->strBuffers[1].append(strJson);
      }
    }
    m_mutex.unlock();
  }
  // }}}
  delete ptJson;
}
// }}}
// {{{ socket()
void Link::socket(string strPrefix)
{
  // {{{ prep work
  SSL_CTX *ctxC = NULL, *ctxS = NULL;
  string strError, strJson;
  stringstream ssMessage;
  strPrefix += "->Link::socket()";
  setlocale(LC_ALL, "");
  // }}}
  if ((ctxS = m_pUtility->sslInitServer(m_strData + "/server.crt", m_strData + "/server.key", strError)) != NULL)
  {
    // {{{ prep work
    SSL_CTX_set_mode(ctxS, SSL_MODE_AUTO_RETRY);
    // }}}
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
          size_t unIndex, unLink = m_unLink, unPosition;
          time_t CBroadcastTime[2], unBroadcastSleep = 5;
          ssMessage.str("");
          ssMessage << strPrefix << "->listen():  Listening to incoming socket.";
          log(ssMessage.str());
          time(&CBroadcastTime[0]);
          // }}}
          while (!bExit)
          {
            // {{{ prep work
            m_mutex.lock();
            fds = new pollfd[m_links.size() + 1];
            m_mutex.unlock();
            unIndex = 0;
            fds[unIndex].fd = fdSocket;
            fds[unIndex].events = POLLIN;
            unIndex++;
            m_mutex.lock();
            for (auto &link : m_links)
            {
              fds[unIndex].events = POLLIN;
              if (link->fdSocket == -1)
              {
                memset(&hints, 0, sizeof(addrinfo));
                hints.ai_family = AF_UNSPEC;
                hints.ai_socktype = SOCK_STREAM;
                if ((nReturn = getaddrinfo(link->strServer.c_str(), link->strPort.c_str(), &hints, &result)) == 0)
                {
                  bool bConnected[3] = {false, false, false};
                  int fdLink = -1;
                  addrinfo *rp;
                  SSL *ssl = NULL;
                  for (rp = result; !bConnected[2] && rp != NULL; rp = rp->ai_next)
                  {
                    bConnected[0] = bConnected[1] = false;
                    if ((fdLink = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
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
                    ptWrite->m["You"] = new Json;
                    ptWrite->m["You"]->insert("Server", link->strServer);
                    ptWrite->m["Me"] = new Json;
                    ptWrite->m["Me"]->insert("Node", m_ptLink->m["Node"]->v);
                    ptWrite->m["Me"]->insert("Port", m_ptLink->m["Port"]->v, 'n');
                    if (!m_strServer.empty())
                    {
                      ptWrite->m["Me"]->insert("Server", m_strServer);
                    }
                    link->strBuffers[1].append(ptWrite->json(strJson) + "\n");
                    delete ptWrite;
                    if (!m_strMaster.empty())
                    {
                      ptWrite = new Json;
                      ptWrite->insert("_function", "master");
                      ptWrite->insert("Node", m_strMaster);
                      delete ptWrite;
                    }
                    if (m_unLink == RADIAL_LINK_MASTER)
                    {
                      storageTransmit(strPrefix, link);
                    }
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
              fds[unIndex].fd = link->fdSocket;
              if (!link->strBuffers[1].empty())
              {
                fds[unIndex].events |= POLLOUT;
              }
              unIndex++;
            }
            m_mutex.unlock();
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
                if ((fdLink = accept(fdSocket, (sockaddr *)&cli_addr, &clilen)) >= 0)
                {
                  // {{{ prep work
                  sockaddr_storage addr;
                  socklen_t len = sizeof(addr);
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
                    ERR_clear_error();
                    // }}}
                    if ((ssl = m_pUtility->sslAccept(ctxS, fdLink, strError)) != NULL)
                    {
                      size_t unResult;
                      Json *ptWrite = new Json;
                      radial_link *ptLink = new radial_link;
                      ptLink->bAuthenticated = false;
                      ptLink->fdSocket = fdLink;
                      ptLink->ssl = ssl;
                      ptWrite->insert("_function", "handshake");
                      m_mutex.lock();
                      if (!m_links.empty())
                      {
                        ptWrite->m["Links"] = new Json;
                        for (auto &link : m_links)
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
                      m_mutex.unlock();
                      ptWrite->m["You"] = new Json;
                      ptWrite->m["You"]->insert("Server", szIP);
                      ptWrite->m["Me"] = new Json;
                      ptWrite->m["Me"]->insert("Node", m_ptLink->m["Node"]->v);
                      ptWrite->m["Me"]->insert("Port", m_ptLink->m["Port"]->v, 'n');
                      if (!m_strServer.empty())
                      {
                        ptWrite->m["Me"]->insert("Server", m_strServer);
                      }
                      ptLink->strBuffers[1].append(ptWrite->json(strJson) + "\n");
                      delete ptWrite;
                      if (!m_strMaster.empty())
                      {
                        ptWrite = new Json;
                        ptWrite->insert("_function", "master");
                        ptWrite->insert("Node", m_strMaster);
                        ptLink->strBuffers[1].append(ptWrite->json(strJson) + "\n");
                        delete ptWrite;
                      }
                      m_mutex.lock();
                      unResult = add(ptLink);
                      m_mutex.unlock();
                      if (unResult <= 0)
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << "->Link::add() error:  Failed to add link.";
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
              m_mutex.lock();
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
                          thread threadLinkRequest(&Link::request, this, strPrefix, fds[i].fd, ptJson);
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
              m_mutex.unlock();
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
            m_mutex.lock();
            if (!m_links.empty())
            {
              list <string> nodes;
              for (auto &link : m_links)
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
                list<list<radial_link *>::iterator> duplicates;
                for (auto linkIter = m_links.begin(); linkIter != m_links.end(); linkIter++)
                {
                  if ((*linkIter)->strNode == node && (*linkIter)->fdSocket != -1)
                  {
                    duplicates.push_back(linkIter);
                  }
                }
                if (duplicates.size() > 1)
                {
                  duplicates.pop_back();
                  for (auto &duplicate : duplicates)
                  {
                    removals.push_back((*duplicate)->fdSocket);
                  }
                }
              }
            }
            m_mutex.unlock();
            if (!removals.empty())
            {
              removals.sort();
              removals.unique();
              for (auto &i : removals)
              {
                m_mutex.lock();
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
                  if ((*removeIter)->ssl != NULL)
                  {
                    SSL_shutdown((*removeIter)->ssl);
                    SSL_free((*removeIter)->ssl);
                  }
                  if ((*removeIter)->fdSocket != -1)
                  {
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
                  delete (*removeIter);
                  m_links.erase(removeIter);
                }
                m_mutex.unlock();
              }
              removals.clear();
            }
            // }}}
            // {{{ broadcast master
            time(&CBroadcastTime[1]);
            if ((CBroadcastTime[1] - CBroadcastTime[0]) > unBroadcastSleep)
            {
              unsigned int unSeed = CBroadcastTime[1];
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
                strJson += "\n";
                m_mutex.lock();
                for (auto &link : m_links)
                {
                  link->strBuffers[1].append(strJson);
                }
                m_mutex.unlock();
              }
              CBroadcastTime[0] = CBroadcastTime[1];
            }
            // }}}
            // {{{ bootstrap links
            if (m_ptLink->m.find("Links") != m_ptLink->m.end())
            {
              bool bReady = true;
              m_mutex.lock();
              for (auto &link : m_links)
              {
                if (link->strNode.empty() || link->strServer.empty() || link->strPort.empty() || !link->bAuthenticated)
                {
                  bReady = false;
                }
              }
              m_mutex.unlock();
              if (bReady)
              {
                Json *ptBoot = new Json;
                for (auto &ptLink : m_ptLink->m["Links"]->l)
                {
                  if (ptLink->m.find("Server") != ptLink->m.end() && !ptLink->m["Server"]->v.empty() && ptLink->m.find("Port") != ptLink->m.end() && !ptLink->m["Port"]->v.empty())
                  {
                    bool bFound = false;
                    m_mutex.lock();
                    for (auto &link : m_links)
                    {
                      if (link->strServer == ptLink->m["Server"]->v && link->strPort == ptLink->m["Port"]->v)
                      {
                        bFound = true;
                      }
                    }
                    m_mutex.unlock();
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
                for (auto &ptBootLink : ptBoot->l)
                {
                  size_t unResult;
                  radial_link *ptLink = new radial_link;
                  ptLink->bAuthenticated = true;
                  ptLink->strServer = ptBootLink->m["Server"]->v;
                  ptLink->strPort = ptBootLink->m["Port"]->v;
                  ptLink->fdSocket = -1;
                  ptLink->ssl = NULL;
                  m_mutex.lock();
                  unResult = add(ptLink);
                  m_mutex.unlock();
                  if (unResult <= 0)
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Link::add() error [" << ptBootLink->m["Server"]->v << "]:  Failed to add link.";
                    notify(ssMessage.str());
                  }
                  delete ptLink;
                }
                delete ptBoot;
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
              ssMessage.str("");
              ssMessage << strPrefix << ":  Switched to " << ((m_unLink == RADIAL_LINK_MASTER)?"master":"slave") << " mode.";
              log(ssMessage.str());
            }
            // }}}
            if (shutdown())
            {
              bExit = true;
            }
            // }}}
          }
          // {{{ post work
          m_mutex.lock();
          for (auto &link : m_links)
          {
            if (link->ssl != NULL)
            {
              SSL_shutdown(link->ssl);
              SSL_free(link->ssl);
            }
            if (link->fdSocket != -1)
            {
              close(link->fdSocket);
              ssMessage.str("");
              ssMessage << strPrefix << "->close() [" << link->strNode << "]:  Closed link socket.";
              log(ssMessage.str());
            }
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
// {{{ storageTransmit()
void Link::storageTransmit(string strPrefix, radial_link *ptLink)
{
  // {{{ prep work
  string strError;
  stringstream ssMessage;
  strPrefix += "->Link::storageTransmit()";
  Json *ptStorage = new Json;
  // }}}
  if (storageRetrieve(ptStorage, strError))
  {
    stringstream ssWrite;
    Json *ptWrite = new Json;
    ptWrite->insert("Interface", "storage");
    ptWrite->insert("Function", "update");
    ptWrite->insert("Request", ptStorage);
    ssWrite << ptWrite << endl;
    delete ptWrite;
    ptLink->strBuffers[1].append(ssWrite.str());
  }
  // {{{ post work
  delete ptStorage;
  // }}}
}
// }}}
}
}
