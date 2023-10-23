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
      if (exist(ptJson, "Response"))
      {
        delete ptJson->m["Response"];
      }
      ptJson->m["Response"] = new Json;
      status(ptJson->m["Response"]);
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
// {{{ status()
void Status::status(Json *ptStatus)
{
  ptStatus->m["Nodes"] = new Json;
  ptStatus->m["Nodes"]->m[m_strNode] = new Json;
  m_mutexShare.lock();
  for (auto &i : m_i)
  {
    stringstream ssPid;
    ssPid << i.second->nPid;
    ptStatus->m["Nodes"]->m[m_strNode]->m[i.first] = new Json;
    ptStatus->m["Nodes"]->m[m_strNode]->m[i.first]->i("PID", ssPid.str(), 'n');
    ptStatus->m["Nodes"]->m[m_strNode]->m[i.first]->i("Respawn", ((i.second->bRespawn)?"1":"0"), ((i.second->bRespawn)?1:0));
    ptStatus->m["Nodes"]->m[m_strNode]->m[i.first]->i("Restricted", ((i.second->bRestricted)?"1":"0"), ((i.second->bRestricted)?1:0));
    ptStatus->m["Nodes"]->m[m_strNode]->m[i.first]->i("AccessFunction", i.second->strAccessFunction);
    ptStatus->m["Nodes"]->m[m_strNode]->m[i.first]->i("Command", i.second->strCommand);
  }
  for (auto &l : m_l)
  {
    if (!l->strNode.empty())
    {
      ptStatus->m["Nodes"]->m[l->strNode] = new Json;
      for (auto &i : l->interfaces)
      {
        stringstream ssPid;
        ssPid << i.second->nPid;
        ptStatus->m["Nodes"]->m[l->strNode]->m[i.first] = new Json;
        ptStatus->m["Nodes"]->m[l->strNode]->m[i.first]->i("PID", ssPid.str(), 'n');
        ptStatus->m["Nodes"]->m[l->strNode]->m[i.first]->i("Respawn", ((i.second->bRespawn)?"1":"0"), ((i.second->bRespawn)?1:0));
        ptStatus->m["Nodes"]->m[l->strNode]->m[i.first]->i("Restricted", ((i.second->bRestricted)?"1":"0"), ((i.second->bRestricted)?1:0));
        ptStatus->m["Nodes"]->m[l->strNode]->m[i.first]->i("AccessFunction", i.second->strAccessFunction);
        ptStatus->m["Nodes"]->m[l->strNode]->m[i.first]->i("Command", i.second->strCommand);
      }
    }
  }
  m_mutexShare.unlock();
}
// }}}
// {{{ schedule()
void Status::schedule(string strPrefix)
{
  string strError;
  time_t CTime[2];

  threadIncrement();
  strPrefix += "->Status::schedule()";
  time(&(CTime[0]));
  while (!shutdown())
  {
    time(&(CTime[1]));
    if ((CTime[1] - CTime[0]) >= 5)
    {
      Json *ptMessage = new Json;
      CTime[0] = CTime[1];
      status(ptMessage);
      ptMessage->i("Action", "status");
      live("Radial", "", ptMessage, strError);
      delete ptMessage;
    }
    msleep(250);
  }
  threadDecrement();
}
// }}}
}
}
