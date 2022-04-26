// -*- C++ -*-
// Radial
// -------------------------------------
// file       : hub.cpp
// author     : Ben Kietzman
// begin      : 2022-03-19
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include "include/Hub"
using namespace radial;
extern char **environ;
Hub *gpHub;
void sighandle(const int nSignal);
int main(int argc, char **argv)
{
  string strPrefix = "hub->main()";
  gpHub = new Hub(strPrefix, argc, argv, environ, sighandle);
  gpHub->process(strPrefix);
  delete gpHub;
  return 0;
}
void sighandle(const int nSignal)
{
  string strSignal;
  stringstream ssMessage, ssPrefix;
  sethandles(sigdummy);
  ssPrefix << "hub->sighandle(" << nSignal << ")";
  ssMessage << ssPrefix.str() << ":  " << sigstring(strSignal, nSignal);
  if (nSignal != SIGINT && nSignal != SIGTERM)
  {
    gpHub->notify(ssMessage.str());
  }
  else
  {
    gpHub->log(ssMessage.str());
  }
  gpHub->setShutdown(ssPrefix.str());
}
