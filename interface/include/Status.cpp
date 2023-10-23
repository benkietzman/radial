// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Status.cpp
// author     : Ben Kietzman
// begin      : 2023-10-20
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Status"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Status()
Status::Status(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "status", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Status()
Status::~Status()
{
}
// }}}
// {{{ callback()
void Status::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Status::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (exist(ptJson, "Function"))
  {
    if (ptJson->m["Function"]->v == "status")
    {
      bResult = true;
      pushStatus();
    }
    else
    {
      strError = "Please provide a valid Function:  status";
    }
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
// {{{ pushStatus()
void Status::pushStatus()
{
  string strError;
  Json *ptMessage = new Json;

  ptMessage->i("Action", "status");
  ptMessage->m["Nodes"] = new Json;
  ptMessage->m["Nodes"]->m[m_strNode] = new Json;
  m_mutexShare.lock();
  for (auto &i : m_i)
  {
    stringstream ssPid;
    ssPid << i.second->nPid;
    ptMessage->m["Nodes"]->m[m_strNode]->m[i.first] = new Json;
    ptMessage->m["Nodes"]->m[m_strNode]->m[i.first]->i("PID", ssPid.str(), 'n');
    ptMessage->m["Nodes"]->m[m_strNode]->m[i.first]->i("Respawn", ((i.second->bRespawn)?"1":"0"), ((i.second->bRespawn)?1:0));
    ptMessage->m["Nodes"]->m[m_strNode]->m[i.first]->i("Restricted", ((i.second->bRestricted)?"1":"0"), ((i.second->bRestricted)?1:0));
    ptMessage->m["Nodes"]->m[m_strNode]->m[i.first]->i("AccessFunction", i.second->strAccessFunction);
    ptMessage->m["Nodes"]->m[m_strNode]->m[i.first]->i("Command", i.second->strCommand);
  }
  for (auto &l : m_l)
  {
    if (!l->strNode.empty())
    {
      ptMessage->m["Nodes"]->m[l->strNode] = new Json;
      for (auto &i : l->interfaces)
      {
        stringstream ssPid;
        ssPid << i.second->nPid;
        ptMessage->m["Nodes"]->m[l->strNode]->m[i.first] = new Json;
        ptMessage->m["Nodes"]->m[l->strNode]->m[i.first]->i("PID", ssPid.str(), 'n');
        ptMessage->m["Nodes"]->m[l->strNode]->m[i.first]->i("Respawn", ((i.second->bRespawn)?"1":"0"), ((i.second->bRespawn)?1:0));
        ptMessage->m["Nodes"]->m[l->strNode]->m[i.first]->i("Restricted", ((i.second->bRestricted)?"1":"0"), ((i.second->bRestricted)?1:0));
        ptMessage->m["Nodes"]->m[l->strNode]->m[i.first]->i("AccessFunction", i.second->strAccessFunction);
        ptMessage->m["Nodes"]->m[l->strNode]->m[i.first]->i("Command", i.second->strCommand);
      }
    }
  }
  m_mutexShare.unlock();
  live("Radial", "", ptMessage, strError);
  delete ptMessage;
}
// }}}
// {{{ schedule()
void Status::schedule(string strPrefix)
{
  time_t CTime[2];

  threadIncrement();
  strPrefix += "->Status::schedule()";
  time(&(CTime[0]));
  while (!shutdown())
  {
    time(&(CTime[1]));
    if ((CTime[1] - CTime[0]) >= 5)
    {
      CTime[0] = CTime[1];
      pushStatus();
    }
    msleep(250);
  }
  threadDecrement();
}
// }}}
}
}
