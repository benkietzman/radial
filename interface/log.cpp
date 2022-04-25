// -*- C++ -*-
// Radial
// -------------------------------------
// file       : log.cpp
// author     : Ben Kietzman
// begin      : 2022-04-19
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Log"
using namespace radial;
int main(int argc, char *argv[])
{
  string strPrefix = "log->main()";
  Log log(argc, argv, bind(&Log::callback, &log, placeholders::_1, placeholders::_2, placeholders::_3));
  log.process(strPrefix);
  return 0;
}
