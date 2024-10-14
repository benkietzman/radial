// -*- C++ -*-
// Radial
// -------------------------------------
// file       : status.cpp
// author     : Ben Kietzman
// begin      : 2023-10-20
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Status"
using namespace radial;
Status *gpStatus = NULL;
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster);
void callback(string strPrefix, const string strPacket, const bool bResponse);
void callbackInotify(string strPrefix, const string strPath, const string strFile);
int main(int argc, char *argv[])
{
  string strPrefix = "status->main()";
  gpStatus = new Status(strPrefix, argc, argv, &callback, &callbackInotify);
  gpStatus->enableWorkers();
  gpStatus->setApplication("Radial");
  gpStatus->setAutoMode(&autoMode);
  gpStatus->process(strPrefix);
  delete gpStatus;
  return 0;
}
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{ 
  thread threadAutoMode(&radial::Status::autoMode, gpStatus, strPrefix, strOldMaster, strNewMaster);
  pthread_setname_np(threadAutoMode.native_handle(), "autoMode");
  threadAutoMode.detach();
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpStatus->callback(strPrefix, strPacket, bResponse);
}
void callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  gpStatus->callbackInotify(strPrefix, strPath, strFile);
}
