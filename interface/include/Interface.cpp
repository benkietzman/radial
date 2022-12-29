// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Interface.cpp
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
#include "Interface"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Interface()
Interface::Interface(string strPrefix, const string strName, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Base(argc, argv)
{
  strPrefix += "->Interface::Interface()";
  signal(SIGBUS, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGCONT, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGSEGV, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  signal(SIGWINCH, SIG_IGN);
  m_bMaster = false;
  m_bMasterSettled = false;
  m_pAutoModeCallback = NULL;
  m_pCallback = pCallback;
  m_pJunction->setProgram(strName);
  m_strName = strName;
}
// }}}
// {{{ ~Interface()
Interface::~Interface()
{
}
// }}}
// {{{ alert()
void Interface::alert(const string strMessage)
{
  log("alert", strMessage);
}
// }}}
// {{{ auth()
bool Interface::auth(Json *ptJson, string &strError)
{
  bool bResult = false;
  string strTarget = "auth";
  Json *ptAuth = new Json(ptJson);

  keyRemovals(ptAuth);
  if (ptAuth->m.find("Interface") != ptAuth->m.end() && !ptAuth->m["Interface"]->v.empty())
  {
    if (ptAuth->m.find("Request") != ptAuth->m.end())
    {
      delete ptAuth->m["Request"];
    }
    ptAuth->m["Request"] = new Json;
    ptAuth->m["Request"]->i("Interface", ptAuth->m["Interface"]->v);
    ptAuth->i("Interface", "auth");
  }
  if (ptJson->m.find("Node") != ptJson->m.end() && !ptJson->m["Node"]->v.empty())
  {
    strTarget = "link";
  }
  if (hub(strTarget, ptAuth, strError))
  {
    bResult = true;
  }
  delete ptAuth;

  return bResult;
}
// }}}
// {{{ db
// {{{ dbfree()
void Interface::dbfree(list<map<string, string> > *rows)
{
  if (rows != NULL)
  {
    rows->clear();
    delete rows;
    rows = NULL;
  }
}
// }}}
// {{{ dbquery()
list<map<string, string> > *Interface::dbquery(const string strDatabase, const string strQuery, string &strError)
{
  unsigned long long ullRows;

  return dbquery(strDatabase, strQuery, ullRows, strError);
}
list<map<string, string> > *Interface::dbquery(const string strDatabase, const string strQuery, unsigned long long &ullRows, string &strError)
{
  list<map<string, string> > *rows = NULL;
  Json *ptJson = new Json;

  ullRows = 0;
  ptJson->i("Database", strDatabase);
  ptJson->i("Query", strQuery);
  if (hub("database", ptJson, strError))
  {
    if (ptJson->m.find("Response") != ptJson->m.end())
    {
      rows = new list<map<string, string> >;
      for (auto &ptRow : ptJson->m["Response"]->l) 
      {
        map<string, string> row;
        ptRow->flatten(row, true, false);
        rows->push_back(row);
      }
    }
    if (ptJson->m.find("Rows") != ptJson->m.end() && !ptJson->m["Rows"]->v.empty())
    {
      stringstream ssRows(ptJson->m["Rows"]->v);
      ssRows >> ullRows;
    }
  }
  delete ptJson;

  return rows;
}
// }}}
// {{{ dbupdate()
bool Interface::dbupdate(const string strDatabase, const string strUpdate, string &strError)
{
  unsigned long long ullID, ullRows;

  return dbupdate(strDatabase, strUpdate, ullID, ullRows, strError);
}
bool Interface::dbupdate(const string strDatabase, const string strUpdate, string &strID, string &strError)
{
  bool bResult = false;
  unsigned long long ullID, ullRows;

  if (dbupdate(strDatabase, strUpdate, ullID, ullRows, strError))
  {
    stringstream ssID;
    bResult = true;
    ssID << ullID;
    strID = ssID.str();
  }

  return bResult;
}
bool Interface::dbupdate(const string strDatabase, const string strUpdate, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ullID = ullRows = 0;
  ptJson->i("Database", strDatabase);
  ptJson->i("Update", strUpdate);
  if (hub("database", ptJson, strError))
  {
    bResult = true;
    if (ptJson->m.find("ID") != ptJson->m.end() && !ptJson->m["ID"]->v.empty())
    {
      stringstream ssID(ptJson->m["ID"]->v);
      ssID >> ullID;
    }
    if (ptJson->m.find("Rows") != ptJson->m.end() && !ptJson->m["Rows"]->v.empty())
    {
      stringstream ssRows(ptJson->m["Rows"]->v);
      ssRows >> ullRows;
    }
  }
  delete ptJson;

  return bResult;
}
// }}}
// }}}
// {{{ hub()
void Interface::hub(Json *ptJson, const bool bWait)
{
  hub("", ptJson, bWait);
}
void Interface::hub(const string strTarget, Json *ptJson, const bool bWait)
{
  string strJson;

  if (!strTarget.empty())
  {
    ptJson->i("_t", strTarget);
  }
  if (bWait)
  {
    int fdUnique[2] = {-1, -1}, nReturn;
    stringstream ssMessage;
    ptJson->i("_s", m_strName);
    if ((nReturn = pipe(fdUnique)) == 0)
    {
      bool bExit = false, bResult = false;
      long lArg;
      size_t unPosition, unUnique = 0;
      string strBuffer, strError;
      stringstream ssUnique;
      if ((lArg = fcntl(fdUnique[0], F_GETFL, NULL)) >= 0)
      {
        lArg |= O_NONBLOCK;
        fcntl(fdUnique[0], F_SETFL, lArg);
      }
      if ((lArg = fcntl(fdUnique[1], F_GETFL, NULL)) >= 0)
      {
        lArg |= O_NONBLOCK;
        fcntl(fdUnique[1], F_SETFL, lArg);
      }
      m_mutexShare.lock();
      ssUnique << m_strName << "_" << unUnique;
      while (m_waiting.find(ssUnique.str()) != m_waiting.end())
      {
        unUnique++;
        ssUnique.str("");
        ssUnique << m_strName << "_" << unUnique;
      }
      ptJson->i("_u", ssUnique.str());
      m_waiting[ssUnique.str()] = fdUnique[1];
      ptJson->j(strJson);
      m_responses.push_back(strJson);
      m_mutexShare.unlock();
      while (!bExit)
      {
        pollfd fds[1];
        fds[0].fd = fdUnique[0];
        fds[0].events = POLLIN;
        if ((nReturn = poll(fds, 1, 10)) > 0)
        {
          if (fds[0].revents & (POLLHUP | POLLIN))
          {
            if (!m_pUtility->fdRead(fds[0].fd, strBuffer, nReturn))
            {
              bExit = true;
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << "Utility::fdRead(" << errno << ") " << strerror(errno);
                strError = ssMessage.str();
              }
              else if ((unPosition = strBuffer.find("\n")) != string::npos)
              {
                bResult = true;
              }
              else if (!strBuffer.empty())
              {
                ssMessage.str("");
                ssMessage << "Invalid response. --- " << strBuffer;
                strError = ssMessage.str();
              }
              else
              {
                strError = "Failed to receive a response.";
              }
            }
          }
        }
        else if (nReturn < 0 && errno != EINTR)
        {
          bExit = true;
          ssMessage.str("");
          ssMessage << "poll(" << errno << ") " << strerror(errno);
          strError = ssMessage.str();
        }
      }
      close(fdUnique[0]);
      m_mutexShare.lock();
      m_waiting.erase(ssUnique.str());
      m_mutexShare.unlock();
      if (bResult)
      {
        ptJson->parse(strBuffer.substr(0, unPosition));
      }
      else
      {
        ptJson->i("Status", "error");
        ptJson->i("Error", ((!strError.empty())?strError:"Encountered an uknown error."));
      }
    }
    else
    {
      ssMessage.str("");
      ssMessage << "pipe(" << errno << ") " << strerror(errno);
      ptJson->i("Status", "error");
      ptJson->i("Error", ssMessage.str());
    }
    keyRemovals(ptJson);
  }
  else
  {
    ptJson->j(strJson);
    m_mutexShare.lock();
    m_responses.push_back(strJson);
    m_mutexShare.unlock();
  }
}
bool Interface::hub(Json *ptJson, string &strError)
{
  return hub("", ptJson, strError);
}
bool Interface::hub(const string strTarget, Json *ptJson, string &strError)
{
  bool bResult = false;

  hub(strTarget, ptJson, true);
  if (ptJson->m.find("Status") != ptJson->m.end() && ptJson->m["Status"]->v == "okay")
  {
    bResult = true;
  }
  else if (ptJson->m.find("Error") != ptJson->m.end() && !ptJson->m["Error"]->v.empty())
  {
    strError = ptJson->m["Error"]->v;
  }
  else
  {
    strError = "Encountered an unknown error.";
  }

  return bResult;
}
// }}}
// {{{ interfaces()
void Interface::interfaces(string strPrefix, Json *ptJson)
{
  strPrefix += "->Interface::interfaces()";
  m_mutexShare.lock();
  for (auto &i : m_interfaces)
  {
    delete i.second;
  }
  m_interfaces.clear();
  if (ptJson->m.find("Interfaces") != ptJson->m.end())
  {
    for (auto &interface : ptJson->m["Interfaces"]->m)
    {
      m_interfaces[interface.first] = new radialInterface;
      if (interface.second->m.find("AccessFunction") != interface.second->m.end() && !interface.second->m["AccessFunction"]->v.empty())
      {
        m_interfaces[interface.first]->strAccessFunction = interface.second->m["AccessFunction"]->v;
      }
      if (interface.second->m.find("Command") != interface.second->m.end() && !interface.second->m["Command"]->v.empty())
      {
        m_interfaces[interface.first]->strCommand = interface.second->m["Command"]->v;
      }
      m_interfaces[interface.first]->nPid = -1;
      if (interface.second->m.find("PID") != interface.second->m.end() && !interface.second->m["PID"]->v.empty())
      {
        stringstream ssPid(interface.second->m["PID"]->v);
        ssPid >> m_interfaces[interface.first]->nPid;
      }
      m_interfaces[interface.first]->bRespawn = ((interface.second->m.find("Respawn") != interface.second->m.end() && interface.second->m["Respawn"]->v == "1")?true:false);
      m_interfaces[interface.first]->bRestricted = ((interface.second->m.find("Restricted") != interface.second->m.end() && interface.second->m["Restricted"]->v == "1")?true:false);
    }
  }
  m_mutexShare.unlock();
}
// }}}
// {{{ isMaster()
bool Interface::isMaster()
{
  return m_bMaster;
}
// }}}
// {{{ isMasterSettled()
bool Interface::isMasterSettled()
{
  return m_bMasterSettled;
}
// }}}
// {{{ chat()
bool Interface::chat(const string strTarget, const string strMessage)
{
  string strError;

  return chat(strTarget, strMessage, strError);
}
bool Interface::chat(const string strTarget, const string strMessage, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ptJson->i("Function", "chat");
  ptJson->i("Target", strTarget);
  ptJson->i("Message", strMessage);
  if (hub("irc", ptJson, strError))
  {
    bResult = true;
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ keyRemovals()
void Interface::keyRemovals(Json *ptJson)
{
  list<string> removals;

  for (auto &i : ptJson->m)
  {
    if (!i.first.empty() && i.first[0] == '_')
    {
      removals.push_back(i.first);
    }
  }
  while (!removals.empty())
  {
    if (ptJson->m.find(removals.front()) != ptJson->m.end())
    {
      delete ptJson->m[removals.front()];
      ptJson->m.erase(removals.front());
    }
    removals.pop_front();
  }
}
// }}}
// {{{ links()
void Interface::links(string strPrefix, Json *ptJson)
{
  strPrefix += "->Interface::links()";
  m_mutexShare.lock();
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
  if (ptJson->m.find("Links") != ptJson->m.end())
  {
    for (auto &link : ptJson->m["Links"]->m)
    {
      radialLink *ptLink = new radialLink;
      ptLink->strNode = link.first;
      if (link.second->m.find("Server") != link.second->m.end() && !link.second->m["Server"]->v.empty())
      {
        ptLink->strServer = link.second->m["Server"]->v;
      }
      if (link.second->m.find("Port") != link.second->m.end() && !link.second->m["Port"]->v.empty())
      {
        ptLink->strPort = link.second->m["Port"]->v;
      }
      if (link.second->m.find("Interfaces") != link.second->m.end())
      {
        for (auto &interface : link.second->m["Interfaces"]->m)
        {
          ptLink->interfaces[interface.first] = new radialInterface;
          if (interface.second->m.find("AccessFunction") != interface.second->m.end() && !interface.second->m["AccessFunction"]->v.empty())
          {
            ptLink->interfaces[interface.first]->strAccessFunction = interface.second->m["AccessFunction"]->v;
          }
          if (interface.second->m.find("Command") != interface.second->m.end() && !interface.second->m["Command"]->v.empty())
          {
            ptLink->interfaces[interface.first]->strCommand = interface.second->m["Command"]->v;
          }
          ptLink->interfaces[interface.first]->nPid = -1;
          if (interface.second->m.find("PID") != interface.second->m.end() && !interface.second->m["PID"]->v.empty())
          {
            stringstream ssPid(interface.second->m["PID"]->v);
            ssPid >> ptLink->interfaces[interface.first]->nPid;
          }
          ptLink->interfaces[interface.first]->bRespawn = ((interface.second->m.find("Respawn") != interface.second->m.end() && interface.second->m["Respawn"]->v == "1")?true:false);
          ptLink->interfaces[interface.first]->bRestricted = ((interface.second->m.find("Restricted") != interface.second->m.end() && interface.second->m["Restricted"]->v == "1")?true:false);
        }
      }
      m_links.push_back(ptLink);
    }
  }
  m_mutexShare.unlock();
}
// }}}
// {{{ log()
void Interface::log(const string strMessage)
{
  log("log", strMessage);
}
void Interface::log(const string strFunction, const string strMessage)
{
  Json *ptJson = new Json;

  ptJson->i("Function", strFunction);
  ptJson->i("Message", strMessage);
  hub("log", ptJson, false);

  delete ptJson;
}
// }}}
// {{{ master()
string Interface::master()
{
  return m_strMaster;
}
// }}}
// {{{ monitor()
void Interface::monitor(string strPrefix)
{
  strPrefix += "->Interface::monitor()";
  if (!shutdown())
  {
    string strMessage;
    if (Base::monitor(strMessage) == 2)
    {
      stringstream ssMessage;
      ssMessage << strPrefix << "->Base::monitor():  " << strMessage;
      notify(ssMessage.str());
      setShutdown();
    }
  }
}
// }}}
// {{{ mysql
// {{{ mysql()
bool Interface::mysql(const string strServer, const unsigned int unPort, const string strUser, const string strPassword, const string strDatabase, const string strType, const string strQuery, unsigned long long &ullID, unsigned long long &ullRows, list<map<string, string> > &rows, string &strError)
{
  bool bResult;
  stringstream ssPort;
  Json *ptJson = new Json;

  ptJson->i("Server", strServer);
  ssPort << unPort;
  ptJson->i("Port", ssPort.str(), 'n');
  ptJson->i("User", strUser);
  ptJson->i("Password", strPassword);
  ptJson->i("Database", strDatabase);
  ptJson->i(strType, strQuery);
  if (hub("mysql", ptJson, strError))
  {
    bResult = true;
    if (ptJson->m.find("Response") != ptJson->m.end())
    {
      for (auto &ptRow : ptJson->m["Response"]->l)
      {
        map<string, string> row;
        ptRow->flatten(row, true, false);
        rows.push_back(row);
      }
    }
    if (ptJson->m.find("ID") != ptJson->m.end() && !ptJson->m["ID"]->v.empty())
    {
      stringstream ssID(ptJson->m["ID"]->v);
      ssID >> ullID;
    }
    if (ptJson->m.find("Rows") != ptJson->m.end() && !ptJson->m["Rows"]->v.empty())
    {
      stringstream ssRows(ptJson->m["Rows"]->v);
      ssRows >> ullRows;
    }
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ mysqlQuery()
bool Interface::mysqlQuery(const string strServer, const unsigned int unPort, const string strUser, const string strPassword, const string strDatabase, const string strQuery, unsigned long long &ullRows, list<map<string, string> > &rows, string &strError)
{
  unsigned long long ullID;

  return mysql(strServer, unPort, strUser, strPassword, strDatabase, "Query", strQuery, ullID, ullRows, rows, strError);
}
// }}}
// {{{ mysqlUpdate()
bool Interface::mysqlUpdate(const string strServer, const unsigned int unPort, const string strUser, const string strPassword, const string strDatabase, const string strQuery, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  list<map<string, string> > rows;

  return mysql(strServer, unPort, strUser, strPassword, strDatabase, "Update", strQuery, ullID, ullRows, rows, strError);
}
// }}}
// }}}
// {{{ notify()
void Interface::notify(const string strMessage)
{
  log("notify", strMessage);
}
// }}}
// {{{ process()
void Interface::process(string strPrefix)
{
  bool bExit = false;
  int nReturn;
  list<int> uniqueRemovals;
  map<int, string> uniques;
  long lArg;
  pollfd *fds;
  size_t unIndex, unPosition;
  string strError, strJson, strLine;
  stringstream ssMessage;
  time_t CBroadcast, CMaster, CTime, unBroadcastSleep = 10;

  strPrefix += "->Interface::process()";
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
  time(&CBroadcast);
  CMaster = CBroadcast;
  while (!bExit)
  {
    fds = new pollfd[uniques.size() + 2];
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
    for (auto &unique : uniques)
    {
      fds[unIndex].fd = -1;
      if (!unique.second.empty())
      {
        fds[unIndex].fd = unique.first;
      }
      fds[unIndex].events = POLLOUT;
      unIndex++;
    }
    if ((nReturn = poll(fds, unIndex, 10)) > 0)
    {
      if (fds[0].revents & (POLLHUP | POLLIN))
      {
        if (m_pUtility->fdRead(fds[0].fd, m_strBuffers[0], nReturn))
        {
          while ((unPosition = m_strBuffers[0].find("\n")) != string::npos)
          {
            int fdUnique = -1;
            Json *ptJson;
            strLine = m_strBuffers[0].substr(0, unPosition);
            m_strBuffers[0].erase(0, (unPosition + 1));
            ptJson = new Json(strLine);
            if (ptJson->m.find("_s") != ptJson->m.end() && ptJson->m["_s"]->v == m_strName && ptJson->m.find("_u") != ptJson->m.end() && !ptJson->m["_u"]->v.empty())
            {
              m_mutexShare.lock();
              if (m_waiting.find(ptJson->m["_u"]->v) != m_waiting.end())
              {
                fdUnique = m_waiting[ptJson->m["_u"]->v];
              }
              m_mutexShare.unlock();
            }
            if (fdUnique != -1)
            {
              uniques[fdUnique] = strLine + "\n";
            }
            else if (ptJson->m.find("_s") != ptJson->m.end() && ptJson->m["_s"]->v == "hub")
            {
              if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
              {
                // {{{ interfaces
                if (ptJson->m["Function"]->v == "interfaces")
                {
                  interfaces(strPrefix, ptJson);
                }
                // }}}
                // {{{ links
                else if (ptJson->m["Function"]->v == "links")
                {
                  links(strPrefix, ptJson);
                }
                // }}}
                // {{{ shutdown
                else if (ptJson->m["Function"]->v == "shutdown")
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << ":  Shutting down.";
                  log(ssMessage.str());
                  setShutdown();
                }
                // }}}
              }
            }
            else if (ptJson->m.find("Function") != ptJson->m.end() && ptJson->m["Function"]->v == "master")
            {
              ssMessage.str("");
              ssMessage << strPrefix << " [" << m_strMaster << "]:  Received master. --- " << ptJson;
              log(ssMessage.str());
              if (ptJson->m.find("Master") != ptJson->m.end() && !ptJson->m["Master"]->v.empty() && m_strMaster != ptJson->m["Master"]->v)
              {
                string strMaster = m_strMaster;
                m_strMaster = ptJson->m["Master"]->v;
                ssMessage.str("");
                ssMessage << strPrefix << " [" << strMaster << "," << m_strMaster << "]:  Master changed.";
                log(ssMessage.str());
                m_bMaster = ((m_strMaster == m_strNode)?true:false);
                m_bMasterSettled = false;
                time(&CMaster);
                if (m_pAutoModeCallback != NULL)
                {
                  m_pAutoModeCallback(strPrefix, strMaster, m_strMaster);
                }
              }
            }
            else if (m_pCallback != NULL)
            {
              if (ptJson->m.find("wsRequestID") != ptJson->m.end() && !ptJson->m["wsRequestID"]->v.empty())
              {
                string strIdentity, strName, strNode;
                stringstream ssRequestID(ptJson->m["wsRequestID"]->v);
                ssRequestID >> strNode >> strName >> strIdentity;
                if (strName != m_strName)
                {
                  Json *ptLive = new Json;
                  ptLive->i("radialProcess", m_strName);
                  ptLive->i("radialPrefix", strPrefix);
                  ptLive->i("radialPurpose", "status");
                  if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
                  {
                    ptLive->i("radialInterface", ptJson->m["Interface"]->v);
                  }
                  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
                  {
                    ptLive->i("radialFunction", ptJson->m["Function"]->v);
                  }
                  hub(strName, ptLive, false);
                  delete ptLive;
                }
              }
              m_pCallback(strPrefix, ptJson, true);
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
            ssMessage << strPrefix << "->Utility::fdRead(" << errno << ") [" << fds[0].fd << "]:  " << strerror(errno);
            log(ssMessage.str());
          }
        }
      }
      if (fds[1].revents & POLLOUT)
      {
        if (!m_pUtility->fdWrite(fds[1].fd, m_strBuffers[1], nReturn))
        {
          bExit = true;
          if (nReturn < 0)
          {
            ssMessage.str("");
            ssMessage << strPrefix << "->Utility::fdWrite(" << errno << ") [" << fds[1].fd << "]:  " << strerror(errno);
            log(ssMessage.str());
          }
        }
      }
      for (size_t i = 2; i < unIndex; i++)
      {
        if (fds[i].revents & POLLOUT)
        {
          if (m_pUtility->fdWrite(fds[i].fd, uniques[fds[i].fd], nReturn))
          {
            if (uniques[fds[i].fd].empty())
            {
              uniqueRemovals.push_back(fds[i].fd);
            }
          }
          else
          {
            uniqueRemovals.push_back(fds[i].fd);
            if (nReturn < 0)
            {
              ssMessage.str("");
              ssMessage << strPrefix << "->m_pUtility->fdWrite(" << errno << ") [" << fds[i].fd << "]:  " << strerror(errno);
              log(ssMessage.str());
            }
          }
        }
      }
    }
    else if (nReturn < 0 && errno != EINTR)
    {
      bExit = true;
      ssMessage.str("");
      ssMessage << strPrefix << "->poll(" << errno << "):  " << strerror(errno);
      notify(ssMessage.str());
    }
    delete[] fds;
    uniqueRemovals.sort();
    uniqueRemovals.unique();
    while (!uniqueRemovals.empty())
    {
      close(uniqueRemovals.front());
      uniqueRemovals.pop_front();
    }
    monitor(strPrefix);
    if (m_pAutoModeCallback != NULL)
    {
      time(&CTime);
      if (!m_bMasterSettled && (CTime - CMaster) > 30)
      {
        m_bMasterSettled = true;
      }
      if ((CTime - CBroadcast) > unBroadcastSleep)
      {
        string strMaster = m_strMaster;
        unsigned int unSeed = CTime;
        srand(unSeed);
        unBroadcastSleep = (rand_r(&unSeed) % 5) + 1;
        if (!m_strMaster.empty() && m_strMaster != m_strNode)
        {
          bool bFound = false;
          m_mutexShare.lock();
          for (auto linkIter = m_links.begin(); !bFound && linkIter != m_links.end(); linkIter++)
          {
            if ((*linkIter)->strNode == m_strMaster)
            {
              bFound = true;
            }
          }
          m_mutexShare.unlock();
          if (!bFound)
          {
            m_strMaster.clear();
          }
        }
        if (m_strMaster.empty())
        {
          m_strMaster = m_strNode;
        }
        if (!m_strMaster.empty())
        {
          Json *ptJson = new Json;
          ptJson->i("Interface", m_strName);
          ptJson->i("Function", "master");
          ptJson->i("Master", m_strMaster);
          hub("link", ptJson, false);
          delete ptJson;
        }
        if (strMaster != m_strMaster)
        {
          m_bMaster = ((m_strMaster == m_strNode)?true:false);
          m_bMasterSettled = false;
          time(&CMaster);
          m_pAutoModeCallback(strPrefix, strMaster, m_strMaster);
        }
        CBroadcast = CTime;
      }
    }
    if (shutdown())
    {
      m_mutexShare.lock();
      if (m_strBuffers[0].empty() && m_strBuffers[1].empty() && m_responses.empty() && m_waiting.empty())
      {
        bExit = true;
      }
      m_mutexShare.unlock();
    }
  }
  setShutdown();
}
// }}}
// {{{ setAutoMode()
void Interface::setAutoMode(void (*pCallback)(string, const string, const string))
{
  m_pAutoModeCallback = pCallback;
}
// }}}
// {{{ storage
// {{{ storage()
bool Interface::storage(const string strFunction, const list<string> keys, Json *ptJson, string &strError)
{
  bool bResult = false;
  string strJson;
  Json *ptSubJson = new Json;

  ptSubJson->i("Function", strFunction);
  if (!keys.empty())
  {
    ptSubJson->i("Keys", keys);
  }
  if ((strFunction == "add" || strFunction == "update") && ptJson != NULL)
  {
    ptSubJson->i("Request", ptJson);
  }
  if (hub("storage", ptSubJson, strError))
  {
    bResult = true;
    if ((strFunction == "retrieve" || strFunction == "retrieveKeys") && ptSubJson->m.find("Response") != ptSubJson->m.end())
    {
      ptSubJson->m["Response"]->j(strJson);
      ptJson->parse(strJson);
    }
  }
  delete ptSubJson;

  return bResult;
}
// }}}
// {{{ storageAdd()
bool Interface::storageAdd(const list<string> keys, Json *ptJson, string &strError)
{
  return storage("add", keys, ptJson, strError);
}
// }}}
// {{{ storageRemove()
bool Interface::storageRemove(const list<string> keys, string &strError)
{
  return storage("remove", keys, NULL, strError);
}
// }}}
// {{{ storageRetrieve()
bool Interface::storageRetrieve(Json *ptJson, string &strError)
{
  return storage("retrieve", {}, ptJson, strError);
}
bool Interface::storageRetrieve(const list<string> keys, Json *ptJson, string &strError)
{
  return storage("retrieve", keys, ptJson, strError);
}
// }}}
// {{{ storageRetrieveKeys()
bool Interface::storageRetrieveKeys(const list<string> keysIn, list<string> &keysOut, string &strError)
{
  bool bResult = false;
  Json *ptKeys = new Json;

  keysOut.clear();
  if (storage("retrieveKeys", keysIn, ptKeys, strError))
  {
    bResult = true;
    for (auto &key : ptKeys->l)
    {
      keysOut.push_back(key->v);
    }
  }
  delete ptKeys;

  return bResult;
}
// }}}
// {{{ storageUpdate()
bool Interface::storageUpdate(const list<string> keys, Json *ptJson, string &strError)
{
  return storage("update", keys, ptJson, strError);
}
// }}}
// }}}
}
}
