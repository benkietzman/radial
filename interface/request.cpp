// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Radial
// -------------------------------------
// file       : request.cpp
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
/*! \file request.cpp
* \brief Radial Request
*
* Provides the request interface.
*/
// {{{ includes
#include "include/Request"
using namespace radial;
// }}}
// {{{ global variables
Request *gpRequest;
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  string strError, strPrefix = "main()";

  gpRequest = new Request(argc, argv);
  gpRequest->accept(strPrefix);
  delete gpRequest;

  return 0;
}
// }}}
