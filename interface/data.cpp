// -*- C++ -*-
// Radial
// -------------------------------------
// file       : database.cpp
// author     : Ben Kietzman
// begin      : 2025-03-11
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Data"
using namespace radial;
Data *gpData = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
void callbackInotify(string strPrefix, const string strPath, const string strFile);
int main(int argc, char *argv[])
{
  string strPrefix = "data->main()";
  gpData = new Data(strPrefix, argc, argv, &callback, &callbackInotify);
  gpData->enableWorkers();
  gpData->process(strPrefix);
  delete gpData;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpData->callback(strPrefix, strPacket, bResponse);
}
void callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  gpData->callbackInotify(strPrefix, strPath, strFile);
}
