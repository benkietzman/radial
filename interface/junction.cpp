// -*- C++ -*-
// Radial
// -------------------------------------
// file       : junction.cpp
// author     : Ben Kietzman
// begin      : 2023-05-18
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Junction"
using namespace radial;
Junction *gpJunction;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "junction->main()";
  gpJunction = new Junction(strPrefix, argc, argv, &callback);
  gpJunction->enableWorkers();
  gpJunction->process(strPrefix);
  delete gpJunction;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpJunction->callback(strPrefix, strPacket, bResponse);
}
