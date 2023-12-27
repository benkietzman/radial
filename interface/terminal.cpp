// -*- C++ -*-
// Radial
// -------------------------------------
// file       : ssh.cpp
// author     : Ben Kietzman
// begin      : 2023-12-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Terminal"
radial::Terminal *gpTerminal = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "ssj->main()";
  gpTerminal = new radial::Terminal(strPrefix, argc, argv, &callback);
  gpTerminal->enableWorkers();
  thread threadSchedule(&radial::Terminal::schedule, gpTerminal, strPrefix);
  pthread_setname_np(threadSchedule.native_handle(), "schedule");
  gpTerminal->process(strPrefix);
  threadSchedule.join();
  delete gpTerminal;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpTerminal->callback(strPrefix, strPacket, bResponse);
}
