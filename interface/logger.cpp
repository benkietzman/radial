// -*- C++ -*-
// Radial
// -------------------------------------
// file       : logger.cpp
// author     : Ben Kietzman
// begin      : 2023-05-17
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Logger"
using namespace radial;
Logger *gpLogger;
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "logger->main()";
  gpLogger = new Logger(strPrefix, argc, argv, &callback);
  thread threadPush(&Logger::push, gpLogger, strPrefix);
  pthread_setname_np(threadPush.native_handle(), "push");
  gpLogger->process(strPrefix);
  threadPush.join();
  delete gpLogger;
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&Logger::callback, gpLogger, strPrefix, new Json(ptJson), bResponse);
  pthread_setname_np(threadCallback.native_handle(), "callback");
  threadCallback.detach();
}
