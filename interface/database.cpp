// -*- C++ -*-
// Radial
// -------------------------------------
// file       : database.cpp
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
#include "include/Database"
using namespace radial;
Database *gpDatabase = NULL;
void callback(string strPrefix, Json *ptJson, const bool bResponse);
bool mysql(const string strType, const string strName, const string strQuery, list<map<string, string> > *rows, unsigned long long &ullID, unsigned long long &ullRows, string &strError);
int main(int argc, char *argv[])
{
  string strPrefix = "database->main()";
  gpDatabase = new Database(strPrefix, argc, argv, &callback, &mysql);
  gpDatabase->process(strPrefix);
  delete gpDatabase;
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&Database::callback, gpDatabase, strPrefix, new Json(ptJson), bResponse);
  threadCallback.detach();
} 
bool mysql(const string strType, const string strName, const string strQuery, list<map<string, string> > *rows, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  bool bResult = false;
  
  if (rows != NULL)
  {
    rows->clear();
  }
  if (gpDatabase->databases() != NULL && gpDatabase->databases()->m.find(strName) != gpDatabase->databases()->m.end())
  {
    if (!strType.empty())
    {
      if (strType == "query" || strType == "update")
      {
        if (strType == "update" || rows != NULL)
        {
          Json *ptJson = new Json(gpDatabase->databases()->m[strName]);
          ptJson->insert("Type", strType);
          ptJson->insert(((strType == "query")?"Query":"Update"), strQuery);
          if (gpDatabase->target("mysql", ptJson, strError))
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
