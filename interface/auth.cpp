// -*- C++ -*-
// Radial
// -------------------------------------
// file       : auth.cpp
// author     : Ben Kietzman
// begin      : 2022-05-27
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Auth"
using namespace radial;
Auth *gpAuth;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "auth->main()";
  gpAuth = new Auth(strPrefix, argc, argv, &callback);
  gpAuth->enableWorkers();
  thread threadProcess(&Auth::process, gpAuth, strPrefix);
  pthread_setname_np(threadProcess.native_handle(), "process");
  if (!gpAuth->init())
  {
    gpAuth->setShutdown();
  }
  threadProcess.join();
  delete gpAuth;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpAuth->callback(strPrefix, strPacket, bResponse);
}
