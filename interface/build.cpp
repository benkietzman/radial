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
void callbackInotify(string strPrefix, const string strPath, const string strFile);
int main(int argc, char *argv[])
{
  string strPrefix = "build->main()";
  gpBuild = new Build(strPrefix, argc, argv, &callback, &callbackInotify);
  gpBuild->enableWorkers();
  gpBuild->process(strPrefix);
  delete gpBuild;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpBuild->callback(strPrefix, strPacket, bResponse);
}
void callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  gpBuild->callbackInotify(strPrefix, strPath, strFile);
}
