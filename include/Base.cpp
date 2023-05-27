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
  string strError, strProxyPort, strProxyServer;
  utsname tServer;

  setlocale(LC_ALL, "");
  m_argc = argc;
  m_argv = argv;
  m_bShutdown = false;
  m_cDelimiter = char(26);
  time(&m_CMonitor[0]);
  m_strApplication = "Radial";
  uname(&tServer);
  m_strNode = tServer.nodename;
  m_ulMaxResident = 40 * 1024;
  m_unMonitor = 0;
  m_unThreads = 0;
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
    else if ((strArg.size() > 12 && strArg.substr(0, 12) == "--proxyport="))
    {
      strProxyPort = strArg.substr(12, strArg.size() - 12);
      m_manip.purgeChar(strProxyPort, strProxyPort, "'");
      m_manip.purgeChar(strProxyPort, strProxyPort, "\"");
    }
    else if ((strArg.size() > 14 && strArg.substr(0, 14) == "--proxyserver="))
    {
      strProxyServer = strArg.substr(14, strArg.size() - 14);
      m_manip.purgeChar(strProxyServer, strProxyServer, "'");
      m_manip.purgeChar(strProxyServer, strProxyServer, "\"");
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
  m_pJunction = new ServiceJunction(strError);
  m_pJunction->setApplication(m_strApplication);
  m_pJunction->setTimeout("300");
  m_pJunction->setThrottle(100);
  m_pJunction->useSecureJunction(true);
  m_pLogger = new Logger(strError);
  m_pLogger->setTimeout("10");
  m_pLogger->setThrottle(100);
  m_pUtility = new Utility(strError);
  if (!strProxyServer.empty() && !strProxyPort.empty())
  {
    m_pUtility->setProxy(strProxyServer, strProxyPort);
  }
  m_pWarden = NULL;
  if (!m_strWarden.empty())
  {
    Json *ptCred = new Json;
    m_pWarden = new Warden(m_strApplication, m_strWarden, strError);
    if (m_pWarden->vaultRetrieve({"logger"}, ptCred, strError) && !empty(ptCred, "Password") && !empty(ptCred, "User"))
    {
      m_pLogger->setCredentials("Radial", ptCred->m["User"]->v, ptCred->m["Password"]->v);
    }
    delete ptCred;
  }
}
// }}}
// {{{ ~Base()
Base::~Base()
{
  size_t unThreads;

  for (auto &i : m_i)
  {
    delete (i.second);
  }
  m_i.clear();
  for (auto &i : m_l)
  {
    delete i;
  }
  m_l.clear();
  do 
  {
    m_mutexBase.lock();
    unThreads = m_unThreads;
    m_mutexBase.unlock();
    if (unThreads > 0)
    {
      msleep(100);
    }
  } while (unThreads > 0);
  delete m_pCentral;
  delete m_pJunction;
  delete m_pUtility;
  if (m_pWarden != NULL)
  {
    delete m_pWarden;
  }
}
// }}}
// {{{ empty()
bool Base::empty(Json *ptJson, const string strField)
{
  return (!exist(ptJson, strField) || ptJson->m[strField]->v.empty());
} 
// }}}
// {{{ esc()
string Base::esc(const string strValue)
{
  string strEscaped;

  return m_manip.escape(strValue, strEscaped);
}
// }}}
// {{{ exist()
bool Base::exist(Json *ptJson, const string strField)
{
  return (ptJson->m.find(strField) != ptJson->m.end());
}
// }}}
// {{{ monitor()
size_t Base::monitor(string &strMessage)
{
  return monitor(getpid(), m_CMonitor, m_unMonitor, m_ulMaxResident, strMessage);
}
size_t Base::monitor(const pid_t nPid, time_t CMonitor[2], size_t &unMonitor, unsigned long ulMaxResident, string &strMessage)
{
  size_t unResult = 0;

  time(&CMonitor[1]);
  if ((CMonitor[1] - CMonitor[0]) > 30)
  {
    float fCpu = 0, fMem = 0;
    string strError;
    stringstream ssMessage;
    time_t CTime = 0;
    unsigned long ulImage = 0, ulResident = 0;
    CMonitor[0] = CMonitor[1];
    m_pCentral->getProcessStatus(nPid, CTime, fCpu, fMem, ulImage, ulResident);
    if (ulResident >= ulMaxResident)
    {
      unResult = 2;
      ssMessage << "The process has a resident size of " << (ulResident / 1024) << " MB which exceeds the maximum resident restriction of " << (ulMaxResident / 1024) << " MB.  Shutting down process.";
      strMessage = ssMessage.str();
    }
    else if (++unMonitor == 60)
    {
      unResult = 1;
      unMonitor = 0;
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
// {{{ pack()
string Base::pack(radialPacket &p, string &d)
{
  stringstream ssData;
  Json *r = new Json;

  if (!p.d.empty())
  {
    r->i("_d", p.d);
  }
  if (!p.s.empty())
  {
    r->i("_s", p.s);
  }
  if (!p.t.empty())
  {
    r->i("_t", p.t);
  }
  if (!p.u.empty())
  {
    r->i("_u", p.u);
  }
  ssData << r << m_cDelimiter << p.p;
  delete r;
  d = ssData.str();

  return d;
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
// {{{ thread
// {{{ threadDecrement()
void Base::threadDecrement()
{
  m_mutexBase.lock();
  if (m_unThreads > 0)
  {
    m_unThreads--;
  }
  m_mutexBase.unlock();
}
// }}}
// {{{ threadIncrement()
void Base::threadIncrement()
{
  m_mutexBase.lock();
  m_unThreads++;
  m_mutexBase.unlock();
}
// }}}
// }}}
// {{{ unpack()
void Base::unpack(const string d, radialPacket &p)
{
  string strRoute;
  stringstream ssData(d);
  Json *r;

  getline(ssData, strRoute, m_cDelimiter);
  r = new Json(strRoute);
  getline(ssData, p.p, m_cDelimiter);
  if (!empty(r, "_d"))
  {
    p.d = r->m["_d"]->v;
  }
  if (!empty(r, "_s"))
  {
    p.s = r->m["_s"]->v;
  }
  if (!empty(r, "_t"))
  {
    p.t = r->m["_t"]->v;
  }
  if (!empty(r, "_u"))
  {
    p.u = r->m["_u"]->v;
  }
}
// }}}
}
}
