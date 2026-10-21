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
JSValue js_flow(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_flow(ctx, obj, argc, argv);}
JSValue js_getError(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getError(ctx, obj, argc, argv);}
JSValue js_getRequest(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getRequest(ctx, obj, argc, argv);}
JSValue js_getResponse(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getResponse(ctx, obj, argc, argv);}
JSValue js_getStatus(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_getStatus(ctx, obj, argc, argv);}
JSValue js_live(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_live(ctx, obj, argc, argv);}
JSValue js_log(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_log(ctx, obj, argc, argv);}
JSValue js_logger(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_logger(ctx, obj, argc, argv);}
JSValue js_mysqlQuery(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_mysqlQuery(ctx, obj, argc, argv);}
JSValue js_mysqlUpdate(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_mysqlUpdate(ctx, obj, argc, argv);}
JSValue js_pageGroup(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_pageGroup(ctx, obj, argc, argv);}
JSValue js_pageUser(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_pageUser(ctx, obj, argc, argv);}
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
  gpMaestro->jsreg("alert", js_alert, 3, "alert(user, message, html)", {}, "Provides the ability to send a message to a given user via the Alert framework.");
  gpMaestro->jsreg("application", js_application, 2, "application(application, message)", {}, "Provides the ability to send a message to a given application that is connected to Radial.");
  gpMaestro->jsreg("centralmon", js_centralmon, 2, "centralmon(server, process)", {}, "Provides the ability to retrieve server and/or process information from Central Monitor.");
  gpMaestro->jsreg("chat", js_chat, 3, "chat(target, message, source)", {}, "Provides the ability to send a message to a given channel or user in Internet Relay Chat (IRC) by default or to a user in Live using a Source of live.");
  gpMaestro->jsreg("command", js_command, 5, "command(command, arguments, input, timeout, node)", {"all"}, "Provides the ability to send a command to any Radial server.");
  gpMaestro->jsreg("db", js_db, 2, "db(function, input)", {"dbCentralAccountTypes", "dbCentralApplicationAccountAdd", "dbCentralApplicationAccountRemove", "dbCentralApplicationAccounts", "dbCentralApplicationAccountUpdate", "dbCentralApplicationAdd", "dbCentralApplicationDependAdd", "dbCentralApplicationDependRemove", "dbCentralApplicationDepends", "dbCentralApplicationGroupAdd", "dbCentralApplicationGroupRemove", "dbCentralApplicationGroups", "dbCentralApplicationIssueAdd", "dbCentralApplicationIssueCommentAdd", "dbCentralApplicationIssueComments", "dbCentralApplicationIssueCommentUpdate", "dbCentralApplicationIssues", "dbCentralApplicationIssueUpdate", "dbCentralApplicationRemove", "dbCentralApplicationRepoAdd", "dbCentralApplicationRepoRemove", "dbCentralApplicationRepos", "dbCentralApplicationRepoUpdate", "dbCentralApplications", "dbCentralApplicationServerAdd", "dbCentralApplicationServerDetailAdd", "dbCentralApplicationServerDetailRemove", "dbCentralApplicationServerDetails", "dbCentralApplicationServerDetailUpdate", "dbCentralApplicationServerRemove", "dbCentralApplicationServers", "dbCentralApplicationUpdate", "dbCentralApplicationUserAdd", "dbCentralApplicationUserRemove", "dbCentralApplicationUsers", "dbCentralApplicationUserUpdate", "dbCentralContactTypes", "dbCentralDependents", "dbCentralGroupAdd", "dbCentralGroupRemove", "dbCentralGroups", "dbCentralGroupUpdate", "dbCentralGroupUserAdd", "dbCentralGroupUserRemove", "dbCentralGroupUsers", "dbCentralGroupUserUpdate", "dbCentralLoginTypes", "dbCentralMenuAccesses", "dbCentralNotifyPriorities", "dbCentralPackageTypes", "dbCentralPhpSession", "dbCentralPhpSessionAdd", "dbCentralPhpSessionRemove", "dbCentralRepos", "dbCentralServerAdd", "dbCentralServerDetails", "dbCentralServerGroupAdd", "dbCentralServerGroupRemove", "dbCentralServerGroups", "dbCentralServerRemove", "dbCentralServers", "dbCentralServerUpdate", "dbCentralServerUserAdd", "dbCentralServerUserRemove", "dbCentralServerUsers", "dbCentralServerUserUpdate", "dbCentralUserAdd", "dbCentralUserPasskeyAdd", "dbCentralUserPasskeyRemove", "dbCentralUserPasskeys", "dbCentralUserPasskeyUpdate", "dbCentralUserReminderAdd", "dbCentralUserReminderRemove", "dbCentralUserReminders", "dbCentralUserReminderUpdate", "dbCentralUserRemove", "dbCentralUsers", "dbCentralUserUpdate"}, "Provides the ability to perform a given database function.");
  gpMaestro->jsreg("dbquery", js_dbquery, 2, "dbquery(database, query)", {"central_r", "feedback_r"}, "Provides the ability to query (SQL: select) a given database.");
  gpMaestro->jsreg("dbupdate", js_dbupdate, 2, "dbupdate(database, update)", {"central", "feedback"}, "Provides the ability to update (SQL: delete, insert, update) a given database.");
  gpMaestro->jsreg("email", js_email, 8, "email(from, to, cc, bcc, subject, text, html, file)", {}, "Provides the ability to send an email.");
  gpMaestro->jsreg("flow", js_flow, 3, "flow(plan, flow, request)");
  gpMaestro->jsreg("getError", js_getError, 0, "getError()", {}, "Provides the ability to retrieve the error from the last Maestro function.");
  gpMaestro->jsreg("getRequest", js_getRequest, 0, "getRequest()", {}, "Provides the ability to retrieve the request being provides to the given flow.");
  gpMaestro->jsreg("getResponse", js_getResponse, 0, "getResponse()", {}, "Provides the ability to retrieve the response from teh last Maestro function.");
  gpMaestro->jsreg("getStatus", js_getStatus, 0, "getStatus()", {}, "Provides the ability to retrieve the status from teh last Maestro function.");
  gpMaestro->jsreg("live", js_live, 4, "live(application, user, message, wait)", {}, "Provides the ability to send a live message to an application and/or user.");
  gpMaestro->jsreg("log", js_log, 1, "log(message)", {}, "Provides the ability to write a log message to the local Radial instance.");
  gpMaestro->jsreg("logger", js_logger, 4, "logger(application, function, label, message)", {}, "Provides the ability to send a message to a particular application registered in Logger.");
  gpMaestro->jsreg("mysqlQuery", js_mysqlQuery, 6, "mysqlQuery(server, port, user, password, database, query)", {}, "Provides the ability to query (SQL: select) a given MySQL server and database.");
  gpMaestro->jsreg("mysqlUpdate", js_mysqlUpdate, 6, "mysqlUpdate(server, port, user, password, database, query)", {}, "Provides the ability to update (SQL: delete, insert, update) a given MySQL server and database.");
  gpMaestro->jsreg("pageGroup", js_pageGroup, 2, "pageGroup(group, message)", {}, "Provides the ability to send a page to a group.");
  gpMaestro->jsreg("pageUser", js_pageUser, 2, "pageUser(user, message)", {}, "Provides the ability to send a page to a user.");
  gpMaestro->jsreg("setError", js_setError, 1, "setError(error)", {}, "Provides the ability to set the error prior to exiting the given flow.");
  gpMaestro->jsreg("setRequest", js_setRequest, 1, "setRequest(request)", {}, "Provides the ability to set the request prior to entering a Maestro function.  NOTE: This is not typically required as the requested is normally set as an argument in the function.");
  gpMaestro->jsreg("setResponse", js_setResponse, 1, "setResponse(response)", {}, "Provides the ability to set the response prior to exiting the given flow.");
  gpMaestro->jsreg("setStatus", js_setStatus, 1, "setStatus(status)", {}, "Provides the ability to set the status prior to exiting the given flow.  The status should typically be set to a value of either \"okay\" or \"error\".");
  gpMaestro->jsreg("sqliteCreate", js_sqliteCreate, 2, "sqliteCreate(database, node)", {"all"}, "Provides the ability to create a given SQLite database.");
  gpMaestro->jsreg("sqliteDatabases", js_sqliteDatabases, 0, "sqliteDatabases()", {}, "Provides the ability to retrieve a list SQLite databases.");
  gpMaestro->jsreg("sqliteDrop", js_sqliteDrop, 2, "sqliteDrop(database, node)", {"all"}, "Provides the ability to drop a given SQLite database.");
  gpMaestro->jsreg("sqliteQuery", js_sqliteQuery, 2, "sqliteQuery(database, statement)", {"all"}, "Provides the ability to query (SQL: delete, insert, select, update) a given SQLite database.");
  gpMaestro->jsreg("sshConnect", js_sshConnect, 5, "sshConnect(server, port, user, password, privatekey)", {}, "Provides the ability to connect a SSH session.");
  gpMaestro->jsreg("sshDisconnect", js_sshDisconnect, 1, "sshDisconnect(session)", {}, "Provides the ability to disconnect a SSH session.");
  gpMaestro->jsreg("sshSend", js_sshSend, 3, "sshSend(session, command, wait)", {}, "Provides the ability to send a command over a SSH session.");
  gpMaestro->jsreg("storageAdd", js_storageAdd, 2, "storageAdd(keys, data)", {"all"}, "Provides the ability to add data to storage.");
  gpMaestro->jsreg("storageRemove", js_storageAdd, 1, "storageRemove(keys)", {"all"}, "Provides the ability to remove data from storage.");
  gpMaestro->jsreg("storageRetrieve", js_storageRetrieve, 2, "storageRetrieve(keys, data)", {}, "Provides the ability to retrieve data from storage.");
  gpMaestro->jsreg("storageRetrieveKeys", js_storageRetrieve, 2, "storageRetrieveKeys(keys, data)", {}, "Provides the ability to retrieve key data from storage.");
  gpMaestro->jsreg("storageUpdate", js_storageUpdate, 2, "storageUpdate(keys, data)", {"all"}, "Provides the ability to update data into storage.");
  gpMaestro->process(strPrefix);
  delete gpMaestro;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpMaestro->callback(strPrefix, strPacket, bResponse);
}
void callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  gpMaestro->callbackInotify(strPrefix, strPath, strFile);
}
