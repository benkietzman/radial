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
  if (strName != "log")
  {
    msleep(500);
  }
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

  if (target("auth", ptAuth, strError))
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
  if (target("database", ptJson, strError))
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
  if (target("database", ptJson, strError))
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
  target("log", ptJson, false);

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
  if (target("mysql", ptJson, strError))
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
  char szBuffer[65536];
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
      fds[unIndex].fd = unique.first;
      fds[unIndex].events = POLLOUT;
      unIndex++;
    }
    if ((nReturn = poll(fds, unIndex, 100)) > 0)
    {
      if (fds[0].revents & (POLLHUP | POLLIN))
      {
        if ((nReturn = read(fds[0].fd, szBuffer, 65536)) > 0)
        {
          m_strBuffers[0].append(szBuffer, nReturn);
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
            else
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
            ssMessage << strPrefix << "->read(" << errno << ") [0]:  " << strerror(errno);
            notify(ssMessage.str());
          }
        }
      }
      if (fds[1].revents & POLLOUT)
      {
        if ((nReturn = write(fds[1].fd, m_strBuffers[1].c_str(), m_strBuffers[1].size())) > 0)
        {
          m_strBuffers[1].erase(0, nReturn);
        }
        else
        {
          bExit = true;
          if (nReturn < 0)
          {
            ssMessage.str("");
            ssMessage << strPrefix << "->write(" << errno << ") [1]:  " << strerror(errno);
            notify(ssMessage.str());
          }
        }
      }
      for (size_t i = 2; i < unIndex; i++)
      {
        if (fds[i].revents & POLLOUT)
        {
          if ((nReturn = write(fds[i].fd, uniques[fds[i].fd].c_str(), uniques[fds[i].fd].size())) > 0)
          {
            uniques[fds[i].fd].erase(0, nReturn);
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
              ssMessage << strPrefix << "->write(" << errno << ") [" << fds[i].fd << "]:  " << strerror(errno);
              notify(ssMessage.str());
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
// {{{ response()
void Interface::response(Json *ptJson)
{
  string strJson;

  ptJson->json(strJson);
  m_mutexShare.lock();
  m_responses.push_back(strJson);
  m_mutexShare.unlock();
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
  if (target("storage", ptSubJson, strError))
  {
    bResult = true;
    if ((strFunction == "retrieve" || strFunction == "retrieveKeys") && ptSubJson->m.find("Response") != ptSubJson->m.end())
    {
      ptSubJson->m["Response"]->json(strJson);
      ptJson->clear();
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
// {{{ target()
void Interface::target(Json *ptJson, const bool bWait)
{
  target("", ptJson, bWait);
}
void Interface::target(const string strTarget, Json *ptJson, const bool bWait)
{
  size_t unUnique = 0;
  string strJson;
  stringstream ssMessage;

  if (!strTarget.empty())
  {
    ptJson->insert("_target", strTarget);
  }
  if (bWait)
  {
    int nReturn, readpipe[2] = {-1, -1};
    stringstream ssUnique;
    ptJson->insert("_source", m_strName);
    if ((nReturn = pipe(readpipe)) == 0)
    {
      long lArg;
      if ((lArg = fcntl(readpipe[0], F_GETFL, NULL)) >= 0)
      {
        lArg |= O_NONBLOCK;
        fcntl(readpipe[0], F_SETFL, lArg);
      }
      if ((lArg = fcntl(readpipe[1], F_GETFL, NULL)) >= 0)
      {
        lArg |= O_NONBLOCK;
        fcntl(readpipe[1], F_SETFL, lArg);
      }
    }
    m_mutexShare.lock();
    ssUnique.str("");
    ssUnique << m_strName << "_" << unUnique;
    while (m_waiting.find(ssUnique.str()) != m_waiting.end())
    {
      unUnique++;
      ssUnique.str("");
      ssUnique << m_strName << "_" << unUnique;
    }
    ptJson->insert("_unique", ssUnique.str());
    if (nReturn == 0)
    {
      m_waiting[ssUnique.str()] = readpipe[1];
    }
    m_mutexShare.unlock();
    if (nReturn == 0)
    {
      bool bExit = false;
      char szBuffer[65536];
      size_t unPosition;
      strJson.clear();
      response(ptJson);
      while (!bExit)
      {
        pollfd fds[1];
        fds[1].fd = readpipe[0];
        fds[1].events = POLLIN;
        if ((nReturn = poll(fds, 1, 100)) > 0)
        {
          if (fds[0].revents & (POLLHUP | POLLIN))
          {
            if ((nReturn = read(fds[0].fd, szBuffer, 65536)) > 0)
            {
              strJson.append(szBuffer, nReturn);
            }
            else
            {
              bExit = true;
            }
          }
        }
        else if (nReturn < 0 && errno != EINTR)
        {
          bExit = true;
        }
      }
      close(readpipe[0]);
      m_mutexShare.lock();
      m_waiting.erase(ssUnique.str());
      m_mutexShare.unlock();
      if ((unPosition = strJson.find("\n")) != string::npos)
      {
        ptJson->clear();
        ptJson->parse(strJson.substr(0, unPosition));
      }
      else
      {
        ptJson->insert("Status", "error");
        ssMessage.str("");
        ssMessage << "read(" << errno << ") " << strerror(errno);
        ptJson->insert("Error", ssMessage.str());
      }
    }
    else
    {
      ssMessage.str("");
      ssMessage << "pipe(" << errno << ") " << strerror(errno);
      ptJson->insert("Status", "error");
      ptJson->insert("Error", ssMessage.str());
    }
  }
  else
  {
    response(ptJson);
  }
}
bool Interface::target(Json *ptJson, string &strError)
{
  return target("", ptJson, strError);
}
bool Interface::target(const string strTarget, Json *ptJson, string &strError)
{
  bool bResult = false;

  target(strTarget, ptJson, true);
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
}
}
