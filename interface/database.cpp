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
radial::Database *gpDatabase = NULL;
bool mysql(const string strType, const string strName, const string strQuery, list<map<string, string> > *rows, unsigned long long &ullID, unsigned long long &ullRows, string &strError);
int main(int argc, char *argv[])
{
  string strPrefix = "database->main()";
  gpDatabase = new radial::Database(strPrefix, argc, argv, bind(&radial::Database::callback, gpDatabase, placeholders::_1, placeholders::_2, placeholders::_3), &mysql);
  gpDatabase->process(strPrefix);
  delete gpDatabase;
  return 0;
}
bool mysql(const string strType, const string strName, const string strQuery, list<map<string, string> > *rows, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  bool bResult = false;
  
  if (rows != NULL)
  {
    rows->clear();
  }
  if (gpDatabase->databases()->m.find(strName) != gpDatabase->databases()->m.end())
  {
    if (!strType.empty())
    {
      if (strType == "query")
      {
        if (rows != NULL)
        {
          Json *ptJson = new Json(gpDatabase->databases()->m[strName]);
          ptJson->insert("Type", strType);
          ptJson->insert("Query", strQuery);
          gpDatabase->target("mysql", ptJson);
          if (ptJson->m.find("Status") != ptJson->m.end() && ptJson->m["Status"]->v == "okay")
          {
            bResult = true;
            if (ptJson->m.find("Response") != ptJson->m.end())
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
          else if (ptJson->m.find("Error") != ptJson->m.end() && !ptJson->m["Error"]->v.empty())
          {
            strError = ptJson->m["Error"]->v;
          }
          else
          {
            strError = "Encountered an unknown error.";
          }
          delete ptJson;
        }
        else
        {
          strError = "Please provide a placeholder for the resultant rows.";
        }
      }
      else if (strType == "update")
      {
        Json *ptJson = new Json(gpDatabase->databases()->m[strName]);
        ptJson->insert("Type", strType);
        ptJson->insert("Update", strQuery);
        gpDatabase->target("mysql", ptJson);
        if (ptJson->m.find("Status") != ptJson->m.end() && ptJson->m["Status"]->v == "okay")
        {
          bResult = true;
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