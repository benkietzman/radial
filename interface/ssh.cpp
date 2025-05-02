// -*- C++ -*-
// Radial
// -------------------------------------
// file       : ssh.cpp
// author     : Ben Kietzman
// begin      : 2023-12-21
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Ssh"
using namespace radial;
Ssh *gpSsh = NULL;
void callback(string strPrefix, const string strPacket, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "ssh->main()";
  gpSsh = new Ssh(strPrefix, argc, argv, &callback);
  gpSsh->enableWorkers();
  gpSsh->process(strPrefix);
  delete gpSsh;
  return 0;
}
void callback(string strPrefix, const string strPacket, const bool bResponse)
{
  gpSsh->callback(strPrefix, strPacket, bResponse);
}
