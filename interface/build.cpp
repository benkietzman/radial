// -*- C++ -*-
// Radial
// -------------------------------------
// file       : build.cpp
// author     : Ben Kietzman
// begin      : 2025-10-13
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Build"
using namespace radial;
Build *gpBuild = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "build->main()";
  gpBuild = new Build(strPrefix, argc, argv, &callback);
  gpBuild->enableWorkers();
  gpBuild->process(strPrefix);
  delete gpBuild;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpBuild->callback(strPrefix, strPacket, bResponse);
}
