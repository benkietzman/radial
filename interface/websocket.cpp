// -*- C++ -*-
// Radial
// -------------------------------------
// file       : websocket.cpp
// author     : Ben Kietzman
// begin      : 2022-06-03
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include <StringManip>
#include <Utility>
#include "include/Websocket"
radial::Websocket *gpWebsocket;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "websocket->main()";
  gpWebsocket = new radial::Websocket(strPrefix, argc, argv, &callback);
  gpWebsocket->enableWorkers();
  thread threadSocket(&radial::Websocket::socket, gpWebsocket, strPrefix);
  pthread_setname_np(threadSocket.native_handle(), "socket");
  gpWebsocket->process(strPrefix);
  threadSocket.join();
  delete gpWebsocket;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpWebsocket->callback(strPrefix, strPacket, bResponse);
}
