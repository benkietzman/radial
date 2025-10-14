// -*- C++ -*-
// Radial
// -------------------------------------
// file       : builder.cpp
// author     : Ben Kietzman
// begin      : 2025-10-13
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Builder"
using namespace radial;
Builder *gpBuilder = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
void callbackInotify(string strPrefix, const string strPath, const string strFile);
int main(int argc, char *argv[])
{
  string strPrefix = "builder->main()";
  gpBuilder = new Builder(strPrefix, argc, argv, &callback, &callbackInotify);
  gpBuilder->enableWorkers();
  gpBuilder->process(strPrefix);
  delete gpBuilder;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpBuilder->callback(strPrefix, strPacket, bResponse);
}
void callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  gpBuilder->callbackInotify(strPrefix, strPath, strFile);
}
