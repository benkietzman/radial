// -*- C++ -*-
// Radial
// -------------------------------------
// file       : email.cpp
// author     : Ben Kietzman
// begin      : 2023-03-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Email"
using namespace radial;
Email *gpEmail;
void callback(string strPrefix, Json *ptJson, const bool bResponse);
int main(int argc, char *argv[])
{
  string strPrefix = "email->main()";
  gpEmail = new Email(strPrefix, argc, argv, &callback);
  gpEmail->process(strPrefix);
  delete gpEmail;
  return 0;
}
void callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  thread threadCallback(&Email::callback, gpEmail, strPrefix, new Json(ptJson), bResponse);
  pthread_setname_np(threadCallback.native_handle(), "callback");
  threadCallback.detach();
}