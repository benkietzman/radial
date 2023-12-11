// -*- C++ -*-
// Radial
// -------------------------------------
// file       : feedback.cpp
// author     : Ben Kietzman
// begin      : 2023-06-02
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Feedback"
using namespace radial;
Feedback *gpFeedback = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "feedback->main()";
  gpFeedback = new Feedback(strPrefix, argc, argv, &callback);
  gpFeedback->enableWorkers();
  gpFeedback->setApplication("Feedback");
  thread threadSchedule(&Feedback::schedule, gpFeedback, strPrefix);
  pthread_setname_np(threadSchedule.native_handle(), "schedule");
  gpFeedback->process(strPrefix);
  threadSchedule.join();
  delete gpFeedback;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpFeedback->callback(strPrefix, strPacket, bResponse);
}
