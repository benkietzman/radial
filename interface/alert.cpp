// -*- C++ -*-
// Radial
// -------------------------------------
// file       : alert.cpp
// author     : Ben Kietzman
// begin      : 2023-09-22
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Alert"
using namespace radial;
Alert *gpAlert;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "alert->main()";
  gpAlert = new Alert(strPrefix, argc, argv, &callback);
  gpAlert->enableWorkers();
  gpAlert->process(strPrefix);
  delete gpAlert;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpAlert->callback(strPrefix, strPacket, bResponse);
}
