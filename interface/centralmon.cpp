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
radial::CentralMon *gpCentralMon = NULL;
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "centralMon->main()";
  gpCentralMon = new radial::CentralMon(strPrefix, argc, argv, &callback);
  gpCentralMon->process(strPrefix);
  delete gpCentralMon;
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&radial::CentralMon::callback, gpCentralMon, strPrefix, new Json(ptJson), bResponse);
  pthread_setname_np(threadCallback.native_handle(), "callback");
  threadCallback.detach();
}
