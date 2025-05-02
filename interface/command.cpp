// -*- C++ -*-
// Radial
// -------------------------------------
// file       : command.cpp
// author     : Ben Kietzman
// begin      : 2023-05-16
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
#include "include/Command"
using namespace radial;
int main(int argc, char *argv[])
{
  string strPrefix = "command->main()";
  Command command(strPrefix, argc, argv);
  command.process(strPrefix);
  return 0;
}
