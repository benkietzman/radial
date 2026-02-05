// -*- C++ -*-
// Radial
// -------------------------------------
// file       : emulator.cpp
// author     : Ben Kietzman
// begin      : 2026-02-05
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Emulator"
using namespace radial;
Emulator *gpEmulator;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "emulator->main()";
  gpEmulator = new Emulator(strPrefix, argc, argv, &callback);
  gpEmulator->enableWorkers();
  gpEmulator->setApplication("Emulator");
  gpEmulator->process(strPrefix);
  delete gpEmulator;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpEmulator->callback(strPrefix, strPacket, bResponse);
}
