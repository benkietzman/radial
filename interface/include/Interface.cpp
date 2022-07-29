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
  sigignore(SIGBUS);
  sigignore(SIGCHLD);
  sigignore(SIGCONT);
  sigignore(SIGPIPE);
  sigignore(SIGSEGV);
  sigignore(SIGTERM);
  sigignore(SIGWINCH);
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
  Json *ptAuth = new Json(ptJson);

  keyRemovals(ptAuth);
  if (ptAuth->m.find("Interface") != ptAuth->m.end() && !ptAuth->m["Interface"]->v.empty())
  {
    if (ptAuth->m.find("Request") != ptAuth->m.end())
    {
      delete ptAuth->m["Request"];
    }
    ptAuth->m["Request"] = new Json;
    ptAuth->m["Request"]->insert("Interface", ptAuth->m["Interface"]->v);
  }
  if (hub("auth", ptAuth, strError))
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
  ptJson->insert("Database", strDatabase);
  ptJson->insert("Query", strQuery);
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
  ptJson->insert("Database", strDatabase);
  ptJson->insert("Update", strUpdate);
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
    ptJson->insert("_target", strTarget);
  }
  if (bWait)
  {
    int fdUnique[2] = {-1, -1}, nReturn;
    stringstream ssMessage;
    ptJson->insert("_source", m_strName);
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
      ptJson->insert("_unique", ssUnique.str());
      m_waiting[ssUnique.str()] = fdUnique[1];
      ptJson->json(strJson);
      m_responses.push_back(strJson);
      m_mutexShare.unlock();
      while (!bExit)
      {
        pollfd fds[1];
        fds[0].fd = fdUnique[0];
        fds[0].events = POLLIN;
        if ((nReturn = poll(fds, 1, 100)) > 0)
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
        ptJson->insert("Status", "error");
        ptJson->insert("Error", ((!strError.empty())?strError:"Encountered an uknown error."));
      }
    }
    else
    {
      ssMessage.str("");
      ssMessage << "pipe(" << errno << ") " << strerror(errno);
      ptJson->insert("Status", "error");
      ptJson->insert("Error", ssMessage.str());
    }
    keyRemovals(ptJson);
  }
  else
  {
    ptJson->json(strJson);
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
// {{{ log()
void Interface::log(const string strMessage)
{
  log("log", strMessage);
}
void Interface::log(const string strFunction, const string strMessage)
{
  Json *ptJson = new Json;

  ptJson->insert("Function", strFunction);
  ptJson->insert("Message", strMessage);
  hub("log", ptJson, false);

  delete ptJson;
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

  ptJson->insert("Server", strServer);
  ssPort << unPort;
  ptJson->insert("Port", ssPort.str(), 'n');
  ptJson->insert("User", strUser);
  ptJson->insert("Password", strPassword);
  ptJson->insert("Database", strDatabase);
  ptJson->insert(strType, strQuery);
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
  string strError, strLine;
  stringstream ssMessage;

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
    if ((nReturn = poll(fds, unIndex, 100)) > 0)
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
            if (ptJson->m.find("_source") != ptJson->m.end() && ptJson->m["_source"]->v == m_strName && ptJson->m.find("_unique") != ptJson->m.end() && !ptJson->m["_unique"]->v.empty())
            {
              m_mutexShare.lock();
              if (m_waiting.find(ptJson->m["_unique"]->v) != m_waiting.end())
              {
                fdUnique = m_waiting[ptJson->m["_unique"]->v];
              }
              m_mutexShare.unlock();
            }
            if (fdUnique != -1)
            {
              uniques[fdUnique] = strLine + "\n";
            }
            else if (ptJson->m.find("_source") != ptJson->m.end() && ptJson->m["_source"]->v == "hub")
            {
              if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
              {
                if (ptJson->m["Function"]->v == "shutdown")
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << ":  Shutting down.";
                  log(ssMessage.str());
                  setShutdown();
                }
              }
            }
            else if (m_pCallback != NULL)
            {
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
// {{{ storage
// {{{ storage()
bool Interface::storage(const string strFunction, const list<string> keys, Json *ptJson, string &strError)
{
  bool bResult = false;
  string strJson;
  Json *ptSubJson = new Json;

  ptSubJson->insert("Function", strFunction);
  if (!keys.empty())
  {
    ptSubJson->insert("Keys", keys);
  }
  if ((strFunction == "add" || strFunction == "update") && ptJson != NULL)
  {
    ptSubJson->insert("Request", ptJson);
  }
  if (hub("storage", ptSubJson, strError))
  {
    bResult = true;
    if ((strFunction == "retrieve" || strFunction == "retrieveKeys") && ptSubJson->m.find("Response") != ptSubJson->m.end())
    {
      ptSubJson->m["Response"]->json(strJson);
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
