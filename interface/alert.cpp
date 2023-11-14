// -*- C++ -*-
// Radial
// -------------------------------------
// file       : alert.cpp
// author     : Ben Kietzman
// begin      : 2023-09-22
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Alert"
using namespace radial;
Alert *gpAlert;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "alert->main()";
  gpAlert = new Alert(strPrefix, argc, argv, &callback);
  gpAlert->enableWorkers();
  gpAlert->process(strPrefix);
  delete gpAlert;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpAlert->callback(strPrefix, strPacket, bResponse);
}
