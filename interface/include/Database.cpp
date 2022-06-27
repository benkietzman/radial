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
Database::Database(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool), bool (*pMysql)(const string, const string, const string, list<map<string, string> > *, unsigned long long &, unsigned long long &, string &)) : Interface(strPrefix, "database", argc, argv, pCallback)
{
  string strError;
  if (pMysql != NULL)
  {
    m_pCentral->setMysql(pMysql);
  }
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
void Database::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;

  strPrefix += "->Database::callback()";
  if (ptJson->m.find("Database") != ptJson->m.end() && !ptJson->m["Database"]->v.empty())
  {
    if (ptJson->m.find("Query") != ptJson->m.end() && !ptJson->m["Query"]->v.empty())
    {
      auto rows = m_pCentral->query(ptJson->m["Database"]->v, ptJson->m["Query"]->v, strError);
      if (rows != NULL)
      {
        bResult = true;
        ptJson->m["Response"] = new Json;
        for (auto &row : *rows)
        {
          ptJson->m["Response"]->push_back(row);
        }
      }
      m_pCentral->free(rows);
    }
    else if (ptJson->m.find("Update") != ptJson->m.end() && !ptJson->m["Update"]->v.empty())
    {
      if (m_pCentral->update(ptJson->m["Database"]->v, ptJson->m["Update"]->v, strError))
      {
        bResult = true;
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
  ptJson->insert("Status", ((bResult)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->insert("Error", strError);
  }
  if (bResponse)
  {
    response(ptJson);
  }
  delete ptJson;
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
  if (m_ptDatabases != NULL && m_ptDatabases->m.find(strName) != m_ptDatabases->m.end())
  {
    if (!strType.empty())
    {
      if (strType == "query" || strType == "update")
      {
        if (strType == "update" || rows != NULL)
        {
          Json *ptJson = new Json(m_ptDatabases->m[strName]);
          ptJson->insert("Type", strType);
          ptJson->insert(((strType == "query")?"Query":"Update"), strQuery);
          if (target("mysql", ptJson, strError))
          {
            bResult = true;
            if (ptJson->m.find("ID") != ptJson->m.end() && !ptJson->m["ID"]->v.empty())
            {
              stringstream ssID(ptJson->m["ID"]->v);
              ssID >> ullID;
            }
            if (strType == "query" && ptJson->m.find("Response") != ptJson->m.end())
            {
              for (auto &i : ptJson->m["Response"]->l)
              {
                map<string, string> row;
                i->flatten(row, true, false);
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
