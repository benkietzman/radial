// -*- C++ -*-
// Radial
// -------------------------------------
// file       : application.cpp
// author     : Ben Kietzman
// begin      : 2025-05-27
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Application"
using namespace radial;
Application *gpApplication = NULL;
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster);
void callback(string strPrefix, const string strPacket, const bool bResponse);
void callbackInotify(string strPrefix, const string strPath, const string strFile);
int main(int argc, char *argv[])
{
  string strPrefix = "application->main()";
  gpApplication = new Application(strPrefix, argc, argv, &callback, &callbackInotify);
  gpApplication->enableWorkers();
  gpApplication->setAutoMode(&autoMode);
  gpApplication->process(strPrefix);
  delete gpApplication;
  return 0;
}
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  thread threadAutoMode(&Application::autoMode, gpApplication, strPrefix, strOldMaster, strNewMaster);
  pthread_setname_np(threadAutoMode.native_handle(), "autoMode");
  threadAutoMode.detach();
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpApplication->callback(strPrefix, strPacket, bResponse);
}
void callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  gpApplication->callbackInotify(strPrefix, strPath, strFile);
}
