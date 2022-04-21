// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Radial
// -------------------------------------
// file       : Hub.cpp
// author     : Ben Kietzman
// begin      : 2022-04-19
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
**************************************************************************/
/*! \file Hub.cpp
* \brief Hub Class
*
* Provides Hub functionality.
*/
// {{{ includes
#include "Hub"
// }}}
extern "C++"
{ 
namespace radial
{
// {{{ Hub()
Hub::Hub(int argc, char **argv, char **env) : Base(argc, argv)
{
  m_env = env;
}
// }}}
// {{{ ~Hub()
Hub::~Hub()
{
  for (auto &i : m_interfaces)
  {
    close(i.second->fdRead);
    close(i.second->fdWrite);
    delete i.second;
  }
}
// }}}
// {{{ alert()
void Hub::alert(const string strMessage)
{
  log("alert", strMessage);
}
// }}}
// {{{ interfaceAdd()
bool Hub::interfaceAdd(const string strName, const string strCommand, const bool bRespawn, string &strError)
{
  bool bResult = false;
  stringstream ssMessage;

  if (m_interfaces.find(strName) == m_interfaces.end() || bRespawn)
  {
    char *args[100], *pszArgument;
    int readpipe[2] = {-1, -1}, writepipe[2] = {-1, -1};
    size_t unIndex = 0;
    pid_t nPid;
    string strArgument;
    stringstream ssCommand(strCommand);
    while (ssCommand >> strArgument)
    {
      pszArgument = new char[strArgument.size() + 1];
      strcpy(pszArgument, strArgument.c_str());
      args[unIndex++] = pszArgument;
    }
    args[unIndex] = NULL;
    if (pipe(readpipe) == 0)
    {
      if (pipe(writepipe) == 0)
      {
        if ((nPid = fork()) == 0)
        {
          close(writepipe[1]);
          close(readpipe[0]);
          dup2(writepipe[0], 0);
          close(writepipe[0]);
          dup2(readpipe[1], 1);
          close(readpipe[1]);
          execve(args[0], args, m_env);
          _exit(1);
        }
        else if (nPid > 0)
        {
          bResult = true;
          close(writepipe[0]);
          close(readpipe[1]);
          if (m_interfaces.find(strName) == m_interfaces.end())
          {
            m_interfaces[strName] = new radialHubInterface;
          }
          m_interfaces[strName]->bRespawn = bRespawn;
          m_interfaces[strName]->fdRead = readpipe[0];
          m_interfaces[strName]->fdWrite = writepipe[1];
          m_interfaces[strName]->nPid = nPid;
          m_interfaces[strName]->strCommand = strCommand;
        }
        else
        {
          ssMessage.str("");
          ssMessage << "fork(" << errno << ") " << strerror(errno);
          strError = ssMessage.str();
        }
      }
      else
      {
        ssMessage.str("");
        ssMessage << "pipe(write," << errno << ") " << strerror(errno);
        strError = ssMessage.str();
      }
    }
    else
    {
      ssMessage.str("");
      ssMessage << "pipe(read," << errno << ") " << strerror(errno);
      strError = ssMessage.str();
    }
  }
  else
  {
    strError = "Interface already exists.";
  }

  return bResult;
}
// }}}
// {{{ interfaceRemove()
void Hub::interfaceRemove(const string strName)
{
  string strError;

  if (m_interfaces.find(strName) != m_interfaces.end())
  {
    close(m_interfaces[strName]->fdRead);
    m_interfaces[strName]->fdRead = false;
    close(m_interfaces[strName]->fdWrite);
    m_interfaces[strName]->fdWrite = false;
    m_interfaces[strName]->strBuffers[0].clear();
    m_interfaces[strName]->strBuffers[1].clear();
    if (!shutdown() && m_interfaces[strName]->bRespawn)
    {
      interfaceAdd(strName, m_interfaces[strName]->strCommand, true, strError);
    }
    else
    {
      delete m_interfaces[strName];
      m_interfaces.erase(strName);
    }
  }
}
// }}}
// {{{ interfacesLoad()
bool Hub::interfacesLoad(string &strError)
{
  bool bResult = false;
  ifstream inInterfaces;
  stringstream ssInterfaces, ssMessage;

  ssInterfaces << m_strData << "/interfaces.json";
  inInterfaces.open(ssInterfaces.str().c_str());
  if (inInterfaces)
  {
    string strLine;
    stringstream ssJson;
    Json *ptInterfaces;
    bResult = true;
    while (getline(inInterfaces, strLine))
    {
      ssJson << strLine;
    }
    ptInterfaces = new Json(ssJson.str());
    for (auto &i : ptInterfaces->m)
    {
      if (i.second->m.find("Command") != i.second->m.end() && !i.second->m["Command"]->v.empty())
      {
        if (interfaceAdd(i.first, i.second->m["Command"]->v, ((i.second->m.find("Respawn") != i.second->m.end() && i.second->m["Respawn"]->v == "1")?true:false), strError))
        {
          ssMessage.str("");
          ssMessage << "Hub::interfaceAdd() [" << ssInterfaces.str() << "," << i.first << "] Loaded interface.";
          log(ssMessage.str());
        }
        else
        {
          bResult = false;
          ssMessage.str("");
          ssMessage << "Hub::interfaceAdd(" << i.first << ") [" << ssInterfaces.str() << "," << i.first << "] " << strError;
          strError = ssMessage.str();
        }
      }
      else
      {
        bResult = false;
        ssMessage.str("");
        ssMessage << "[" << ssInterfaces.str() << "," << i.first << "] Please provide the Command.";
        strError = ssMessage.str();
      }
    }
    delete ptInterfaces;
  }
  else
  {
    ssMessage.str("");
    ssMessage << "ifstream::open(" << errno << ") [" << ssInterfaces.str() << "] " << strerror(errno);
    strError = ssMessage.str();
  }
  inInterfaces.close();

  return bResult;
}
// }}}
// {{{ log()
void Hub::log(const string strMessage)
{
  log("log", strMessage);
}
void Hub::log(const string strFunction, const string strMessage)
{
  Json *ptJson = new Json;

  ptJson->insert("Function", strFunction);
  ptJson->insert("Message", strMessage);
  target("log", ptJson);
  delete ptJson;
}
// }}}
// {{{ notify()
void Hub::notify(const string strMessage)
{
  log("alert", strMessage);
}
// }}}
// {{{ process()
bool Hub::process(string strPrefix, string &strError)
{
  bool bResult = false;

  strPrefix += "->Hub::process()";
  if (interfacesLoad(strError))
  {
    // {{{ prep work
    bool bExit = false;
    char szBuffer[65536];
    stringstream ssMessage;
    int nReturn;
    size_t unIndex, unPosition;
    string strLine;
    pollfd *fds;
    bResult = true;
    // }}}
    while (!bExit)
    {
      // {{{ prep work
      list<string> removals;
      map<int, string> sockets;
      fds = new pollfd[m_interfaces.size()*2];
      unIndex = 0;
      for (auto &i : m_interfaces)
      {
        sockets[i.second->fdRead] = i.first;
        fds[unIndex].fd = i.second->fdRead;
        fds[unIndex].events = POLLIN;
        unIndex++;
        sockets[i.second->fdWrite] = i.first;
        fds[unIndex].fd = -1;
        fds[unIndex].events = POLLOUT;
        if (i.second->strBuffers[1].empty() && !i.second->buffers[1].empty())
        {
          i.second->strBuffers[1] = i.second->buffers[1].front() + "\n";
          i.second->buffers[1].pop_front();
        }
        if (!i.second->strBuffers[1].empty())
        {
          fds[unIndex].fd = i.second->fdWrite;
        }
        unIndex++;
      }
      // }}}
      if ((nReturn = poll(fds, unIndex, 250)) > 0)
      {
        for (size_t i = 0; i < unIndex; i++)
        {
          // {{{ read
          if (fds[i].revents & (POLLHUP | POLLIN))
          {
            if ((nReturn = read(fds[i].fd, szBuffer, 65536)) > 0)
            {
              // {{{ prep work
              m_interfaces[sockets[fds[i].fd]]->strBuffers[0].append(szBuffer, nReturn);
              while ((unPosition = m_interfaces[sockets[fds[i].fd]]->strBuffers[0].find("\n")) != string::npos)
              {
                m_interfaces[sockets[fds[i].fd]]->buffers[0].push_back(m_interfaces[sockets[fds[i].fd]]->strBuffers[0].substr(0, unPosition));
                m_interfaces[sockets[fds[i].fd]]->strBuffers[0].erase(0, (unPosition + 1));
              }
              // }}}
              for (auto &strLine : m_interfaces[sockets[fds[i].fd]]->buffers[0])
              {
                Json *ptJson = new Json(strLine);
                if (ptJson->m.find("Target") != ptJson->m.end() && !ptJson->m["Target"]->v.empty() && m_interfaces.find(ptJson->m["Target"]->v) != m_interfaces.end())
                {
                  if (ptJson->m["Target"]->v != sockets[fds[i].fd])
                  {
                    m_interfaces[ptJson->m["Target"]->v]->buffers[1].push_back(strLine);
                  }
                  else if (ptJson->m.find("_unique") != ptJson->m.end() && !ptJson->m["_unique"]->v.empty() && ptJson->m.find("Source") != ptJson->m.end() && !ptJson->m["Source"]->v.empty() && m_interfaces.find(ptJson->m["Source"]->v) != m_interfaces.end())
                  {
                    m_interfaces[ptJson->m["Source"]->v]->buffers[1].push_back(strLine);
                  }
                }
                else
                {
                  // {{{ prep work
                  bool bSubResult = false;
                  strError.clear();
                  // }}}
                  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
                  {
                    // {{{ addInterface
                    if (ptJson->m["Function"]->v == "addInterface")
                    {
                      if (ptJson->m.find("Name") != ptJson->m.end() && !ptJson->m["Name"]->v.empty())
                      {
                        if (ptJson->m.find("Command") != ptJson->m.end() && !ptJson->m["Command"]->v.empty())
                        {
                          if (interfaceAdd(ptJson->m["Name"]->v, ptJson->m["Command"]->v, ((ptJson->m.find("Respawn") != ptJson->m.end() && ptJson->m["Respawn"]->v == "1")?true:false), strError))
                          {
                            bSubResult = true;
                            ssMessage.str("");
                            ssMessage << strPrefix << " [" << sockets[fds[i].fd] << "," << fds[i].fd << ",addInterface," << ptJson->m["Name"]->v << "]:  Interface added.";
                            log(ssMessage.str());
                          }
                          else
                          {
                            ssMessage.str("");
                            ssMessage << strPrefix << " error [" << sockets[fds[i].fd] << "," << fds[i].fd << ",addInterface," << ptJson->m["Name"]->v << "]:  " << strError;
                            log(ssMessage.str());
                          }
                        }
                        else
                        {
                          ssMessage.str("");
                          ssMessage << strPrefix << " error [" << sockets[fds[i].fd] << "," << fds[i].fd << ",addInterface," << ptJson->m["Name"]->v << "]:  Please provide the Command.";
                          log(ssMessage.str());
                        }
                      }
                      else
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " error [" << sockets[fds[i].fd] << "," << fds[i].fd << ",addInterface]:  Please provide the Name.";
                        log(ssMessage.str());
                      }
                    }
                    // }}}
                    // {{{ removeInterface
                    else if (ptJson->m["Function"]->v == "removeInterface")
                    {
                      if (ptJson->m.find("Name") != ptJson->m.end() && !ptJson->m["Name"]->v.empty())
                      {
                        if (m_interfaces.find(ptJson->m["Name"]->v) != m_interfaces.end())
                        {
                          bSubResult = true;
                          removals.push_back(ptJson->m["Name"]->v);
                        }
                        else
                        {
                          ssMessage.str("");
                          ssMessage << strPrefix << " error [" << sockets[fds[i].fd] << "," << fds[i].fd << ",removeInterface]:  Interface not found.";
                          log(ssMessage.str());
                        }
                      }
                      else
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " error [" << sockets[fds[i].fd] << "," << fds[i].fd << ",removeInterface]:  Please provide the Name.";
                        log(ssMessage.str());
                      }
                    }
                    // }}}
                    // {{{ shutdown
                    else if (ptJson->m["Function"]->v == "shutdown")
                    {
                      bSubResult = true;
                      setShutdown();
                      ssMessage.str("");
                      ssMessage << strPrefix << " [" << sockets[fds[i].fd] << "," << fds[i].fd << "]:  Shutdown requested." << endl;
                      log(ssMessage.str());
                      for (auto &j : m_interfaces)
                      {
                        string strSubLine;
                        Json *ptSubJson = new Json;
                        ptSubJson->insert("Function", "shutdown");
                        ptSubJson->json(strSubLine);
                        j.second->buffers[1].push_back(strSubLine);
                      }
                    }
                    // }}}
                  }
                  // {{{ post work
                  ptJson->insert("Status", ((bSubResult)?"okay":"error"));
                  if (!strError.empty())
                  {
                    ptJson->insert("Error", strError);
                  }
                  ptJson->json(strLine);
                  m_interfaces[sockets[fds[i].fd]]->buffers[1].push_back(strLine);
                  // }}}
                }
                delete ptJson;
              }
              m_interfaces[sockets[fds[i].fd]]->buffers[0].clear();
            }
            else
            {
              removals.push_back(sockets[fds[i].fd]);
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->read(" << errno << ") error [" << sockets[fds[i].fd] << "," << fds[i].fd << "]:  " << strerror(errno);
                log(ssMessage.str());
              }
            }
          }
          // }}}
          // {{{ write
          if (fds[i].revents & POLLOUT)
          {
            if ((nReturn = write(fds[i].fd, m_interfaces[sockets[fds[i].fd]]->strBuffers[1].c_str(), m_interfaces[sockets[fds[i].fd]]->strBuffers[1].size())) > 0)
            {
              m_interfaces[sockets[fds[i].fd]]->strBuffers[1].erase(0, nReturn);
            }
            else
            {
              removals.push_back(sockets[fds[i].fd]);
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->read(" << errno << ") error [" << sockets[fds[i].fd] << "," << fds[i].fd << "]:  " << strerror(errno);
                log(ssMessage.str());
              }
            }
          }
          // }}}
        }
      }
      else if (nReturn < 0)
      {
        bExit = true;
        bResult = false;
        ssMessage.str("");
        ssMessage << "poll(" << errno << ") " << strerror(errno);
        strError = ssMessage.str();
      }
      // {{{ post work
      delete[] fds;
      removals.sort();
      removals.unique();
      for (auto &i : removals)
      {
        interfaceRemove(i);
      }
      if (shutdown() && m_interfaces.empty())
      {
        bExit = true;
      }
      // }}}
    }
  }

  return bResult;
}
// }}}
// {{{ target()
void Hub::target(const string strTarget, Json *ptJson)
{
  string strJson;

  if (m_interfaces.find(strTarget) != m_interfaces.end())
  {
    ptJson->insert("Target", strTarget);
    m_interfaces[strTarget]->buffers[1].push_back(ptJson->json(strJson));
  }
}
// }}}
}
}
