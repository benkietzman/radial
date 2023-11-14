// -*- C++ -*-
// Radial
// -------------------------------------
// file       : session.cpp
// author     : Ben Kietzman
// begin      : 2023-07-11
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Session"
using namespace radial;
Session *gpSession;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "session->main()";
  gpSession = new Session(strPrefix, argc, argv, &callback);
  gpSession->enableWorkers();
  gpSession->process(strPrefix);
  delete gpSession;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpSession->callback(strPrefix, strPacket, bResponse);
}
