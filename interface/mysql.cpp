// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Radial
// -------------------------------------
// file       : mysql.cpp
// author     : Ben Kietzman
// begin      : 2022-04-21
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
**************************************************************************/
/*! \file mysql.cpp
* \brief Radial MySQL
*
* Provides the mysql interface.
*/
// {{{ includes
#include "include/Mysql"
using namespace radial;
// }}}
// {{{ global variables
Mysql *gpMysql;
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  string strError, strPrefix = "main()";

  gpMysql = new Mysql(argc, argv);
  if (!gpMysql->process(strPrefix, bind(&Mysql::callback, gpMysql, placeholders::_1, placeholders::_2), strError))
  {
    cerr << strPrefix << "->Mysql::process() error:  " << strError << endl;
  }
  delete gpMysql;

  return 0;
}
// }}}
