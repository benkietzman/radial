// -*- C++ -*-
// Radial
// -------------------------------------
// file       : live.cpp
// author     : Ben Kietzman
// begin      : 2023-01-18
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Live"
radial::Live *gpLive;
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "live->main()";
  gpLive = new radial::Live(strPrefix, argc, argv, &callback);
  gpLive->process(strPrefix);
  delete gpLive;
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&radial::Live::callback, gpLive, strPrefix, new Json(ptJson), bResponse);
  pthread_setname_np(threadCallback.native_handle(), "callback");
  threadCallback.detach();
}
