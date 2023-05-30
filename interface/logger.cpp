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
radial::Logger *gpLogger;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "logger->main()";
  gpLogger = new radial::Logger(strPrefix, argc, argv, &callback);
  gpLogger->process(strPrefix);
  delete gpLogger;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  thread threadCallback(&radial::Logger::callback, gpLogger, strPrefix, strPacket, bResponse);
  pthread_setname_np(threadCallback.native_handle(), "callback");
  threadCallback.detach();
}
