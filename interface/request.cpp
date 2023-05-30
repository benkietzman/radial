// -*- C++ -*-
// Radial
// -------------------------------------
// file       : request.cpp
// author     : Ben Kietzman
// begin      : 2022-04-21
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Request"
using namespace radial;
Request *gpRequest;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "request->main()";
  gpRequest = new Request(strPrefix, argc, argv, &callback);
  thread threadAccept(&Request::accept, gpRequest, strPrefix);
  pthread_setname_np(threadAccept.native_handle(), "accept");
  gpRequest->process(strPrefix);
  threadAccept.join();
  delete gpRequest;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  thread threadCallback(&Request::callback, gpRequest, strPrefix, strPacket, bResponse);
  pthread_setname_np(threadCallback.native_handle(), "callback");
  threadCallback.detach();
}
