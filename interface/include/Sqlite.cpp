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
}
// }}}
// {{{ ~Sqlite()
Sqlite::~Sqlite()
{
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
            bResult = true;
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
}
}
