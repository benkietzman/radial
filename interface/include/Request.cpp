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
// {{{ process()
void Request::process(string strPrefix)
{
  // {{{ prep work
  SSL_CTX *ctx = NULL;
  string strError, strJson;
  stringstream ssMessage;
  strPrefix += "->Request::process()";
  m_pUtility->fdNonBlocking(0, strError);
  m_pUtility->fdNonBlocking(1, strError);
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
        list<int> removals;
        map<int, radialRequestConn *> conns;
        pollfd *fds;
        size_t unIndex, unPosition, unUnique = 0;
        string strLine;
        ssMessage.str("");
        ssMessage << strPrefix << "->listen():  Listening to incoming socket.";
        log(ssMessage.str());
        // }}}
        while (!bExit)
        {
          // {{{ prep work
          fds = new pollfd[conns.size() + 3];
          unIndex = 0;
          fds[unIndex].fd = 0;
          fds[unIndex].events = POLLIN;
          unIndex++;
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
          fds[unIndex].fd = fdSocket;
          fds[unIndex].events = POLLIN;
          unIndex++;
          for (auto &conn : conns)
          {
            fds[unIndex].events = POLLIN;
            if (conn.second->bRetry)
            {
              if ((nReturn = SSL_accept(conn.second->ssl)) > 0)
              {
                conn.second->bRetry = false;
              }
              else
              {
                strError = m_pUtility->sslstrerror(conn.second->ssl, nReturn, conn.second->bRetry);
                if (!conn.second->bRetry)
                { 
                  conn.second->bRetry = true;
                  removals.push_back(conn.first);
                }
              }
            }
            if (!conn.second->bRetry)
            {
              fds[unIndex].fd = conn.first;
              if (conn.second->strBuffers[1].empty())
              {
                while (!conn.second->responses.empty())
                {
                  conn.second->strBuffers[1].append(conn.second->responses.front() + "\n");
                  conn.second->responses.pop_front();
                }
              }
              if (!conn.second->strBuffers[1].empty())
              {
                fds[unIndex].events |= POLLOUT;
              }
            }
            else
            {
              fds[unIndex].fd = -1;
            }
            unIndex++;
          }
          // }}}
          if ((nReturn = poll(fds, unIndex, 500)) > 0)
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
                  if (exist(ptJson, "_r"))
                  {
                    Json *ptSubJson = ptJson->m["_r"];
                    if (exist(ptSubJson, "_s") && ptSubJson->m["_s"]->v == m_strName && !empty(ptSubJson, "_u"))
                    {
                      int fdClient;
                      size_t unUnique;
                      string strValue;
                      stringstream ssUnique(ptSubJson->m["_u"]->v);
                      ssUnique >> strValue >> fdClient >> unUnique;
                      if (conns.find(fdClient) != conns.end() && conns[fdClient]->unUnique == unUnique)
                      {
                        if (exist(ptJson, "_t"))
                        {
                          if (ptJson->m["_t"]->v == "auth" || ptJson->m["_t"]->v == "link")
                          {
                            if (exist(ptJson, "Status") && ptJson->m["Status"]->v == "okay")
                            {
                              hub(ptSubJson, false);
                            }
                            else
                            {
                              keyRemovals(ptSubJson);
                              ptSubJson->i("Status", "error");
                              ptSubJson->i("Error", ((!empty(ptJson, "Error"))?ptJson->m["Error"]->v:"Encountered an unknown error."));
                              conns[fdClient]->responses.push_back(ptSubJson->j(strJson));
                            }
                          }
                          else
                          {
                            keyRemovals(ptSubJson);
                            ptSubJson->i("Status", "error");
                            ptSubJson->i("Error", "Invalid internal target.");
                            conns[fdClient]->responses.push_back(ptSubJson->j(strJson));
                          }
                        }
                        else
                        {
                          keyRemovals(ptSubJson);
                          ptSubJson->i("Status", "error");
                          ptSubJson->i("Error", "Invalid path of logic.");
                          conns[fdClient]->responses.push_back(ptSubJson->j(strJson));
                        }
                      }
                      else
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " error [stdin," << fdClient << "," << unUnique << "]:  Client no longer exists.";
                        log(ssMessage.str());
                      }
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << " error [stdin]:  ";
                      if (!exist(ptSubJson, "_s"))
                      {
                        ssMessage << "Internal source does not exist.";
                      }
                      else if (ptSubJson->m["_s"]->v != m_strName)
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
                  else if (exist(ptJson, "_s") && ptJson->m["_s"]->v == m_strName && !empty(ptJson, "_u"))
                  {
                    int fdClient;
                    size_t unUnique;
                    string strValue;
                    stringstream ssUnique(ptJson->m["_u"]->v);
                    ssUnique >> strValue >> fdClient >> unUnique;
                    if (conns.find(fdClient) != conns.end() && conns[fdClient]->unUnique == unUnique)
                    {
                      keyRemovals(ptJson);
                      conns[fdClient]->responses.push_back(ptJson->j(strJson));
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << " error [stdin," << fdClient << "," << unUnique << "]:  Client no longer exists.";
                      log(ssMessage.str());
                    }
                  }
                  else if (exist(ptJson, "_s") && ptJson->m["_s"]->v == "hub")
                  {
                    if (!empty(ptJson, "Function"))
                    {
                      if (ptJson->m["Function"]->v == "interfaces")
                      {
                        interfaces(strPrefix, ptJson);
                      }
                      else if (ptJson->m["Function"]->v == "links")
                      {
                        links(strPrefix, ptJson);
                      }
                      else if (ptJson->m["Function"]->v == "shutdown")
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " [stdin,hub," << ptJson->m["Function"]->v << "]:  Shutting down.";
                        log(ssMessage.str());
                        setShutdown();
                      }
                      else
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " error [stdin,hub," << ptJson->m["Function"]->v << "]:  Please provide a valid Function:  interfaces, links, shutdown.";
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
                    if (!empty(ptJson, "Function"))
                    {
                      if (ptJson->m["Function"]->v == "ping")
                      {
                        bProcessed = true;
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
                    ptJson->i("Status", ((bProcessed)?"okay":"error"));
                    if (!strError.empty())
                    {
                      ptJson->i("Error", strError);
                    }
                    hub(ptJson, false);
                  }
                  delete ptJson;
                }
              }
              else
              {
                bExit = true;
                if (nReturn < 0)
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
              int fdClient;
              sockaddr_in cli_addr;
              socklen_t clilen = sizeof(cli_addr);
              if ((fdClient = accept(fds[2].fd, (sockaddr *)&cli_addr, &clilen)) >= 0)
              {
                radialRequestConn *ptConn = new radialRequestConn;
                m_pUtility->fdNonBlocking(fdClient, strError);
                ptConn->bRetry = false;
                ptConn->ssl = NULL;
                ptConn->unUnique = unUnique++;
                if ((ptConn->ssl = m_pUtility->sslAccept(ctx, fdClient, ptConn->bRetry, strError)) != NULL)
                {
                  conns[fdClient] = ptConn;
                }
                else
                {
                  close(fdClient);
                  delete ptConn;
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Utility::sslAccept() error [accept," << fdClient << "]:  " << strError;
                  log(ssMessage.str());
                }
              }
              else
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->accept(" << errno << ") error [accept," << fds[2].fd << "]:  " << strerror(errno);
                notify(ssMessage.str());
              }
            }
            // }}}
            for (size_t i = 3; i < unIndex; i++)
            {
              // {{{ prep work
              bool bGood = true;
              // }}}
              // {{{ read
              if (fds[i].revents & POLLIN)
              {
                // {{{ accept
                if (conns[fds[i].fd]->bRetry)
                {
                  if ((nReturn = SSL_accept(conns[fds[i].fd]->ssl)) == 1)
                  {
                    conns[fds[i].fd]->bRetry = false;
                  }
                  else
                  {
                    strError = m_pUtility->sslstrerror(conns[fds[i].fd]->ssl, nReturn, conns[fds[i].fd]->bRetry);
                    if (!conns[fds[i].fd]->bRetry)
                    {
                      bGood = false;
                      removals.push_back(fds[i].fd);
                      ssMessage.str("");
                      ssMessage << strPrefix << "->SSL_accept() error [read," << fds[i].fd << "]:  " << strError;
                      log(ssMessage.str());
                    }
                  }     
                }
                // }}}
                if (bGood && m_pUtility->sslRead(conns[fds[i].fd]->ssl, conns[fds[i].fd]->strBuffers[0], nReturn))
                {
                  if ((unPosition = conns[fds[i].fd]->strBuffers[0].find("\n")) != string::npos)
                  {
                    Json *ptJson = new Json(conns[fds[i].fd]->strBuffers[0].substr(0, unPosition));
                    conns[fds[i].fd]->strBuffers[0].erase(0, (unPosition + 1));
                    keyRemovals(ptJson);
                    if (!shutdown())
                    {
                      if (!empty(ptJson, "Interface"))
                      {
                        if (ptJson->m["Interface"]->v == "hub")
                        {
                          if (!empty(ptJson, "Function"))
                          {
                            if (ptJson->m["Function"]->v == "list" || ptJson->m["Function"]->v == "ping")
                            {
                              stringstream ssUnique;
                              ptJson->i("_s", m_strName);
                              ssUnique << m_strName << " " << fds[i].fd << " " << conns[fds[i].fd]->unUnique;
                              ptJson->i("_u", ssUnique.str());
                              hub(ptJson, false);
                            }
                            else
                            {
                              ptJson->i("Status", "error");
                              ptJson->i("Error", "Please provide a valid Function:  list, ping.");
                              ptJson->j(strJson);
                              conns[fds[i].fd]->responses.push_back(strJson);
                            }
                          }
                          else
                          {
                            ptJson->i("Status", "error");
                            ptJson->i("Error", "Please provide the Function.");
                            ptJson->j(strJson);
                            conns[fds[i].fd]->responses.push_back(strJson);
                          }
                        }
                        else
                        {
                          bool bRestricted = false;
                          string strTarget = ptJson->m["Interface"]->v, strTargetAuth = "auth";
                          stringstream ssUnique;
                          if (m_interfaces.find(ptJson->m["Interface"]->v) != m_interfaces.end())
                          {
                            bRestricted = m_interfaces[ptJson->m["Interface"]->v]->bRestricted;
                          }
                          else
                          {
                            list<radialLink *>::iterator linkIter = m_links.end();
                            for (auto j = m_links.begin(); linkIter == m_links.end() && j != m_links.end(); j++)
                            {
                              if ((*j)->interfaces.find(ptJson->m["Interface"]->v) != (*j)->interfaces.end())
                              { 
                                linkIter = j;
                              }
                            }
                            if (linkIter != m_links.end() && m_interfaces.find("link") != m_interfaces.end() && (bRestricted = (*linkIter)->interfaces[ptJson->m["Interface"]->v]->bRestricted))
                            {
                              ptJson->i("Node", (*linkIter)->strNode);
                              strTarget = strTargetAuth = "link";
                            }
                          }
                          ptJson->i("_s", m_strName);
                          ptJson->i("_t", strTarget);
                          ssUnique << m_strName << " " << fds[i].fd << " " << conns[fds[i].fd]->unUnique;
                          ptJson->i("_u", ssUnique.str());
                          if (!bRestricted)
                          {
                            hub(ptJson, false);
                          }
                          else
                          {
                            Json *ptAuth = new Json(ptJson);
                            ptAuth->i("Interface", "auth");
                            if (exist(ptAuth, "Request"))
                            {
                              delete ptAuth->m["Request"];
                            }
                            ptAuth->m["Request"] = new Json;
                            ptAuth->m["Request"]->i("Interface", ptJson->m["Interface"]->v);
                            ptAuth->m["_r"] = new Json(ptJson);
                            ptAuth->i("_t", strTargetAuth);
                            hub(ptAuth, false);
                            delete ptAuth;
                          }
                        }
                      }
                      else
                      {
                        ptJson->i("Status", "error");
                        ptJson->i("Error", "Please provide the Interface.");
                        conns[fds[i].fd]->responses.push_back(ptJson->j(strJson));
                      }
                    }
                    else
                    {
                      ptJson->i("Status", "error");
                      ptJson->i("Error", "Radial is shutting down.");
                      conns[fds[i].fd]->responses.push_back(ptJson->j(strJson));
                    }
                    delete ptJson;
                  }
                }
                else if (bGood)
                {
                  removals.push_back(fds[i].fd);
                  if (nReturn < 0 && errno != 104)
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Utility::sslRead(" << SSL_get_error(conns[fds[i].fd]->ssl, nReturn) << ") error [read," << fds[i].fd << "]:  " << m_pUtility->sslstrerror(conns[fds[i].fd]->ssl, nReturn);
                    log(ssMessage.str());
                  }
                }
              }
              // }}}
              // {{{ write
              if (fds[i].revents & POLLOUT)
              {
                if (bGood && !m_pUtility->sslWrite(conns[fds[i].fd]->ssl, conns[fds[i].fd]->strBuffers[1], nReturn))
                {
                  removals.push_back(fds[i].fd);
                  if (nReturn < 0)
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Utility::sslWrite(" << SSL_get_error(conns[fds[i].fd]->ssl, nReturn) << ") error [write," << fds[i].fd << "]:  " << m_pUtility->sslstrerror(conns[fds[i].fd]->ssl, nReturn);
                    log(ssMessage.str());
                  }
                }
              }
              // }}}
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
          monitor(strPrefix);
          if (shutdown())
          {
            if (fdSocket != -1)
            {
              close(fdSocket);
              fdSocket = -1;
            }
            bExit = true;
            for (auto &conn : conns)
            {
              if (!conn.second->strBuffers[0].empty() || !conn.second->strBuffers[1].empty())
              {
                bExit = false;
              }
              else
              {
                if (!conn.second->responses.empty())
                {
                  bExit = false;
                }
                else
                {
                  removals.push_back(conn.first);
                }
              }
            }
          }
          if (!removals.empty())
          {
            removals.sort();
            removals.unique();
            while (!removals.empty())
            {
              SSL_shutdown(conns[removals.front()]->ssl);
              SSL_free(conns[removals.front()]->ssl);
              close(removals.front());
              delete conns[removals.front()];
              conns.erase(removals.front());
              removals.pop_front();
            }
          }
          // }}}
        }
        // {{{ post work
        for (auto &conn : conns)
        {
          SSL_shutdown(conn.second->ssl);
          SSL_free(conn.second->ssl);
          close(conn.first);
          delete conn.second;
        }
        // }}}
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->listen(" << errno << ") error:  " << strerror(errno);
        notify(ssMessage.str());
      }
      // {{{ post work
      if (fdSocket != -1)
      {
        close(fdSocket);
      }
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
  // }}}
}
// }}}
}
}
