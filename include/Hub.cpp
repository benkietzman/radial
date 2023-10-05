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
  m_CLoadModify = 0;
  m_env = env;
}
// }}}
// {{{ ~Hub()
Hub::~Hub()
{
  for (auto &i : m_i)
  {
    delete (i.second);
  }
}
// }}}
// {{{ add()
bool Hub::add(string strPrefix, const string strName, const string strAccessFunction, const string strCommand, const unsigned long ulMemory, const bool bRespawn, const bool bRestricted)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Hub::add()";
  ssMessage << strPrefix;
  if (m_i.find(strName) == m_i.end() || bRespawn)
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
          if (m_i.find(strName) == m_i.end())
          {
            m_i[strName] = new radialInterface;
          }
          m_i[strName]->bRespawn = bRespawn;
          m_i[strName]->bRestricted = bRestricted;
          m_i[strName]->bShutdown = false;
          m_i[strName]->CKill = 0;
          m_i[strName]->CMonitor[0] = 0;
          m_i[strName]->CMonitor[1] = 0;
          m_i[strName]->CShutdown = 0;
          time(&(m_i[strName]->CWrote));
          m_i[strName]->fdRead = readpipe[0];
          m_i[strName]->fdWrite = writepipe[1];
          m_i[strName]->nPid = nPid;
          m_i[strName]->strAccessFunction = strAccessFunction;
          m_i[strName]->strCommand = strCommand;
          m_i[strName]->ulMemory = ulMemory;
          m_i[strName]->unMonitor = 0;
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

  ptJson->i("Function", "interfaces");
  ptJson->m["Interfaces"] = new Json;
  for (auto &i : m_i)
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
  for (auto &i : m_i)
  {
    target(i.first, ptJson, "hub");
  }
  delete ptJson;
}
// }}}
// {{{ links()
void Hub::links()
{
  Json *ptJson = new Json;

  ptJson->i("Function", "links");
  ptJson->m["Links"] = new Json;
  for (auto &link : m_l)
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
  for (auto &interface : m_i)
  {
    if (interface.first != "link")
    {
      Json *ptSubJson = new Json(ptJson);
      target(interface.first, ptSubJson, "hub");
      delete ptSubJson;
    }
  }
  delete ptJson;
}
// }}}
// {{{ load()
bool Hub::load(string strPrefix, string &strError)
{
  bool bResult = false;
  stringstream ssInterfaces, ssMessage;
  struct stat tStat;

  strPrefix += "->Hub::load()";
  ssInterfaces << m_strData << "/interfaces.json";
  if (stat(ssInterfaces.str().c_str(), &tStat) == 0)
  {
    if (m_CLoadModify != tStat.st_mtime)
    {
      ifstream inInterfaces;
      Json *ptInterfaces = NULL;
      inInterfaces.open(ssInterfaces.str().c_str());
      if (inInterfaces)
      {
        string strLine;
        stringstream ssJson;
        m_CLoadModify = tStat.st_mtime;
        while (getline(inInterfaces, strLine))
        {
          ssJson << strLine;
        }
        ptInterfaces = new Json(ssJson.str());
        if (ptInterfaces != NULL)
        {
          if (!ptInterfaces->m.empty())
          {
            bResult = true;
            if (exist(ptInterfaces, "log") && !empty(ptInterfaces->m["log"], "Command") && m_i.find("log") == m_i.end())
            {
              stringstream ssMemory((!empty(ptInterfaces->m["log"], "Memory"))?ptInterfaces->m["log"]->m["Memory"]->v:"40");
              unsigned long ulMemory;
              ssMemory >> ulMemory;
              ulMemory *= 1024;
              if (!add(strPrefix, "log", ((!empty(ptInterfaces->m["log"], "AccessFunction"))?ptInterfaces->m["log"]->m["AccessFunction"]->v:"Function"), ptInterfaces->m["log"]->m["Command"]->v, ulMemory, ((!empty(ptInterfaces->m["log"], "Respawn") && ptInterfaces->m["log"]->m["Respawn"]->v == "1")?true:false), ((!empty(ptInterfaces->m["log"], "Restricted") && ptInterfaces->m["log"]->m["Restricted"]->v == "1")?true:false)))
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
                stringstream ssMemory((!empty(i.second, "Memory"))?i.second->m["Memory"]->v:"40");
                unsigned long ulMemory;
                ssMemory >> ulMemory;
                ulMemory *= 1024;
                if (m_i.find(i.first) != m_i.end())
                {
                  m_i[i.first]->bRespawn = ((!empty(i.second, "Respawn") && i.second->m["Respawn"]->v == "1")?true:false);
                  m_i[i.first]->bRestricted = ((!empty(i.second, "Restricted") && i.second->m["Restricted"]->v == "1")?true:false);
                  m_i[i.first]->strAccessFunction = ((!empty(i.second, "AccessFunction"))?i.second->m["AccessFunction"]->v:"Function");
                  if (m_i[i.first]->strCommand != i.second->m["Command"]->v)
                  {
                    m_i[i.first]->strCommand = i.second->m["Command"]->v;
                    if (!m_i[i.first]->bShutdown && m_i[i.first]->bRespawn)
                    {
                      ssMessage.str("");
                      ssMessage << strPrefix << " [" << i.first << "]:  Restarting interface due to configuration change.";
                      log(ssMessage.str());
                      setShutdown(strPrefix, i.first);
                    }
                  }
                  m_i[i.first]->ulMemory = ulMemory;
                }
                else if (!empty(i.second, "Respawn") && i.second->m["Respawn"]->v == "1" && !add(strPrefix, i.first, ((!empty(i.second, "AccessFunction"))?i.second->m["AccessFunction"]->v:"Function"), i.second->m["Command"]->v, ulMemory, ((!empty(i.second, "Respawn") && i.second->m["Respawn"]->v == "1")?true:false), ((!empty(i.second, "Restricted") && i.second->m["Restricted"]->v == "1")?true:false)))
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
            for (auto &i : m_i)
            {
              if (i.first != "log")
              {
                bool bFound = false;
                for (auto j = ptInterfaces->m.begin(); !bFound && j != ptInterfaces->m.end(); j++)
                {
                  if (i.first == j->first)
                  {
                    bFound = true;
                  }
                }
                if (!bFound)
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << " [" << i.first << "]:  Stopping interface due to non-existence in configuration.";
                  log(ssMessage.str());
                  setShutdown(strPrefix, i.first, true);
                }
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
          else
          {
            ssMessage.str("");
            ssMessage << "[" << ssInterfaces.str() << "] No interfaces configured.";
            strError = ssMessage.str();
          }
        }
        else
        {
          ssMessage.str("");
          ssMessage << "[" << ssInterfaces.str() << "] Invalid configuration.";
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
    }
    else
    {
      bResult = true;
    }
  }
  else
  {
    ssMessage.str("");
    ssMessage << "stat(" << errno << ") [" << ssInterfaces.str() << "] " << strerror(errno);
    strError = ssMessage.str();
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
void Hub::monitor(string strPrefix, const pid_t nPid)
{
  strPrefix += "->Hub::monitor()";
  if (!shutdown())
  {
    string strMessage;
    if (Base::monitor(nPid, strMessage) == 2)
    {
      stringstream ssMessage;
      ssMessage << strPrefix << "->Base::monitor() [" << m_strNode << "]:  " << strMessage;
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
          map<int, vector<string> > m, s;
          map<string, map<string, size_t> > t;
          pid_t nPid = getpid();
          pollfd *fds;
          size_t unIndex, unPosition;
          string strJson, strValue;
          time_t CLoad, CShutdownTime[2] = {0, 0}, CThroughput, CTime;
          time(&CLoad);
          CThroughput = CLoad;
          // }}}
          while (!bExit)
          {
            // {{{ prep work
            list<int> managerRemovals;
            list<string> removals;
            map<int, string> s;
            fds = new pollfd[(1+m.size()+m_i.size()*2)];
            unIndex = 0;
            fds[unIndex].fd = fdUnix;
            fds[unIndex].events = POLLIN;
            unIndex++;
            for (auto &manager : m)
            {
              fds[unIndex].fd = manager.first;
              fds[unIndex].events = POLLIN;
              if (!manager.second[1].empty())
              {
                fds[unIndex].events |= POLLOUT;
              }
              unIndex++;
            }
            for (auto &interface : m_i)
            {
              s[interface.second->fdRead] = interface.first;
              fds[unIndex].fd = interface.second->fdRead;
              fds[unIndex].events = POLLIN;
              unIndex++;
              s[interface.second->fdWrite] = interface.first;
              fds[unIndex].fd = -1;
              fds[unIndex].events = POLLOUT;
              if (!interface.second->strBuffers[1].empty())
              {
                fds[unIndex].fd = interface.second->fdWrite;
                time(&CTime);
                if (!interface.second->bShutdown && (CTime - interface.second->CWrote) > 60)
                {
                  strError = "Unable to write to the interface for the last 60 seconds.";
                  ssMessage.str("");
                  ssMessage << char(3) << "11,10 " << m_strNode << " " << char(3) << " " << char(3) << "07,05 " << interface.first << " " << char(3) << " " << strPrefix << ":  " << strError;
                  chat("#radial", ssMessage.str());
                  ssMessage.str("");
                  ssMessage << strPrefix << " [" << interface.first << "]:  " << strError;
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
            if ((nReturn = poll(fds, unIndex, 2000)) > 0)
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
                  m[fdClient] = {"", ""};
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
                if (s.find(fds[i].fd) != s.end())
                {
                  // {{{ read
                  if (fds[i].revents & (POLLHUP | POLLIN))
                  {
                    if (m_pUtility->fdRead(fds[i].fd, m_i[s[fds[i].fd]]->strBuffers[0], nReturn))
                    {
                      while ((unPosition = m_i[s[fds[i].fd]]->strBuffers[0].find("\n")) != string::npos)
                      {
                        radialPacket p;
                        unpack(m_i[s[fds[i].fd]]->strBuffers[0].substr(0, unPosition), p);
                        m_i[s[fds[i].fd]]->strBuffers[0].erase(0, (unPosition + 1));
                        if (!p.t.empty())
                        {
                          if (p.d.empty())
                          {
                            if (m_i.find(p.t) != m_i.end())
                            {
                              p.d = "t";
                              m_i[p.t]->strBuffers[1].append(pack(p, strValue) + "\n");
                            }
                            else
                            {
                              list<radialLink *>::iterator linkIter = m_l.end();
                              for (auto i = m_l.begin(); linkIter == m_l.end() && i != m_l.end(); i++)
                              {
                                if ((*i)->interfaces.find(p.t) != (*i)->interfaces.end())
                                {
                                  linkIter = i;
                                }
                              }
                              if (linkIter != m_l.end() && m_i.find("link") != m_i.end())
                              {
                                Json *ptJson = new Json(p.p);
                                p.d = "t";
                                ptJson->i("Node", (*linkIter)->strNode);
                                ptJson->j(p.p);
                                delete ptJson;
                                m_i["link"]->strBuffers[1].append(pack(p, strValue) + "\n");
                              }
                              else if (!p.s.empty() && m_i.find(p.s) != m_i.end())
                              {
                                Json *ptJson = new Json(p.p);
                                p.d = "s";
                                ptJson->i("Status", "error");
                                ptJson->i("Error", "Interface does not exist.");
                                ptJson->j(p.p);
                                delete ptJson;
                                m_i[p.s]->strBuffers[1].append(pack(p, strValue) + "\n");
                              }
                            }
                            for (auto &manager : m)
                            {
                              if (manager.second.size() == 3 && (manager.second[2].empty() || manager.second[2] == p.s || manager.second[2] == p.t))
                              {
                                ssMessage.str("");
                                ssMessage << "[" << p.s << "-->" << p.t << "] " << p.p << endl;
                                manager.second[1].append(ssMessage.str());
                              }
                            }
                          }
                          else if (p.d == "t" && !p.s.empty() && m_i.find(p.s) != m_i.end())
                          {
                            p.d = "s";
                            m_i[p.s]->strBuffers[1].append(pack(p, strValue) + "\n");
                            for (auto &manager : m)
                            {
                              if (manager.second.size() == 3 && (manager.second[2].empty() || manager.second[2] == p.s || manager.second[2] == p.t))
                              {
                                ssMessage.str("");
                                ssMessage << "[" << p.s << "<--" << p.t << "] " << p.p << endl;
                                manager.second[1].append(ssMessage.str());
                              }
                            }
                          }
                        }
                        else
                        {
                          Json *ptJson = new Json(p.p);
                          if (s[fds[i].fd] == "link" && !empty(ptJson, "Function") && ptJson->m["Function"]->v == "links")
                          {
                            for (auto &link : m_l)
                            {
                              for (auto &interface : link->interfaces)
                              {
                                delete interface.second;
                              }
                              link->interfaces.clear();
                              delete link;
                            }
                            m_l.clear();
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
                                m_l.push_back(ptLink);
                              }
                            }
                            links();
                          }
                          else
                          {
                            // {{{ prep work
                            bool bRespond = true, bResult = false;
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
                                  unsigned long ulMemory = 40 * 1024;
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
                                      stringstream ssMemory((!empty(ptInterfaces->m[ptJson->m["Name"]->v], "Memory"))?ptInterfaces->m[ptJson->m["Name"]->v]->m["Memory"]->v:"40");
                                      ssMemory >> ulMemory;
                                      ulMemory *= 1024;
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
                                  if (!empty(ptJson, "Memory"))
                                  {
                                    stringstream ssMemory(ptJson->m["Memory"]->v);
                                    ssMemory >> ulMemory;
                                    ulMemory *= 1024;
                                  }
                                  if (!empty(ptJson, "Respawn"))
                                  {
                                    bRespawn = ((ptJson->m["Respawn"]->v == "1")?true:false);
                                  }
                                  if (!empty(ptJson, "Restricted"))
                                  {
                                    bRestricted = ((ptJson->m["Restricted"]->v == "1")?true:false);
                                  }
                                  if (add(strPrefix, ptJson->m["Name"]->v, strAccessFunction, strCommand, ulMemory, bRespawn, bRestricted))
                                  {
                                    bResult = true;
                                    interfaces();
                                  }
                                }
                                else
                                {
                                  ssMessage.str("");
                                  ssMessage << strPrefix << " error [" << s[fds[i].fd] << "," << fds[i].fd << ",add]:  Please provide the Name.";
                                  log(ssMessage.str());
                                }
                              }
                              // }}}
                              // {{{ list
                              else if (ptJson->m["Function"]->v == "list")
                              {
                                bResult = true;
                                ptJson->m["Response"] = new Json;
                                for (auto &j : m_i)
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
                                  if (m_i.find(ptJson->m["Name"]->v) != m_i.end())
                                  {
                                    bResult = true;
                                    setShutdown(strPrefix, ptJson->m["Name"]->v, true);
                                  }
                                  else
                                  {
                                    ssMessage.str("");
                                    ssMessage << strPrefix << " error [" << s[fds[i].fd] << "," << fds[i].fd << ",remove," << ptJson->m["Name"]->v << "]:  Interface not found.";
                                    log(ssMessage.str());
                                  }
                                }
                                else
                                {
                                  ssMessage.str("");
                                  ssMessage << strPrefix << " error [" << s[fds[i].fd] << "," << fds[i].fd << ",remove]:  Please provide the Name.";
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
                              // {{{ throughput
                              else if (ptJson->m["Function"]->v == "throughput")
                              {
                                bRespond = false;
                                if (!p.s.empty())
                                {
                                  if (t.find(p.s) == t.end())
                                  {
                                    t[p.s] = {};
                                  }
                                  if (exist(ptJson, "Response"))
                                  {
                                    for (auto &throughput : ptJson->m["Response"]->m)
                                    {
                                      size_t unThroughput;
                                      stringstream ssThroughput(throughput.second->v);
                                      ssThroughput >> unThroughput;
                                      if (t[p.s].find(throughput.first) == t[p.s].end())
                                      {
                                        t[p.s][throughput.first] = 0;
                                      }
                                      t[p.s][throughput.first] += unThroughput;
                                    }
                                  }
                                }
                              }
                              // }}}
                              // {{{ invalid
                              else
                              {
                                strError = "Please provide a valid Function:  add, list, ping, remove, shutdown, throughput.";
                              }
                              // }}}
                            }
                            // {{{ post work
                            if (bRespond)
                            {
                              p.d = "s";
                              ptJson->i("Status", ((bResult)?"okay":"error"));
                              if (!strError.empty())
                              {
                                ptJson->i("Error", strError);
                              }
                              ptJson->j(p.p);
                              m_i[s[fds[i].fd]]->strBuffers[1].append(pack(p, strValue) + "\n");
                            }
                            // }}}
                          }
                          delete ptJson;
                        }
                      }
                    }
                    else
                    {
                      removals.push_back(s[fds[i].fd]);
                      if (nReturn < 0 || errno == EINVAL)
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << "->Utility::fdRead(" << errno << ") error [" << s[fds[i].fd] << "," << fds[i].fd << "]:  " << strerror(errno);
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
                    if (m_pUtility->fdWrite(fds[i].fd, m_i[s[fds[i].fd]]->strBuffers[1], nReturn))
                    {
                      time(&(m_i[s[fds[i].fd]]->CWrote));
                    }
                    else
                    {
                      removals.push_back(s[fds[i].fd]);
                      if (nReturn < 0)
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << "->Utility::fdWrite(" << errno << ") error [" << s[fds[i].fd] << "," << fds[i].fd << "]:  " << strerror(errno);
                        log(ssMessage.str());
                      }
                    }
                  }
                  // }}}
                }
                // }}}
                // {{{ managers
                else if (m.find(fds[i].fd) != m.end())
                {
                  // {{{ read
                  if (fds[i].revents & POLLIN)
                  {
                    if (m_pUtility->fdRead(fds[i].fd, m[fds[i].fd][0], nReturn))
                    {
                      while ((unPosition = m[fds[i].fd][0].find("\n")) != string::npos)
                      {
                        bool bProcessed = false;
                        ptJson = new Json(m[fds[i].fd][0].substr(0, unPosition));
                        m[fds[i].fd][0].erase(0, (unPosition + 1));
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
                            else if (ptJson->m["Function"]->v == "sniff")
                            {
                              if (!strInterface.empty())
                              {
                                if (exist(ptInterfaces, strInterface))
                                {
                                  if (m_i.find(strInterface) != m_i.end())
                                  {
                                    bProcessed = true;
                                    if (m[fds[i].fd].size() < 3)
                                    {
                                      m[fds[i].fd].push_back("");
                                    }
                                    m[fds[i].fd][2] = strInterface;
                                  }
                                  else
                                  {
                                    ssMessage << "Interface is not running.";
                                  }
                                }
                                else
                                {
                                  ssMessage << "Please provide a valid Interface:  " << ssInterfaces.str() << ".";
                                }
                              }
                              else
                              {
                                bProcessed = true;
                                if (m[fds[i].fd].size() < 3)
                                {
                                  m[fds[i].fd].push_back("");
                                }
                                m[fds[i].fd][2] = "";
                              }
                            }
                            else if (ptJson->m["Function"]->v == "start")
                            {
                              if (!strInterface.empty())
                              {
                                if (exist(ptInterfaces, strInterface))
                                {
                                  if (m_i.find(strInterface) == m_i.end())
                                  {
                                    stringstream ssMemory((!empty(ptInterfaces->m[strInterface], "Memory"))?ptInterfaces->m[strInterface]->m["Memory"]->v:"40");
                                    unsigned long ulMemory;
                                    ssMemory >> ulMemory;
                                    ulMemory *= 1024;
                                    if (add(strPrefix, strInterface, ((!empty(ptInterfaces->m[strInterface], "AccessFunction"))?ptInterfaces->m[strInterface]->m["AccessFunction"]->v:"Function"), ptInterfaces->m[strInterface]->m["Command"]->v, ulMemory, ((!empty(ptInterfaces->m[strInterface], "Respawn") && ptInterfaces->m[strInterface]->m["Respawn"]->v == "1")?true:false), ((!empty(ptInterfaces->m[strInterface], "Restricted") && ptInterfaces->m[strInterface]->m["Restricted"]->v == "1")?true:false)))
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
                                  ptJson->i("Response", ((m_i.find(strInterface) != m_i.end())?"online":"offline"));
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
                                  if (m_i.find(strInterface) != m_i.end())
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
                        m[fds[i].fd][1] = ptJson->j(strJson) + "\n";
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
                    if (m_pUtility->fdWrite(fds[i].fd, m[fds[i].fd][1], nReturn))
                    {
                      if (m[fds[i].fd].size() == 2 && m[fds[i].fd][1].empty())
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
              m.erase(managerRemovals.front());
              close(managerRemovals.front());
              managerRemovals.pop_front();
            }
            time(&CTime);
            if ((CTime - CLoad) > 60)
            {
              CLoad = CTime;
              if (!load(strPrefix, strError))
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Hub::load() error:  " << strError;
                log(ssMessage.str());
              }
              interfaces();
            }
            if ((CTime - CThroughput) >= 3600)
            {
              CThroughput = CTime;
              ptJson = new Json;
              for (auto &i : t)
              {
                ptJson->m[i.first] = new Json;
                for (auto &j : i.second)
                {
                  stringstream ssThroughput;
                  ssThroughput << j.second;
                  ptJson->m[i.first]->i(j.first, ssThroughput.str(), 'n');
                }
                i.second.clear();
              }
              t.clear();
              ssMessage.str("");
              ssMessage << strPrefix << ":  THROUGHPUT " << ptJson;
              delete ptJson;
              log(ssMessage.str());
            }
            for (auto &i : m_i)
            {
              string strMessage;
              if (!i.second->bShutdown && Base::monitor(i.second->nPid, i.second->CMonitor, i.second->unMonitor, i.second->ulMemory, strMessage) == 2)
              {
                ssMessage.str("");
                ssMessage << char(3) << "11,10 " << m_strNode << " " << char(3) << " " << char(3) << "07,05 " << i.first << " " << char(3) << " " << strPrefix << ":  " << strMessage;
                chat("#radial", ssMessage.str());
                ssMessage.str("");
                ssMessage << strPrefix << " [" << i.first << "]:  " << strMessage;
                log(ssMessage.str());
                setShutdown(strPrefix, i.first);
              }
              else if (i.second->CShutdown > 0 && (CTime - i.second->CShutdown) > 20)
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
            monitor(strPrefix, nPid);
            if (shutdown())
            {
              if (m_i.empty())
              {
                bExit = true;
              }
              else if (m_i.size() == 1)
              {
                if (m_i.find("log") != m_i.end())
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
          for (auto &manager : m)
          {
            close(manager.first);
          }
          m.clear();
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
  if (m_i.find(strName) != m_i.end())
  {
    m_i[strName]->CKill = 0;
    m_i[strName]->CShutdown = 0;
    close(m_i[strName]->fdRead);
    m_i[strName]->fdRead = -1;
    close(m_i[strName]->fdWrite);
    m_i[strName]->fdWrite = -1;
    m_i[strName]->strBuffers[0].clear();
    m_i[strName]->strBuffers[1].clear();
    ssMessage.str("");
    ssMessage << strPrefix << " [" << strName << "]:  Interface removed.";
    log(ssMessage.str());
    if (!shutdown() && m_i[strName]->bRespawn)
    {
      add(strPrefix, strName, m_i[strName]->strAccessFunction, m_i[strName]->strCommand, m_i[strName]->ulMemory, true, m_i[strName]->bRestricted);
    }
    else
    {
      delete m_i[strName];
      m_i.erase(strName);
    }
  }
}
// }}}
// {{{ setShutdown()
void Hub::setShutdown(string strPrefix, const string strTarget, const bool bStop)
{
  string strValue;
  stringstream ssMessage;
  Json *ptJson = new Json;
  radialPacket p;

  strPrefix += "->shutdown()";
  p.s = "hub";
  ptJson->i("Function", "shutdown");
  ptJson->j(p.p);
  delete ptJson;
  if (strTarget.empty())
  {
    Base::setShutdown();
  }
  for (auto &interface : m_i)
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
      interface.second->strBuffers[1].append(pack(p, strValue) + "\n");
      time(&(interface.second->CShutdown));
    }
  }
}
// }}}
// {{{ target()
void Hub::target(radialPacket &p)
{
  if (m_i.find(p.t) != m_i.end())
  {
    string strValue;
    p.d = "t";
    m_i[p.t]->strBuffers[1].append(pack(p, strValue) + "\n");
  }
}
void Hub::target(const string t, Json *j, const string s)
{
  radialPacket p;

  if (!s.empty())
  {
    p.s = s;
  }
  p.t = t;
  j->j(p.p);
  target(p);
}
// }}}
}
}
