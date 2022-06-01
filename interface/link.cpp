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
Link *gpLink;
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "link->main()";
  gpLink = new Link(strPrefix, argc, argv, &callback);
  thread threadSocket(&Link::socket, gpLink, strPrefix);
  gpLink->process(strPrefix);
  threadSocket.join();
  delete gpLink
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&Link::callback, gpLink, strPrefix, new Json(ptJson), bResponse);
  threadCallback.detach();
} 
