// -*- C++ -*-
// Radial
// -------------------------------------
// file       : mythtv.cpp
// author     : Ben Kietzman
// begin      : 2025-04-24
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/MythTv"
using namespace radial;
MythTv *gpMythTv = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "mythtv->main()";
  gpMythTv = new MythTv(strPrefix, argc, argv, &callback);
  gpMythTv->enableWorkers();
  gpMythTv->process(strPrefix);
  delete gpMythTv;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpMythTv->callback(strPrefix, strPacket, bResponse);
}
