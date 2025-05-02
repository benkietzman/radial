// -*- C++ -*-
// Radial
// -------------------------------------
// file       : logger.cpp
// author     : Ben Kietzman
// begin      : 2023-05-17
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Logger"
using namespace radial;
radial::Logger *gpLogger;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "logger->main()";
  gpLogger = new radial::Logger(strPrefix, argc, argv, &callback);
  gpLogger->enableWorkers();
  gpLogger->process(strPrefix);
  delete gpLogger;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpLogger->callback(strPrefix, strPacket, bResponse);
}
