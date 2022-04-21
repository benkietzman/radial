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
// {{{ prototypes
void callback(string strPrefix, Json *ptJson, string &strError);
void incoming(string strPrefix);
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  string strError, strPrefix = "main()";

  gpRequest = new Request(argc, argv);
  thread threadIncoming(incoming, strPrefix);
  pthread_setname_np(threadIncoming.native_handle(), "incoming");
  if (!gpRequest->process(strPrefix, callback, strError))
  {
    cerr << strPrefix << "->Request::process() error:  " << strError << endl;
  }
  delete gpRequest;
  threadIncoming.join();

  return 0;
}
// }}}
// {{{ callback()
void callback(string strPrefix, Json *ptJson, string &strError)
{
  gpRequest->callback(strPrefix, ptJson, strError);
}
// }}}
// {{{ incoming()
void incoming(string strPrefix)
{
  strPrefix += "->incoming()";
  gpRequest->incoming(strPrefix);
}
// }}}
