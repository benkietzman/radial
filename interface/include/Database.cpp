// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Database.cpp
// author     : Ben Kietzman
// begin      : 2022-05-30
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Database"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Database()
Database::Database(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), void (*pCallbackInotify)(string, const string, const string), bool (*pMysql)(const string, const string, const string, list<map<string, string> > *, unsigned long long &, unsigned long long &, string &), bool (*pSqlite)(const string, const string, const string, list<map<string, string> > *, size_t &, size_t &, string &)) : Interface(strPrefix, "database", argc, argv, pCallback)
{
  map<string, list<string> > watches;
  string strError;
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "--mysql" && pMysql != NULL)
    {
      m_pCentral->setMysql(pMysql);
    }
    else if (strArg == "--sqlite" && pMysql != NULL)
    {
      m_pCentral->setSqlite(pSqlite);
    }
  }
  // }}}
  m_ptDatabases = NULL;
  load(strPrefix, true);
  watches[m_strData] = {".cred"};
  m_pThreadInotify = new thread(&Database::inotify, this, strPrefix, watches, pCallbackInotify);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
}
// }}}
// {{{ ~Database()
Database::~Database()
{
  m_pThreadInotify->join();
  delete m_pThreadInotify;
  if (m_ptDatabases != NULL)
  {
    delete m_ptDatabases;
  }
}
// }}}
// {{{ callback()
void Database::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Database::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Database"))
  {
    if (!empty(ptJson, "Query"))
    {
      unsigned long long ullRows;
      auto rows = m_pCentral->query(ptJson->m["Database"]->v, ptJson->m["Query"]->v, ullRows, strError);
      if (rows != NULL)
      {
        stringstream ssRows;
        bResult = true;
        ssRows << ullRows;
        ptJson->i("Rows", ssRows.str(), 'n');
        if (exist(ptJson, "Response"))
        {
          delete ptJson->m["Response"];
        }
        ptJson->m["Response"] = new Json;
        for (auto &row : *rows)
        {
          ptJson->m["Response"]->pb(row);
        }
      }
      m_pCentral->free(rows);
    }
    else if (!empty(ptJson, "Update"))
    {
      unsigned long long ullID, ullRows;
      if (m_pCentral->update(ptJson->m["Database"]->v, ptJson->m["Update"]->v, ullID, ullRows, strError))
      {
        stringstream ssID, ssRows;
        bResult = true;
        ssID << ullID;
        ptJson->i("ID", ssID.str());
        ssRows << ullRows;
        ptJson->i("Rows", ssRows.str());
      }
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
}
// }}}
// {{{ callbackInotify()
void Database::callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Database::callbackInotify()";
  if (strPath == m_strData && strFile == ".cred")
  {
    load(strPrefix);
  }
}
// }}}
// {{{ load()
void Database::load(string strPrefix, const bool bSilent)
{
  string strError;
  stringstream ssMessage;
  Json *ptDatabases = new Json;

  strPrefix += "->Database::load()";
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"database"}, ptDatabases, strError))
  {
    map<string, map<string, string> > creds;
    Json *ptOld = NULL;
    for (auto &database : ptDatabases->m)
    {
      map<string, string> cred;
      database.second->flatten(cred, true, false);
      creds[database.first] = cred;
    }
    m_mutex.lock();
    if (m_ptDatabases != NULL)
    {
      for (auto &database : m_ptDatabases->m)
      {
        m_pCentral->removeDatabase(database.first);
      }
    }
    for (auto &cred : creds)
    {
      m_pCentral->addDatabase(cred.first, cred.second, strError);
    }
    if (m_ptDatabases != NULL)
    {
      ptOld = m_ptDatabases;
    }
    m_ptDatabases = ptDatabases;
    m_mutex.unlock();
    if (ptOld != NULL)
    {
      delete ptOld;
    }
  }
  else
  {
    delete ptDatabases;
    if (!bSilent)
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->Warden::vaultRetrieve() error [database]:  " << strError;
      log(ssMessage.str());
    }
  }
}
// }}}
// {{{ mysql()
bool Database::mysql(const string strType, const string strName, const string strQuery, list<map<string, string> > *rows, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  bool bResult = false;
  Json *ptJson = NULL;

  if (rows != NULL)
  {
    rows->clear();
  }
  m_mutex.lock();
  if (m_ptDatabases != NULL && exist(m_ptDatabases, strName))
  {
    ptJson = new Json(m_ptDatabases->m[strName]);
  }
  m_mutex.unlock();
  if (ptJson != NULL)
  {
    if (!strType.empty())
    {
      if (strType == "query" || strType == "update")
      {
        if (strType == "update" || rows != NULL)
        {
          ptJson->i("Type", strType);
          ptJson->i(((strType == "query")?"Query":"Update"), strQuery);
          if (hub("mysql", ptJson, strError))
          {
            bResult = true;
            if (!empty(ptJson, "ID"))
            {
              stringstream ssID(ptJson->m["ID"]->v);
              ssID >> ullID;
            }
            if (strType == "query" && exist(ptJson, "Response"))
            {
              for (auto &i : ptJson->m["Response"]->l)
              {
                map<string, string> row;
                i->flatten(row, true, false);
                rows->push_back(row);
              }
            }
            if (!empty(ptJson, "Rows"))
            {
              stringstream ssRows(ptJson->m["Rows"]->v);
              ssRows >> ullRows;
            }
          }
        }
        else
        {
          strError = "Please provide a placeholder for the resultant rows.";
        }
      }
      else
      {
        strError = "Please provide a valid Type:  query, update.";
      }
    }
    else
    {
      strError = "Please provide the Type.";
    }
    delete ptJson;
  }
  else
  {
    strError = "Please provide a valid database Name.";
  }

  return bResult;
}
// }}}
// {{{ sqlite()
bool Database::sqlite(const string strType, const string strName, const string strQuery, list<map<string, string> > *rows, size_t &unID, size_t &unRows, string &strError)
{
  bool bReadOnly = true, bResult = false;
  string strDatabase;
  Json *ptJson = new Json;

  if (rows != NULL)
  {
    rows->clear();
  }
  m_mutex.lock();
  if (m_ptDatabases != NULL && exist(m_ptDatabases, strName) && !empty(m_ptDatabases->m[strName], "Database"))
  {
    strDatabase = m_ptDatabases->m[strName]->m["Database"]->v;
    if (!empty(m_ptDatabases->m[strName], "Access") && m_ptDatabases->m[strName]->m["Access"]->v == "rw")
    {
      bReadOnly = false;
    }
  }
  m_mutex.unlock();
  if (!strDatabase.empty())
  {
    if (!strType.empty())
    {
      if (strType == "query" || strType == "update")
      {
        if (strType == "query")
        {
          bReadOnly = true;
        }
        if (strType == "update" || rows != NULL)
        {
          ptJson->i("Interface", "sqlite");
          ptJson->i("Function", "query");
          ptJson->m["Request"] = new Json;
          ptJson->m["Request"]->i("Access", ((bReadOnly)?"r":"rw"));
          ptJson->m["Request"]->i("Database", strDatabase);
          ptJson->m["Request"]->i("Statement", strQuery);
          if (hub("sqlite", ptJson, strError))
          {
            bResult = true;
            if (exist(ptJson, "Response"))
            {
              if (!empty(ptJson->m["Response"], "ID"))
              {
                stringstream ssID(ptJson->m["Response"]->m["ID"]->v);
                ssID >> unID;
              }
              if (strType == "query" && rows != NULL && exist(ptJson, "Response"))
              {
                for (auto &i : ptJson->m["Response"]->m["ResultSet"]->l)
                {
                  map<string, string> row;
                  i->flatten(row, true, false);
                  rows->push_back(row);
                }
              }
              if (!empty(ptJson->m["Response"], "Rows"))
              {
                stringstream ssRows(ptJson->m["Response"]->m["Rows"]->v);
                ssRows >> unRows;
              }
            }
          }
        }
        else
        {
          strError = "Please provide a placeholder for the resultant rows.";
        }
      }
      else
      {
        strError = "Please provide a valid Type:  query, update.";
      }
    }
    else
    {
      strError = "Please provide the Type.";
    }
  }
  else
  {
    strError = "Please provide a valid database Name.";
  }
  delete ptJson;

  return bResult;
}
// }}}
}
}
