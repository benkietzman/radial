// -*- C++ -*-
// Radial
// -------------------------------------
// file       : command.cpp
// author     : Ben Kietzman
// begin      : 2023-05-16
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Command"
using namespace radial;
Command *gpCommand = NULL;
int main(int argc, char *argv[])
{
  string strPrefix = "command->main()";
  gpCommand = new Command(strPrefix, argc, argv, NULL);
  gpCommand->process(strPrefix);
  delete gpCommand;
  return 0;
}
