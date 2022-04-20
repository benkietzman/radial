// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Radial
// -------------------------------------
// file       : log.cpp
// author     : Ben Kietzman
// begin      : 2022-03-19
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
* \brief Radial Hub
*
* Provides the central hub to which interfaces connect radially.
*/
// {{{ includes
#include "../include/Log"
using namespace radial;
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  string strError, strPrefix = "main()";
  Log log(argc, argv);

  if (!log.process(strPrefix, log.callback, strError))
  {
    cerr << strPrefix << "->Log::process() error:  " << strError << endl;
  }

  return 0;
}
// }}}
