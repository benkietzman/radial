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
  utsname tServer;

  m_argc = argc;
  m_argv = argv;
  m_bShutdown = false;
  time(&m_CMonitor[0]);
  m_strApplication = "Radial";
  uname(&tServer);
  m_strNode = tServer.nodename;
  m_ulMaxResident = 40 * 1024;
  m_unMonitor = 0;
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
    else if (strArg == "-w" || (strArg.size() > 9 && strArg.substr(0, 9) == "--warden="))
    {
      if (strArg == "-w" && i + 1 < argc && argv[i+1][0] != '-')
      {
        m_strWarden = argv[++i];
      }
      else
      {
        m_strWarden = strArg.substr(9, strArg.size() - 9);
      }
      m_manip.purgeChar(m_strWarden, m_strWarden, "'");
      m_manip.purgeChar(m_strWarden, m_strWarden, "\"");
    }
  }
  // }}}
  m_pCentral = new Central(strError);
  m_pCentral->setApplication(m_strApplication);
  m_pUtility = new Utility(strError);
  m_pWarden = NULL;
  if (!m_strWarden.empty())
  {
    m_pWarden = new Warden(m_strApplication, m_strWarden, strError);
  }
}
// }}}
// {{{ ~Base()
Base::~Base()
{
  delete m_pCentral;
  if (m_pWarden != NULL)
  {
    delete m_pWarden;
  }
}
// }}}
// {{{ monitor()
size_t Base::monitor(string &strMessage)
{
  size_t unResult = 0;

  time(&m_CMonitor[1]);
  if (m_CMonitor[1] - m_CMonitor[0] > 10)
  {
    float fCpu, fMem;
    string strError;
    stringstream ssMessage;
    time_t CTime;
    unsigned long ulImage, ulResident;
    m_CMonitor[0] = m_CMonitor[1];
    m_pCentral->getProcessStatus(CTime, fCpu, fMem, ulImage, ulResident);
    if (ulResident >= m_ulMaxResident)
    {
      unResult = 2;
      ssMessage << "The process has a resident size of " << ulResident << " KB which exceeds the maximum resident restriction of " << m_ulMaxResident << " KB.  Shutting down process.";
      strMessage = ssMessage.str();
    }
    else if (++m_unMonitor == 60)
    {
      unResult = 1;
      m_unMonitor = 0;
      ssMessage << "Resident size is " << ulResident << ".";
      strMessage = ssMessage.str();
    }
  }

  return unResult;
}
// }}}
// {{{ msleep()
void Base::msleep(const unsigned long ulMilliSec)
{
  m_pUtility->msleep(ulMilliSec);
}
// }}}
// {{{ setShutdown()
void Base::setShutdown()
{
  m_bShutdown = true;
}
// }}}
// {{{ shutdown()
bool Base::shutdown()
{
  return m_bShutdown;
}
// }}}
}
}
