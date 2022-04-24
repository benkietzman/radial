// -*- C++ -*-
// Radial
// -------------------------------------
// file       : request.cpp
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
#include "include/Request"
using namespace radial;
int main(int argc, char *argv[])
{
  string strPrefix = "request->main()"
  Request request(argc, argv);
  thread threadAccept(&Request::accept, &request);
  request.process(strPrefix, bind(&Request::callback, &request, placeholders::_1, placeholders::_2));
  threadAccept.join();
  return 0;
}
