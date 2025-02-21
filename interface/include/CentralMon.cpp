// -*- C++ -*-
// Radial
// -------------------------------------
// file       : CentralMon.cpp
// author     : Ben Kietzman
// begin      : 2023-03-03
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "CentralMon"
// }}}
extern "C++"
{
namespace radial
{
// {{{ CentralMon()
CentralMon::CentralMon(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "centralmon", argc, argv, pCallback)
{
  m_bUpdate = true;
  m_pThreadSchedule = new thread(&Db::schedule, this, strPrefix);
  pthread_setname_np(m_pThreadSchedule->native_handle(), "schedule");
}
// }}}
// {{{ ~CentralMon()
CentralMon::~CentralMon()
{
  m_pThreadSchedule->join();
  delete m_pThreadSchedule;
}
// }}}
// {{{ autoMode()
void CentralMon::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->CentralMon::autoMode()";
  if (strOldMaster != strNewMaster)
  {
    stringstream ssMessage;
    ssMessage << strPrefix << " [" << strNewMaster << "]:  Updated master.";
    log(ssMessage.str());
    m_mutex.lock();
    m_bUpdate = true;
    m_mutex.unlock();
  }
  threadDecrement();
}
// }}}
// {{{ callback()
void CentralMon::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->CentralMon::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    string strFunction = ptJson->m["Function"]->v;
    // {{{ data
    if (strFunction == "data")
    {
      if (!empty(ptJson, "Server"))
      {
        if (exist(ptJson, "System"))
        {
          stringstream ssTime;
          time_t CTime;
          Json *ptData = new Json;
          time(&CTime);
          ssTime << CTime;
          ptData->i("_time", ssTime.str());
          ptData->m["system"] = new Json(ptJson->m["System"]);
          if (exist(ptJson, "Processes"))
          {
            ptData->m["processes"] = new Json(ptJson->m["Processes"]);
          }
          if (storageAdd({"centralmon", "servers", ptJson->m["Server"]->v, "data"}, ptData, strError))
          {
            bResult = true;
          }
          delete ptData;
        }
        else
        {
          strError = "Please provide the System.";
        }
      }
      else
      {
        strError = "Please provide the Server.";
      }
    }
    // }}}
    // {{{ process
    if (strFunction == "process")
    {
      if (!empty(ptJson, "Server"))
      {
        if (!empty(ptJson, "Process"))
        {
        }
        else
        {
          strError = "Please provide the Process.";
        }
      }
      else
      {
        strError = "Please provide the Server.";
      }
    }
    // }}}
    // {{{ system
    else if (strFunction == "system")
    {
      if (!empty(ptJson, "Server"))
      {
      }
      else
      {
        strError = "Please provide the Server.";
      }
    }
    // }}}
    // {{{ update
    else if (strFunction == "update")
    {
      bResult = true;
      
    }
    // }}}
    // {{{ invalid
    else
    {
      strError = "Please provide a valid Function:  process, system, update.";
    }
    // }}}
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
// {{{ schedule()
void CentralMon::schedule(string strPrefix)
{
  string strError;
  stringstream ssMessage;
  time_t CAnalyze = 0, CNow;

  threadIncrement();
  strPrefix += "->CentralMon::schedule()";
  while (!shutdown())
  {
    time(&CNow);
    if ((CNow - CAnalyze) > 10)
    {
      if (isMasterSettled() && isMaster())
      {
        bool bUpdate;
        Json *ptJson;
        m_mutex.lock();
        bUpdate = m_bUpdate;
        m_mutex.unlock();
        if (bUpdate)
        {
          stringstream ssQuery;
          ssQuery.str("");
          ssQuery << "select c.id, c.name, c.cpu_usage, c.disk_size, c.main_memory, c.processes, c.swap_memory from application a, application_server b, `server` c where a.id = b.application_id and b.server_id = c.id and (a.name = 'Central Monitor' or a.name = 'System Information') order by c.name;";
          auto getServer = dbquery("central_r", ssQuery.str(), strError);
          if (getServer != NULL)
          {
            map<string, map<string, string> > servers;
            m_mutex.lock();
            m_bUpdate = false;
            m_mutex.unlock();
            for (auto &getServerRow : *getServer)
            {
              servers[getServerRow["name"]] = getServerRow;
            }
            ptJson = new Json;
            if (storageRetrieve({"centralmon", "servers"}, ptJson, strError))
            {
              for (auto &server : ptJson->m)
              {
                if (servers.find(server.first) == servers.end())
                {
                  storageRemove({"centralmon", "servers", server.first}, strError);
                }
              }
              for (auto &server : servers)
              {
                Json *ptConf = new Json;
                ptConf->m["system"] = new Json;
                ptConf->m["system"]->i("cpuUsage", server.second["cpu_usage"]);
                ptConf->m["system"]->i("diskSize", server.second["disk_size"]);
                ptConf->m["system"]->i("mainMemory", server.second["main_memory"]);
                ptConf->m["system"]->i("processes", server.second["processes"]);
                ptConf->m["system"]->i("swapMemory", server.second["swap_memory"]);
                ssQuery.str("");
                ssQuery << "select b.application_id, a.name, c.daemon, c.owner, c.delay, c.min_processes, c.max_processes, c.min_image, c.max_image, c.min_resident, c.max_resident from application a, application_server b, application_server_detail c where a.id = b.application_id and b.id = c.application_server_id and b.server_id = " << server.second;
                auto getDetail = dbQuery("central_r", ssQuery.str(), strError);
                if (getDetail != NULL)
                {
                  ptConf->m["processes"] = new Json;
                  for (auto &getDetailRow : *getDetail)
                  {
                    // TODO
                  }
                }
                else
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Interface::dbquery(central_r," << ssQuery.str() << ") error [" << server.first << "]:  " << strError;
                  log(ssMessage.str());
                }
                dbfree(getDetail);
                if (!storageAdd({"centralmon", "servers", server.first, "conf"}, ptConf, strError))
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Interface::storageAdd() error [centralmon,servers," << server.first << ",conf]:  " << strError;
                  log(ssMessage.str());
                }
                delete ptConf;
              }
            }
            delete ptJson;
          }
          else
          {
            ssMessage.str("");
            ssMessage << strPrefix << "->Interface::dbquery(central_r," << ssQuery.str() << ") error:  " << strError;
            log(ssMessage.str());
          }
          dbfree(getServer);
        }
        ptJson = new Json;
        if (storageRetrieve({"centralmon", "servers"}, ptJson, strError))
        {
          for (auto &server : ptJson->m)
          {
            if (exist(server.second, "conf"))
            {
              Json *ptConf = server.second->m["conf"];
              if (!empty(ptConf, "_time"))
              {
                time_t CConf = atoi(ptConf->m["_time"]->v.c_str());
                if ((CNow - CConf) < 14400)
                {
                  stringstream ssAlarms;
                  if (exist(server.second, "data"))
                  {
                    Json *ptData = server.second->m["data"];
                    if (!empty(ptData, "_time"))
                    {
                      time_t CData = atoi(ptData->m["_time"]->v.c_str());
                      if (CData >= CAnalyze)
                      {
                        if (exist(ptData, "system"))
                        {
                          Json *ptSystem = ptData->m["system"];
                          if (exist(ptData, "processes"))
                          {
                            for (auto &ptProcess : ptData->m["processes"]->m)
                            {
                            }
                          }
                        }
                        else
                        {
                          // Missing system data.
                        }
                      }
                    }
                  }
                  else
                  {
                    // Server might be offline.
                  }
                  if (!ssAlarms.str().empty())
                  {
                    if (empty(server.second, "alarms") || server.second->m["alarms"]->v != ssAlarms.str())
                    {
                    }
                  }
                }
                else
                {
                  storageRemove({"centralmon", "servers", server.first}, strError);
                }
              }
              else
              {
                storageRemove({"centralmon", "servers", server.first}, strError);
              }
            }
            else
            {
              storageRemove({"centralmon", "servers", server.first}, strError);
            }
          }
        }
        delete ptJson;
      }
      CAnalyze = CNow;
    }
    msleep(1000);
  }
  threadDecrement();
}
// }}}
}
}
