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
int websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
struct lws_protocols gtProtocols[] =
{
  {"http-only", lws_callback_http_dummy, 0, 0},
  {"radial", websocket, 0, 0},
  {NULL, NULL, 0, 0}
};
radial::Websocket *gpWebsocket;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "websocket->main()";
  gpWebsocket = new radial::Websocket(strPrefix, argc, argv, &callback, &websocket);
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
int websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
  return gpWebsocket->websocket(wsi, reason, user, in, len);
}
