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
Link::Link(string strPrefix, int argc, char **argv) : Interface(strPrefix, "link", argc, argv, NULL)
{
  ifstream inLink((m_strData + "/link.json").c_str());
  string strError;

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
      ptAdd->bRetry = ptLink->bRetry;
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
  if (unResult > 0)
  {
  }

  return unResult;
}
// }}}
// {{{ process()
void Link::process(string strPrefix)
{
  // {{{ prep work
  SSL_CTX *ctxC = NULL, *ctxS = NULL;
  string strError, strJson;
  stringstream ssMessage;
  strPrefix += "->Link::process()";
  m_pUtility->fdNonBlocking(0, strError);
  m_pUtility->fdNonBlocking(1, strError);
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
        size_t unIndex, unPosition, unThroughput = 0, unUnique = 0;
        string strLine;
        time_t CBootstrap, CThroughput, CTime, unBootstrapSleep = 0;
        Json *ptBoot = new Json;
        ssMessage.str("");
        ssMessage << strPrefix << "->listen():  Listening to incoming socket.";
        log(ssMessage.str());
        time(&CTime);
        CBootstrap = CThroughput = CTime;
        // }}}
        while (!bExit)
        {
          // {{{ prep work
          fds = new pollfd[links.size() + m_l.size() + 3];
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
            // {{{ SSL_accept
            if (link->bRetry)
            {
              if ((nReturn = SSL_accept(link->ssl)) > 0)
              {
                link->bRetry = false;
              }
              else
              {
                strError = m_pUtility->sslstrerror(link->ssl, nReturn, link->bRetry);
                if (!link->bRetry)
                {
                  link->bRetry = true;
                  removals.push_back(link->fdSocket);
                  ssMessage.str("");
                  ssMessage << strPrefix << "->SSL_accept() error:  " << strError;
                  log(ssMessage.str());
                }
              }
            }
            // }}}
            // {{{ fds
            if (!link->bRetry)
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
          // {{{ m_l
          for (auto &link : m_l)
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
                  removals.push_back(link->fdSocket);
                  ssMessage.str("");
                  ssMessage << strPrefix << "->getaddrinfo(" << nReturn << ") error [" << link->strNode << "]:  " << gai_strerror(nReturn);
                  log(ssMessage.str());
                }
              }
              // }}}
              // {{{ socket
              else if (link->fdConnecting == -1)
              {
                if ((link->fdConnecting = socket(link->rp->ai_family, link->rp->ai_socktype, link->rp->ai_protocol)) >= 0)
                {
                  m_pUtility->fdNonBlocking(link->fdConnecting, strError);
                }
                else
                {
                  link->rp = link->rp->ai_next;
                  if (link->rp == NULL)
                  {
                    freeaddrinfo(link->result);
                    removals.push_back(link->fdSocket);
                    ssMessage.str("");
                    ssMessage << strPrefix << "->socket(" << errno << ") error [" << link->strNode << "]:  " << strerror(errno);
                    log(ssMessage.str());
                  }
                }
              }
              // }}}
              // {{{ connect
              else if (connect(link->fdConnecting, link->rp->ai_addr, link->rp->ai_addrlen) == 0)
              {
                link->fdSocket = link->fdConnecting;
                link->fdConnecting = -1;
                if ((link->ssl = m_pUtility->sslConnect(ctxC, link->fdSocket, link->bRetry, strError)) != NULL)
                {
                  if (!link->bRetry)
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Utility::sslConnect() [" << link->strNode << "]:  Connected link.";
                    log(ssMessage.str());
                  }
                }
                else
                {
                  removals.push_back(link->fdSocket);
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Utility::sslConnect() error [" << link->strNode << "]:  " << strError;
                  log(ssMessage.str());
                }
              }
              // }}}
              // {{{ error
              else if (errno != EAGAIN && errno != EALREADY && errno != EINPROGRESS)
              {
                close(link->fdConnecting);
                link->fdConnecting = -1;
                link->rp = link->rp->ai_next;
                if (link->rp == NULL)
                {
                  freeaddrinfo(link->result);
                  removals.push_back(link->fdSocket);
                  ssMessage.str("");
                  ssMessage << strPrefix << "->connect(" << errno << ") error [" << link->strNode << "]:  " << strerror(errno);
                  log(ssMessage.str());
                }
              }
              // }}}
              // {{{ payload
              if (link->fdSocket != -1)
              {
                Json *ptWrite = new Json;
                freeaddrinfo(link->result);
                ptWrite->i("_f", "handshake");
                ptWrite->i("Password", m_strPassword);
                ptWrite->m["Links"] = new Json;
                for (auto &subLink : m_l)
                {
                  if (!subLink->strNode.empty() && !subLink->strServer.empty() && !subLink->strPort.empty())
                  {
                    Json *ptLink = new Json;
                    ptLink->i("Node", subLink->strNode);
                    ptLink->i("Server", subLink->strServer);
                    ptLink->i("Port", subLink->strPort, 'n');
                    ptWrite->m["Links"]->pb(ptLink);
                    delete ptLink;
                  }
                }
                ptWrite->m["You"] = new Json;
                ptWrite->m["You"]->i("Server", link->strServer);
                ptWrite->m["Me"] = new Json;
                ptWrite->m["Me"]->i("Node", m_ptLink->m["Node"]->v);
                ptWrite->m["Me"]->i("Server", m_ptLink->m["Server"]->v);
                ptWrite->m["Me"]->i("Port", m_ptLink->m["Port"]->v, 'n');
                link->responses.push_back(ptWrite->j(strJson));
                delete ptWrite;
                ptWrite = new Json;
                ptWrite->i("_f", "interfaces");
                ptWrite->m["Interfaces"] = new Json;
                for (auto &interface : m_i)
                {
                  stringstream ssPid;
                  ssPid << interface.second->nPid;
                  ptWrite->m["Interfaces"]->m[interface.first] = new Json;
                  ptWrite->m["Interfaces"]->m[interface.first]->i("AccessFunction", interface.second->strAccessFunction);
                  ptWrite->m["Interfaces"]->m[interface.first]->i("Command", interface.second->strCommand);
                  ptWrite->m["Interfaces"]->m[interface.first]->i("PID", ssPid.str(), 'n');
                  ptWrite->m["Interfaces"]->m[interface.first]->i("Respawn", ((interface.second->bRespawn)?"1":"0"), ((interface.second->bRespawn)?'1':'0'));
                  ptWrite->m["Interfaces"]->m[interface.first]->i("Restricted", ((interface.second->bRestricted)?"1":"0"), ((interface.second->bRestricted)?'1':'0'));
                }
                link->responses.push_back(ptWrite->j(strJson));
                delete ptWrite;
              }
              // }}}
            }
            // {{{ SSL_connect
            if (link->bRetry)
            {
              if ((nReturn = SSL_connect(link->ssl)) == 1)
              {
                link->bRetry = false;
                ssMessage.str("");
                ssMessage << strPrefix << "->SSL_connect() [" << link->strNode << "]:  Connected link.";
                log(ssMessage.str());
              }
              else
              {
                strError = m_pUtility->sslstrerror(link->ssl, nReturn, link->bRetry);
                if (!link->bRetry)
                {
                  link->bRetry = true;
                  removals.push_back(link->fdSocket);
                  ssMessage.str("");
                  ssMessage << strPrefix << "->SSL_connect() error [" << link->strNode << "]:  " << strError;
                  log(ssMessage.str());
                }
              }
            }
            // }}}
            // {{{ fds
            if (!link->bRetry)
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
          if (fds[0].fd != 0 || (fds[1].fd != -1 && fds[1].fd != 1))
          {
            bExit = true;
          }
          // }}}
          if (!bExit && (nReturn = poll(fds, unIndex, 2000)) > 0)
          {
            // {{{ stdin
            if (fds[0].revents & (POLLHUP | POLLIN))
            {
              if (m_pUtility->fdRead(fds[0].fd, m_strBuffers[0], nReturn))
              {
                while ((unPosition = m_strBuffers[0].find("\n")) != string::npos)
                {
                  string strPayload, strRoute;
                  stringstream ssData;
                  Json *ptJson, *ptRoute;
                  radialPacket p;
                  unThroughput++;
                  strLine = m_strBuffers[0].substr(0, unPosition);
                  m_strBuffers[0].erase(0, (unPosition + 1));
                  unpack(strLine, p);
                  ssData.str(strLine);
                  getline(ssData, strRoute, m_cDelimiter);
                  ptRoute = new Json(strRoute);
                  ptJson = new Json(p.p);
                  ptJson->merge(ptRoute, true, false);
                  delete ptRoute;
                  ptJson->j(strLine);
                  if (exist(ptJson, "_s") && ptJson->m["_s"]->v == m_strName && !empty(ptJson, "_u"))
                  {
                    int fdLink;
                    size_t unUnique;
                    string strValue;
                    stringstream ssUnique(ptJson->m["_u"]->v);
                    radialLink *ptLink = NULL;
                    ssUnique >> strValue >> fdLink >> unUnique;
                    for (auto i = links.begin(); ptLink == NULL && i != links.end(); i++)
                    {
                      if ((*i)->fdSocket == fdLink && (*i)->unUnique == unUnique)
                      {
                        ptLink = (*i);
                      }
                    }
                    if (ptLink == NULL)
                    {
                      for (auto i = m_l.begin(); ptLink == NULL && i != m_l.end(); i++)
                      {
                        if ((*i)->fdSocket == fdLink && (*i)->unUnique == unUnique)
                        {
                          ptLink = (*i);
                        }
                      }
                    }
                    if (ptLink != NULL)
                    {
                      Json *ptSubLink = NULL;
                      if (exist(ptJson, "_l"))
                      {
                        ptSubLink = ptJson->m["_l"];
                        ptJson->m.erase("_l");
                      }
                      keyRemovals(ptJson);
                      if (ptSubLink != NULL)
                      {
                        ptJson->m["_l"] = ptSubLink;
                      }
                      ptLink->responses.push_back(ptJson->j(strJson));
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << " error [stdin," << fdLink << "," << unUnique << "]:  Link no longer exists.";
                      log(ssMessage.str());
                    }
                  }
                  else if (exist(ptJson, "_s") && ptJson->m["_s"]->v == "hub")
                  {
                    if (!empty(ptJson, "Function"))
                    {
                      // {{{ interfaces
                      if (ptJson->m["Function"]->v == "interfaces")
                      {
                        interfaces(strPrefix, ptJson);
                        if (exist(ptJson, "Interfaces"))
                        {
                          Json *ptWrite = new Json;
                          ptWrite->i("_f", "interfaces");
                          ptWrite->m["Interfaces"] = new Json(ptJson->m["Interfaces"]);
                          ptWrite->j(strJson);
                          delete ptWrite;
                          for (auto &link : links)
                          {
                            link->responses.push_back(strJson);
                          }
                          for (auto &link : m_l)
                          {
                            link->responses.push_back(strJson);
                          }
                        }
                      }
                      // }}}
                      // {{{ shutdown
                      else if (ptJson->m["Function"]->v == "shutdown")
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " [stdin,hub," << ptJson->m["Function"]->v << "]:  Shutting down.";
                        log(ssMessage.str());
                        setShutdown();
                      }
                      // }}}
                      // {{{ invalid
                      else
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " error [stdin,hub," << ptJson->m["Function"]->v << "]:  Please provide a valid Function:  interfaces, shutdown.";
                        log(ssMessage.str());
                      }
                      // }}}
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << " error [stdin,hub]:  Please provide a Function.";
                      log(ssMessage.str());
                    }
                  }
                  else if (!empty(ptJson, "Interface") && ptJson->m["Interface"]->v != "link")
                  {
                    if (!empty(ptJson, "Node"))
                    {
                      list<radialLink *>::iterator linkIter = m_l.end();
                      for (auto i = m_l.begin(); linkIter == m_l.end() && i != m_l.end(); i++)
                      {
                        if ((*i)->strNode == ptJson->m["Node"]->v)
                        {
                          linkIter = i;
                        }
                      }
                      if (linkIter != m_l.end())
                      {
                        Json *ptLink = new Json;
                        delete ptJson->m["Node"];
                        ptJson->m.erase("Node");
                        for (auto &i : ptJson->m)
                        {
                          if (!i.first.empty() && i.first[0] == '_')
                          {
                            ptLink->i(i.first, i.second);
                          }
                        }
                        keyRemovals(ptJson);
                        ptJson->m["_l"] = ptLink;
                        (*linkIter)->responses.push_back(ptJson->j(strJson));
                      }
                      else
                      {
                        ptJson->i("Status", "error");
                        ptJson->i("Error", "Linked Node does not exist.");
                        ptJson->j(p.p);
                        hub(p, false);
                      }
                    }
                    else
                    {
                      for (auto &link : m_l)
                      {
                        if ((!exist(ptJson, "Node") || empty(ptJson, "Node") || link->strNode == ptJson->m["Node"]->v) && link->interfaces.find(ptJson->m["Interface"]->v) != link->interfaces.end())
                        {
                          link->responses.push_back(strLine);
                        }
                      }
                    }
                  }
                  else
                  {
                    bool bProcessed = false;
                    strError.clear();
                    if (!empty(ptJson, "|function"))
                    {
                      if (ptJson->m["|function"]->v == "status")
                      {
                        float fCpu = 0, fMem = 0;
                        pid_t nPid = getpid();
                        stringstream ssImage, ssPid, ssResident;
                        time_t CTime = 0;
                        unsigned long ulImage = 0, ulResident = 0;
                        bProcessed = true;
                        if (exist(ptJson, "Response"))
                        {
                          delete ptJson->m["Response"];
                        }
                        ptJson->m["Response"] = new Json;
                        m_pCentral->getProcessStatus(nPid, CTime, fCpu, fMem, ulImage, ulResident);
                        ptJson->m["Response"]->m["Memory"] = new Json;
                        ssImage << ulImage;
                        ptJson->m["Response"]->m["Memory"]->i("Image", ssImage.str(), 'n');
                        ssResident << ulResident;
                        ptJson->m["Response"]->m["Memory"]->i("Resident", ssResident.str(), 'n');
                        ssPid << nPid;
                        ptJson->m["Response"]->i("PID", ssPid.str(), 'n');
                      }
                      else
                      {
                        strError = "Please provide a valid |function.";
                      }
                    }
                    else if (!empty(ptJson, "Function"))
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
                        if (!empty(m_ptLink, "Node"))
                        {
                          ptStatus->i("Node", m_ptLink->m["Node"]->v);
                        }
                        for (auto &link : m_l)
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
                          ptStatus->i("Links", subLinks);
                        }
                        ptJson->i("Response", ptStatus);
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
                    ptJson->i("Status", ((bProcessed)?"okay":"error"));
                    if (!strError.empty())
                    {
                      ptJson->i("Error", strError);
                    }
                    ptJson->j(p.p);
                    hub(p, false);
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
                bool bRetry;
                SSL *ssl;
                m_pUtility->fdNonBlocking(fdLink, strError);
                // }}}
                if ((ssl = m_pUtility->sslAccept(ctxS, fdLink, bRetry, strError)) != NULL)
                {
                  size_t unReturn;
                  Json *ptWrite = new Json;
                  radialLink *ptLink = new radialLink;
                  ptLink->bAuthenticated = false;
                  ptLink->bRetry = bRetry;
                  ptLink->fdConnecting = -1;
                  ptLink->fdSocket = fdLink;
                  ptLink->rp = NULL;
                  ptLink->ssl = ssl;
                  ptLink->unUnique = unUnique++;
                  ptWrite->i("_f", "handshake");
                  if (!m_l.empty())
                  {
                    ptWrite->m["Links"] = new Json;
                    for (auto &link : m_l)
                    {
                      if (!link->strNode.empty() && !link->strServer.empty() && !link->strPort.empty())
                      {
                        Json *ptSubLink = new Json;
                        ptSubLink->i("Node", link->strNode);
                        ptSubLink->i("Server", link->strServer);
                        ptSubLink->i("Port", link->strPort);
                        ptWrite->m["Links"]->pb(ptSubLink);
                        delete ptSubLink;
                      }
                    }
                  }
                  ptWrite->m["Me"] = new Json;
                  ptWrite->m["Me"]->i("Node", m_ptLink->m["Node"]->v);
                  ptWrite->m["Me"]->i("Server", m_ptLink->m["Server"]->v);
                  ptWrite->m["Me"]->i("Port", m_ptLink->m["Port"]->v, 'n');
                  ptLink->responses.push_back(ptWrite->j(strJson));
                  delete ptWrite;
                  ptWrite = new Json;
                  ptWrite->i("_f", "interfaces");
                  ptWrite->m["Interfaces"] = new Json;
                  for (auto &interface : m_i)
                  {
                    stringstream ssPid;
                    ssPid << interface.second->nPid;
                    ptWrite->m["Interfaces"]->m[interface.first] = new Json;
                    ptWrite->m["Interfaces"]->m[interface.first]->i("AccessFunction", interface.second->strAccessFunction);
                    ptWrite->m["Interfaces"]->m[interface.first]->i("Command", interface.second->strCommand);
                    ptWrite->m["Interfaces"]->m[interface.first]->i("PID", ssPid.str(), 'n');
                    ptWrite->m["Interfaces"]->m[interface.first]->i("Respawn", ((interface.second->bRespawn)?"1":"0"), ((interface.second->bRespawn)?'1':'0'));
                    ptWrite->m["Interfaces"]->m[interface.first]->i("Restricted", ((interface.second->bRestricted)?"1":"0"), ((interface.second->bRestricted)?'1':'0'));
                  }
                  ptLink->responses.push_back(ptWrite->j(strJson));
                  delete ptWrite;
                  if ((unReturn = add(links, ptLink)) == 0)
                  {
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
              radialLink *ptLink = NULL;
              for (auto j = links.begin(); ptLink == NULL && j != links.end(); j++)
              {
                if ((*j)->fdSocket == fds[i].fd)
                {
                  ptLink = (*j);
                }
              }
              if (ptLink == NULL)
              {
                for (auto j = m_l.begin(); ptLink == NULL && j != m_l.end(); j++)
                {
                  if ((*j)->fdSocket == fds[i].fd)
                  {
                    ptLink = (*j);
                  }
                }
              }
              // }}}
              if (ptLink != NULL)
              {
                // {{{ read
                if (fds[i].revents & POLLIN)
                {
                  if (m_pUtility->sslRead(ptLink->ssl, ptLink->strBuffers[0], nReturn))
                  {
                    while ((unPosition = ptLink->strBuffers[0].find("\n")) != string::npos)
                    {
                      Json *ptJson = new Json(ptLink->strBuffers[0].substr(0, unPosition));
                      unThroughput++;
                      ptLink->strBuffers[0].erase(0, (unPosition + 1));
                      // {{{ _f
                      if (!empty(ptJson, "_f"))
                      {
                        // {{{ handshake
                        if (ptJson->m["_f"]->v == "handshake")
                        {
                          // {{{ Me
                          if (exist(ptJson, "Me"))
                          {
                            if (!empty(ptJson->m["Me"], "Node"))
                            {
                              ptLink->strNode = ptJson->m["Me"]->m["Node"]->v;
                            }
                            if (!empty(ptJson->m["Me"], "Server"))
                            {
                              ptLink->strServer = ptJson->m["Me"]->m["Server"]->v;
                            }
                            if (!empty(ptJson->m["Me"], "Port"))
                            {
                              ptLink->strPort = ptJson->m["Me"]->m["Port"]->v;
                            }
                          }
                          // }}}
                          // {{{ Links
                          if (exist(ptJson, "Links"))
                          {
                            for (auto &ptSubLink : ptJson->m["Links"]->l)
                            {
                              if (!empty(ptSubLink, "Node") && !empty(ptSubLink, "Server") && !empty(ptSubLink, "Port"))
                              {
                                bool bFound = false;
                                for (auto j = m_l.begin(); !bFound && j != m_l.end(); j++)
                                {
                                  if (ptSubLink->m["Node"]->v == (*j)->strNode)
                                  {
                                    bFound = true;
                                  }
                                }
                                if (!bFound && ptSubLink->m["Node"]->v != m_ptLink->m["Node"]->v)
                                {
                                  size_t unReturn;
                                  radialLink *ptDeepLink = new radialLink;
                                  ptDeepLink->bAuthenticated = true;
                                  ptDeepLink->bRetry = false;
                                  ptDeepLink->strNode = ptSubLink->m["Node"]->v;
                                  ptDeepLink->strServer = ptSubLink->m["Server"]->v;
                                  ptDeepLink->strPort = ptSubLink->m["Port"]->v;
                                  ptDeepLink->fdConnecting = -1;
                                  ptDeepLink->fdSocket = -1;
                                  ptDeepLink->rp = NULL;
                                  ptDeepLink->ssl = NULL;
                                  ptDeepLink->unUnique = unUnique++;
                                  if ((unReturn = add(m_l, ptDeepLink)) == 0)
                                  {
                                    ssMessage.str("");
                                    ssMessage << strPrefix << "->Utility::sslRead()->Link::add() error [" << ptJson->m["_f"]->v << "," << ptLink->strNode << "," << ptSubLink->m["Node"]->v << "]:  Failed to add link.";
                                    log(ssMessage.str());
                                  }
                                  delete ptDeepLink;
                                }
                              }
                            }
                          }
                          // }}}
                          // {{{ Password
                          if (exist(ptJson, "Password"))
                          {
                            if (ptJson->m["Password"]->v == m_strPassword)
                            {
                              ptLink->bAuthenticated = true;
                              ssMessage.str("");
                              ssMessage << strPrefix << "->Utility::sslRead() [" << ptJson->m["_f"]->v << "," << ptLink->strNode << "]:  Authenticated link.";
                              log(ssMessage.str());
                            }
                          }
                          // }}}
                        }
                        // }}}
                        // {{{ interfaces
                        else if (ptJson->m["_f"]->v == "interfaces")
                        {
                          Json *ptLinks = new Json;
                          for (auto &interface : ptLink->interfaces)
                          {
                            delete interface.second;
                          }
                          ptLink->interfaces.clear();
                          if (exist(ptJson, "Interfaces"))
                          {
                            for (auto &interface : ptJson->m["Interfaces"]->m)
                            {
                              ptLink->interfaces[interface.first] = new radialInterface;
                              if (!empty(interface.second, "AccessFunction"))
                              {
                                ptLink->interfaces[interface.first]->strAccessFunction = interface.second->m["AccessFunction"]->v;
                              }
                              if (!empty(interface.second, "Command"))
                              {
                                ptLink->interfaces[interface.first]->strCommand = interface.second->m["Command"]->v;
                              }
                              ptLink->interfaces[interface.first]->nPid = -1;
                              if (!empty(interface.second, "PID"))
                              { 
                                stringstream ssPid(interface.second->m["PID"]->v);
                                ssPid >> ptLink->interfaces[interface.first]->nPid;
                              }
                              ptLink->interfaces[interface.first]->bRespawn = ((exist(interface.second, "Respawn") && interface.second->m["Respawn"]->v == "1")?true:false);
                              ptLink->interfaces[interface.first]->bRestricted = ((exist(interface.second, "Restricted") && interface.second->m["Restricted"]->v == "1")?true:false);
                            }
                          }
                          ptLinks->i("Function", "links");
                          ptLinks->m["Links"] = new Json;
                          for (auto &link : m_l)
                          {
                            ptLinks->m["Links"]->m[link->strNode] = new Json;
                            ptLinks->m["Links"]->m[link->strNode]->i("Server", link->strServer);
                            ptLinks->m["Links"]->m[link->strNode]->i("Port", link->strPort);
                            ptLinks->m["Links"]->m[link->strNode]->m["Interfaces"] = new Json;
                            for (auto &interface : link->interfaces)
                            {
                              stringstream ssPid;
                              ssPid << interface.second->nPid;
                              ptLinks->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first] = new Json;
                              ptLinks->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first]->i("AccessFunction", interface.second->strAccessFunction);
                              ptLinks->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first]->i("Command", interface.second->strCommand);
                              ptLinks->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first]->i("PID", ssPid.str(), 'n');
                              ptLinks->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first]->i("Respawn", ((interface.second->bRespawn)?"1":"0"), ((interface.second->bRespawn)?'1':'0'));
                              ptLinks->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first]->i("Restricted", ((interface.second->bRestricted)?"1":"0"), ((interface.second->bRestricted)?'1':'0'));
                            }
                          }
                          hub(ptLinks, false);
                          delete ptLinks;
                        }
                        // }}}
                      }
                      // }}}
                      // {{{ _l
                      else if (exist(ptJson, "_l"))
                      {
                        if (!exist(ptJson, "Status"))
                        {
                          stringstream ssUnique;
                          Json *ptSubLink = ptJson->m["_l"];
                          radialPacket p;
                          ptJson->m.erase("_l");
                          for (auto &j : ptSubLink->m)
                          {
                            ptJson->i(j.first, j.second);
                          }
                          delete ptSubLink;
                          if (exist(ptJson, "_d"))
                          {
                            delete ptJson->m["_d"];
                            ptJson->m.erase("_d");
                          }
                          p.s = m_strName;
                          if (!empty(ptJson, "_t"))
                          {
                            p.t = ptJson->m["_t"]->v;
                            delete ptJson->m["_t"];
                            ptJson->m.erase("_t");
                          }
                          if (p.t == "link" && !empty(ptJson, "Interface"))
                          {
                            if (ptJson->m["Interface"]->v == "hub")
                            {
                              p.t.clear();
                              delete ptJson->m["Interface"];
                              ptJson->m.erase("Interface");
                            }
                            else
                            {
                              p.t = ptJson->m["Interface"]->v;
                            }
                          }
                          ssUnique << m_strName << " " << ptLink->fdSocket << " " << ptLink->unUnique;
                          p.u = ssUnique.str();
                          ptJson->j(p.p);
                          hub(p, false);
                        }
                        else
                        {
                          Json *ptSubLink = ptJson->m["_l"];
                          radialPacket p;
                          ptJson->m.erase("_l");
                          keyRemovals(ptJson);
                          for (auto &j : ptSubLink->m)
                          {
                            ptJson->i(j.first, j.second);
                          }
                          delete ptSubLink;
                          if (!empty(ptJson, "_d"))
                          {
                            p.d = ptJson->m["_d"]->v;
                            delete ptJson->m["_d"];
                            ptJson->m.erase("_d");
                          }
                          if (!empty(ptJson, "_s"))
                          {
                            p.s = ptJson->m["_s"]->v;
                            delete ptJson->m["_s"];
                            ptJson->m.erase("_s");
                          }
                          if (!empty(ptJson, "_t"))
                          {
                            p.t = ptJson->m["_t"]->v;
                            delete ptJson->m["_t"];
                            ptJson->m.erase("_t");
                          }
                          if (!empty(ptJson, "_u"))
                          {
                            p.u = ptJson->m["_u"]->v;
                            delete ptJson->m["_u"];
                            ptJson->m.erase("_u");
                          }
                          ptJson->j(p.p);
                          hub(p, false);
                        }
                      }
                      // }}}
                      // {{{ Interface
                      else if (!empty(ptJson, "Interface"))
                      {
                        if (!exist(ptJson, "Status"))
                        {
                          if (ptLink->bAuthenticated)
                          {
                            if (m_i.find(ptJson->m["Interface"]->v) != m_i.end())
                            {
                              stringstream ssUnique;
                              radialPacket p;
                              keyRemovals(ptJson);
                              p.s = m_strName;
                              p.t = ptJson->m["Interface"]->v;
                              ssUnique << m_strName << " " << ptLink->fdSocket << " " << ptLink->unUnique;
                              p.u = ssUnique.str();
                              ptJson->j(p.p);
                              hub(p, false);
                            }
                            else
                            {
                              ptJson->i("Status", "error");
                              ptJson->i("Error", "Interface does not exist.");
                              ptLink->responses.push_back(ptJson->j(strJson));
                            }
                          }
                          else
                          {
                            ptJson->i("Status", "error");
                            ptJson->i("Error", "Failed authentication.");
                            ptLink->responses.push_back(ptJson->j(strJson));
                          }
                        }
                      }
                      // }}}
                      // {{{ invalid
                      else
                      {
                        ptJson->i("Status", "error");
                        ptJson->i("Error", "Please provide the _f or Interface.");
                        ptLink->responses.push_back(ptJson->j(strJson));
                      }
                      // }}}
                      delete ptJson;
                    }
                  }
                  else
                  {
                    removals.push_back(ptLink->fdSocket);
                    if (nReturn < 0)
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Utility::sslRead(" << SSL_get_error(ptLink->ssl, nReturn) << ") error [" << ptLink->strNode << "]:  " << m_pUtility->sslstrerror(ptLink->ssl, nReturn);
                      log(ssMessage.str());
                    }
                  }
                }
                // }}}
                // {{{ write
                if (fds[i].revents & POLLOUT)
                {
                  if (!m_pUtility->sslWrite(ptLink->ssl, ptLink->strBuffers[1], nReturn))
                  {
                    removals.push_back(ptLink->fdSocket);
                    if (nReturn < 0)
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << "->Utility::sslWrite(" << SSL_get_error(ptLink->ssl, nReturn) << ") error [" << ptLink->strNode << "|" << ptLink->strServer << ":" << ptLink->strPort << "|" << ptLink->fdSocket << "]:  " << m_pUtility->sslstrerror(ptLink->ssl, nReturn);
                      log(ssMessage.str());
                    }
                  }
                }
                // }}}
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
          delete[] fds;
          time(&CTime);
          // {{{ removals
          if (!removals.empty())
          {
            removals.sort();
            removals.unique();
            for (auto &i : removals)
            {
              list<radialLink *>::iterator removeIter = links.end();
              for (auto k = links.begin(); removeIter == links.end() && k != links.end(); k++)
              {
                if ((*k)->fdSocket == i)
                {
                  removeIter = k;
                }
              }
              if (removeIter != links.end())
              {
                ssMessage.str("");
                ssMessage << strPrefix << " [removals,accepted," << (*removeIter)->strNode << "]:  Removed link.";
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
                delete (*removeIter);
                links.erase(removeIter);
              }
              removeIter = m_l.end();
              for (auto k = m_l.begin(); removeIter == m_l.end() && k != m_l.end(); k++)
              {
                if ((*k)->fdSocket == i)
                {
                  removeIter = k;
                }
              }
              if (removeIter != m_l.end())
              {
                ssMessage.str("");
                ssMessage << strPrefix << " [removals,connected," << (*removeIter)->strNode << "]:  Removed link.";
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
                delete (*removeIter);
                m_l.erase(removeIter);
              }
            }
            removals.clear();
          }
          // }}}
          // {{{ bootstrap links
          if (exist(m_ptLink, "Links"))
          {
            if ((CTime - CBootstrap) > unBootstrapSleep)
            {
              unsigned int unSeed = CTime + getpid();
              srand(unSeed);
              unBootstrapSleep = (rand_r(&unSeed) % 30) + 1;
              if (ptBoot->l.empty())
              {
                bool bReady = true;
                for (auto i = m_l.begin(); bReady && i != m_l.end(); i++)
                {
                  if ((*i)->bRetry || (*i)->fdSocket == -1 || (*i)->strNode.empty() || (*i)->strPort.empty() || (*i)->strServer.empty())
                  {
                    bReady = false;
                  }
                }
                if (bReady)
                {
                  for (auto &ptLink : m_ptLink->m["Links"]->l)
                  {
                    if (!empty(ptLink, "Node") && !empty(ptLink, "Server") && !empty(ptLink, "Port"))
                    {
                      bool bFound = false;
                      for (auto i = m_l.begin(); !bFound && i != m_l.end(); i++)
                      {
                        if ((*i)->strNode == ptLink->m["Node"]->v && (*i)->strServer == ptLink->m["Server"]->v && (*i)->strPort == ptLink->m["Port"]->v)
                        {
                          bFound = true;
                        }
                      }
                      if (!bFound)
                      {
                        Json *ptSubLink = new Json;
                        ptSubLink->i("Node", ptLink->m["Node"]->v);
                        ptSubLink->i("Server", ptLink->m["Server"]->v);
                        ptSubLink->i("Port", ptLink->m["Port"]->v, 'n');
                        ptBoot->pb(ptSubLink);
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
              ptLink->bRetry = false;
              ptLink->strNode = ptBoot->l.front()->m["Node"]->v;
              ptLink->strServer = ptBoot->l.front()->m["Server"]->v;
              ptLink->strPort = ptBoot->l.front()->m["Port"]->v;
              ptLink->fdConnecting = -1;
              ptLink->fdSocket = -1;
              ptLink->rp = NULL;
              ptLink->ssl = NULL;
              ptLink->unUnique = unUnique++;
              if ((unReturn = add(m_l, ptLink)) == 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Link::add() error [bootstrap," << ptLink->strNode << "]:  Failed to add link.";
                log(ssMessage.str());
              }
              delete ptLink;
              delete ptBoot->l.front();
              ptBoot->l.pop_front();
            }
          }
          // }}}
          // {{{ throughput
          if ((CTime - CThroughput) >= 60)
          {
            stringstream ssThroughput;
            Json *ptJson = new Json;
            radialPacket p;
            CThroughput = CTime;
            p.s = m_strName;
            ssThroughput << unThroughput;
            unThroughput = 0;
            ptJson->i("Function", "throughput");
            ptJson->m["Response"] = new Json;
            ptJson->m["Response"]->i("request", ssThroughput.str(), 'n');
            throughput(ptJson->m["Response"]);
            ptJson->j(p.p);
            delete ptJson;
            hub(p, false);
          }
          // }}}
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
        for (auto &link : m_l)
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
        m_l.clear();
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
