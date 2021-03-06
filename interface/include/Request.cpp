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
  long lArg;
  SSL_CTX *ctx = NULL;
  string strError, strJson;
  stringstream ssMessage;
  strPrefix += "->Request::socket()";
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
            fds[unIndex].fd = conn.first;
            fds[unIndex].events = POLLIN;
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
            unIndex++;
          }
          // }}}
          if ((nReturn = poll(fds, unIndex, 100)) > 0)
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
                      int fdClient;
                      size_t unUnique;
                      stringstream ssUnique(ptSubJson->m["_unique"]->v);
                      ssUnique >> fdClient >> unUnique;
                      if (conns.find(fdClient) != conns.end() && conns[fdClient]->unUnique == unUnique)
                      {
                        if (ptJson->m.find("_target") != ptJson->m.end())
                        {
                          if (ptJson->m["_target"]->v == "auth")
                          {
                            if (ptJson->m.find("Status") != ptJson->m.end() && ptJson->m["Status"]->v == "okay")
                            {
                              hub(ptSubJson, false);
                            }
                            else
                            {
                              keyRemovals(ptSubJson);
                              ptSubJson->insert("Status", "error");
                              ptSubJson->insert("Error", ((ptJson->m.find("Error") != ptJson->m.end() && !ptJson->m["Error"]->v.empty())?ptJson->m["Error"]->v:"Encountered an unknown error."));
                              conns[fdClient]->responses.push_back(ptSubJson->json(strJson));
                            }
                          }
                          else
                          {
                            keyRemovals(ptSubJson);
                            ptSubJson->insert("Status", "error");
                            ptSubJson->insert("Error", "Invalid internal target.");
                            conns[fdClient]->responses.push_back(ptSubJson->json(strJson));
                          }
                        }
                        else if (ptJson->m.find("Function") != ptJson->m.end() && ptJson->m["Function"]->v == "list")
                        {
                          if (ptJson->m.find("Response") != ptJson->m.end())
                          {
                            if (ptSubJson->m.find("Interface") != ptSubJson->m.end() && !ptSubJson->m["Interface"]->v.empty())
                            {
                              if (ptJson->m["Response"]->m.find(ptSubJson->m["Interface"]->v) != ptJson->m["Response"]->m.end())
                              {
                                if (ptJson->m["Response"]->m[ptSubJson->m["Interface"]->v]->m.find("Restricted") == ptJson->m["Response"]->m[ptSubJson->m["Interface"]->v]->m.end() || ptJson->m["Response"]->m[ptSubJson->m["Interface"]->v]->m["Restricted"]->v == "0")
                                {
                                  hub(ptSubJson, false);
                                }
                                else
                                {
                                  Json *ptAuth = new Json(ptSubJson);
                                  keyRemovals(ptAuth);
                                  if (ptAuth->m.find("Request") != ptAuth->m.end())
                                  {
                                    delete ptAuth->m["Request"];
                                  }
                                  ptAuth->m["Request"] = new Json;
                                  ptAuth->m["Request"]->insert("Interface", ptSubJson->m["Interface"]->v);
                                  ptAuth->insert("_target", "auth");
                                  ptAuth->m["_request"] = new Json(ptSubJson);
                                  ptAuth->insert("_source", m_strName);
                                  hub(ptAuth, false);
                                  delete ptAuth;
                                }
                              }
                              else
                              {
                                keyRemovals(ptSubJson);
                                ptSubJson->insert("Status", "error");
                                ptSubJson->insert("Error", "Interface does not exist.");
                                conns[fdClient]->responses.push_back(ptSubJson->json(strJson));
                              }
                            }
                            else
                            {
                              keyRemovals(ptSubJson);
                              ptSubJson->insert("Status", "error");
                              ptSubJson->insert("Error", "Please provide the Interface.");
                              conns[fdClient]->responses.push_back(ptSubJson->json(strJson));
                            }
                          }
                          else
                          {
                            keyRemovals(ptSubJson);
                            ptSubJson->insert("Status", "error");
                            ptSubJson->insert("Error", "Failed to retrieve interfaces.");
                            conns[fdClient]->responses.push_back(ptSubJson->json(strJson));
                          }
                        }
                        else
                        {
                          keyRemovals(ptSubJson);
                          ptSubJson->insert("Status", "error");
                          ptSubJson->insert("Error", "Invalid path of logic.");
                          conns[fdClient]->responses.push_back(ptSubJson->json(strJson));
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
                    int fdClient;
                    size_t unUnique;
                    stringstream ssUnique(ptJson->m["_unique"]->v);
                    ssUnique >> fdClient >> unUnique;
                    if (conns.find(fdClient) != conns.end() && conns[fdClient]->unUnique == unUnique)
                    {
                      keyRemovals(ptJson);
                      conns[fdClient]->responses.push_back(ptJson->json(strJson));
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << " error [stdin," << fdClient << "," << unUnique << "]:  Client no longer exists.";
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
                    if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
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
                ptConn->ssl = NULL;
                ptConn->unUnique = unUnique++;
                ptConn->eSocketType = COMMON_SOCKET_UNKNOWN;
                conns[fdClient] = ptConn;
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
              // {{{ read
              if (fds[i].revents & POLLIN)
              {
                bool bGood = true;
                if (conns[fds[i].fd]->eSocketType == COMMON_SOCKET_UNKNOWN)
                {
                  if (m_pUtility->socketType(fds[i].fd, conns[fds[i].fd]->eSocketType, strError))
                  {
                    if (conns[fds[i].fd]->eSocketType == COMMON_SOCKET_ENCRYPTED)
                    {
                      if ((conns[fds[i].fd]->ssl = m_pUtility->sslAccept(ctx, fds[i].fd, strError)) == NULL)
                      {
                        bGood = false;
                        removals.push_back(fds[i].fd);
                        ssMessage.str("");
                        ssMessage << strPrefix << "->Utility::sslAccept() error [read," << fds[i].fd << "]:  " << strError;
                        log(ssMessage.str());
                      }
                    }
                    if ((lArg = fcntl(fds[i].fd, F_GETFL, NULL)) >= 0)
                    {
                      lArg |= O_NONBLOCK;
                      fcntl(fds[i].fd, F_SETFL, lArg);
                    }
                  }
                  else
                  {
                    bGood = false;
                    removals.push_back(fds[i].fd);
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Utility::socketType() error [read," << fds[i].fd << "]:  " << strError;
                    log(ssMessage.str());
                  }
                }
                if (bGood && ((conns[fds[i].fd]->eSocketType == COMMON_SOCKET_ENCRYPTED && m_pUtility->sslRead(conns[fds[i].fd]->ssl, conns[fds[i].fd]->strBuffers[0], nReturn)) || (conns[fds[i].fd]->eSocketType == COMMON_SOCKET_UNENCRYPTED && m_pUtility->fdRead(fds[i].fd, conns[fds[i].fd]->strBuffers[0], nReturn))))
                {
                  if ((unPosition = conns[fds[i].fd]->strBuffers[0].find("\n")) != string::npos)
                  {
                    Json *ptJson = new Json(conns[fds[i].fd]->strBuffers[0].substr(0, unPosition));
                    conns[fds[i].fd]->strBuffers[0].erase(0, (unPosition + 1));
                    keyRemovals(ptJson);
                    if (!shutdown())
                    {
                      if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
                      {
                        if (ptJson->m["Interface"]->v == "hub")
                        {
                          if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
                          {
                            if (ptJson->m["Function"]->v == "list" || ptJson->m["Function"]->v == "ping")
                            {
                              stringstream ssUnique;
                              ptJson->insert("_source", m_strName);
                              ssUnique << fds[i].fd << " " << conns[fds[i].fd]->unUnique;
                              ptJson->insert("_unique", ssUnique.str());
                              hub(ptJson, false);
                            }
                            else
                            {
                              ptJson->insert("Node", m_strNode);
                              ptJson->insert("Status", "error");
                              ptJson->insert("Error", "Please provide a valid Function:  list, ping.");
                              ptJson->json(strJson);
                              conns[fds[i].fd]->responses.push_back(strJson);
                            }
                          }
                          else
                          {
                            ptJson->insert("Node", m_strNode);
                            ptJson->insert("Status", "error");
                            ptJson->insert("Error", "Please provide the Function.");
                            ptJson->json(strJson);
                            conns[fds[i].fd]->responses.push_back(strJson);
                          }
                        }
                        else
                        {
                          stringstream ssUnique;
                          Json *ptInterfaces = new Json;
                          ptJson->insert("_source", m_strName);
                          ptJson->insert("_target", ptJson->m["Interface"]->v);
                          ssUnique << fds[i].fd << " " << conns[fds[i].fd]->unUnique;
                          ptJson->insert("_unique", ssUnique.str());
                          ptInterfaces->insert("Function", "list");
                          ptInterfaces->m["_request"] = new Json(ptJson);
                          ptInterfaces->insert("_source", m_strName);
                          hub(ptInterfaces, false);
                          delete ptInterfaces;
                        }
                      }
                      else
                      {
                        ptJson->insert("Node", m_strNode);
                        ptJson->insert("Status", "error");
                        ptJson->insert("Error", "Please provide the Interface.");
                        ptJson->json(strJson);
                        conns[fds[i].fd]->responses.push_back(strJson);
                      }
                    }
                    else
                    {
                      ptJson->insert("Status", "error");
                      ptJson->insert("Error", "Radial is shutting down.");
                      ptJson->insert("Node", m_strNode);
                      ptJson->json(strJson);
                      conns[fds[i].fd]->responses.push_back(strJson);
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
                    ssMessage << strPrefix << "->Utility::";
                    if (conns[fds[i].fd]->eSocketType == COMMON_SOCKET_ENCRYPTED)
                    {
                      ssMessage << "sslRead(" << SSL_get_error(conns[fds[i].fd]->ssl, nReturn) << ") error [read," << fds[i].fd << "]:  " << m_pUtility->sslstrerror(conns[fds[i].fd]->ssl, nReturn);
                    }
                    else
                    {
                      ssMessage << "fdRead(" << errno << ") error [read," << fds[i].fd << "]:  " << strerror(errno);
                    }
                    log(ssMessage.str());
                  }
                }
              }
              // }}}
              // {{{ write
              if (fds[i].revents & POLLOUT)
              {
                if ((conns[fds[i].fd]->eSocketType == COMMON_SOCKET_ENCRYPTED && !m_pUtility->sslWrite(conns[fds[i].fd]->ssl, conns[fds[i].fd]->strBuffers[1], nReturn)) || (conns[fds[i].fd]->eSocketType == COMMON_SOCKET_UNENCRYPTED && !m_pUtility->fdWrite(fds[i].fd, conns[fds[i].fd]->strBuffers[1], nReturn)))
                {
                  removals.push_back(fds[i].fd);
                  if (nReturn < 0)
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Utility::";
                    if (conns[fds[i].fd]->eSocketType == COMMON_SOCKET_ENCRYPTED)
                    {
                      ssMessage << "sslWrite(" << SSL_get_error(conns[fds[i].fd]->ssl, nReturn) << ") error [write," << fds[i].fd << "]:  " << m_pUtility->sslstrerror(conns[fds[i].fd]->ssl, nReturn);
                    }
                    else
                    {
                      ssMessage << "fdWrite(" << errno << ") error [write," << fds[i].fd << "]:  " << strerror(errno);
                    }
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
              if (conns[removals.front()]->eSocketType == COMMON_SOCKET_ENCRYPTED)
              {
                SSL_shutdown(conns[removals.front()]->ssl);
                SSL_free(conns[removals.front()]->ssl);
              }
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
          if (conn.second->eSocketType == COMMON_SOCKET_ENCRYPTED)
          {
            SSL_shutdown(conn.second->ssl);
            SSL_free(conn.second->ssl);
          }
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
