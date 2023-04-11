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
  if (!empty(ptAuth, "Interface"))
  {
    if (exist(ptAuth, "Request"))
    {
      delete ptAuth->m["Request"];
    }
    ptAuth->m["Request"] = new Json;
    ptAuth->m["Request"]->i("Interface", ptAuth->m["Interface"]->v);
    ptAuth->i("Interface", "auth");
  }
  if (!empty(ptJson, "Node"))
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
// {{{ centralmon()
bool Interface::centralmon(const string strServer, const string strProcess, Json *ptData, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json; 

  ptJson->i("Function", ((!strProcess.empty())?"process":"system"));
  ptJson->i("Server", strServer);
  if (!strProcess.empty())
  {
    ptJson->i("Process", strProcess);
  }
  if (hub("centralmon", ptJson, strError))
  {
    bResult = true;
    if (exist(ptJson, "Response"))
    {
      ptData->merge(ptJson->m["Response"], true, false);
    }
  }
  delete ptJson;

  return bResult;
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
  stringstream ssMessage;
  Json *ptJson = new Json;

  ssMessage << char(3) << "11,10 " << m_strNode << " " << char(3) << " " << char(3) << "07,05 " << m_strName << " " << char(3) << " " << strMessage;
  ptJson->i("Function", "chat");
  ptJson->i("Target", strTarget);
  ptJson->i("Message", ssMessage.str());
  if (hub("irc", ptJson, strError))
  {
    bResult = true;
  }
  delete ptJson;

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
    if (exist(ptJson, "Response"))
    {
      rows = new list<map<string, string> >;
      for (auto &ptRow : ptJson->m["Response"]->l) 
      {
        map<string, string> row;
        ptRow->flatten(row, true, false);
        rows->push_back(row);
      }
    }
    if (!empty(ptJson, "Rows"))
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
    if (!empty(ptJson, "ID"))
    {
      stringstream ssID(ptJson->m["ID"]->v);
      ssID >> ullID;
    }
    if (!empty(ptJson, "Rows"))
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
// {{{ email()
void Interface::email(const string strFrom, const string strTo, const string strSubject, const string strText, const string strHtml, string &strError)
{
  list<string> to;

  to.push_back(strTo);
  email(strFrom, to, strSubject, strText, strHtml, strError);
}
void Interface::email(const string strFrom, list<string> to, const string strSubject, const string strText, const string strHtml, string &strError)
{
  list<string> bcc, cc;
  map<string, string> file;

  email(strFrom, to, cc, bcc, strSubject, strText, strHtml, file, strError);
}
void Interface::email(const string strFrom, list<string> to, list<string> cc, list<string> bcc, const string strSubject, const string strText, const string strHtml, map<string, string> file, string &strError)
{
  Json *ptJson = new Json;

  ptJson->i("From", strFrom);
  if (!to.empty())
  {
    ptJson->m["To"] = new Json;
    for (auto &i : to)
    {
      ptJson->m["To"]->pb(i);
    }
  }
  if (!cc.empty())
  {
    ptJson->m["Cc"] = new Json;
    for (auto &i : cc)
    {
      ptJson->m["Cc"]->pb(i);
    }
  }
  if (!bcc.empty())
  {
    ptJson->m["Bcc"] = new Json;
    for (auto &i : bcc)
    {
      ptJson->m["Bcc"]->pb(i);
    }
  }
  if (!strSubject.empty())
  {
    ptJson->i("Subject", strSubject);
  }
  if (!strText.empty())
  {
    ptJson->i("Text", strText);
  }
  if (!strHtml.empty())
  {
    ptJson->i("Html", strHtml);
  }
  if (!file.empty())
  {
    ptJson->m["File"] = new Json;
    for (auto &i : file)
    {
      ptJson->m["file"]->i(i.first, i.second);
    }
  }
  hub("email", ptJson, false);
  delete ptJson;
}
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
      size_t unPosition, unUnique = 0;
      string strBuffer, strError;
      stringstream ssUnique;
      m_pUtility->fdNonBlocking(fdUnique[0], strError);
      m_pUtility->fdNonBlocking(fdUnique[1], strError);
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
        if ((nReturn = poll(fds, 1, 500)) > 0)
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
  if (exist(ptJson, "Status") && ptJson->m["Status"]->v == "okay")
  {
    bResult = true;
  }
  else if (!empty(ptJson, "Error"))
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
// {{{ interfaceAdd()
bool Interface::interfaceAdd(const string strInterface, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ptJson->i("Function", "add");
  ptJson->i("Name", strInterface);
  if (hub(ptJson, strError))
  {
    bResult = true;
  }
  delete ptJson;

  return bResult;
}
bool Interface::interfaceAdd(const string strNode, const string strInterface, string &strError)
{
  bool bResult = false;

  if (!strNode.empty() && strNode != m_strNode)
  {
    Json *ptJson = new Json;
    ptJson->i("Node", strNode);
    ptJson->i("Interface", "hub");
    ptJson->i("Function", "add");
    ptJson->i("Name", strInterface);
    if (hub("link", ptJson, strError))
    {
      bResult = true;
    }
    delete ptJson;
  }
  else if (interfaceAdd(strInterface, strError))
  {
    bResult = true;
  }

  return bResult;
}
// }}}
// {{{ interfaceRemove()
bool Interface::interfaceRemove(const string strInterface, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ptJson->i("Function", "remove");
  ptJson->i("Name", strInterface);
  if (hub(ptJson, strError))
  {
    bResult = true;
  }
  delete ptJson;

  return bResult;
}
bool Interface::interfaceRemove(const string strNode, const string strInterface, string &strError)
{
  bool bResult = false;

  if (!strNode.empty() && strNode != m_strNode)
  {
    Json *ptJson = new Json;
    ptJson->i("Node", strNode);
    ptJson->i("Interface", "hub");
    ptJson->i("Function", "remove");
    ptJson->i("Name", strInterface);
    if (hub("link", ptJson, strError))
    {
      bResult = true;
    }
    delete ptJson;
  }
  else if (interfaceRemove(strInterface, strError))
  {
    bResult = true;
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
  if (exist(ptJson, "Interfaces"))
  {
    for (auto &interface : ptJson->m["Interfaces"]->m)
    {
      m_interfaces[interface.first] = new radialInterface;
      if (!empty(interface.second, "AccessFunction"))
      {
        m_interfaces[interface.first]->strAccessFunction = interface.second->m["AccessFunction"]->v;
      }
      if (!empty(interface.second, "Command"))
      {
        m_interfaces[interface.first]->strCommand = interface.second->m["Command"]->v;
      }
      m_interfaces[interface.first]->nPid = -1;
      if (!empty(interface.second, "PID"))
      {
        stringstream ssPid(interface.second->m["PID"]->v);
        ssPid >> m_interfaces[interface.first]->nPid;
      }
      m_interfaces[interface.first]->bRespawn = ((exist(interface.second, "Respawn") && interface.second->m["Respawn"]->v == "1")?true:false);
      m_interfaces[interface.first]->bRestricted = ((exist(interface.second, "Restricted") && interface.second->m["Restricted"]->v == "1")?true:false);
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
// {{{ jwt()
bool Interface::jwt(const string strSigner, const string strSecret, string &strPayload, Json *ptPayload, string &strError)
{
  bool bDecode = false, bResult = false;
  Json *ptJson = new Json;

  if (!strPayload.empty())
  {
    bDecode = true;
    ptJson->i("Function", "decode");
  }
  else
  {
    ptJson->i("Function", "encode");
  }
  ptJson->i("Signer", strSigner);
  if (!strSecret.empty())
  {
    ptJson->i("Secret", strSecret);
  }
  if (bDecode)
  {
    ptJson->i("Payload", strPayload);
  }
  else
  {
    ptJson->m["Payload"] = new Json(ptPayload);
  }
  if (hub("jwt", ptJson, strError))
  {
    if (exist(ptJson, "Response"))
    {
      if (exist(ptJson->m["Response"], "Payload"))
      {
        if (bDecode)
        {
          bResult = true;
          ptPayload->merge(ptJson->m["Response"]->m["Payload"], true, false);
        }
        else if (!ptJson->m["Response"]->m["Payload"]->v.empty())
        {
          bResult = true;
          strPayload = ptJson->m["Response"]->m["Payload"]->v;
        }
        else
        {
          strError = "The Payload was empty within the Response.";
        }
      }
      else
      {
        strError = "Failed to find the Payload within the Response.";
      }
    }
    else
    {
      strError = "Failed to receive the Response.";
    }
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
    if (exist(ptJson, removals.front()))
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
if (m_strNode == "office" && m_strName == "irc" && link.first == "basement" && interface.first == "mysql")
{
  stringstream ssMessage;
  ssMessage << "== 6 == Interface::links():  office|irc --- " << interface.second;
  log(ssMessage.str());
}
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
  m_mutexShare.unlock();
}
// }}}
// {{{ live()
bool Interface::live(const string strApplication, const string strUser, map<string, string> message, string &strError)
{
  bool bResult = false;
  Json *ptMessage = new Json(message);

  if (live(strApplication, strUser, ptMessage, strError))
  {
    bResult = true;
  }
  delete ptMessage;

  return bResult;
}
bool Interface::live(const string strApplication, const string strUser, Json *ptMessage, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ptJson->i("Function", "message");
  ptJson->m["Request"] = new Json;
  if (!strApplication.empty())
  {
    ptJson->m["Request"]->i("Application", strApplication);
  }
  if (!strUser.empty())
  {
    ptJson->m["Request"]->i("User", strUser);
  }
  ptJson->m["Request"]->m["Message"] = new Json(ptMessage);
  if (hub("live", ptJson, strError))
  {
    bResult = true;
  }
  delete ptJson;

  return bResult;
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
      chat("#radial", ssMessage.str());
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
    if (exist(ptJson, "Response"))
    {
      for (auto &ptRow : ptJson->m["Response"]->l)
      {
        map<string, string> row;
        ptRow->flatten(row, true, false);
        rows.push_back(row);
      }
    }
    if (!empty(ptJson, "ID"))
    {
      stringstream ssID(ptJson->m["ID"]->v);
      ssID >> ullID;
    }
    if (!empty(ptJson, "Rows"))
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
  pollfd *fds;
  size_t unIndex, unPosition;
  string strError, strJson, strLine;
  stringstream ssMessage;
  time_t CBroadcast, CMaster[2], CShutdown = 0, CTime, unBroadcastSleep = 15;

  strPrefix += "->Interface::process()";
  m_pUtility->fdNonBlocking(0, strError);
  m_pUtility->fdNonBlocking(1, strError);
  time(&CBroadcast);
  CMaster[0] = CMaster[1] = CBroadcast;
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
    if (fds[0].fd != 0 || (fds[1].fd != -1 && fds[1].fd != 1))
    {
      bExit = true;
    }
    if (!bExit && (nReturn = poll(fds, unIndex, 500)) > 0)
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
            if (exist(ptJson, "_s") && ptJson->m["_s"]->v == m_strName && !empty(ptJson, "_u"))
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
            else if (exist(ptJson, "_s") && ptJson->m["_s"]->v == "hub")
            {
              if (!empty(ptJson, "Function"))
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
if (m_strNode == "office" && m_strName == "irc")
{
  stringstream ssMessage;
  ssMessage << "== 5 == Interface::process():  office|irc --- " << ptJson;
  log(ssMessage.str()); 
}
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
            else if (exist(ptJson, "Function") && ptJson->m["Function"]->v == "master")
            {
              if (!empty(ptJson, "Master"))
              {
                time(&CMaster[0]);
                if (m_strMaster != ptJson->m["Master"]->v)
                {
                  string strMaster = m_strMaster;
                  m_strMaster = ptJson->m["Master"]->v;
                  m_bMaster = ((m_strMaster == m_strNode)?true:false);
                  m_bMasterSettled = false;
                  time(&CMaster[1]);
                  if (m_pAutoModeCallback != NULL)
                  {
                    m_pAutoModeCallback(strPrefix, strMaster, m_strMaster);
                  }
                }
              }
            }
            else if (m_pCallback != NULL)
            {
              if (!empty(ptJson, "wsRequestID"))
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
                  if (!empty(ptJson, "Interface"))
                  {
                    ptLive->i("radialInterface", ptJson->m["Interface"]->v);
                  }
                  if (!empty(ptJson, "Function"))
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
    else if (!bExit && nReturn < 0 && errno != EINTR)
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
      uniques.erase(uniqueRemovals.front());
      close(uniqueRemovals.front());
      uniqueRemovals.pop_front();
    }
    monitor(strPrefix);
    if (m_pAutoModeCallback != NULL)
    {
      time(&CTime);
      if (!m_bMasterSettled && (CTime - CMaster[1]) > 30)
      {
        m_bMasterSettled = true;
      }
      if ((CTime - CBroadcast) > unBroadcastSleep)
      {
        string strMaster = m_strMaster;
        unsigned int unSeed = CTime + getpid();
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
        if ((CTime - CMaster[0]) > 15)
        {
          m_strMaster.clear();
        }
        if (m_strMaster.empty())
        {
          m_strMaster = m_strNode;
        }
        if (!m_strMaster.empty() && m_strMaster == m_strNode)
        {
          Json *ptJson = new Json;
          CMaster[0] = CTime;
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
          time(&CMaster[1]);
          m_pAutoModeCallback(strPrefix, strMaster, m_strMaster);
        }
        CBroadcast = CTime;
      }
    }
    if (shutdown())
    {
      time(&CTime);
      if (CShutdown == 0)
      {
        CShutdown = CTime;
      }
      m_mutexShare.lock();
      if ((CTime - CShutdown) > 10 && m_strBuffers[0].empty() && m_strBuffers[1].empty() && m_responses.empty() && m_waiting.empty())
      {
        bExit = true;
      }
      m_mutexShare.unlock();
    }
  }
  for (auto &unique : uniques)
  {
    close(unique.first);
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
    if ((strFunction == "retrieve" || strFunction == "retrieveKeys") && exist(ptSubJson, "Response"))
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
