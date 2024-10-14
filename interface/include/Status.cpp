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
Status::Status(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), void (*pCallbackInotify)(string, const string, const string)) : Interface(strPrefix, "status", argc, argv, pCallback)
{
  map<string, list<string> > watches;
  string strError;
  Json *ptConfiguration = new Json;

  m_functions["action"] = &Status::action;
  m_functions["status"] = &Status::status;
  if (!storageRetrieve({"radial", "nodes", m_strNode, "interfaces", m_strName, "configuration"}, ptConfiguration, strError))
  {
    store(strPrefix);
  }
  delete ptConfiguration;
  watches[m_strData] = {"interfaces.json"};
  m_pThreadInotify = new thread(&Status::inotify, this, strPrefix, watches, pCallbackInotify);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
  m_pThreadSchedule = new thread(&Status::schedule, this, strPrefix);
  pthread_setname_np(m_pThreadSchedule->native_handle(), "schedule");
}
// }}}
// {{{ ~Status()
Status::~Status()
{
  m_pThreadSchedule->join();
  delete m_pThreadSchedule;
}
// }}}
// {{{ autoMode()
void Status::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->Status::autoMode()";
  if (strOldMaster != strNewMaster)
  {
    stringstream ssMessage;
    ssMessage << strPrefix << " [" << strNewMaster << "]:  Updated master.";
    log(ssMessage.str());
  }
  threadDecrement();
}
// }}}
// {{{ callback()
void Status::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;
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
// {{{ callbackInotify()
void Status::callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Status::callbackInotify()";
  if (strPath == m_strData && strFile == "interfaces.json")
  {
    store(strPrefix);
  }
}
// }}}
// {{{ status()
bool Status::status(Json *o, string &e)
{
  bool b = false;
  Json *ptNodes = new Json;

  if (storageRetrieve({"radial", "nodes"}, ptNodes, e))
  {
    b = true;
    if (!ptNodes->m.empty())
    {
      o->m["Nodes"] = new Json;
      for (auto &n : ptNodes->m)
      {
        if (exist(n.second, "interfaces") && !n.second->m["interfaces"]->m.empty())
        {
          o->m["Nodes"]->m[n.first] = new Json;
          for (auto &i : n.second->m["interfaces"]->m)
          {
            if (exist(i.second, "configuration"))
            {
              o->m["Nodes"]->m[n.first]->m[i.first] = new Json(i.second->m["configuration"]);
              if (n.first == m_strNode && i.first == "status")
              {
                float fCpu = 0, fMem = 0;
                pid_t nPid = getpid();
                stringstream ssImage, ssPid, ssResident;
                time_t CTime = 0;
                unsigned long ulImage = 0, ulResident = 0;
                m_pCentral->getProcessStatus(nPid, CTime, fCpu, fMem, ulImage, ulResident);
                ssPid << nPid;
                o->m["Nodes"]->m[n.first]->m[i.first]->i("PID", ssPid.str(), 'n');
                o->m["Nodes"]->m[n.first]->m[i.first]->m["Memory"] = new Json;
                ssImage << ulImage;
                o->m["Nodes"]->m[n.first]->m[i.first]->m["Memory"]->i("Image", ssImage.str(), 'n');
                ssResident << ulResident;
                o->m["Nodes"]->m[n.first]->m[i.first]->m["Memory"]->i("Resident", ssResident.str(), 'n');
                if (!m_strMaster.empty())
                {
                  o->m["Nodes"]->m[n.first]->m[i.first]->m["Master"] = new Json;
                  o->m["Nodes"]->m[n.first]->m[i.first]->m["Master"]->i("Node", m_strMaster);
                  o->m["Nodes"]->m[n.first]->m[i.first]->m["Master"]->i("Settled", ((m_bMasterSettled)?"1":"0"), ((m_bMasterSettled)?'1':'0'));
                }
                m_mutexBase.lock();
                if (m_unThreads > 0)
                {
                  stringstream ssThreads;
                  ssThreads << m_unThreads;
                  o->m["Nodes"]->m[n.first]->m[i.first]->i("Threads", ssThreads.str(), 'n');
                }
                m_mutexBase.unlock();
              }
              else
              {
                bool bFoundInterface = false;
                m_mutexShare.lock();
                if (n.first == m_strNode)
                {
                  if (m_i.find(i.first) != m_i.end())
                  {
                    bFoundInterface = true;
                  }
                }
                else
                {
                  bool bFoundNode = false;
                  for (auto l = m_l.begin(); !bFoundNode && l != m_l.end(); l++)
                  {
                    if ((*l)->strNode == n.first)
                    {
                      bFoundNode = true;
                      if ((*l)->interfaces.find(i.first) != (*l)->interfaces.end())
                      {
                        bFoundInterface = true;
                      }
                    }
                  }
                }
                m_mutexShare.unlock();
                if (bFoundInterface)
                {
                  Json *ptStat = new Json;
                  ptStat->i("|function", "status");
                  if (n.first != m_strNode)
                  {
                    ptStat->i("Interface", i.first);
                    ptStat->i("Node", n.first);
                  }
                  if (((n.first == m_strNode && hub(i.first, ptStat, e)) || (n.first != m_strNode && hub("link", ptStat, e))) && exist(ptStat, "Response"))
                  {
                    o->m["Nodes"]->m[n.first]->m[i.first]->merge(ptStat->m["Response"], true, false);
                  }
                  delete ptStat;
                }
              }
              if (exist(i.second, "throughput"))
              {
                o->m["Nodes"]->m[n.first]->m[i.first]->i("Throughput", i.second->m["throughput"]);
              }
            }
          }
        }
      }
    }
  }
  delete ptNodes;

  return b;
}
bool Status::status(radialUser &d, string &e)
{
  return status(d.p->m["o"], e);
}
// }}}
// {{{ store()
void Status::store(string strPrefix)
{
  ifstream inInterfaces;
  string strError;
  stringstream ssInterfaces, ssMessage;
  Json *ptInterfaces = NULL;

  strPrefix += "->Status::store()";
  ssInterfaces << m_strData << "/interfaces.json";
  inInterfaces.open(ssInterfaces.str());
  if (inInterfaces)
  {
    string strLine;
    stringstream ssJson;
    while (getline(inInterfaces, strLine))
    {
      ssJson << strLine;
    }
    ptInterfaces = new Json(ssJson.str());
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->ifstream::open(" << errno << ") error [" << ssInterfaces.str() << "]:  " << strerror(errno);
    log(ssMessage.str());
  }
  inInterfaces.close();
  if (ptInterfaces != NULL)
  {
    for (auto &i : ptInterfaces->m)
    {
      if (!storageAdd({"radial", "nodes", m_strNode, "interfaces", i.first, "configuration"}, i.second, strError))
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->Interface::storageAdd() error [radial,nodes," << m_strNode << ",interfaces," << i.first << ",configuration]:  " << strError;
        log(ssMessage.str());
      }
    }
    delete ptInterfaces;
  }
}
// }}}
// {{{ schedule()
void Status::schedule(string strPrefix)
{
  size_t unCount = 0;
  string strError;
  stringstream ssInterfaces, ssMessage;
  time_t CTime[2];

  threadIncrement();
  strPrefix += "->Status::schedule()";
  ssInterfaces << m_strData << "/interfaces.json";
  time(&(CTime[0]));
  while (!shutdown())
  {
    time(&(CTime[1]));
    if ((CTime[1] - CTime[0]) >= 60)
    {
      Json *ptConfiguration = new Json;
      CTime[0] = CTime[1];
      if (!storageRetrieve({"radial", "nodes", m_strNode, "interfaces", m_strName, "configuration"}, ptConfiguration, strError))
      {
        store(strPrefix);
      }
      delete ptConfiguration;
      if (isMasterSettled() && isMaster())
      {
        Json *ptMessage = new Json;
        status(ptMessage, strError);
        ptMessage->i("Action", "status");
        live("Central", "", ptMessage);
        delete ptMessage;
        if (unCount++ > 480)
        {
          unCount = 0;
          storageRemove({"radial"}, strError);
        }
      }
    }
    msleep(250);
  }
  threadDecrement();
}
// }}}
}
}
