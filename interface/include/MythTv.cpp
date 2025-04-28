// -*- C++ -*-
// Radial
// -------------------------------------
// file       : MythTv.cpp
// author     : Ben Kietzman
// begin      : 2025-04-24
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "MythTv"
// }}}
extern "C++"
{
namespace radial
{
// {{{ MythTv()
MythTv::MythTv(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "mythtv", argc, argv, pCallback)
{
  m_strPort = "6544";
  m_strServer = "localhost";
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg.size() > 7 && strArg.substr(0, 7) == "--port=")
    {
      m_strPort = strArg.substr(7, strArg.size() - 7);
      m_manip.purgeChar(m_strPort, m_strPort, "'");
      m_manip.purgeChar(m_strPort, m_strPort, "\"");
    }
    else if (strArg.size() > 9 && strArg.substr(0, 9) == "--server=")
    {
      m_strServer = strArg.substr(9, strArg.size() - 9);
      m_manip.purgeChar(m_strServer, m_strServer, "'");
      m_manip.purgeChar(m_strServer, m_strServer, "\"");
    }
  }
  // }}}
  // {{{ functions
  m_functions["action"] = &MythTv::action;
  m_functions["dvrGetRecordedList"] = &MythTv::dvrGetRecordedList;
  m_functions["dvrGetUpcomingList"] = &MythTv::dvrGetUpcomingList;
  m_functions["guideGetProgramGuide"] = &MythTv::guideGetProgramGuide;
  m_functions["status"] = &MythTv::status;
  // }}}
  m_pThreadSchedule = new thread(&MythTv::schedule, this, strPrefix);
  pthread_setname_np(m_pThreadSchedule->native_handle(), "schedule");
}
// }}}
// {{{ ~MythTv()
MythTv::~MythTv()
{
  m_pThreadSchedule->join();
  delete m_pThreadSchedule;
}
// }}}
// {{{ callback()
void MythTv::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->MythTv::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    string strFunction = ptJson->m["Function"]->v;
    radialUser d;
    userInit(ptJson, d);
    if (m_functions.find(strFunction) != m_functions.end())
    {
      if ((this->*m_functions[strFunction])(d, strError))
      {
        bResult = true;
      }
    }
    else
    {
      strError = "Please provide a valid Function.";
    }
    if (bResult)
    {
      if (exist(ptJson, "Response"))
      {
        delete ptJson->m["Response"];
      }
      ptJson->m["Response"] = d.p->m["o"];
      d.p->m.erase("o");
    }
    userDeinit(d);
  }
  else
  {
    strError = "Please provide the Function.";
  }
  ptJson->i("Status", ((bResult)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->i("Error", strError);
  }
  if (bResponse)
  {
    ptJson->j(p.p);
    hub(p, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
// {{{ dvrGetRecordedList()
bool MythTv::dvrGetRecordedList(radialUser &d, string &e)
{ 
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (request("Dvr", "GetRecordedList", i, o, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ dvrGetUpcomingList()
bool MythTv::dvrGetUpcomingList(radialUser &d, string &e)
{ 
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (request("Dvr", "GetUpcomingList", i, o, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ guideGetProgramGuide()
bool MythTv::guideGetProgramGuide(radialUser &d, string &e)
{ 
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (request("Guide", "GetProgramGuide", i, o, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ request()
bool MythTv::request(const string strService, const string strFunction, Json *ptRequest, Json *ptResponse, string &strError)
{ 
  bool bResult = false;
  list<Json *> in, out;
  Json *ptReq = new Json;

  ptReq->i("Service", "mythtv");
  ptReq->i("Server", m_strServer);
  ptReq->i("Port", m_strPort);
  in.push_back(ptReq);
  ptReq = new Json(ptRequest);
  ptReq->i("Service", strService);
  ptReq->i("Function", strFunction);
  in.push_back(ptReq);
  if (junction(in, out, strError))
  {
    bResult = true;
    if (out.size() == 3)
    {
      ptResponse->merge(out.back(), true, false);
    }
  }
  for (auto &i : in)
  {
    delete i;
  }
  for (auto &i : out)
  {
    delete i;
  }

  return bResult;
}
// }}}
// {{{ schedule()
void MythTv::schedule(string strPrefix)
{
  list<string> removals;
  time_t CTime[2];

  threadIncrement();
  strPrefix += "->MythTv::schedule()";
  time(&(CTime[0]));
  while (!shutdown())
  {
    time(&(CTime[1]));
    if ((CTime[1] - CTime[0]) > 300)
    {
      CTime[0] = CTime[1];
    }
    msleep(1000);
  }
  threadDecrement();
}
// }}}
}
}
