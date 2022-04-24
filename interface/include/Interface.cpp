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
Interface::Interface(const string strName, int argc, char **argv) : Base(argc, argv)
{
  m_strName = strName;
  m_unUnique = 0;
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
// {{{ notify()
void Interface::notify(const string strMessage)
{
  log("notify", strMessage);
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
    if (ptJson->m.find("Data") != ptJson->m.end())
    {
      for (auto &ptRow : ptJson->m["Data"]->l)
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
// {{{ process()
void Interface::process(string strPrefix, function<void(string, Json *)> callback)
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
    if (!m_strBuffers[1].empty())
    {
      fds[1].fd = 1;
    }
    m_mutex.unlock();
    if ((nReturn = poll(fds, 2, 250)) > 0)
    {
      if (fds[0].revents & POLLIN)
      {
        if ((nReturn = read(fds[0].fd, szBuffer, 65536)) > 0)
        {
          m_strBuffers[0].append(szBuffer, nReturn);
          while ((unPosition = m_strBuffers[0].find("\n")) != string::npos)
          {
            Json *ptJson;
            strLine = m_strBuffers[0].substr(0, unPosition);
            m_strBuffers[0].erase(0, (unPosition + 1));
            ptJson = new Json(strLine);
            if (ptJson->m.find("_unique") != ptJson->m.end())
            {
              size_t unUnique;
              stringstream ssUnique(ptJson->m["_unique"]->v);
              ssUnique >> unUnique;
              m_mutex.lock();
              if (m_waiting.find(unUnique) != m_waiting.end())
              {
                int nReturn;
                while (!strLine.empty() && (nReturn = write(m_waiting[unUnique], strLine.c_str(), strLine.size())) > 0)
                {
                  strLine.erase(0, nReturn);
                }
                close(m_waiting[unUnique]);
              }
              m_mutex.unlock();
            }
            else
            {
              callback(strPrefix, ptJson);
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
            ssMessage << "read(" << errno << ") " << strerror(errno);
            notify(ssMessage.str());
          }
        }
      }
      if (fds[1].revents & POLLOUT)
      {
        m_mutex.lock();
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
            ssMessage << "write(" << errno << ") " << strerror(errno);
            notify(ssMessage.str());
          }
        }
        m_mutex.unlock();
      }
    }
    else if (nReturn < 0)
    {
      bExit = true;
      ssMessage.str("");
      ssMessage << "poll(" << errno << ") " << strerror(errno);
      notify(ssMessage.str());
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
// {{{ target()
void Interface::target(const string strTarget, Json *ptJson, const bool bWait)
{
  size_t unUnique;
  string strJson;
  stringstream ssMessage;

  ptJson->insert("Target", strTarget);
  if (bWait)
  {
    bool bGood = false;
    int readpipe[2] = {-1, -1};
    stringstream ssUnique;
    ptJson->insert("Source", m_strName);
    m_mutex.lock();
    while (m_waiting.find(m_unUnique) != m_waiting.end())
    {
      m_unUnique++;
    }
    unUnique = m_unUnique++;
    ssUnique << unUnique;
    ptJson->insert("_unique", ssUnique.str(),'n');
    if (pipe(readpipe) == 0)
    {
      bGood = true;
      m_responses.push_back(ptJson->json(strJson));
      m_waiting[unUnique] = readpipe[1];
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
      while ((nReturn = read(readpipe[0], szBuffer, 65536)) > 0)
      {
        strJson.append(szBuffer, nReturn);
      }
      ptJson->clear();
      ptJson->parse(strJson);
      close(readpipe[0]);
      m_mutex.lock();
      m_waiting.erase(unUnique);
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
