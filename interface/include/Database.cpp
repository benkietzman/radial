// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Database.cpp
// author     : Ben Kietzman
// begin      : 2022-05-30
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Database"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Database()
Database::Database(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), bool (*pMysql)(const string, const string, const string, list<map<string, string> > *, unsigned long long &, unsigned long long &, string &)) : Interface(strPrefix, "database", argc, argv, pCallback)
{
  string strError;
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "--mysql" && pMysql != NULL)
    {
      m_pCentral->setMysql(pMysql);
    }
  }
  // }}}
  m_ptDatabases = new Json;
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"database"}, m_ptDatabases, strError))
  {
    for (auto &database : m_ptDatabases->m)
    {
      map<string, string> cred;
      database.second->flatten(cred, true, false);
      m_pCentral->addDatabase(database.first, cred, strError);
    }
  }
}
// }}}
// {{{ ~Database()
Database::~Database()
{
  delete m_ptDatabases;
}
// }}}
// {{{ callback()
void Database::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
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
  threadDecrement();
}
// }}}
// {{{ mysql()
bool Database::mysql(const string strType, const string strName, const string strQuery, list<map<string, string> > *rows, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  bool bResult = false;

  if (rows != NULL)
  {
    rows->clear();
  }
  if (m_ptDatabases != NULL && exist(m_ptDatabases, strName))
  {
    if (!strType.empty())
    {
      if (strType == "query" || strType == "update")
      {
        if (strType == "update" || rows != NULL)
        {
          Json *ptJson = new Json(m_ptDatabases->m[strName]);
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
          delete ptJson;
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

  return bResult;
}
// }}}
}
}
