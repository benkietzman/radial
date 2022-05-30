// -*- C++ -*-
// Radial
// -------------------------------------
// file       : auth.cpp
// author     : Ben Kietzman
// begin      : 2022-05-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Auth"
int main(int argc, char *argv[])
{
  string strPrefix = "auth->main()";
  radial::Auth auth(strPrefix, argc, argv, bind(&radial::Auth::callback, &auth, placeholders::_1, placeholders::_2, placeholders::_3));
  auth.process(strPrefix);
  return 0;
}
