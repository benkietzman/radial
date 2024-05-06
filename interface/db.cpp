// -*- C++ -*-
// Radial
// -------------------------------------
// file       : db.cpp
// author     : Ben Kietzman
// begin      : 2023-05-18
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
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
