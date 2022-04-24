// -*- C++ -*-
// Radial
// -------------------------------------
// file       : hub.cpp
// author     : Ben Kietzman
// begin      : 2022-03-19
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Hub"
using namespace radial;
extern char **environ;
int main(int argc, char **argv)
{
  Hub hub(argc, argv, environ);
  hub.process();
  return 0;
}
