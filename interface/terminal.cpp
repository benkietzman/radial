// -*- C++ -*-
// Radial
// -------------------------------------
// file       : ssh.cpp
// author     : Ben Kietzman
// begin      : 2023-12-27
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Terminal"
radial::Terminal *gpTerminal = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "terminal->main()";
  gpTerminal = new radial::Terminal(strPrefix, argc, argv, &callback);
  gpTerminal->enableWorkers();
  gpTerminal->process(strPrefix);
  delete gpTerminal;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpTerminal->callback(strPrefix, strPacket, bResponse);
}
