// -*- C++ -*-
// Radial
// -------------------------------------
// file       : auth.cpp
// author     : Ben Kietzman
// begin      : 2022-05-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Auth"
using namespace radial;
Auth *gpAuth;
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "auth->main()";
  gpAuth = new Auth(strPrefix, argc, argv, &callback);
  thread threadProcess(&Auth::process, gpAuth, strPrefix);
  if (!gpAuth->init())
  {
    gpAuth->setShutdown();
  }
  threadProcess.join();
  delete gpAuth;
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&Auth::callback, gpAuth, strPrefix, new Json(ptJson), bResponse);
  threadCallback.detach();
}
