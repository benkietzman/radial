// -*- C++ -*-
// Radial
// -------------------------------------
// file       : link.cpp
// author     : Ben Kietzman
// begin      : 2022-04-28
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Link"
using namespace radial;
int main(int argc, char *argv[])
{
  string strPrefix = "link->main()";
  Link link(strPrefix, argc, argv, bind(&Link::callback, &link, placeholders::_1, placeholders::_2, placeholders::_3));
  thread threadSocket(&Link::socket, &link, strPrefix);
  link.process(strPrefix);
  threadSocket.join();
  return 0;
}
