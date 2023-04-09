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
  signal(SIGBUS, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGCONT, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGSEGV, SIG_IGN);
  signal(SIGWINCH, SIG_IGN);
  m_env = env;
}
// }}}
// {{{ ~Hub()
Hub::~Hub()
{
  for (auto &i : m_interfaces)
  {
    delete (i.second);
  }
}
// }}}
// {{{ add()
bool Hub::add(string strPrefix, const string strName, const string strAccessFunction, const string strCommand, const bool bRespawn, const bool bRestricted)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Hub::add()";
  ssMessage << strPrefix;
  if (m_interfaces.find(strName) == m_interfaces.end() || bRespawn)
  {
    int readpipe[2] = {-1, -1};
    if (pipe(readpipe) == 0)
    {
      int writepipe[2] = {-1, -1};
      if (pipe(writepipe) == 0)
      {
        pid_t nPid;
        if ((nPid = fork()) == 0)
        {
          char *args[100], *pszArgument;
          size_t unIndex = 0;
          string strArgument;
          stringstream ssCommand(strCommand);
          while (ssCommand >> strArgument)
          {
            pszArgument = new char[strArgument.size() + 1];
            strcpy(pszArgument, strArgument.c_str());
            args[unIndex++] = pszArgument;
          }
          args[unIndex] = NULL;
          close(writepipe[1]);
          close(readpipe[0]);
          dup2(writepipe[0], 0);
          close(writepipe[0]);
          dup2(readpipe[1], 1);
          close(readpipe[1]);
          execve(args[0], args, m_env);
          for (size_t i = 0; i < unIndex; i++)
          {
            delete[] args[i];
          }
          _exit(1);
        }
        else if (nPid > 0)
        {
          bResult = true;
          ssMessage << " [" << strName << "]:  Interface added.";
          close(writepipe[0]);
          close(readpipe[1]);
          m_pUtility->fdNonBlocking(readpipe[0], strError);
          m_pUtility->fdNonBlocking(writepipe[1], strError);
          if (m_interfaces.find(strName) == m_interfaces.end())
          {
            m_interfaces[strName] = new radialInterface;
          }
          m_interfaces[strName]->bRespawn = bRespawn;
          m_interfaces[strName]->bRestricted = bRestricted;
          m_interfaces[strName]->bShutdown = false;
          m_interfaces[strName]->CKill = 0;
          m_interfaces[strName]->CShutdown = 0;
          time(&(m_interfaces[strName]->CWrote));
          m_interfaces[strName]->fdRead = readpipe[0];
          m_interfaces[strName]->fdWrite = writepipe[1];
          m_interfaces[strName]->nPid = nPid;
          m_interfaces[strName]->strAccessFunction = strAccessFunction;
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
// {{{ chat()
void Hub::chat(const string strTarget, const string strMessage)
{
  Json *ptJson = new Json;

  ptJson->i("Function", "chat");
  ptJson->i("Target", strTarget);
  ptJson->i("Message", strMessage);
  target("irc", ptJson);
  delete ptJson;
}
// }}}
// {{{ interfaces()
void Hub::interfaces()
{
  Json *ptJson = new Json;

  ptJson->i("_s", "hub");
  ptJson->i("Function", "interfaces");
  ptJson->m["Interfaces"] = new Json;
  for (auto &i : m_interfaces)
  {
    stringstream ssPid;
    ssPid << i.second->nPid;
    ptJson->m["Interfaces"]->m[i.first] = new Json;
    ptJson->m["Interfaces"]->m[i.first]->i("AccessFunction", i.second->strAccessFunction);
    ptJson->m["Interfaces"]->m[i.first]->i("Command", i.second->strCommand);
    ptJson->m["Interfaces"]->m[i.first]->i("PID", ssPid.str(), 'n');
    ptJson->m["Interfaces"]->m[i.first]->i("Respawn", ((i.second->bRespawn)?"1":"0"), ((i.second->bRespawn)?'1':'0'));
    ptJson->m["Interfaces"]->m[i.first]->i("Restricted", ((i.second->bRestricted)?"1":"0"), ((i.second->bRestricted)?'1':'0'));
  }
  for (auto &i : m_interfaces)
  {
    target(i.first, ptJson);
  }
  delete ptJson;
}
// }}}
// {{{ links()
void Hub::links()
{
  Json *ptJson = new Json;

  ptJson->i("_s", "hub");
  ptJson->i("Function", "links");
  ptJson->m["Links"] = new Json;
  for (auto &link : m_links)
  {
    ptJson->m["Links"]->m[link->strNode] = new Json;
    ptJson->m["Links"]->m[link->strNode]->i("Server", link->strServer);
    ptJson->m["Links"]->m[link->strNode]->i("Port", link->strPort);
    ptJson->m["Links"]->m[link->strNode]->m["Interfaces"] = new Json;
    for (auto &interface : link->interfaces)
    {
      stringstream ssPid;
      ssPid << interface.second->nPid;
      ptJson->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first] = new Json;
      ptJson->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first]->i("AccessFunction", interface.second->strAccessFunction);
      ptJson->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first]->i("Command", interface.second->strCommand);
      ptJson->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first]->i("PID", ssPid.str(), 'n');
      ptJson->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first]->i("Respawn", ((interface.second->bRespawn)?"1":"0"), ((interface.second->bRespawn)?'1':'0'));
      ptJson->m["Links"]->m[link->strNode]->m["Interfaces"]->m[interface.first]->i("Restricted", ((interface.second->bRestricted)?"1":"0"), ((interface.second->bRestricted)?'1':'0'));
    }
  }
  for (auto &interface : m_interfaces)
  {
    if (interface.first != "link")
    {
      target(interface.first, ptJson);
    }
  }
  delete ptJson;
}
// }}}
// {{{ load()
bool Hub::load(string strPrefix, string &strError)
{
  bool bResult = false;
  ifstream inInterfaces;
  stringstream ssInterfaces, ssMessage;
  Json *ptInterfaces = NULL;

  strPrefix += "->Hub::load()";
  ssInterfaces << m_strData << "/interfaces.json";
  inInterfaces.open(ssInterfaces.str().c_str());
  if (inInterfaces)
  {
    string strLine;
    stringstream ssJson;
    while (getline(inInterfaces, strLine))
    {
      ssJson << strLine;
    }
    ptInterfaces = new Json(ssJson.str());
  }
  else
  {
    ssMessage.str("");
    ssMessage << "ifstream::open(" << errno << ") [" << ssInterfaces.str() << "] " << strerror(errno);
    strError = ssMessage.str();
  }
  inInterfaces.close();
  if (ptInterfaces != NULL)
  {
    bResult = true;
    if (exist(ptInterfaces, "log") && !empty(ptInterfaces->m["log"], "Command"))
    {
      if (!add(strPrefix, "log", ((!empty(ptInterfaces->m["log"], "AccessFunction"))?ptInterfaces->m["log"]->m["AccessFunction"]->v:"Function"), ptInterfaces->m["log"]->m["Command"]->v, ((!empty(ptInterfaces->m["log"], "Respawn") && ptInterfaces->m["log"]->m["Respawn"]->v == "1")?true:false), ((!empty(ptInterfaces->m["log"], "Restricted") && ptInterfaces->m["log"]->m["Restricted"]->v == "1")?true:false)))
      {
        bResult = false;
      }
      delete ptInterfaces->m["log"];
      ptInterfaces->m.erase("log");
    }
    for (auto &i : ptInterfaces->m)
    {
      if (!empty(i.second, "Command"))
      {
        if (!add(strPrefix, i.first, ((!empty(i.second, "AccessFunction"))?i.second->m["AccessFunction"]->v:"Function"), i.second->m["Command"]->v, ((!empty(i.second, "Respawn") && i.second->m["Respawn"]->v == "1")?true:false), ((!empty(i.second, "Restricted") && i.second->m["Restricted"]->v == "1")?true:false)))
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
    interfaces();
  }

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

  ptJson->i("Function", strFunction);
  ptJson->i("Message", strMessage);
  target("log", ptJson);
  delete ptJson;
}
// }}}
// {{{ monitor()
void Hub::monitor(string strPrefix)
{
  strPrefix += "->Hub::monitor()";
  if (!shutdown())
  {
    string strMessage;
    if (Base::monitor(strMessage) == 2)
    {
      stringstream ssMessage;
      ssMessage << strPrefix << "->Base::monitor():  " << strMessage;
      chat("#radial", ssMessage.str());
      notify(ssMessage.str());
      setShutdown(strPrefix);
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
  // {{{ prep work
  int fdUnix;
  string strError;
  stringstream ssUnix, ssMessage;
  Json *ptJson;
  strPrefix += "->Hub::process()";
  ssUnix << "/tmp/rdl_mgr";
  ::remove(ssUnix.str().c_str());
  // }}}
  if ((fdUnix = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0)
  {
    // {{{ prep work
    sockaddr_un addr;
    ssMessage.str("");
    ssMessage << strPrefix << "->socket():  Created manager socket.";
    log(ssMessage.str());
    memset(&addr, 0, sizeof(sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, ssUnix.str().c_str(), sizeof(addr.sun_path) - 1);
    // }}}
    if (bind(fdUnix, (sockaddr *)&addr, sizeof(sockaddr_un)) == 0)
    {
      // {{{ prep work
      ssMessage.str("");
      ssMessage << strPrefix << "->bind():  Bound manager socket.";
      log(ssMessage.str());
      chmod(ssUnix.str().c_str(), 00770);
      // }}}
      if (listen(fdUnix, 5) == 0)
      {
        // {{{ prep work
        ssMessage.str("");
        ssMessage << strPrefix << "->listen():  Listening to manager socket.";
        log(ssMessage.str());
        // }}}
        if (load(strPrefix, strError))
        {
          // {{{ prep work
          bool bExit = false;
          int nReturn;
          map<int, vector<string> > managers;
          pollfd *fds;
          size_t unIndex, unPosition;
          string strJson;
          time_t CShutdownTime[2] = {0, 0}, CTime;
          // }}}
          while (!bExit)
          {
            // {{{ prep work
            list<int> managerRemovals;
            list<string> removals;
            map<int, string> sockets;
            fds = new pollfd[(1+managers.size()+m_interfaces.size()*2)];
            unIndex = 0;
            fds[unIndex].fd = fdUnix;
            fds[unIndex].events = POLLIN;
            unIndex++;
            for (auto &manager : managers)
            {
              fds[unIndex].fd = manager.first;
              fds[unIndex].events = POLLIN;
              if (!manager.second[1].empty())
              {
                fds[unIndex].events |= POLLOUT;
              }
              unIndex++;
            }
            for (auto &interface : m_interfaces)
            {
              sockets[interface.second->fdRead] = interface.first;
              fds[unIndex].fd = interface.second->fdRead;
              fds[unIndex].events = POLLIN;
              unIndex++;
              sockets[interface.second->fdWrite] = interface.first;
              fds[unIndex].fd = -1;
              fds[unIndex].events = POLLOUT;
              if (!interface.second->strBuffers[1].empty())
              {
                fds[unIndex].fd = interface.second->fdWrite;
                time(&CTime);
                if (!interface.second->bShutdown && (CTime - interface.second->CWrote) > 10)
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << " [" << interface.first << "]:  Unable to write to the interface for the last 10 seconds.";
                  log(ssMessage.str());
                  setShutdown(strPrefix, interface.first);
                }
              }
              else
              {
                time(&(interface.second->CWrote));
              }
              unIndex++;
            }
            // }}}
            if ((nReturn = poll(fds, unIndex, 500)) > 0)
            {
              // {{{ accept
              if (fds[0].revents & POLLIN)
              {
                int fdClient;
                sockaddr_un cli_addr;
                socklen_t clilen = sizeof(cli_addr);
                if ((fdClient = accept(fds[0].fd, (sockaddr *)&cli_addr, &clilen)) >= 0)
                {
                  m_pUtility->fdNonBlocking(fdClient, strError);
                  managers[fdClient] = {"", ""};
                }
                else
                {
                  bExit = true;
                  cerr << strPrefix << "->accept(" << errno << ") error [" << ssUnix.str() << "]:  " << strerror(errno) << endl;
                }
              }
              // }}}
              for (size_t i = 1; i < unIndex; i++)
              {
                // {{{ interfaces
                if (sockets.find(fds[i].fd) != sockets.end())
                {
                  // {{{ read
                  if (fds[i].revents & (POLLHUP | POLLIN))
                  {
                    if (m_pUtility->fdRead(fds[i].fd, m_interfaces[sockets[fds[i].fd]]->strBuffers[0], nReturn))
                    {
                      while ((unPosition = m_interfaces[sockets[fds[i].fd]]->strBuffers[0].find("\n")) != string::npos)
                      {
                        ptJson = new Json(m_interfaces[sockets[fds[i].fd]]->strBuffers[0].substr(0, unPosition));
                        m_interfaces[sockets[fds[i].fd]]->strBuffers[0].erase(0, (unPosition + 1));
                        if (!empty(ptJson, "_t"))
                        {
                          if (!exist(ptJson, "_d"))
                          {
                            if (m_interfaces.find(ptJson->m["_t"]->v) != m_interfaces.end())
                            {
                              ptJson->i("_d", "t");
                              m_interfaces[ptJson->m["_t"]->v]->strBuffers[1].append(ptJson->j(strJson) + "\n");
                            }
                            else
                            {
                              list<radialLink *>::iterator linkIter = m_links.end();
                              for (auto i = m_links.begin(); linkIter == m_links.end() && i != m_links.end(); i++)
                              {
                                if ((*i)->interfaces.find(ptJson->m["_t"]->v) != (*i)->interfaces.end())
                                {
                                  linkIter = i;
                                }
                              }
                              if (linkIter != m_links.end() && m_interfaces.find("link") != m_interfaces.end())
                              {
                                ptJson->i("_d", "t");
                                ptJson->i("Node", (*linkIter)->strNode);
                                m_interfaces["link"]->strBuffers[1].append(ptJson->j(strJson) + "\n");
                              }
                              else if (!empty(ptJson, "_s") && m_interfaces.find(ptJson->m["_s"]->v) != m_interfaces.end())
                              {
                                ptJson->i("_d", "s");
                                ptJson->i("Status", "error");
                                ptJson->i("Error", "Interface does not exist.");
                                m_interfaces[ptJson->m["_s"]->v]->strBuffers[1].append(ptJson->j(strJson) + "\n");
                              }
                            }
                          }
                          else if (ptJson->m["_d"]->v == "t" && !empty(ptJson, "_s") && m_interfaces.find(ptJson->m["_s"]->v) != m_interfaces.end())
                          {
                            ptJson->i("_d", "s");
                            m_interfaces[ptJson->m["_s"]->v]->strBuffers[1].append(ptJson->j(strJson) + "\n");
                          }
                        }
                        else if (sockets[fds[i].fd] == "link" && !empty(ptJson, "Function") && ptJson->m["Function"]->v == "links")
                        {
                          for (auto &link : m_links)
                          {
                            for (auto &interface : link->interfaces)
                            {
                              delete interface.second;
                            }
                            link->interfaces.clear();
                            delete link;
                          }
                          m_links.clear();
                          if (exist(ptJson, "Links"))
                          {
                            for (auto &link : ptJson->m["Links"]->m)
                            {
                              radialLink *ptLink = new radialLink;
                              ptLink->strNode = link.first;
                              if (!empty(link.second, "Server"))
                              {
                                ptLink->strServer = link.second->m["Server"]->v;
                              }
                              if (!empty(link.second, "Port"))
                              {
                                ptLink->strPort = link.second->m["Port"]->v;
                              }
                              if (exist(link.second, "Interfaces"))
                              {
                                for (auto &interface : link.second->m["Interfaces"]->m)
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
                              m_links.push_back(ptLink);
                            }
                          }
                          links();
                        }
                        else
                        {
                          // {{{ prep work
                          bool bResult = false;
                          strError.clear();
                          // }}}
                          if (!empty(ptJson, "Function"))
                          {
                            // {{{ add
                            if (ptJson->m["Function"]->v == "add")
                            {
                              if (!empty(ptJson, "Name"))
                              {
                                bool bRespawn = false, bRestricted = false;
                                ifstream inInterfaces;
                                string strAccessFunction, strCommand;
                                stringstream ssInterfaces;
                                Json *ptInterfaces = NULL;
                                ssInterfaces << m_strData << "/interfaces.json";
                                inInterfaces.open(ssInterfaces.str().c_str());
                                if (inInterfaces)
                                {
                                  string strLine;
                                  stringstream ssJson;
                                  while (getline(inInterfaces, strLine))
                                  {
                                    ssJson << strLine;
                                  }
                                  ptInterfaces = new Json(ssJson.str());
                                }
                                inInterfaces.close();
                                if (ptInterfaces != NULL)
                                {
                                  if (exist(ptInterfaces, ptJson->m["Name"]->v))
                                  {
                                    if (!empty(ptInterfaces->m[ptJson->m["Name"]->v], "AccessFunction"))
                                    {
                                      strAccessFunction = ptInterfaces->m[ptJson->m["Name"]->v]->m["AccessFunction"]->v;
                                    }
                                    if (!empty(ptInterfaces->m[ptJson->m["Name"]->v], "Command"))
                                    {
                                      strCommand = ptInterfaces->m[ptJson->m["Name"]->v]->m["Command"]->v;
                                    }
                                    if (!empty(ptInterfaces->m[ptJson->m["Name"]->v], "Respawn") && ptInterfaces->m[ptJson->m["Name"]->v]->m["Respawn"]->v == "1")
                                    {
                                      bRespawn = true;
                                    }
                                    if (!empty(ptInterfaces->m[ptJson->m["Name"]->v], "Restricted") && ptInterfaces->m[ptJson->m["Name"]->v]->m["Restricted"]->v == "1")
                                    {
                                      bRestricted = true;
                                    }
                                  }
                                  delete ptInterfaces;
                                }
                                if (!empty(ptJson, "AccessFunction"))
                                {
                                  strAccessFunction = ptJson->m["AccessFunction"]->v;
                                }
                                if (!empty(ptJson, "Command"))
                                {
                                  strCommand = ptJson->m["Command"]->v;
                                }
                                if (!empty(ptJson, "Respawn"))
                                {
                                  bRespawn = ((ptJson->m["Respawn"]->v == "1")?true:false);
                                }
                                if (!empty(ptJson, "Restricted"))
                                {
                                  bRestricted = ((ptJson->m["Restricted"]->v == "1")?true:false);
                                }
                                if (add(strPrefix, ptJson->m["Name"]->v, strAccessFunction, strCommand, bRespawn, bRestricted))
                                {
                                  bResult = true;
                                  interfaces();
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
                              for (auto &j : m_interfaces)
                              {
                                stringstream ssPid;
                                ssPid << j.second->nPid;
                                ptJson->m["Response"]->m[j.first] = new Json;
                                ptJson->m["Response"]->m[j.first]->i("AccessFunction", j.second->strAccessFunction);
                                ptJson->m["Response"]->m[j.first]->i("Command", j.second->strCommand);
                                ptJson->m["Response"]->m[j.first]->i("PID", ssPid.str(), 'n');
                                ptJson->m["Response"]->m[j.first]->i("Respawn", ((j.second->bRespawn)?"1":"0"), ((j.second->bRespawn)?'1':'0'));
                                ptJson->m["Response"]->m[j.first]->i("Restricted", ((j.second->bRestricted)?"1":"0"), ((j.second->bRestricted)?'1':'0'));
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
                              if (!empty(ptJson, "Name"))
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
                              setShutdown(strPrefix, ((!empty(ptJson, "Target"))?ptJson->m["Target"]->v:""));
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
                          ptJson->i("_d", "s");
                          ptJson->i("Status", ((bResult)?"okay":"error"));
                          if (!strError.empty())
                          {
                            ptJson->i("Error", strError);
                          }
                          m_interfaces[sockets[fds[i].fd]]->strBuffers[1].append(ptJson->j(strJson) + "\n");
                          // }}}
                        }
                        delete ptJson;
                      }
                    }
                    else
                    {
                      removals.push_back(sockets[fds[i].fd]);
                      if (nReturn < 0 || errno == EINVAL)
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << "->Utility::fdRead(" << errno << ") error [" << sockets[fds[i].fd] << "," << fds[i].fd << "]:  " << strerror(errno);
                        if (errno == EINVAL)
                        {
                          ssMessage << " --- POSSIBLE CORE DUMP";
                        }
                        log(ssMessage.str());
                      }
                    }
                  }
                  // }}}
                  // {{{ write
                  if (fds[i].revents & POLLOUT)
                  {
                    if (m_pUtility->fdWrite(fds[i].fd, m_interfaces[sockets[fds[i].fd]]->strBuffers[1], nReturn))
                    {
                      time(&(m_interfaces[sockets[fds[i].fd]]->CWrote));
                    }
                    else
                    {
                      removals.push_back(sockets[fds[i].fd]);
                      if (nReturn < 0)
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << "->Utility::fdWrite(" << errno << ") error [" << sockets[fds[i].fd] << "," << fds[i].fd << "]:  " << strerror(errno);
                        log(ssMessage.str());
                      }
                    }
                  }
                  // }}}
                }
                // }}}
                // {{{ managers
                else if (managers.find(fds[i].fd) != managers.end())
                {
                  // {{{ read
                  if (fds[i].revents & POLLIN)
                  {
                    if (m_pUtility->fdRead(fds[i].fd, managers[fds[i].fd][0], nReturn))
                    {
                      while ((unPosition = managers[fds[i].fd][0].find("\n")) != string::npos)
                      {
                        bool bProcessed = false;
                        ptJson = new Json(managers[fds[i].fd][0].substr(0, unPosition));
                        managers[fds[i].fd][0].erase(0, (unPosition + 1));
                        strError.clear();
                        if (!empty(ptJson, "Function"))
                        {
                          ifstream inInterfaces;
                          string strInterface;
                          stringstream ssInterfaces;
                          Json *ptInterfaces = NULL;
                          if (!empty(ptJson, "Interface"))
                          {
                            strInterface = ptJson->m["Interface"]->v;
                          }
                          ssInterfaces << m_strData << "/interfaces.json";
                          inInterfaces.open(ssInterfaces.str().c_str());
                          if (inInterfaces)
                          {
                            string strLine;
                            stringstream ssJson;
                            while (getline(inInterfaces, strLine))
                            {
                              ssJson << strLine;
                            }
                            ptInterfaces = new Json(ssJson.str());
                          }
                          else
                          {
                            ssMessage.str("");
                            ssMessage << "ifstream::open(" << errno << ") [" << ssInterfaces.str() << "] " << strerror(errno);
                            strError = ssMessage.str();
                          }
                          inInterfaces.close();
                          if (ptInterfaces != NULL)
                          {
                            ssInterfaces.str("");
                            for (auto interfaceIter = ptInterfaces->m.begin(); interfaceIter != ptInterfaces->m.end(); interfaceIter++)
                            {
                              if (interfaceIter != ptInterfaces->m.begin())
                              {
                                ssInterfaces << ", ";
                              }
                              ssInterfaces << interfaceIter->first;
                            }
                            if (ptJson->m["Function"]->v == "list")
                            {
                              bProcessed = true;
                              ptJson->i("Response", ssInterfaces.str());
                            }
                            else if (ptJson->m["Function"]->v == "start")
                            {
                              if (!strInterface.empty())
                              {
                                if (exist(ptInterfaces, strInterface))
                                {
                                  if (m_interfaces.find(strInterface) == m_interfaces.end())
                                  {
                                    if (add(strPrefix, strInterface, ((!empty(ptInterfaces->m[strInterface], "AccessFunction"))?ptInterfaces->m[strInterface]->m["AccessFunction"]->v:"Function"), ptInterfaces->m[strInterface]->m["Command"]->v, ((!empty(ptInterfaces->m[strInterface], "Respawn") && ptInterfaces->m[strInterface]->m["Respawn"]->v == "1")?true:false), ((!empty(ptInterfaces->m[strInterface], "Restricted") && ptInterfaces->m[strInterface]->m["Restricted"]->v == "1")?true:false)))
                                    {
                                      bProcessed = true;
                                      interfaces();
                                    }
                                    else
                                    {
                                      strError = "Failed to start interface.";
                                    }
                                  }
                                  else
                                  {
                                    bProcessed = true;
                                    strError = "Interface is already started.";
                                  }
                                }
                                else
                                {
                                  ssMessage.str("");
                                  ssMessage << "Please provide a valid Interface:  " << ssInterfaces.str() << ".";
                                  strError = ssMessage.str();
                                }
                              }
                              else
                              {
                                strError = "Please provide the Interface.";
                              }
                            }
                            else if (ptJson->m["Function"]->v == "status")
                            {
                              if (!strInterface.empty())
                              {
                                if (exist(ptInterfaces, strInterface))
                                {
                                  bProcessed = true;
                                  ptJson->i("Response", ((m_interfaces.find(strInterface) != m_interfaces.end())?"online":"offline"));
                                }
                                else
                                {
                                  ssMessage.str("");
                                  ssMessage << "Please provide a valid Interface:  " << ssInterfaces.str() << ".";
                                  strError = ssMessage.str();
                                }
                              }
                              else
                              {
                                strError = "Please provide the Interface.";
                              }
                            }
                            else if (ptJson->m["Function"]->v == "stop")
                            {
                              if (!strInterface.empty())
                              {
                                if (exist(ptInterfaces, strInterface))
                                {
                                  bProcessed = true;
                                  if (m_interfaces.find(strInterface) != m_interfaces.end())
                                  {
                                    setShutdown(strPrefix, strInterface, true);
                                  }
                                  else
                                  {
                                    strError = "Interface is already stopped.";
                                  }
                                }
                                else
                                {
                                  ssMessage.str("");
                                  ssMessage << "Please provide a valid Interface:  " << ssInterfaces.str() << ".";
                                  strError = ssMessage.str();
                                }
                              }
                              else
                              {
                                strError = "Please provide the Interface.";
                              }
                            }
                            else
                            {
                              strError = "Please provide a valid Function:  list, start, status, stop.";
                            }
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
                        managers[fds[i].fd][1] = ptJson->j(strJson) + "\n";
                        delete ptJson;
                      }
                    }
                    else
                    {
                      managerRemovals.push_back(fds[i].fd);
                      if (nReturn < 0)
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << "->Utility::fdRead(" << errno << ") error [manager," << fds[i].fd << "]:  " << strerror(errno);
                        log(ssMessage.str());
                      }
                    }
                  }
                  // }}}
                  // {{{ write
                  if (fds[i].revents & POLLOUT)
                  {
                    if (m_pUtility->fdWrite(fds[i].fd, managers[fds[i].fd][1], nReturn))
                    {
                      if (managers[fds[i].fd][1].empty())
                      {
                        managerRemovals.push_back(fds[i].fd);
                      }
                    }
                    else
                    {
                      managerRemovals.push_back(fds[i].fd);
                      if (nReturn < 0)
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << "->Utility::fdWrite(" << errno << ") error [manager," << fds[i].fd << "]:  " << strerror(errno);
                        log(ssMessage.str());
                      }
                    }
                  }
                  // }}}
                }
                // }}}
              }
            }
            else if (nReturn < 0 && errno != EINTR)
            {
              bExit = true;
              cerr << strPrefix << "->poll(" << errno << ") " << strerror(errno) << endl;
            }
            // {{{ post work
            delete[] fds;
            managerRemovals.sort();
            managerRemovals.unique();
            while (!managerRemovals.empty())
            {
              managers.erase(managerRemovals.front());
              close(managerRemovals.front());
              managerRemovals.pop_front();
            }
            time(&CTime);
            for (auto &i : m_interfaces)
            {
              if (i.second->CShutdown > 0 && (CTime - i.second->CShutdown) > 20)
              {
                ssMessage.str("");
                ssMessage << strPrefix << " [" << i.first << "]:  Sent terminate (SIGTERM) signal.";
                log(ssMessage.str());
                kill(i.second->nPid, SIGTERM);
                i.second->CShutdown = 0;
                i.second->CKill = CTime;
              }
              else if (i.second->CKill > 0 && (CTime - i.second->CKill) > 10)
              {
                ssMessage.str("");
                ssMessage << strPrefix << " [" << i.first << "]:  Sent kill (SIGKILL) signal.";
                log(ssMessage.str());
                kill(i.second->nPid, SIGKILL);
                i.second->CKill = 0;
              }
            }
            removals.sort();
            removals.unique();
            if (!removals.empty())
            {
              for (auto &i : removals)
              {
                remove(strPrefix, i);
              }
              interfaces();
            }
            monitor(strPrefix);
            if (shutdown())
            {
              if (m_interfaces.empty())
              {
                bExit = true;
              }
              else if (m_interfaces.size() == 1)
              {
                if (m_interfaces.find("log") != m_interfaces.end())
                {
                  time(&CShutdownTime[1]);
                  if (CShutdownTime[0] == 0)
                  {
                    CShutdownTime[0] = CShutdownTime[1];
                    ssMessage.str("");
                    ssMessage << strPrefix << " [log]:  Interface shutdown.";
                    log(ssMessage.str());
                  }
                  if (CShutdownTime[1] - CShutdownTime[0] > 2)
                  {
                    setShutdown(strPrefix, "log", true);
                  }
                }
                else
                {
                  bExit = true;
                }
              }
            }
            // }}}
          }
          // {{{ post work
          for (auto &manager : managers)
          {
            close(manager.first);
          }
          managers.clear();
          // }}}
        }
        else
        {
          cerr << strPrefix << "->Hub::load() error:  " << strError << endl;
        }
      }
      else
      {
        cerr << strPrefix << "->listen(" << errno << ") error [" << ssUnix.str() << "]:  " << strerror(errno) << endl;
      }
    }
    else
    {
      cerr << strPrefix << "->bound(" << errno << ") error [" << ssUnix.str() << "]:  " << strerror(errno) << endl;
    }
    // {{{ post work
    close(fdUnix);
    // }}}
  }
  else
  {
    cerr << strPrefix << "->socket(" << errno << ") error [" << ssUnix.str() << "]:  " << strerror(errno);
  }
  // {{{ post work
  ::remove(ssUnix.str().c_str());
  // }}}
}
// }}}
// {{{ remove()
void Hub::remove(string strPrefix, const string strName)
{
  stringstream ssMessage;

  strPrefix += "->Hub::remove()";
  if (m_interfaces.find(strName) != m_interfaces.end())
  {
    m_interfaces[strName]->CKill = 0;
    m_interfaces[strName]->CShutdown = 0;
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
      add(strPrefix, strName, m_interfaces[strName]->strAccessFunction, m_interfaces[strName]->strCommand, true, m_interfaces[strName]->bRestricted);
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
  ptJson->i("_s", "hub");
  ptJson->i("Function", "shutdown");
  ptJson->j(strJson);
  delete ptJson;
  if (strTarget.empty())
  {
    Base::setShutdown();
  }
  for (auto &interface : m_interfaces)
  {
    if (!interface.second->bShutdown && (strTarget.empty() || interface.first == strTarget) && (interface.first != "log" || strTarget == "log"))
    {
      ssMessage.str("");
      ssMessage << strPrefix << " [" << interface.first << "]:  Interface shutdown.";
      log(ssMessage.str());
      if (bStop)
      {
        interface.second->bRespawn = false;
      }
      interface.second->bShutdown = true;
      interface.second->strBuffers[1].append(strJson + "\n");
      time(&(interface.second->CShutdown));
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
    ptJson->i("_t", strTarget);
    ptJson->i("_d", "t");
    m_interfaces[strTarget]->strBuffers[1].append(ptJson->j(strJson) + "\n");
  }
}
// }}}
}
}
