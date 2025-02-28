// -*- C++ -*-
// Radial
// -------------------------------------
// file       : centralmon.cpp
// author     : Ben Kietzman
// begin      : 2025-02-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/

#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
using namespace std;
#include <Radial>
#include <StringManip>
#include <Warden>
using namespace common;

int main(int argc, char *argv[])
{
  string d = ((argc >= 2)?argv[1]:"/data/warden/socket"), e;
  Warden w("Radial", d, e);

  if (e.empty())
  {
    string c;
    if (w.vaultRetrieve({"radial", "radial", "Password"}, c, e))
    {
      Radial r(e);
      r.setCredentials("radial", c);
      r.useSingleSocket(true);
    }
    else
    {
      cerr << "Warden::vaultRetrieve() error [" << d << ",radial,radial,Password]:  " << e << endl;
    }
  }
  else
  {
    cerr << "Warden::Warden() error [" << d << "]:  " << e << endl;
  }

  return 0;
}
