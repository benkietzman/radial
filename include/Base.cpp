// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Base.cpp
// author     : Ben Kietzman
// begin      : 2022-04-19
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Base"
// }}}
extern "C++"
{ 
namespace radial
{
// {{{ Base()
Base::Base(int argc, char **argv)
{
  string strError;

  m_argc = argc;
  m_argv = argv;
  m_bShutdown = false;
  m_strApplication = "Radial";
  m_ulMaxResident = 40 * 1024;
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-d" || (strArg.size() > 7 && strArg.substr(0, 7) == "--data="))
    {
      if (strArg == "-d" && i + 1 < argc && argv[i+1][0] != '-')
      {
        m_strData = argv[++i];
      }
      else
      {
        m_strData = strArg.substr(7, strArg.size() - 7);
      }
      m_manip.purgeChar(m_strData, m_strData, "'");
      m_manip.purgeChar(m_strData, m_strData, "\"");
    }
    else if (strArg == "-m" || (strArg.size() > 9 && strArg.substr(0, 9) == "--memory="))
    {
      stringstream ssMaxResident;
      if (strArg == "-m" && i + 1 < argc && argv[i+1][0] != '-')
      {
        ssMaxResident.str(argv[++i]);
      }
      else
      {
        ssMaxResident.str(strArg.substr(9, strArg.size() - 9));
      }
      ssMaxResident >> m_ulMaxResident;
      m_ulMaxResident *= 1024;
    }
  }
  // }}}
  m_pCentral = new Central(strError);
  m_pCentral->setApplication(m_strApplication);
  m_pUtility = new Utility(strError);
}
// }}}
// {{{ ~Base()
Base::~Base()
{
  delete m_pCentral;
}
// }}}
// {{{ msleep()
void Base::msleep(const unsigned long ulMilliSec)
{
  m_pUtility->msleep(ulMilliSec);
}
// }}}
// {{{ shutdown()
bool Base::shutdown()
{
  return m_bShutdown;
}
// }}}
// {{{ setShutdown()
void Base::setShutdown()
{
  m_bShutdown = true;
}
// }}}
}
}
