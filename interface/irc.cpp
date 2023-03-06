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
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "irc->main()";
  gpIrc = new Irc(strPrefix, argc, argv, &callback);
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
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&Irc::callback, gpIrc, strPrefix, new Json(ptJson), bResponse);
  pthread_setname_np(threadCallback.native_handle(), "callback");
  threadCallback.detach();
}
