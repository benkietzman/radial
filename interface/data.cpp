// -*- C++ -*-
// Radial
// -------------------------------------
// file       : database.cpp
// author     : Ben Kietzman
// begin      : 2025-03-11
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Data"
using namespace radial;
Data *gpData = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
void callbackInotify(string strPrefix, const string strPath, const string strFile);
int main(int argc, char *argv[])
{
  string strPrefix = "data->main()";
  gpData = new Data(strPrefix, argc, argv, &callback, &callbackInotify);
  gpData->enableWorkers();
  gpData->process(strPrefix);
  delete gpData;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpData->callback(strPrefix, strPacket, bResponse);
}
void callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  gpData->callbackInotify(strPrefix, strPath, strFile);
}
