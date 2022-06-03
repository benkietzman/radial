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
  return gpDatabase->mysql(strType, strName, strQuery, rows, ullID, ullRows, strError);
}
