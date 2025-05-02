// -*- C++ -*-
// Radial
// -------------------------------------
// file       : link.cpp
// author     : Ben Kietzman
// begin      : 2022-04-28
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Link"
using namespace radial;
Link *gpLink;
int main(int argc, char *argv[])
{
  string strPrefix = "link->main()";
  gpLink = new Link(strPrefix, argc, argv);
  gpLink->process(strPrefix);
  delete gpLink;
  return 0;
}
