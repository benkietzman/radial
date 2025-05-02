// -*- C++ -*-
// Radial
// -------------------------------------
// file       : live.cpp
// author     : Ben Kietzman
// begin      : 2023-01-18
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Live"
radial::Live *gpLive;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "live->main()";
  gpLive = new radial::Live(strPrefix, argc, argv, &callback);
  gpLive->enableWorkers();
  gpLive->process(strPrefix);
  delete gpLive;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpLive->callback(strPrefix, strPacket, bResponse);
}
