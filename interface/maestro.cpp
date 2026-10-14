// -*- C++ -*-
// Radial
// -------------------------------------
// file       : maestro.cpp
// author     : Ben Kietzman
// begin      : 2026-10-07
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Maestro"
using namespace radial;
Maestro *gpMaestro = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
JSValue js_chat(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv);
JSValue js_email(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv);
JSValue js_flow(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv);
JSValue js_getError(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv);
JSValue js_setError(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv);
int main(int argc, char *argv[])
{
  string strPrefix = "maestro->main()";
  gpMaestro = new Maestro(strPrefix, argc, argv, &callback);
  gpMaestro->enableWorkers();
  gpMaestro->jsreg("chat", js_chat, 3);
  gpMaestro->jsreg("email", js_email, 5);
  gpMaestro->jsreg("flow", js_flow, 2);
  gpMaestro->jsreg("getError", js_getError, 0);
  gpMaestro->jsreg("setError", js_setError, 1);
  gpMaestro->process(strPrefix);
  delete gpMaestro;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpMaestro->callback(strPrefix, strPacket, bResponse);
}
JSValue js_chat(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv)
{
  return gpMaestro->js_chat(ctx, obj, argc, argv);
}
JSValue js_email(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv)
{
  return gpMaestro->js_email(ctx, obj, argc, argv);
}
JSValue js_flow(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv)
{
  return gpMaestro->js_flow(ctx, obj, argc, argv);
}
JSValue js_getError(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv)
{
  return gpMaestro->js_getError(ctx, obj, argc, argv);
}
JSValue js_setError(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv)
{
  return gpMaestro->js_setError(ctx, obj, argc, argv);
}
