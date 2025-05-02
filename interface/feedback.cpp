// -*- C++ -*-
// Radial
// -------------------------------------
// file       : feedback.cpp
// author     : Ben Kietzman
// begin      : 2023-06-02
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
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
  gpFeedback->process(strPrefix);
  delete gpFeedback;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpFeedback->callback(strPrefix, strPacket, bResponse);
}
