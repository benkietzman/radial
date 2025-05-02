// -*- C++ -*-
// Radial
// -------------------------------------
// file       : websocket.cpp
// author     : Ben Kietzman
// begin      : 2022-06-03
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include <StringManip>
#include <Utility>
#include "include/Websocket"
radial::Websocket *gpWebsocket;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
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
