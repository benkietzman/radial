// -*- C++ -*-
// Radial
// -------------------------------------
// file       : junction.cpp
// author     : Ben Kietzman
// begin      : 2023-05-18
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Junction"
using namespace radial;
Junction *gpJunction;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "junction->main()";
  gpJunction = new Junction(strPrefix, argc, argv, &callback);
  gpJunction->process(strPrefix);
  delete gpJunction;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  thread threadCallback(&Junction::callback, gpJunction, strPrefix, strPacket, bResponse);
  pthread_setname_np(threadCallback.native_handle(), "callback");
  threadCallback.detach();
}
