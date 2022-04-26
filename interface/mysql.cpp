// -*- C++ -*-
// Radial
// -------------------------------------
// file       : mysql.cpp
// author     : Ben Kietzman
// begin      : 2022-04-21
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Mysql"
using namespace radial;
int main(int argc, char *argv[])
{
  string strError, strPrefix = "mysql->main()";
  Mysql mysql(strPrefix, argc, argv, bind(&Mysql::callback, &mysql, placeholders::_1, placeholders::_2, placeholders::_3));
  mysql.process(strPrefix);
  return 0;
}
