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
  if (!empty(ptJson, "Database"))
  {
    if (!empty(ptJson, "Query") || !empty(ptJson, "Update"))
    {
      int nReturn;
      sqlite3 *db;
      stringstream ssFile, ssError;
      ssFile << "file:" << m_strData << "/sqlite/" << ptJson->m["Database"]->v << ".db";
      if ((nReturn = sqlite3_open(ssFile.str().c_str(), &db)) == SQLITE_OK)
      {
        if (!empty(ptJson, "Query"))
        {
          char *pszError = NULL;
          list<map<string, string> > rows;
          Json *ptRows = new Json;
          if ((nReturn = sqlite3_exec(db, ptJson->m["Query"]->v.c_str(), m_pCallbackFetch, ptRows, &pszError)) == SQLITE_OK)
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
          if ((nReturn = sqlite3_exec(db, ptJson->m["Update"]->v.c_str(), NULL, NULL, &pszError)) == SQLITE_OK)
          {
            string strAction, strLower;
            stringstream ssQuery(ptJson->m["Update"]->v), ssRows;
            bResult = true;
            ssRows << sqlite3_changes(db);
            ptJson->i("Rows", ssRows.str(), 'n');
            ssQuery >> strAction;
            m_manip.toLower(strLower, strAction);
            if (strLower == "insert")
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
    else
    {
      strError = "Please provide the Query or Update.";
    }
  }
  else
  {
    strError = "Please provide the Database.";
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
// {{{ inotify()
void Sqlite::inotify(string strPrefix)
{
  int fdNotify;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Sqlite::inotify()";
  if ((fdNotify = inotify_init1(IN_NONBLOCK)) != -1)
  {
    int wdNotify;
    if ((wdNotify = inotify_add_watch(fdNotify, (m_strData + "/sqlite").c_str(), (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO))) != -1)
    {
      bool bExit = false;
      inotify_event *pEvent;
      int nReturn;
      list<string> databases, entries;
      string strNotify;
      m_file.directoryList(m_strData + "/sqlite", entries);
      for (auto &i : entries)
      {
        if (i.size() > 3 && i.substr((i.size() - 3), 3) == ".db")
        {
          databases.push_back(i.substr(0, (i.size() - 3)));
        }
      }
      if (!databases.empty())
      {
        Json *ptData;
        databases.sort();
        ptData = new Json(databases);
        databases.clear();
        if (storageAdd({"sqlite", m_strNode}, ptData, strError))
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->Interface::storageAdd() [sqlite," << m_strNode << "]:  Added databases.";
          log(ssMessage.str());
        }
        else
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->Interface::storageAdd() error [sqlite," << m_strNode << "]:  " << strError;
          log(ssMessage.str());
        }
        delete ptData;
      }
      entries.clear();
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
                      Json *ptData = new Json;
                      if (storageRetrieve({"sqlite", m_strNode}, ptData, strError) || strError == "Failed to find key.")
                      {
                        bool bFound = false;
                        for (auto &i : ptData->l)
                        {
                          if (i->v == strDatabase)
                          {
                            bFound = true;
                          }
                          databases.push_back(i->v);
                        }
                        if (!bFound)
                        {
                          databases.push_back(strDatabase);
                          databases.sort();
                          ptData->clear();
                          for (auto &i : databases)
                          {
                            ptData->pb(i);
                          }
                          if (storageAdd({"sqlite", m_strNode}, ptData, strError))
                          {
                            ssMessage.str("");
                            ssMessage << strPrefix << "->Interface::storageAdd() [sqlite," << m_strNode << "," << strDatabase << "]:  Added database.";
                            log(ssMessage.str());
                          }
                          else
                          {
                            ssMessage.str("");
                            ssMessage << strPrefix << "->Interface::storageAdd() error [sqlite," << m_strNode << "," << strDatabase << "]:  " << strError;
                            log(ssMessage.str());
                          }
                        }
                        databases.clear();
                      }
                      else
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << "->Interface::storageRetrieve() error [sqlite," << m_strNode << "]:  " << strError;
                        log(ssMessage.str());
                      }
                      delete ptData;
                    }
                    else if (pEvent->mask & (IN_DELETE | IN_MOVED_FROM))
                    {
                      Json *ptData = new Json;
                      if (storageRetrieve({"sqlite", m_strNode}, ptData, strError))
                      {
                        bool bFound = false;
                        for (auto &i : ptData->l)
                        {
                          if (i->v == strDatabase)
                          {
                            bFound = true;
                          }
                          else
                          {
                            databases.push_back(i->v);
                          }
                        }
                        if (bFound)
                        {
                          ptData->clear();
                          for (auto &i : databases)
                          {
                            ptData->pb(i);
                          }
                          if (storageAdd({"sqlite", m_strNode}, ptData, strError))
                          {
                            ssMessage.str("");
                            ssMessage << strPrefix << "->Interface::storageAdd() [sqlite," << m_strNode << "," << strDatabase << "]:  Removed database.";
                            log(ssMessage.str());
                          }
                          else
                          {
                            ssMessage.str("");
                            ssMessage << strPrefix << "->Interface::storageAdd() error [sqlite," << m_strNode << "," << strDatabase << "]:  " << strError;
                            log(ssMessage.str());
                          }
                        }
                        databases.clear();
                      }
                      else
                      {
                        ssMessage.str("");
                        ssMessage << strPrefix << "->Interface::storageRetrieve() error [sqlite," << m_strNode << "]:  " << strError;
                        log(ssMessage.str());
                      }
                      delete ptData;
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
        if (shutdown())
        {
          bExit = true;
        }
      }
      inotify_rm_watch(fdNotify, wdNotify);
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
  setShutdown();
  threadDecrement();
}
// }}}
}
}
