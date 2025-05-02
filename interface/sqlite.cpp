// -*- C++ -*-
// Radial
// -------------------------------------
// file       : mysql.cpp
// author     : Ben Kietzman
// begin      : 2024-10-21
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
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
