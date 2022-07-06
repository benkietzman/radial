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
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  char *pszCert, *pszKey;
  string strData, strError, strPrefix = "websocket->main()";
  lws_context *ptContext = NULL;
  lws_context_creation_info tInfo;
  common::StringManip manip;
  common::Utility utility(strError);
  utility.sslInit();
  gpWebsocket = new radial::Websocket(strPrefix, argc, argv, &callback, &websocket);
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-d" || (strArg.size() > 7 && strArg.substr(0, 7) == "--data="))
    {
      if (strArg == "-d" && i + 1 < argc && argv[i+1][0] != '-')
      {
        strData = argv[++i];
      }
      else
      {
        strData = strArg.substr(7, strArg.size() - 7);
      }
      manip.purgeChar(strData, strData, "'");
      manip.purgeChar(strData, strData, "\"");
    }
  }
  memset(&tInfo, 0, sizeof(lws_context_creation_info));
  tInfo.gid = -1;
  tInfo.iface = NULL;
  tInfo.max_http_header_data = 32767; // this is the maximum
  tInfo.options = 0 | LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT | LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT | LWS_SERVER_OPTION_REDIRECT_HTTP_TO_HTTPS;
  tInfo.port = 7797;
  tInfo.protocols = gtProtocols;
  pszCert = (char *)malloc((strData.size()+12)*sizeof(char));
  pszCert[0] = '\0';
  strcpy(pszCert, (strData + "/server.crt").c_str());
  tInfo.ssl_cert_filepath = pszCert;
  pszKey = (char *)malloc((strData.size()+12)*sizeof(char));
  pszKey[0] = '\0';
  strcpy(pszKey, (strData + "/server.key").c_str());
  tInfo.ssl_private_key_filepath = pszKey;
  tInfo.uid = -1;
  if ((ptContext = lws_create_context(&tInfo)) != NULL)
  {
    thread threadSocket(&radial::Websocket::socket, gpWebsocket, strPrefix, ptContext);
    pthread_setname_np(threadSocket.native_handle(), "socket");
    gpWebsocket->process(strPrefix);
    threadSocket.join();
  }
  delete[] pszCert;
  delete[] pszKey;
  delete gpWebsocket;
  utility.sslDeinit();
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&radial::Websocket::callback, gpWebsocket, strPrefix, new Json(ptJson), bResponse);
  pthread_setname_np(threadCallback.native_handle(), "callback");
  threadCallback.detach();
}
int websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
  return gpWebsocket->websocket(wsi, reason, user, in, len);
}
