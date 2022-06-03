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
Mysql *gpMysql;
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  string strError, strPrefix = "mysql->main()";
  gpMysql = new Mysql(strPrefix, argc, argv, &callback);
  gpMysql->process(strPrefix);
  delete gpMysql;
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&Mysql::callback, gpMysql, strPrefix, new Json(ptJson), bResponse);
  threadCallback.detach();
}
