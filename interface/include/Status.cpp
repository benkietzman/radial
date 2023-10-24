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
  if (!empty(ptJson, "Function"))
  {
    string strFunction = ptJson->m["Function"]->v;
    if (strFunction == "restart" || strFunction == "start" || strFunction == "stop")
    {
      map<string, string> getApplicationRow;
      Json *ptApplication = new Json;
      radialUser d;
      userInit(ptJson, d);
      ptApplication->i("name", "Radial");
      if (d.g || db("dbCentralApplications", ptApplication, getApplicationRow, strError))
      {
        radialUser a;
        userInit(d, a);
        a.p->m["i"]->i("id", getApplicationRow["id"]);
        if (d.g || isApplicationDeveloper(a, strError))
        {
          if (exist(ptJson, "Request"))
          {
            if (!empty(ptJson->m["Request"], "Interface"))
            {
              list<string> nodes;
              string strInterface = ptJson->m["Request"]->m["Interface"]->v, strNode;
              if (!empty(ptJson->m["Request"], "Node"))
              {
                strNode = ptJson->m["Request"]->m["Node"]->v;
              }
              if (!strNode.empty())
              {
                nodes.push_back(strNode);
              }
              else
              {
                m_mutexShare.lock();
                for (auto &link : m_l)
                {
                  nodes.push_back(link->strNode);
                }
                m_mutexShare.unlock();
                nodes.push_back(m_strNode);
              }
              if (!nodes.empty())
              {
                bResult = true;
                for (auto &node : nodes)
                {
                  if (strInterface == "status" && node == m_strNode && (strFunction == "restart" || strFunction == "stop"))
                  {
                    setShutdown();
                  }
                  else if (strFunction == "start" || interfaceRemove(node, strInterface, strError) || strError == "Encountered an unknown error." || strError == "Interface not found.")
                  {
                    bool bStopped = false;
                    time_t CTime[2];
                    time(&(CTime[0]));
                    CTime[1] = CTime[0];
                    while (!bStopped && (CTime[1] - CTime[0]) < 40)
                    {
                      m_mutexShare.lock();
                      if (node == m_strNode)
                      {
                        if (m_i.find(strInterface) == m_i.end())
                        {
                          bStopped = true;
                        }
                      }
                      else
                      {
                        auto linkIter = m_l.end();
                        for (auto i = m_l.begin(); linkIter == m_l.end() && i != m_l.end(); i++)
                        {
                          if ((*i)->strNode == node)
                          {
                            linkIter = i;
                          }
                        }
                        if (linkIter != m_l.end())
                        {
                          if ((*linkIter)->interfaces.find(strInterface) == (*linkIter)->interfaces.end())
                          {
                            bStopped = true;
                          }
                        }
                        else
                        {
                          bStopped = true;
                        }
                      }
                      m_mutexShare.unlock();
                      if (strFunction == "start")
                      {
                        CTime[1] += 40;
                      }
                      else
                      {
                        msleep(250);
                        time(&(CTime[1]));
                      }
                    }
                    if (bStopped)
                    {
                      if (strFunction == "stop")
                      {
                        interfaceAdd(node, strInterface, strError);
                      }
                    }
                    else if (strFunction == "start")
                    {
                      strError = "Already started.";
                    }
                    else
                    {
                      strError = "Failed to stop.";
                    }
                  }
                }
              }
              else
              {
                strError = "Interface does not exist.";
              }
            }
            else
            {
              strError = "Please provide the Interface.";
            }
          }
          else
          {
            strError = "Please provide the Request.";
          }
        }
        else
        {
          strError = "You are not authorized to perform this action.";
        }
        userDeinit(a);
      }
      delete ptApplication;
      userDeinit(d);
    }
    else if (ptJson->m["Function"]->v == "status")
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
      strError = "Please provide a valid Function:  restart, start, status, stop.";
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
  string strError;

  ptStatus->m["Nodes"] = new Json;
  ptStatus->m["Nodes"]->m[m_strNode] = new Json;
  for (auto &i : m_i)
  {
    stringstream ssPid;
    Json *ptStat = new Json;
    ssPid << i.second->nPid;
    ptStat->i("|function", "status");
    if (hub(i.first, ptStat, strError) && exist(ptStat, "Response"))
    {
      ptStatus->m["Nodes"]->m[m_strNode]->m[i.first] = new Json(ptStat->m["Response"]);
    }
    else
    {
      ptStatus->m["Nodes"]->m[m_strNode]->m[i.first] = new Json;
    }
    delete ptStat;
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
        Json *ptStat = new Json;
        ssPid << i.second->nPid;
        ptStat->i("Interface", i.first);
        ptStat->i("Node", l->strNode);
        ptStat->i("|function", "status");
        if (hub("link", ptStat, strError) && exist(ptStat, "Response"))
        {
          ptStatus->m["Nodes"]->m[l->strNode]->m[i.first] = new Json(ptStat->m["Response"]);
        }
        else
        {
          ptStatus->m["Nodes"]->m[l->strNode]->m[i.first] = new Json;
        }
        delete ptStat;
        ptStatus->m["Nodes"]->m[l->strNode]->m[i.first]->i("PID", ssPid.str(), 'n');
        ptStatus->m["Nodes"]->m[l->strNode]->m[i.first]->i("Respawn", ((i.second->bRespawn)?"1":"0"), ((i.second->bRespawn)?1:0));
        ptStatus->m["Nodes"]->m[l->strNode]->m[i.first]->i("Restricted", ((i.second->bRestricted)?"1":"0"), ((i.second->bRestricted)?1:0));
        ptStatus->m["Nodes"]->m[l->strNode]->m[i.first]->i("AccessFunction", i.second->strAccessFunction);
        ptStatus->m["Nodes"]->m[l->strNode]->m[i.first]->i("Command", i.second->strCommand);
      }
    }
  }
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
    if ((CTime[1] - CTime[0]) >= 60)
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
