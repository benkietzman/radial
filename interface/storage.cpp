// -*- C++ -*-
// Radial
// -------------------------------------
// file       : storage.cpp
// author     : Ben Kietzman
// begin      : 2022-04-27
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Storage"
radial::Storage *gpStorage;
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster);
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "storage->main()";
  gpStorage = new radial::Storage(strPrefix, argc, argv, &callback);
  gpStorage->enableWorkers();
  gpStorage->setAutoMode(&autoMode);
  gpStorage->process(strPrefix);
  delete gpStorage;
  return 0;
}
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  thread threadAutoMode(&radial::Storage::autoMode, gpStorage, strPrefix, strOldMaster, strNewMaster);
  pthread_setname_np(threadAutoMode.native_handle(), "autoMode");
  threadAutoMode.detach();
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpStorage->callback(strPrefix, strPacket, bResponse);
}
