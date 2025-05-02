// -*- C++ -*-
// Radial
// -------------------------------------
// file       : db.cpp
// author     : Ben Kietzman
// begin      : 2023-05-18
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Db"
using namespace radial;
Db *gpDb = NULL;
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster);
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "db->main()";
  gpDb = new Db(strPrefix, argc, argv, &callback);
  gpDb->enableWorkers();
  gpDb->setAutoMode(&autoMode);
  gpDb->process(strPrefix);
  delete gpDb;
  return 0;
}
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  thread threadAutoMode(&Db::autoMode, gpDb, strPrefix, strOldMaster, strNewMaster);
  pthread_setname_np(threadAutoMode.native_handle(), "autoMode");
  threadAutoMode.detach();
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpDb->callback(strPrefix, strPacket, bResponse);
}
