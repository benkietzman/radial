// -*- C++ -*-
// Radial
// -------------------------------------
// file       : central.cpp
// author     : Ben Kietzman
// begin      : 2023-02-18
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Central"
radial::Central *gpCentral = NULL;
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster);
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "central->main()";
  gpCentral = new radial::Central(strPrefix, argc, argv, &callback);
  gpCentral->enableWorkers();
  gpCentral->setApplication("Central");
  gpCentral->setAutoMode(&autoMode);
  gpCentral->process(strPrefix);
  delete gpCentral;
  return 0;
}
void autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  thread threadAutoMode(&radial::Central::autoMode, gpCentral, strPrefix, strOldMaster, strNewMaster);
  pthread_setname_np(threadAutoMode.native_handle(), "autoMode");
  threadAutoMode.detach();
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpCentral->callback(strPrefix, strPacket, bResponse);
}
