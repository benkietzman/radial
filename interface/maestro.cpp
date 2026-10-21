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
JSValue js_setError(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setError(ctx, obj, argc, argv);}
JSValue js_setRequest(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setRequest(ctx, obj, argc, argv);}
JSValue js_setResponse(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setResponse(ctx, obj, argc, argv);}
JSValue js_setStatus(JSContext *ctx, JSValueConst obj, int argc, JSValueConst *argv) {return gpMaestro->js_setStatus(ctx, obj, argc, argv);}
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
  gpMaestro->jsreg("db", js_db, 2, "db(function, input)", {"dbCentralAccountTypes", "dbCentralApplicationAccountAdd", "dbCentralApplicationAccountRemove", "dbCentralApplicationAccounts", "dbCentralApplicationAccountUpdate", "dbCentralApplicationAdd", "dbCentralApplicationDependAdd", "dbCentralApplicationDependRemove", "dbCentralApplicationDepends", "dbCentralApplicationGroupAdd", "dbCentralApplicationGroupRemove", "dbCentralApplicationGroups", "dbCentralApplicationIssueAdd", "dbCentralApplicationIssueCommentAdd", "dbCentralApplicationIssueComments", "dbCentralApplicationIssueCommentUpdate", "dbCentralApplicationIssues", "dbCentralApplicationIssueUpdate", "dbCentralApplicationRemove", "dbCentralApplicationRepoAdd", "dbCentralApplicationRepoRemove", "dbCentralApplicationRepos", "dbCentralApplicationRepoUpdate", "dbCentralApplications", "dbCentralApplicationServerAdd", "dbCentralApplicationServerDetailAdd", "dbCentralApplicationServerDetailRemove", "dbCentralApplicationServerDetails", "dbCentralApplicationServerDetailUpdate", "dbCentralApplicationServerRemove", "dbCentralApplicationServers", "dbCentralApplicationUpdate", "dbCentralApplicationUserAdd", "dbCentralApplicationUserRemove", "dbCentralApplicationUsers", "dbCentralApplicationUserUpdate", "dbCentralContactTypes", "dbCentralDependents", "dbCentralGroupAdd", "dbCentralGroupRemove", "dbCentralGroups", "dbCentralGroupUpdate", "dbCentralGroupUserAdd", "dbCentralGroupUserRemove", "dbCentralGroupUsers", "dbCentralGroupUserUpdate", "dbCentralLoginTypes", "dbCentralMenuAccesses", "dbCentralNotifyPriorities", "dbCentralPackageTypes", "dbCentralPhpSession", "dbCentralPhpSessionAdd", "dbCentralPhpSessionRemove", "dbCentralRepos", "dbCentralServerAdd", "dbCentralServerDetails", "dbCentralServerGroupAdd", "dbCentralServerGroupRemove", "dbCentralServerGroups", "dbCentralServerRemove", "dbCentralServers", "dbCentralServerUpdate", "dbCentralServerUserAdd", "dbCentralServerUserRemove", "dbCentralServerUsers", "dbCentralServerUserUpdate", "dbCentralUserAdd", "dbCentralUserPasskeyAdd", "dbCentralUserPasskeyRemove", "dbCentralUserPasskeys", "dbCentralUserPasskeyUpdate", "dbCentralUserReminderAdd", "dbCentralUserReminderRemove", "dbCentralUserReminders", "dbCentralUserReminderUpdate", "dbCentralUserRemove", "dbCentralUsers", "dbCentralUserUpdate"}, "Provides the ability to perform a specific database function.");
  gpMaestro->jsreg("dbquery", js_dbquery, 2, "dbquery(database, query)", {"central_r", "feedback_r"}, "Provides the ability to query (SQL: select) a specific database.");
  gpMaestro->jsreg("dbupdate", js_dbupdate, 2, "dbupdate(database, update)", {"central", "feedback"}, "Provides the ability to update (SQL: delete, insert, update) a specific database.");
  gpMaestro->jsreg("email", js_email, 8, "email(from, to, cc, bcc, subject, text, html, file)", {}, "Provides the ability to send an email.");
  gpMaestro->jsreg("flow", js_flow, 3, "flow(plan, flow, request)");
  gpMaestro->jsreg("getError", js_getError, 0, "getError()", {}, "Provides the ability to retrieve the error from the last Maestro function.");
  gpMaestro->jsreg("getRequest", js_getRequest, 0, "getRequest()", {}, "Provides the ability to retrieve the request being provides to the given flow.");
  gpMaestro->jsreg("getResponse", js_getResponse, 0, "getResponse()", {}, "Provides the ability to retrieve the response from teh last Maestro function.");
  gpMaestro->jsreg("getStatus", js_getStatus, 0, "getStatus()", {}, "Provides the ability to retrieve the status from teh last Maestro function.");
  gpMaestro->jsreg("live", js_live, 4, "live(application, user, message, wait)", {}, "Provides the ability to send a live message to an application and/or user.");
  gpMaestro->jsreg("log", js_log, 1, "log(message)", {}, "Provides the ability to write a log message to the local Radial instance.");
  gpMaestro->jsreg("logger", js_logger, 4, "logger(application, function, label, message)", {}, "Provides the ability to send a message to a particular application registered in Logger.");
  gpMaestro->jsreg("setError", js_setError, 1, "setError(error)", {}, "Provides the ability to set the error prior to exiting the given flow.");
  gpMaestro->jsreg("setRequest", js_setRequest, 1, "setRequest(request)", {}, "Provides the ability to set the request prior to entering a Maestro function.  NOTE: This is not typically required as the requested is normally set as an argument in the function.");
  gpMaestro->jsreg("setResponse", js_setResponse, 1, "setResponse(response)", {}, "Provides the ability to set the response prior to exiting the given flow.");
  gpMaestro->jsreg("setStatus", js_setStatus, 1, "setStatus(status)", {}, "Provides the ability to set the status prior to exiting the given flow.  The status should typically be set to a value of either \"okay\" or \"error\".");
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
