// -*- C++ -*-
// Radial
// -------------------------------------
// file       : central.cpp
// author     : Ben Kietzman
// begin      : 2023-02-18
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Central"
radial::Central *gpCentral = NULL;
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "central->main()";
  gpCentral = new radial::Central(strPrefix, argc, argv, &callback);
  gpCentral->process(strPrefix);
  delete gpCentral;
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&radial::Central::callback, gpCentral, strPrefix, new Json(ptJson), bResponse);
  pthread_setname_np(threadCallback.native_handle(), "callback");
  threadCallback.detach();
}
