// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Radial
// -------------------------------------
// file       : log.cpp
// author     : Ben Kietzman
// begin      : 2022-04-19
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
/*! \file hub.cpp
* \brief Radial Log
*
* Provides the log interface.
*/
// {{{ includes
#include "include/Log"
using namespace radial;
// }}}
// {{{ global variables
Log *gpLog;
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  string strError, strPrefix = "main()";

  gpLog = new Log(argc, argv);
  if (!gpLog->process(strPrefix, bind(&Log::callback, gpLog, placeholders::_1, placeholders::_2), strError))
  {
    cerr << strPrefix << "->Log::process() error:  " << strError << endl;
  }
  delete gpLog;

  return 0;
}
// }}}
