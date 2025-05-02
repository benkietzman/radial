// -*- C++ -*-
// Radial
// -------------------------------------
// file       : log.cpp
// author     : Ben Kietzman
// begin      : 2022-04-19
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Log"
using namespace radial;
Log *gpLog;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "log->main()";
  gpLog = new Log(strPrefix, argc, argv, &callback);
  gpLog->enableWorkers();
  gpLog->process(strPrefix);
  delete gpLog;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpLog->callback(strPrefix, strPacket, bResponse);
}
