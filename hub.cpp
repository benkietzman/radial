// -*- C++ -*-
// Radial
// -------------------------------------
// file       : hub.cpp
// author     : Ben Kietzman
// begin      : 2022-03-19
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Hub"
using namespace radial;
extern char **environ;
Hub *gpHub;
void sighandle(const int nSignal);
int main(int argc, char **argv)
{
  string strPrefix = "hub->main()";
  umask(002);
  gpHub = new Hub(argc, argv, environ, sighandle);
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
  gpHub->setShutdown(ssPrefix.str(), "", true);
}
