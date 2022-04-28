// -*- C++ -*-
// Radial
// -------------------------------------
// file       : storage.cpp
// author     : Ben Kietzman
// begin      : 2022-04-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Storage"
int main(int argc, char *argv[])
{
  string strPrefix = "storage->main()";
  radial::Storage storage(strPrefix, argc, argv, bind(&radial::Storage::callback, &storage, placeholders::_1, placeholders::_2, placeholders::_3));
  storage.process(strPrefix);
  return 0;
}
