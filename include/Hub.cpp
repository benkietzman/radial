// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Hub.cpp
// author     : Ben Kietzman
// begin      : 2022-04-19
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Hub"
// }}}
extern "C++"
{ 
namespace radial
{
// {{{ Hub()
Hub::Hub(string strPrefix, int argc, char **argv, char **env, void (*function)(const int)) : Base(argc, argv)
{
  strPrefix += "->Hub::Hub()";
  sethandles(function);
  sigignore(SIGBUS);
  sigignore(SIGCHLD);
  sigignore(SIGCONT);
  sigignore(SIGPIPE);
  sigignore(SIGSEGV);
  sigignore(SIGWINCH);
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
    delete (i.second);
  }
}
// }}}
// {{{ add()
bool Hub::add(string strPrefix, const string strName, const string strCommand, const bool bRespawn, const bool bRestricted)
{
  bool bResult = false;
  stringstream ssMessage;

  strPrefix += "->Hub::add()";
  ssMessage << strPrefix;
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
          ssMessage << " [" << strName << "]:  Interface added." << endl;
          close(writepipe[0]);
          close(readpipe[1]);
          if (m_interfaces.find(strName) == m_interfaces.end())
          {
            m_interfaces[strName] = new radialHubInterface;
          }
          m_interfaces[strName]->bRespawn = bRespawn;
          m_interfaces[strName]->bRestricted = bRestricted;
          m_interfaces[strName]->bShutdown = false;
          m_interfaces[strName]->fdRead = readpipe[0];
          m_interfaces[strName]->fdWrite = writepipe[1];
          m_interfaces[strName]->nPid = nPid;
          m_interfaces[strName]->strCommand = strCommand;
        }
        else
        {
          ssMessage << "->fork(" << errno << ") error [" << strName << "]:  " << strerror(errno);
        }
      }
      else
      {
        ssMessage << "->pipe(write," << errno << ") error [" << strName << "]:  " << strerror(errno);
      }
    }
    else
    {
      ssMessage << "->pipe(read," << errno << ") error [" << strName << "]  " << strerror(errno);
    }
  }
  else
  {
    ssMessage << " error:  Interface already exists.";
  }
  log(ssMessage.str());

  return bResult;
}
// }}}
// {{{ alert()
void Hub::alert(const string strMessage)
{
  log("alert", strMessage);
}
// }}}
// {{{ load()
bool Hub::load(string strPrefix, string &strError)
{
  bool bResult = false;
  ifstream inInterfaces;
  stringstream ssInterfaces, ssMessage;

  strPrefix += "->Hub::load()";
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
        if (!add(strPrefix, i.first, i.second->m["Command"]->v, ((i.second->m.find("Respawn") != i.second->m.end() && i.second->m["Respawn"]->v == "1")?true:false), ((i.second->m.find("Restricted") != i.second->m.end() && i.second->m["Restricted"]->v == "1")?true:false)))
        {
          bResult = false;
        }
      }
      else
      {
        bResult = false;
        ssMessage.str("");
        ssMessage << strPrefix << " error [" << i.first << "] Please provide the Command.";
        log(ssMessage.str());
      }
    }
    delete ptInterfaces;
    if (!bResult)
    {
      ssMessage.str("");
      ssMessage << "Encountered an error adding one or more interfaces.  Please check log for more details.";
      strError = ssMessage.str();
    }
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
// {{{ monitor()
void Hub::monitor(string strPrefix)
{
  size_t unResult;
  string strMessage;
  stringstream ssMessage;

  strPrefix += "->Hub::monitor()";
  if ((unResult = Base::monitor(strMessage)) > 0)
  {
    ssMessage << strPrefix << "->Base::monitor():  " << strMessage;
    if (unResult == 2)
    {
      notify(ssMessage.str());
      setShutdown(strPrefix);
    }
    else
    {
      log(ssMessage.str());
    }
  }
}
// }}}
// {{{ notify()
void Hub::notify(const string strMessage)
{
  log("alert", strMessage);
}
// }}}
// {{{ process()
void Hub::process(string strPrefix)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Hub::process()";
  if (load(strPrefix, strError))
  {
    // {{{ prep work
    bool bExit = false;
    char szBuffer[65536];
    int nReturn;
    pollfd *fds;
    size_t unIndex, unPosition;
    string strLine;
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
      if ((nReturn = poll(fds, unIndex, 100)) > 0)
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
                if (ptJson->m.find("_target") != ptJson->m.end() && !ptJson->m["_target"]->v.empty())
                {
                  if (m_interfaces.find(ptJson->m["_target"]->v) != m_interfaces.end())
                  {
                    if (ptJson->m["_target"]->v != sockets[fds[i].fd])
                    {
                      m_interfaces[ptJson->m["_target"]->v]->buffers[1].push_back(strLine);
                    }
                    else if (ptJson->m.find("_unique") != ptJson->m.end() && !ptJson->m["_unique"]->v.empty() && ptJson->m.find("_source") != ptJson->m.end() && !ptJson->m["_source"]->v.empty() && m_interfaces.find(ptJson->m["_source"]->v) != m_interfaces.end())
                    {
                      m_interfaces[ptJson->m["_source"]->v]->buffers[1].push_back(strLine);
                    }
                  }
                  else if (ptJson->m.find("_source") != ptJson->m.end() && !ptJson->m["_source"]->v.empty())
                  {
                    ptJson->insert("Status", "error");
                    ptJson->insert("Error", "Interface does not exist.");
                    ptJson->json(strLine);
                    m_interfaces[sockets[fds[i].fd]]->buffers[1].push_back(strLine);
                  }
                }
                else
                {
                  // {{{ prep work
                  bool bResult = false;
                  strError.clear();
                  // }}}
                  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
                  {
                    // {{{ add
                    if (ptJson->m["Function"]->v == "add")
                    {
                      if (ptJson->m.find("Name") != ptJson->m.end() && !ptJson->m["Name"]->v.empty())
                      {
                        if (ptJson->m.find("Command") != ptJson->m.end() && !ptJson->m["Command"]->v.empty())
                        {
                          if (add(strPrefix, ptJson->m["Name"]->v, ptJson->m["Command"]->v, ((ptJson->m.find("Respawn") != ptJson->m.end() && ptJson->m["Respawn"]->v == "1")?true:false), ((ptJson->m.find("Restricted") != ptJson->m.end() && ptJson->m["Restricted"]->v == "1")?true:false)))
                          {
                            bResult = true;
                          }
                        }
                        else
                        {
                          ssMessage.str("");
                          ssMessage << strPrefix << " error [" << sockets[fds[i].fd] << "," << fds[i].fd << ",add," << ptJson->m["Name"]->v << "]:  Please provide the Command.";
                          log(ssMessage.str());
                        }
                      }
                      else
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " error [" << sockets[fds[i].fd] << "," << fds[i].fd << ",add]:  Please provide the Name.";
                        log(ssMessage.str());
                      }
                    }
                    // }}}
                    // {{{ list
                    else if (ptJson->m["Function"]->v == "list")
                    {
                      bResult = true;
                      ptJson->m["Response"] = new Json;
                      for (auto &i : m_interfaces)
                      {
                        stringstream ssPid;
                        ssPid << i.second->nPid;
                        ptJson->m["Response"]->m[i.first] = new Json;
                        ptJson->m["Response"]->m[i.first]->insert("Command", i.second->strCommand);
                        ptJson->m["Response"]->m[i.first]->insert("PID", ssPid.str(), 'n');
                        ptJson->m["Response"]->m[i.first]->insert("Respawn", ((i.second->bRespawn)?"1":"0"), ((i.second->bRespawn)?'1':'0'));
                        ptJson->m["Response"]->m[i.first]->insert("Restricted", ((i.second->bRestricted)?"1":"0"), ((i.second->bRestricted)?'1':'0'));
                      }
                    }
                    // }}}
                    // {{{ ping
                    else if (ptJson->m["Function"]->v == "ping")
                    {
                      bResult = true;
                    }
                    // }}}
                    // {{{ remove
                    else if (ptJson->m["Function"]->v == "remove")
                    {
                      if (ptJson->m.find("Name") != ptJson->m.end() && !ptJson->m["Name"]->v.empty())
                      {
                        if (m_interfaces.find(ptJson->m["Name"]->v) != m_interfaces.end())
                        {
                          bResult = true;
                          setShutdown(strPrefix, ptJson->m["Name"]->v, true);
                        }
                        else
                        {
                          ssMessage.str("");
                          ssMessage << strPrefix << " error [" << sockets[fds[i].fd] << "," << fds[i].fd << ",remove]:  Interface not found.";
                          log(ssMessage.str());
                        }
                      }
                      else
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << " error [" << sockets[fds[i].fd] << "," << fds[i].fd << ",remove]:  Please provide the Name.";
                        log(ssMessage.str());
                      }
                    }
                    // }}}
                    // {{{ shutdown
                    else if (ptJson->m["Function"]->v == "shutdown")
                    {
                      bResult = true;
                      setShutdown(strPrefix, ((ptJson->m.find("Target") != ptJson->m.end())?ptJson->m["Target"]->v:""));
                    }
                    // }}}
                    // {{{ invalid
                    else
                    {
                      strError = "Please provide a valid Function:  add, list, ping, remove, shutdown.";
                    }
                    // }}}
                  }
                  // {{{ post work
                  ptJson->insert("Status", ((bResult)?"okay":"error"));
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
      else if (nReturn < 0 && errno != EINTR)
      {
        bExit = true;
        ssMessage.str("");
        ssMessage << strPrefix << "->poll(" << errno << ") " << strerror(errno);
        notify(ssMessage.str());
      }
      // {{{ post work
      delete[] fds;
      removals.sort();
      removals.unique();
      for (auto &i : removals)
      {
        remove(strPrefix, i);
      }
      monitor(strPrefix);
      if (shutdown())
      {
        if (m_interfaces.empty())
        {
          bExit = true;
        }
        else if (m_interfaces.size() == 1 && m_interfaces.find("log") != m_interfaces.end())
        {
          setShutdown(strPrefix, "log");
        }
      }
      // }}}
    }
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->Hub::load() error:  " << strError;
    notify(ssMessage.str());
  }
}
// }}}
// {{{ remove()
void Hub::remove(string strPrefix, const string strName)
{
  stringstream ssMessage;

  strPrefix += "->Hub::remove()";
  if (m_interfaces.find(strName) != m_interfaces.end())
  {
    close(m_interfaces[strName]->fdRead);
    m_interfaces[strName]->fdRead = -1;
    close(m_interfaces[strName]->fdWrite);
    m_interfaces[strName]->fdWrite = -1;
    m_interfaces[strName]->strBuffers[0].clear();
    m_interfaces[strName]->strBuffers[1].clear();
    ssMessage.str("");
    ssMessage << strPrefix << " [" << strName << "]:  Interface removed.";
    log(ssMessage.str());
    if (!shutdown() && m_interfaces[strName]->bRespawn)
    {
      add(strPrefix, strName, m_interfaces[strName]->strCommand, true, m_interfaces[strName]->bRestricted);
    }
    else
    {
      delete m_interfaces[strName];
      m_interfaces.erase(strName);
    }
  }
}
// }}}
// {{{ setShutdown()
void Hub::setShutdown(string strPrefix, const string strTarget, const bool bStop)
{
  string strJson;
  stringstream ssMessage;
  Json *ptJson = new Json;

  strPrefix += "->shutdown()";
  ptJson->insert("_source", "hub");
  ptJson->insert("Function", "shutdown");
  ptJson->json(strJson);
  delete ptJson;
  if (strTarget.empty())
  {
    Base::setShutdown();
  }
  for (auto &i : m_interfaces)
  {
    if (!i.second->bShutdown && (strTarget.empty() || i.first == strTarget) && (i.first != "log" || strTarget == "log"))
    {
      ssMessage.str("");
      ssMessage << strPrefix << " [" << i.first << "]:  Interface shutdown.";
      log(ssMessage.str());
      if (bStop)
      {
        i.second->bRespawn = false;
      }
      i.second->bShutdown = true;
      i.second->buffers[1].push_back(strJson);
    }
  }
}
// }}}
// {{{ target()
void Hub::target(const string strTarget, Json *ptJson)
{
  string strJson;

  if (m_interfaces.find(strTarget) != m_interfaces.end())
  {
    ptJson->insert("_target", strTarget);
    m_interfaces[strTarget]->buffers[1].push_back(ptJson->json(strJson));
  }
}
// }}}
}
}
