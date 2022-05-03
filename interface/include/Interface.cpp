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
Interface::Interface(string strPrefix, const string strName, int argc, char **argv, function<void(string, Json *, const bool)> callback) : Base(argc, argv)
{
  strPrefix += "->Interface::Interface()";
  sigignore(SIGBUS);
  sigignore(SIGCHLD);
  sigignore(SIGCONT);
  sigignore(SIGPIPE);
  sigignore(SIGSEGV);
  sigignore(SIGTERM);
  sigignore(SIGWINCH);
  m_callback = callback;
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
  size_t unResult;
  string strMessage;
  stringstream ssMessage;

  strPrefix += "->Interface::monitor()";
  if ((unResult = Base::monitor(strMessage)) > 0)
  {
    ssMessage << strPrefix << "->Base::monitor():  " << strMessage;
    if (unResult == 2)
    {
      notify(ssMessage.str());
      setShutdown();
    } 
    else
    {
      //log(ssMessage.str());
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
  target("mysql", ptJson);
  if (ptJson->m.find("Status") != ptJson->m.end() && ptJson->m["Status"]->v == "okay")
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
  else if (ptJson->m.find("Error") != ptJson->m.end() && !ptJson->m["Error"]->v.empty())
  {
    strError = ptJson->m["Error"]->v;
  }
  else
  {
    strError = "Encountered an unknown error.";
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
  size_t unPosition;
  string strError, strLine;
  stringstream ssMessage;

  strPrefix += "->Interface::process()";
  while (!bExit)
  {
    pollfd fds[2];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[1].fd = -1;
    fds[1].events = POLLOUT;
    m_mutex.lock();
    if (m_strBuffers[1].empty() && !m_responses.empty())
    {
      m_strBuffers[1] = m_responses.front() + "\n";
      m_responses.pop_front();
    }
    m_mutex.unlock();
    if (!m_strBuffers[1].empty())
    {
      fds[1].fd = 1;
    }
    if ((nReturn = poll(fds, 2, 100)) > 0)
    {
      if (fds[0].revents & POLLIN)
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
            m_mutex.lock();
            if (ptJson->m.find("_unique") != ptJson->m.end() && m_waiting.find(ptJson->m["_unique"]->v) != m_waiting.end())
            {
              fdUnique = m_waiting[ptJson->m["_unique"]->v];
            }
            m_mutex.unlock();
            if (fdUnique != -1)
            {
              strLine += "\n";
              while (!strLine.empty() && (nReturn = write(fdUnique, strLine.c_str(), strLine.size())) > 0)
              {
                strLine.erase(0, nReturn);
              }
              close(fdUnique);
            }
            else if (ptJson->m.find("_source") != ptJson->m.end() && ptJson->m["_source"]->v == "hub")
            {
              if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
              {
                if (ptJson->m["Function"]->v == "shutdown")
                {
                  setShutdown();
                }
              }
            }
            else
            {
              m_callback(strPrefix, ptJson, true);
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
            ssMessage << strPrefix << "->read(" << errno << "):  " << strerror(errno);
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
            ssMessage << strPrefix << "->write(" << errno << "):  " << strerror(errno);
            notify(ssMessage.str());
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
    monitor(strPrefix);
    if (shutdown())
    {
      m_mutex.lock();
      if (m_strBuffers[0].empty() && m_strBuffers[1].empty() && m_responses.empty() && m_waiting.empty())
      {
        bExit = true;
        ssMessage.str("");
        ssMessage << strPrefix << ":  Exiting due to shutdown.";
        log(ssMessage.str());
      }
      m_mutex.unlock();
    }
  }
}
// }}}
// {{{ response()
void Interface::response(Json *ptJson)
{
  string strJson;

  ptJson->json(strJson);
  m_mutex.lock();
  m_responses.push_back(strJson);
  m_mutex.unlock();
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
  target("storage", ptSubJson);
  if (ptSubJson->m.find("Status") != ptSubJson->m.end() && ptSubJson->m["Status"]->v == "okay")
  {
    bResult = true;
    if ((strFunction == "retrieve" || strFunction == "retrieveKeys") && ptSubJson->m.find("Response") != ptSubJson->m.end())
    {
      ptSubJson->m["Response"]->json(strJson);
      ptJson->clear();
      ptJson->parse(strJson);
    }
  }
  else if (ptSubJson->m.find("Error") != ptSubJson->m.end() && !ptSubJson->m["Error"]->v.empty())
  {
    strError = ptSubJson->m["Error"]->v;
  }
  else
  {
    strError = "Encountered an unknown error.";
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
  return target("", ptJson, bWait);
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
    bool bGood = false;
    int readpipe[2] = {-1, -1};
    stringstream ssUnique;
    ptJson->insert("_source", m_strName);
    m_mutex.lock();
    ssUnique.str("");
    ssUnique << m_strName << "_" << unUnique;
    while (m_waiting.find(ssUnique.str()) != m_waiting.end())
    {
      unUnique++;
      ssUnique.str("");
      ssUnique << m_strName << "_" << unUnique;
    } 
    ptJson->insert("_unique", ssUnique.str());
    if (pipe(readpipe) == 0)
    {
      bGood = true;
      m_responses.push_back(ptJson->json(strJson));
      m_waiting[ssUnique.str()] = readpipe[1];
    }
    else
    {
      ssMessage.str("");
      ssMessage << "pipe(" << errno << ") " << strerror(errno);
      ptJson->insert("Status", "error");
      ptJson->insert("Error", ssMessage.str());
    }
    m_mutex.unlock();
    if (bGood)
    {
      char szBuffer[65536];
      int nReturn;
      size_t unPosition;
      strJson.clear();
      while ((nReturn = read(readpipe[0], szBuffer, 65536)) > 0)
      {
        strJson.append(szBuffer, nReturn);
      }
      if ((unPosition = strJson.find("\n")) != string::npos)
      {
        ptJson->clear();
        ptJson->parse(strJson.substr(0, unPosition));
      }
      else
      {
        ptJson->insert("Status", "error");
        ssMessage.str("");
        ssMessage << "write(" << errno << ") " << strerror(errno);
        ptJson->insert("Error", ssMessage.str());
      }
      close(readpipe[0]);
      m_mutex.lock();
      m_waiting.erase(ssUnique.str());
      m_mutex.unlock();
    }
  }
  else
  {
    m_mutex.lock();
    m_responses.push_back(ptJson->json(strJson));
    m_mutex.unlock();
  }
}
// }}}
}
}
