// -*- C++ -*-
// Radial
// -------------------------------------
// file       : secure.cpp
// author     : Ben Kietzman
// begin      : 2022-06-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Secure"
using namespace radial;
Secure *gpSecure;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "secure->main()";
  gpSecure = new Secure(strPrefix, argc, argv, &callback);
  gpSecure->enableWorkers();
  gpSecure->process(strPrefix);
  delete gpSecure;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpSecure->callback(strPrefix, strPacket, bResponse);
}
