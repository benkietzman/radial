// -*- C++ -*-
// Radial
// -------------------------------------
// file       : jwt.cpp
// author     : Ben Kietzman
// begin      : 2023-02-17
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Jwt"
using namespace radial;
Jwt *gpJwt = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "jwt->main()";
  gpJwt = new Jwt(strPrefix, argc, argv, &callback);
  gpJwt->process(strPrefix);
  delete gpJwt;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  thread threadCallback(&Jwt::callback, gpJwt, strPrefix, strPacket, bResponse);
  pthread_setname_np(threadCallback.native_handle(), "callback");
  threadCallback.detach();
}
