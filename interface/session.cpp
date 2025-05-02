// -*- C++ -*-
// Radial
// -------------------------------------
// file       : session.cpp
// author     : Ben Kietzman
// begin      : 2023-07-11
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Session"
using namespace radial;
Session *gpSession;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "session->main()";
  gpSession = new Session(strPrefix, argc, argv, &callback);
  gpSession->enableWorkers();
  gpSession->process(strPrefix);
  delete gpSession;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpSession->callback(strPrefix, strPacket, bResponse);
}
