// -*- C++ -*-
// Radial
// -------------------------------------
// file       : centralmon.cpp
// author     : Ben Kietzman
// begin      : 2023-03-03
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/CentralMon"
using namespace radial;
CentralMon *gpCentralMon = NULL;
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster);
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "centralMon->main()";
  gpCentralMon = new CentralMon(strPrefix, argc, argv, &callback);
  gpCentralMon->enableWorkers();
  gpCentralMon->setAutoMode(&autoMode);
  gpCentralMon->process(strPrefix);
  delete gpCentralMon;
  return 0;
}
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  thread threadAutoMode(&CentralMon::autoMode, gpCentralMon, strPrefix, strOldMaster, strNewMaster);
  pthread_setname_np(threadAutoMode.native_handle(), "autoMode");
  threadAutoMode.detach();
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpCentralMon->callback(strPrefix, strPacket, bResponse);
}
