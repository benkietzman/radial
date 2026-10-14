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
JSValue js_alert(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_alert(ctx, obj, argc, argv);}
JSValue js_application(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_application(ctx, obj, argc, argv);}
JSValue js_centralmon(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_centralmon(ctx, obj, argc, argv);}
JSValue js_chat(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_chat(ctx, obj, argc, argv);}
JSValue js_command(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_command(ctx, obj, argc, argv);}
JSValue js_db(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_db(ctx, obj, argc, argv);}
JSValue js_dbQuery(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_dbQuery(ctx, obj, argc, argv);}
JSValue js_dbQuery(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_dbUpdate(ctx, obj, argc, argv);}
JSValue js_email(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_email(ctx, obj, argc, argv);}
JSValue js_flow(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_flow(ctx, obj, argc, argv);}
JSValue js_getError(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getError(ctx, obj, argc, argv);}
JSValue js_getRequest(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getRequest(ctx, obj, argc, argv);}
JSValue js_getResponse(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getResponse(ctx, obj, argc, argv);}
JSValue js_getStatus(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getStatus(ctx, obj, argc, argv);}
JSValue js_setError(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setError(ctx, obj, argc, argv);}
JSValue js_setRequest(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setRequest(ctx, obj, argc, argv);}
JSValue js_setResponse(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setResponse(ctx, obj, argc, argv);}
JSValue js_setStatus(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setStatus(ctx, obj, argc, argv);}
int main(int argc, char *argv[])
{
  string strPrefix = "maestro->main()";
  gpMaestro = new Maestro(strPrefix, argc, argv, &callback);
  gpMaestro->enableWorkers();
  gpMaestro->jsreg("alert", js_alert, 3);
  gpMaestro->jsreg("application", js_application, 2);
  gpMaestro->jsreg("centralmon", js_centralmon, 2);
  gpMaestro->jsreg("chat", js_chat, 3);
  gpMaestro->jsreg("command", js_command, 5);
  gpMaestro->jsreg("db", js_db, 2);
  gpMaestro->jsreg("dbQuery", js_dbQuery, 2);
  gpMaestro->jsreg("dbUpdate", js_dbUpdate, 2);
  gpMaestro->jsreg("email", js_email, 5);
  gpMaestro->jsreg("flow", js_flow, 2);
  gpMaestro->jsreg("getError", js_getError, 0);
  gpMaestro->jsreg("getRequest", js_getRequest, 0);
  gpMaestro->jsreg("getResponse", js_getResponse, 0);
  gpMaestro->jsreg("getStatus", js_getStatus, 0);
  gpMaestro->jsreg("setError", js_setError, 1);
  gpMaestro->jsreg("setRequest", js_setRequest, 1);
  gpMaestro->jsreg("setResponse", js_setResponse, 1);
  gpMaestro->jsreg("setStatus", js_setStatus, 1);
  gpMaestro->process(strPrefix);
  delete gpMaestro;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpMaestro->callback(strPrefix, strPacket, bResponse);
}
