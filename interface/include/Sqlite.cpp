// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Sqlite.cpp
// author     : Ben Kietzman
// begin      : 2024-10-21
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Sqlite"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Sqlite()
Sqlite::Sqlite(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), int (*pCallbackFetch)(void *, int, char **, char **)) : Interface(strPrefix, "sqlite", argc, argv, pCallback)
{
  m_bMasterUpdated = false;
  m_pCallbackFetch = pCallbackFetch;
  m_pThreadInotify = new thread(&Sqlite::inotify, this, strPrefix);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
}
// }}}
// {{{ ~Sqlite()
Sqlite::~Sqlite()
{
  m_pThreadInotify->join();
  delete m_pThreadInotify;
}
// }}}
// {{{ autoMode()
void Sqlite::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->Sqlite::autoMode()";
  if (strOldMaster != strNewMaster)
  {
    stringstream ssMessage;
    ssMessage << strPrefix << " [" << strNewMaster << "]:  Updated master.";
    log(ssMessage.str());
    m_bMasterUpdated = true;
  }
  threadDecrement();
}
// }}}
// {{{ callback()
void Sqlite::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError, strValue;
  stringstream ssMessage;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Sqlite::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    string strFunction = ptJson->m["Function"]->v;
    // {{{ list
    if (strFunction == "list")
    {
      bResult = true;
      if (exist(ptJson, "Response"))
      {
        delete ptJson->m["Response"];
        ptJson->m.erase("Response");
      }
      ptJson->m["Response"] = new Json;
      m_mutex.lock();
      for (auto &i : m_databases)
      {
        ptJson->m["Response"]->m[i.first] = new Json;
        for (auto &j : i.second)
        {
          ptJson->m["Response"]->m[i.first]->i(j.first, ((j.second)?"master":"slave"));
        }
      }
      m_mutex.unlock();
    }
    // }}}
    // {{{ request
    else if (exist(ptJson, "Request"))
    {
      if (!empty(ptJson->m["Request"], "Database"))
      {
        string strDatabase = ptJson->m["Request"]->m["Database"]->v, strNode;
        if (!empty(ptJson->m["Request"], "Node"))
        {
          strNode = ptJson->m["Request"]->m["Node"]->v;
        }
        // {{{ query
        if (strFunction == "query")
        {
          if (!empty(ptJson->m["Request"], "Statement"))
          {
            string strAction, strStatement = ptJson->m["Request"]->m["Statement"]->v, strValue;
            stringstream ssStatement(strStatement);
            ssStatement >> strValue;
            m_manip.toLower(strAction, strValue);
            if (strAction == "select" || isMasterSettled())
            {
              bool bLocal = false;
              m_mutex.lock();
              if (m_databases.find(strDatabase) != m_databases.end() && m_databases[strDatabase].find(m_strNode) != m_databases[strDatabase].end() && (strAction == "query" || m_databases[strDatabase][m_strNode] || (!strNode.empty() && strNode == m_strNode)))
              {
                bLocal = true;
              }
              m_mutex.unlock();
              if (bLocal)
              {
                int nReturn;
                sqlite3 *db;
                stringstream ssFile, ssError;
                ssFile << "file:" << m_strData << "/sqlite/" << ptJson->m["Database"]->v << ".db";
                if ((nReturn = sqlite3_open(ssFile.str().c_str(), &db)) == SQLITE_OK)
                {
                  if (strAction == "select")
                  {
                    char *pszError = NULL;
                    list<map<string, string> > rows;
                    Json *ptRows = new Json;
                    if ((nReturn = sqlite3_exec(db, strStatement.c_str(), m_pCallbackFetch, ptRows, &pszError)) == SQLITE_OK)
                    {
                      size_t unSize = 16;
                      string strJson;
                      stringstream ssRows;
                      ssRows << ptRows->l.size();
                      ptJson->i("Rows", ssRows.str(), 'n');
                      if (exist(ptJson, "Response"))
                      {
                        delete ptJson->m["Response"];
                        ptJson->m.erase("Response");
                      }
                      unSize += ptJson->j(strJson).size() + 13;
                      for (auto i = ptRows->l.begin(); unSize < m_unMaxPayload && i != ptRows->l.end(); i++)
                      {
                        for (auto &j : (*i)->m)
                        {
                          unSize += j.first.size() + j.second->v.size() + 6;
                        }
                      }
                      if (unSize < m_unMaxPayload)
                      {
                        bResult = true;
                        ptJson->m["Response"] = ptRows;
                      }
                      else
                      {
                        delete ptRows;
                        ssMessage.str("");
                        ssMessage << "Payload of " << m_manip.toShortByte(unSize, strValue) << " exceeded " << m_manip.toShortByte(m_unMaxPayload, strValue) << " maximum.  Response has been removed.";
                        strError = ssMessage.str();
                      }
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << "sqlite3_exec(" << nReturn << ") " << pszError;
                      sqlite3_free(pszError);
                      strError = ssMessage.str();
                    }
                  }
                  else
                  {
                    char *pszError = NULL;
                    if ((nReturn = sqlite3_exec(db, strStatement.c_str(), NULL, NULL, &pszError)) == SQLITE_OK)
                    {
                      list<string> nodes;
                      stringstream ssRows;
                      bResult = true;
                      ssRows << sqlite3_changes(db);
                      ptJson->i("Rows", ssRows.str(), 'n');
                      if (strAction == "insert")
                      {
                        char *pszError = NULL;
                        Json *ptRows = new Json;
                        if ((nReturn = sqlite3_exec(db, "select last_insert_rowid()", m_pCallbackFetch, ptRows, &pszError)) == SQLITE_OK)
                        {
                          if (!ptRows->l.empty() && !empty(ptRows->l.front(), "last_insert_rowid()"))
                          {
                            ptJson->i("ID", ptRows->l.front()->m["last_insert_rowid()"]->v, 'n');
                          }
                        }
                        else
                        {
                          sqlite3_free(pszError);
                        }
                        delete ptRows;
                      }
                      m_mutex.lock();
                      if (m_databases.find(strDatabase) != m_databases.end() && m_databases[strDatabase].find(m_strNode) != m_databases[strDatabase].end() && m_databases[strDatabase][m_strNode])
                      {
                        for (auto &i : m_databases[strDatabase])
                        {
                          if (i.first != m_strNode)
                          {
                            nodes.push_back(i.first);
                          }
                        }
                      }
                      m_mutex.unlock();
                      while (!nodes.empty())
                      {
                        Json *ptLink = new Json(ptJson);
                        ptLink->i("Interface", "sqlite");
                        ptLink->i("Node", nodes.front());
                        hub("link", ptLink, strError);
                        delete ptLink;
                        nodes.pop_front();
                      }
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << "sqlite3_exec(" << nReturn << ") " << pszError;
                      sqlite3_free(pszError);
                      strError = ssMessage.str();
                    }
                  }
                }
                else
                {
                  ssMessage.str("");
                  ssMessage << "sqlit3_open(" << nReturn << ") " << sqlite3_errmsg(db);
                }
                sqlite3_close(db);
              }
              else if (strAction == "select")
              {
                vector<string> nodes;
                m_mutex.lock();
                if (m_databases.find(strDatabase) != m_databases.end())
                {
                  for (auto &i : m_databases[strDatabase])
                  {
                    nodes.push_back(i.first);
                  }
                }
                m_mutex.unlock();
                if (!nodes.empty())
                {
                  unsigned int unPick = 0, unSeed = time(NULL);
                  Json *ptLink = new Json(ptJson);
                  srand(unSeed);
                  unPick = rand_r(&unSeed) % nodes.size();
                  ptLink->i("Interface", "sqlite");
                  ptLink->i("Node", nodes[unPick]);
                  if (hub("link", ptLink, strError))
                  {
                    bResult = true;
                    if (!empty(ptLink, "ID"))
                    {
                      ptJson->i("ID", ptLink->m["ID"]->v);
                    }
                    if (exist(ptLink, "Response"))
                    {
                      ptJson->i("Response", ptLink->m["Response"]);
                    }
                    if (!empty(ptLink, "Rows"))
                    {
                      ptJson->i("Rows", ptLink->m["Rows"]->v);
                    }
                  }
                  delete ptLink;
                }
                else
                {
                  strError = "Please provide a valid Database.";
                }
              }
              else
              {
                bool bExists = false;
                string strSubNode;
                m_mutex.lock();
                if (m_databases.find(strDatabase) != m_databases.end())
                {
                  for (auto i = m_databases[strDatabase].begin(); strSubNode.empty() && i != m_databases[strDatabase].end(); i++)
                  {
                    bExists = true;
                    if (i->second)
                    {
                      strSubNode = i->first;
                    }
                  }
                }
                m_mutex.unlock();
                if (!strSubNode.empty())
                {
                  Json *ptLink = new Json(ptJson);
                  ptLink->i("Interface", "sqlite");
                  ptLink->i("Node", strSubNode);
                  if (hub("link", ptLink, strError))
                  {
                    bResult = true;
                    if (!empty(ptLink, "ID"))
                    {
                      ptJson->i("ID", ptLink->m["ID"]->v);
                    }
                    if (exist(ptLink, "Response"))
                    {
                      ptJson->i("Response", ptLink->m["Response"]);
                    }
                    if (!empty(ptLink, "Rows"))
                    {
                      ptJson->i("Rows", ptLink->m["Rows"]->v);
                    }
                  }
                  delete ptLink;
                }
                else if (bExists)
                {
                  strError = "Missing master for this Database.  Try again later.";
                }
                else
                {
                  strError = "Please provide a valid Database.";
                }
              }
            }
            else
            {
              strError = "Master is not settled.";
            }
          }
          else
          {
            strError = "Please provide the Statement within the Request.";
          }
        }
        // }}}
        // {{{ node
        else if (!strNode.empty())
        {
          // {{{ add
          if (strFunction == "add")
          {
            bResult = true;
            if (isMaster())
            {
              bool bMaster = false;
              databaseAdd(strDatabase, strNode, bMaster);
              if (bMaster)
              {
                databaseMaster(strDatabase, strNode);
              }
            }
            else
            {
              m_mutex.lock();
              if (m_databases.find(strDatabase) == m_databases.end())
              {
                m_databases[strDatabase] = {};
              }
              m_databases[strDatabase][strNode] = false;
              m_mutex.unlock();
            }
          }
          // }}}
          // {{{ master
          else if (strFunction == "master")
          {
            bResult = true;
            m_mutex.lock();
            if (m_databases.find(strDatabase) != m_databases.end())
            {
              m_databases[strDatabase][strNode] = true;
            }
            m_mutex.unlock();
          }
          // }}}
          // {{{ remove
          else if (strFunction == "remove")
          {
            bResult = true;
            if (isMaster())
            {
              string strMaster;
              databaseRemove(strDatabase, strNode, strMaster);
              if (!strMaster.empty())
              {
                databaseMaster(strDatabase, strMaster);
              }
            }
            else
            {
              m_mutex.lock();
              if (m_databases.find(strDatabase) != m_databases.end() && m_databases[strDatabase].find(strNode) != m_databases[strDatabase].end())
              {
                m_databases[strDatabase].erase(strNode);
                if (m_databases[strDatabase].empty())
                {
                  m_databases.erase(strDatabase);
                }
              }
              m_mutex.unlock();
            }
          }
          // }}}
          // {{{ invalid
          else
          {
            strError = "Please provide a valid Function:  add, list, master, query. remove.";
          }
          // }}}
        }
        // }}}
        // {{{ invalid
        else
        {
          strError = "Please provide the Node within the Request.";
        }
        // }}}
      }
      else
      {
        strError = "Please provide the Database within the Request.";
      }
    }
    // }}}
    // {{{ invalid
    else
    {
      strError = "Please provide the Request.";
    }
    // }}}
  }
  else
  {
    strError = "Please provide the Function.";
  }
  ptJson->i("Status", ((bResult)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->i("Error", strError);
  }
  if (bResponse)
  {
    ptJson->j(p.p);
    hub(p, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
// {{{ callbackFetch()
int Sqlite::callbackFetch(void *vptRows, int nCols, char *szCols[], char *szNames[])
{
  int nResult = SQLITE_OK;
  map<string, string> row;
  Json *ptRows = (Json *)vptRows;

  for (int i = 0; i < nCols; i++)
  {
    row[szNames[i]] = szCols[i];
  }
  ptRows->push_back(row);

  return nResult;
}
// }}}
// {{{ databaseAdd()
void Sqlite::databaseAdd(const string strDatabase, const string strNode, bool &bMaster)
{
  list<string> nodes;
  string strError;
  Json *ptLink;

  bMaster = false;
  m_mutex.lock();
  if (m_databases.find(strDatabase) == m_databases.end())
  {
    bMaster = true;
    m_databases[strDatabase] = {};
  }
  m_databases[strDatabase][strNode] = false;
  m_mutex.unlock();
  m_mutexShare.lock();
  for (auto &link : m_l)
  {
    if (link->interfaces.find("sqlite") != link->interfaces.end())
    {
      nodes.push_back(link->strNode);
    }
  }
  m_mutexShare.unlock();
  while (!nodes.empty())
  {
    ptLink = new Json;
    ptLink->i("Interface", "sqlite");
    ptLink->i("Node", nodes.front());
    ptLink->i("Function", "add");
    ptLink->m["Request"] = new Json;
    ptLink->m["Request"]->i("Database", strDatabase);
    ptLink->m["Request"]->i("Node", strNode);
    hub("link", ptLink, strError);
    delete ptLink;
    nodes.pop_front();
  }
}
// }}}
// {{{ databaseMaster()
void Sqlite::databaseMaster(const string strDatabase, const string strNode)
{
  list<string> nodes;
  string strError;
  Json *ptLink;

  m_mutex.lock();
  if (m_databases.find(strDatabase) != m_databases.end() && m_databases[strDatabase].find(strNode) != m_databases[strDatabase].end())
  {
    m_databases[strDatabase][strNode] = true;
  }
  m_mutex.unlock();
  m_mutexShare.lock();
  for (auto &link : m_l)
  {
    if (link->interfaces.find("sqlite") != link->interfaces.end())
    {
      nodes.push_back(link->strNode);
    }
  }
  m_mutexShare.unlock();
  while (!nodes.empty())
  {
    ptLink = new Json;
    ptLink->i("Interface", "sqlite");
    ptLink->i("Node", nodes.front());
    ptLink->i("Function", "master");
    ptLink->m["Request"] = new Json;
    ptLink->m["Request"]->i("Database", strDatabase);
    ptLink->m["Request"]->i("Node", strNode);
    hub("link", ptLink, strError);
    delete ptLink;
    nodes.pop_front();
  }
}
// }}}
// {{{ databaseRemove()
void Sqlite::databaseRemove(const string strDatabase, const string strNode, string &strMaster)
{
  list<string> nodes;
  string strError;
  Json *ptLink;

  strMaster.clear();
  m_mutex.lock();
  if (m_databases.find(strDatabase) != m_databases.end() && m_databases[strDatabase].find(strNode) != m_databases[strDatabase].end())
  {
    bool bMaster = m_databases[strDatabase][strNode];
    m_databases[strDatabase].erase(strNode);
    if (m_databases[strDatabase].empty())
    {
      m_databases.erase(strDatabase);
    }
    else if (bMaster)
    {
      unsigned int unPick = 0, unSeed = time(NULL);
      vector<string> subNodes;
      for (auto &i : m_databases[strDatabase])
      {
        subNodes.push_back(i.first);
      }
      srand(unSeed);
      unPick = rand_r(&unSeed) % subNodes.size();
      strMaster = subNodes[unPick];
    }
  }
  m_mutex.unlock();
  m_mutexShare.lock();
  for (auto &link : m_l)
  {
    if (link->interfaces.find("sqlite") != link->interfaces.end())
    {
      nodes.push_back(link->strNode);
    }
  }
  m_mutexShare.unlock();
  while (!nodes.empty())
  {
    ptLink = new Json;
    ptLink->i("Interface", "sqlite");
    ptLink->i("Node", nodes.front());
    ptLink->i("Function", "remove");
    ptLink->m["Request"] = new Json;
    ptLink->m["Request"]->i("Database", strDatabase);
    ptLink->m["Request"]->i("Node", strNode);
    hub("link", ptLink, strError);
    delete ptLink;
    nodes.pop_front();
  }
}
// }}}
// {{{ inotify()
void Sqlite::inotify(string strPrefix)
{
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Sqlite::inotify()";
  while (!shutdown())
  {
    if (isMasterSettled())
    {
      int fdNotify;
      if (isMaster())
      {
        list<string> nodes;
        m_mutexShare.lock();
        for (auto &link : m_l)
        {
          if (link->interfaces.find("sqlite") != link->interfaces.end())
          {
            nodes.push_back(link->strNode);
          }
        }
        m_mutexShare.unlock();
        while (!nodes.empty())
        {
          map<string, map<string, bool> > databases;
          Json *ptJson = new Json;
          ptJson->i("Interface", "sqlite");
          ptJson->i("Node", nodes.front());
          ptJson->i("Function", "list");
          if (hub("link", ptJson, strError) && exist(ptJson, "Response"))
          {
            for (auto &i : ptJson->m["Response"]->m)
            {
              for (auto &j : i.second->m)
              {
                bool bMaster = false;
                databaseAdd(i.first, j.first, bMaster);
                if (bMaster)
                {
                  databaseMaster(i.first, j.first);
                }
              }
            }
          }
          delete ptJson;
          m_mutex.lock();
          databases = m_databases;
          m_mutex.unlock();
          for (auto &i : databases)
          {
            for (auto &j : i.second)
            {
              ptJson = new Json;
              ptJson->i("Interface", "sqlite");
              ptJson->i("Node", nodes.front());
              ptJson->i("Function", "add");
              ptJson->m["Request"] = new Json;
              ptJson->m["Request"]->i("Database", i.first);
              ptJson->m["Request"]->i("Node", j.first);
              hub("link", ptJson, strError);
              delete ptJson;
              if (j.second)
              {
                ptJson = new Json;
                ptJson->i("Interface", "sqlite");
                ptJson->i("Node", nodes.front());
                ptJson->i("Function", "master");
                ptJson->m["Request"] = new Json;
                ptJson->m["Request"]->i("Database", i.first);
                ptJson->m["Request"]->i("Node", j.first);
                hub("link", ptJson, strError);
                delete ptJson;
              }
            }
          }
        }
      }
      else
      {
        Json *ptJson = new Json;
        ptJson->i("Interface", "sqlite");
        ptJson->i("Node", master());
        ptJson->i("Function", "list");
        if (hub("link", ptJson, strError) && exist(ptJson, "Response"))
        {
          m_mutex.lock();
          for (auto &i : ptJson->m["Response"]->m)
          {
            for (auto &j : i.second->m)
            {
              if (m_databases.find(i.first) == m_databases.end())
              {
                m_databases[i.first] = {};
              }
              m_databases[i.first][j.first] = ((j.second->v == "master")?true:false);
            }
          }
          m_mutex.unlock();
        }
        delete ptJson;
      }
      if ((fdNotify = inotify_init1(IN_NONBLOCK)) != -1)
      {
        int wdNotify;
        if ((wdNotify = inotify_add_watch(fdNotify, (m_strData + "/sqlite").c_str(), (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO))) != -1)
        {
          bool bExit = false, bMasterUpdated;
          inotify_event *pEvent;
          int nReturn;
          list<string> entries;
          string strNotify;
          Json *ptLink;
          m_file.directoryList(m_strData + "/sqlite", entries);
          while (!entries.empty())
          {
            if (entries.front().size() > 3 && entries.front().substr((entries.front().size() - 3), 3) == ".db")
            {
              string strDatabase = entries.front().substr(0, (entries.front().size() - 3));
              if (isMaster())
              {
                bool bMaster = false;
                databaseAdd(strDatabase, m_strNode, bMaster);
                if (bMaster)
                {
                  databaseMaster(strDatabase, m_strNode);
                }
              }
              else
              {
                ptLink = new Json;
                ptLink->i("Interface", "sqlite");
                ptLink->i("Node", master());
                ptLink->i("Function", "add");
                ptLink->m["Request"] = new Json;
                ptLink->m["Request"]->i("Database", strDatabase);
                ptLink->m["Request"]->i("Node", m_strNode);
                hub("link", ptLink, strError);
                delete ptLink;
              }
            }
            entries.pop_front();
          }
          while (!bExit)
          {
            pollfd fds[1];
            fds[0].fd = fdNotify;
            fds[0].events = POLLIN;
            if ((nReturn = poll(fds, 1, 2000)) > 0)
            {
              if (fds[0].revents & POLLIN)
              {
                if (m_pUtility->fdRead(fds[0].fd, strNotify, nReturn))
                {
                  while (strNotify.size() >= sizeof(inotify_event))
                  {
                    pEvent = (inotify_event *)strNotify.c_str();
                    if (pEvent->wd == wdNotify && pEvent->len > 0 && (pEvent->mask & (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO)))
                    {
                      string strName = pEvent->name;
                      if (strName.size() > 3 && strName.substr((strName.size() - 3), 3) == ".db")
                      {
                        string strDatabase = strName.substr(0, (strName.size() - 3));
                        if (pEvent->mask & (IN_CREATE | IN_MOVED_TO))
                        {
                          if (isMaster())
                          {
                            bool bMaster = false;
                            databaseAdd(strDatabase, m_strNode, bMaster);
                            if (bMaster)
                            {
                              databaseMaster(strDatabase, m_strNode);
                            }
                          }
                          else
                          {
                            ptLink = new Json;
                            ptLink->i("Interface", "sqlite");
                            ptLink->i("Node", master());
                            ptLink->i("Function", "add");
                            ptLink->m["Request"] = new Json;
                            ptLink->m["Request"]->i("Database", strDatabase);
                            ptLink->m["Request"]->i("Node", m_strNode);
                            hub("link", ptLink, strError);
                            delete ptLink;
                          }
                        }
                        else if (pEvent->mask & (IN_DELETE | IN_MOVED_FROM))
                        {
                          if (isMaster())
                          {
                            string strMaster;
                            databaseRemove(strDatabase, m_strNode, strMaster);
                            if (!strMaster.empty())
                            {
                              databaseMaster(strDatabase, strMaster);
                            }
                          }
                          else
                          {
                            ptLink = new Json;
                            ptLink->i("Interface", "sqlite");
                            ptLink->i("Node", master());
                            ptLink->i("Function", "remove");
                            ptLink->m["Request"] = new Json;
                            ptLink->m["Request"]->i("Database", strDatabase);
                            ptLink->m["Request"]->i("Node", m_strNode);
                            hub("link", ptLink, strError);
                            delete ptLink;
                          }
                        }
                      }
                    }
                    strNotify.erase(0, sizeof(inotify_event));
                  }
                }
                else
                {
                  bExit = true;
                  if (nReturn < 0)
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Utility::fdRead(" << errno << ") error [" << m_strData << "/sqlite]:  " << strerror(errno);
                    log(ssMessage.str());
                  }
                }
              }
            }
            else if (nReturn < 0 && errno != EINTR)
            {
              bExit = true;
              ssMessage.str("");
              ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
              log(ssMessage.str());
            }
            if (m_bMasterUpdated && shutdown())
            {
              bExit = true;
            }
          }
          inotify_rm_watch(fdNotify, wdNotify);
          if (!shutdown())
          {
            m_file.directoryList(m_strData + "/sqlite", entries);
            while (!entries.empty())
            {
              if (entries.front().size() > 3 && entries.front().substr((entries.front().size() - 3), 3) == ".db")
              {
                string strDatabase = entries.front().substr(0, (entries.front().size() - 3));
                if (isMaster())
                {
                  string strMaster;
                  databaseRemove(strDatabase, m_strNode, strMaster);
                  if (!strMaster.empty())
                  {
                    databaseMaster(strDatabase, strMaster);
                  }
                }
                else
                {
                  ptLink = new Json;
                  ptLink->i("Interface", "sqlite");
                  ptLink->i("Node", master());
                  ptLink->i("Function", "remove");
                  ptLink->m["Request"] = new Json;
                  ptLink->m["Request"]->i("Database", strDatabase);
                  ptLink->m["Request"]->i("Node", m_strNode);
                  hub("link", ptLink, strError);
                  delete ptLink;
                }
              }
              entries.pop_front();
            }
          }
        }
        else
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->inotify_add_watch(" << errno << ") error [" << m_strData << "/sqlite]:  " << strerror(errno);
          log(ssMessage.str());
        }
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->inotify_init1(" << errno << ") error:  " << strerror(errno);
        log(ssMessage.str());
      }
      m_databases.clear();
    }
    else
    {
      msleep(2000);
    }
  }
  setShutdown();
  threadDecrement();
}
// }}}
}
}
