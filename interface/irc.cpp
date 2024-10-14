// -*- C++ -*-
// Radial
// -------------------------------------
// file       : irc.cpp
// author     : Ben Kietzman
// begin      : 2022-12-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Irc"
using namespace radial;
Irc *gpIrc;
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster);
void callback(string strPrefix, const string strPacket, const bool bResponse);
void inotifyCallback(string strPrefix, const string strPath, const string strFile);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "irc->main()";
  gpIrc = new Irc(strPrefix, argc, argv, &callback, &inotifyCallback);
  gpIrc->enableWorkers();
  gpIrc->setAutoMode(&autoMode);
  thread threadBot(&Irc::bot, gpIrc, strPrefix);
  pthread_setname_np(threadBot.native_handle(), "bot");
  gpIrc->process(strPrefix);
  threadBot.join();
  delete gpIrc;
  return 0;
}
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  thread threadAutoMode(&Irc::autoMode, gpIrc, strPrefix, strOldMaster, strNewMaster);
  pthread_setname_np(threadAutoMode.native_handle(), "autoMode");
  threadAutoMode.detach();
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpIrc->callback(strPrefix, strPacket, bResponse);
}
void inotifyCallback(string strPrefix, const string strPath, const string strFile)
{
  gpIrc->inotifyCallback(strPrefix, strPath, strFile);
}
