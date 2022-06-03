// -*- C++ -*-
// Radial
// -------------------------------------
// file       : log.cpp
// author     : Ben Kietzman
// begin      : 2022-04-19
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Log"
using namespace radial;
Log *gpLog;
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "log->main()";
  gpLog = new Log(strPrefix, argc, argv, &callback);
  gpLog->process(strPrefix);
  delete gpLog;
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&Log::callback, gpLog, strPrefix, new Json(ptJson), bResponse);
  threadCallback.detach();
}
