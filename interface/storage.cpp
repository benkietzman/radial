// -*- C++ -*-
// Radial
// -------------------------------------
// file       : storage.cpp
// author     : Ben Kietzman
// begin      : 2022-04-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Storage"
radial::Storage *gpStorage;
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "storage->main()";
  gpStorage = new radial::Storage(strPrefix, argc, argv, &callback);
  gpStorage->process(strPrefix);
  delete gpStorage;
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&radial::Storage::callback, gpStorage, strPrefix, new Json(ptJson), bResponse);
  threadCallback.detach();
} 
