// -*- C++ -*-
// Radial
// -------------------------------------
// file       : ssh.cpp
// author     : Ben Kietzman
// begin      : 2023-12-21
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
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
