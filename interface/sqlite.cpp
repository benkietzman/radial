// -*- C++ -*-
// Radial
// -------------------------------------
// file       : mysql.cpp
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
#include "include/Sqlite"
using namespace radial;
Sqlite *gpSqlite;
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster);
void callback(string strPrefix, const string strPacket, const bool bResponse);
int callbackFetch(void *vptRows, int nCols, char *szCols[], char *szNames[]);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "sqlite->main()";
  gpSqlite = new Sqlite(strPrefix, argc, argv, &callback, &callbackFetch);
  gpSqlite->setAutoMode(&autoMode);
  gpSqlite->enableWorkers();
  gpSqlite->process(strPrefix);
  delete gpSqlite;
  return 0;
}
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  thread threadAutoMode(&Sqlite::autoMode, gpSqlite, strPrefix, strOldMaster, strNewMaster);
  pthread_setname_np(threadAutoMode.native_handle(), "autoMode");
  threadAutoMode.detach();
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpSqlite->callback(strPrefix, strPacket, bResponse);
}
int callbackFetch(void *vptRows, int nCols, char *szCols[], char *szNames[])
{
  return gpSqlite->callbackFetch(vptRows, nCols, szCols, szNames);
}
