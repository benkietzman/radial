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
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster);
void callback(string strPrefix, const string strPacket, const bool bResponse);
void callbackInotify(string strPrefix, const string strPath, const string strFile);
JSValue js_alert(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_alert(ctx, obj, argc, argv);}
JSValue js_application(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_application(ctx, obj, argc, argv);}
JSValue js_centralmon(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_centralmon(ctx, obj, argc, argv);}
JSValue js_chat(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_chat(ctx, obj, argc, argv);}
JSValue js_command(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_command(ctx, obj, argc, argv);}
JSValue js_db(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_db(ctx, obj, argc, argv);}
JSValue js_dbquery(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_dbquery(ctx, obj, argc, argv);}
JSValue js_dbupdate(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_dbupdate(ctx, obj, argc, argv);}
JSValue js_email(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_email(ctx, obj, argc, argv);}
JSValue js_esc(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_esc(ctx, obj, argc, argv);}
JSValue js_flow(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_flow(ctx, obj, argc, argv);}
JSValue js_getError(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getError(ctx, obj, argc, argv);}
JSValue js_getRequest(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getRequest(ctx, obj, argc, argv);}
JSValue js_getResponse(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getResponse(ctx, obj, argc, argv);}
JSValue js_getStatus(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getStatus(ctx, obj, argc, argv);}
JSValue js_junction(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_junction(ctx, obj, argc, argv);}
JSValue js_live(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_live(ctx, obj, argc, argv);}
JSValue js_log(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_log(ctx, obj, argc, argv);}
JSValue js_logger(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_logger(ctx, obj, argc, argv);}
JSValue js_mysqlQuery(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_mysqlQuery(ctx, obj, argc, argv);}
JSValue js_mysqlUpdate(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_mysqlUpdate(ctx, obj, argc, argv);}
JSValue js_notify(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_notify(ctx, obj, argc, argv);}
JSValue js_setError(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setError(ctx, obj, argc, argv);}
JSValue js_setRequest(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setRequest(ctx, obj, argc, argv);}
JSValue js_setResponse(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setResponse(ctx, obj, argc, argv);}
JSValue js_setStatus(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setStatus(ctx, obj, argc, argv);}
JSValue js_sqliteCreate(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_sqliteCreate(ctx, obj, argc, argv);}
JSValue js_sqliteDatabases(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_sqliteDatabases(ctx, obj, argc, argv);}
JSValue js_sqliteDrop(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_sqliteDrop(ctx, obj, argc, argv);}
JSValue js_sqliteQuery(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_sqliteQuery(ctx, obj, argc, argv);}
JSValue js_sshConnect(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_sshConnect(ctx, obj, argc, argv);}
JSValue js_sshDisconnect(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_sshDisconnect(ctx, obj, argc, argv);}
JSValue js_sshSend(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_sshSend(ctx, obj, argc, argv);}
JSValue js_storageAdd(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_storageAdd(ctx, obj, argc, argv);}
JSValue js_storageRemove(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_storageRemove(ctx, obj, argc, argv);}
JSValue js_storageRetrieve(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_storageRetrieve(ctx, obj, argc, argv);}
JSValue js_storageRetrieveKeys(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_storageRetrieveKeys(ctx, obj, argc, argv);}
JSValue js_storageUpdate(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_storageUpdate(ctx, obj, argc, argv);}
int main(int argc, char *argv[])
{
  string strPrefix = "maestro->main()";
  gpMaestro = new Maestro(strPrefix, argc, argv, &callback, &callbackInotify);
  gpMaestro->enableWorkers();
  gpMaestro->setAutoMode(&autoMode);
  gpMaestro->jsreg({{"alert", js_alert}, {"application", js_application}, {"centralmon", js_centralmon}, {"chat", js_chat}, {"command", js_command}, {"db", js_db}, {"dbquery", js_dbquery}, {"dbupdate", js_dbupdate}, {"email", js_email}, {"esc", js_esc}, {"flow", js_flow}, {"getError", js_getError}, {"getRequest", js_getRequest}, {"getResponse", js_getResponse}, {"getStatus", js_getStatus}, {"junction", js_junction}, {"live", js_live}, {"log", js_log}, {"logger", js_logger}, {"mysqlQuery", js_mysqlQuery}, {"mysqlUpdate", js_mysqlUpdate}, {"notify", js_notify}, {"setError", js_setError}, {"setRequest", js_setRequest}, {"setResponse", js_setResponse}, {"setStatus", js_setStatus}, {"sqliteCreate", js_sqliteCreate}, {"sqliteDatabases", js_sqliteDatabases}, {"sqliteDrop", js_sqliteDrop}, {"sqliteQuery", js_sqliteQuery}, {"sshConnect", js_sshConnect}, {"sshDisconnect", js_sshDisconnect}, {"sshSend", js_sshSend}, {"storageAdd", js_storageAdd}, {"storageRemove", js_storageAdd}, {"storageRetrieve", js_storageRetrieve}, {"storageRetrieveKeys", js_storageRetrieve}, {"storageUpdate", js_storageUpdate}});
  gpMaestro->process(strPrefix);
  delete gpMaestro;
  return 0;
}
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  thread threadAutoMode(&Maestro::autoMode, gpMaestro, strPrefix, strOldMaster, strNewMaster);
  pthread_setname_np(threadAutoMode.native_handle(), "autoMode");
  threadAutoMode.detach();
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpMaestro->callback(strPrefix, strPacket, bResponse);
}
void callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  gpMaestro->callbackInotify(strPrefix, strPath, strFile);
}
