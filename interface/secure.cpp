// -*- C++ -*-
// Radial
// -------------------------------------
// file       : secure.cpp
// author     : Ben Kietzman
// begin      : 2022-06-27
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Secure"
using namespace radial;
Secure *gpSecure;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "secure->main()";
  gpSecure = new Secure(strPrefix, argc, argv, &callback);
  gpSecure->enableWorkers();
  gpSecure->process(strPrefix);
  delete gpSecure;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpSecure->callback(strPrefix, strPacket, bResponse);
}
