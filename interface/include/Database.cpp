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
Database::Database(string strPrefix, int argc, char **argv, function<void(string, Json *, const bool)> callback, bool (*pMysql)(const string, const string, const string, list<map<string, string> > *, unsigned long long &, unsigned long long &, string &)) : Interface(strPrefix, "database", argc, argv, callback)
{
  string strError, strWarden;
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-w" || (strArg.size() > 9 && strArg.substr(0, 9) == "--warden="))
    {
      if (strArg == "-w" && i + 1 < argc && argv[i+1][0] != '-')
      {
        strWarden = argv[++i];
      }
      else
      {
        strWarden = strArg.substr(9, strArg.size() - 9);
      }
      m_manip.purgeChar(strWarden, strWarden, "'");
      m_manip.purgeChar(strWarden, strWarden, "\"");
    }
  }
  // }}}
  m_pCentral = NULL;
  m_ptDatabases = NULL;
  if (!strWarden.empty())
  {
    m_pCentral = new Central(m_strError);
    if (m_strError.empty())
    {
      Warden *ptWarden = new Warden("Radial", strWarden, m_strError);
      m_pCentral->setMysql(pMysql);
      if (m_strError.empty())
      {
        m_ptDatabases = new Json;
        if (ptWarden->vaultRetrieve({"database"}, m_ptDatabases, m_strError))
        {
          for (auto &database : m_ptDatabases->m)
          {
            map<string, string> cred;
            database.second->flatten(cred, true, false);
            m_pCentral->addDatabase(database.first, cred, strError);
          }
        }
      }
      delete ptWarden;
    }
    else
    {
      delete m_pCentral;
      m_pCentral = NULL;
    }
  }
  else
  {
    m_strError = "Please provide the path to the Warden socket.";
  }
}
// }}}
// {{{ ~Database()
Database::~Database()
{
  if (m_pCentral != NULL)
  {
    delete m_pCentral;
  }
  if (m_ptDatabases != NULL)
  {
    delete m_ptDatabases;
  }
}
// }}}
// {{{ callback()
void Database::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;

  strPrefix += "->Database::callback()";
  if (m_pCentral != NULL)
  {
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
  }
  else
  {
    strError = m_strError;
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
}
// }}}
// {{{ databases()
Json *Database::databases()
{
  return m_ptDatabases;
}
// }}}
}
}
