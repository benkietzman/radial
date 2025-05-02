// -*- C++ -*-
// Radial
// -------------------------------------
// file       : database.cpp
// author     : Ben Kietzman
// begin      : 2022-05-30
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Database"
using namespace radial;
Database *gpDatabase = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
void callbackInotify(string strPrefix, const string strPath, const string strFile);
bool mysql(const string strType, const string strName, const string strQuery, list<map<string, string> > *rows, unsigned long long &ullID, unsigned long long &ullRows, string &strError);
bool sqlite(const string strType, const string strName, const string strQuery, list<map<string, string> > *rows, size_t &unID, size_t &unRows, string &strError);
int main(int argc, char *argv[])
{
  string strPrefix = "database->main()";
  gpDatabase = new Database(strPrefix, argc, argv, &callback, &callbackInotify, &mysql, &sqlite);
  gpDatabase->enableWorkers();
  gpDatabase->process(strPrefix);
  delete gpDatabase;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpDatabase->callback(strPrefix, strPacket, bResponse);
}
void callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  gpDatabase->callbackInotify(strPrefix, strPath, strFile);
}
bool mysql(const string strType, const string strName, const string strQuery, list<map<string, string> > *rows, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  return gpDatabase->mysql(strType, strName, strQuery, rows, ullID, ullRows, strError);
}
bool sqlite(const string strType, const string strName, const string strQuery, list<map<string, string> > *rows, size_t &unID, size_t &unRows, string &strError)
{
  return gpDatabase->sqlite(strType, strName, strQuery, rows, unID, unRows, strError);
}
