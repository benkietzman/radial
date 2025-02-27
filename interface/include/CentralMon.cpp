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
  m_pThreadSchedule = new thread(&CentralMon::schedule, this, strPrefix);
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
    // {{{ config
    if (strFunction == "config")
    {
      if (exist(ptJson, "Request"))
      {
        if (!empty(ptJson->m["Request"], "Server"))
        {
          if (exist(ptJson, "Response"))
          {
            delete ptJson->m["Response"];
          }
          ptJson->m["Response"] = new Json;
          if (storageRetrieve({"centralmon", "servers", ptJson->m["Request"]->m["Server"]->v, "config"}, ptJson->m["Response"], strError))
          {
            bResult = true;
          }
        }
        else
        {
          strError = "Please provide the Server within the Request.";
        }
      }
      else
      {
        strError = "Please provide the Request.";
      }
    }
    // }}}
    // {{{ data
    else if (strFunction == "data")
    {
      if (exist(ptJson, "Request"))
      {
        if (!empty(ptJson->m["Request"], "Server"))
        {
          if (exist(ptJson->m["Request"], "System"))
          {
            stringstream ssTime;
            time_t CTime;
            Json *ptData = new Json;
            time(&CTime);
            ssTime << CTime;
            ptData->i("_time", ssTime.str());
            ptData->m["system"] = new Json(ptJson->m["Request"]->m["System"]);
            if (exist(ptJson->m["Request"], "Processes"))
            {
              ptData->m["processes"] = new Json(ptJson->m["Request"]->m["Processes"]);
            }
            if (storageAdd({"centralmon", "servers", ptJson->m["Request"]->m["Server"]->v, "data"}, ptData, strError))
            {
              bResult = true;
            }
            delete ptData;
          }
          else
          {
            strError = "Please provide the System within the Request.";
          }
        }
        else
        {
          strError = "Please provide the Server within the Request.";
        }
      }
      else
      {
        strError = "Please provide the Request.";
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
      strError = "Please provide a valid Function:  config, data, process, system, update.";
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
  string strError, strValue;
  stringstream ssMessage, ssQuery;
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
        // {{{ update
        if (bUpdate)
        {
          ssQuery.str("");
          ssQuery << "select c.id, c.name, c.cpu_usage, c.disk_size, c.main_memory, c.processes, c.swap_memory from application a, application_server b, `server` c where a.id = b.application_id and b.server_id = c.id and (a.name = 'Central Monitor' or a.name = 'System Information') order by c.name";
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
                Json *ptConfig = new Json;
                ptConfig->m["system"] = new Json;
                ptConfig->m["system"]->i("cpuUsage", server.second["cpu_usage"]);
                ptConfig->m["system"]->i("diskSize", server.second["disk_size"]);
                ptConfig->m["system"]->i("mainMemory", server.second["main_memory"]);
                ptConfig->m["system"]->i("processes", server.second["processes"]);
                ptConfig->m["system"]->i("swapMemory", server.second["swap_memory"]);
                ssQuery.str("");
                ssQuery << "select a.id application_id, a.name, b.id application_server_id, b.server_id, c.id application_server_detail_id, c.daemon, c.owner, c.delay, c.min_processes, c.max_processes, c.min_image, c.max_image, c.min_resident, c.max_resident from application a, application_server b, application_server_detail c where a.id = b.application_id and b.id = c.application_server_id and b.server_id = " << server.second["id"] << " and c.daemon is not null and c.daemon != ''";
                auto getDetail = dbquery("central_r", ssQuery.str(), strError);
                if (getDetail != NULL)
                {
                  ptConfig->m["processes"] = new Json;
                  for (auto &getDetailRow : *getDetail)
                  {
                    if (ptConfig->m["processes"]->m.find(getDetailRow["daemon"]) == ptConfig->m["processes"]->m.end())
                    {
                      Json *ptProcess = new Json;
                      ptProcess->i("applicationId", getDetailRow["application_id"]);
                      ptProcess->i("applicationServerDetailId", getDetailRow["application_server_detail_id"]);
                      ptProcess->i("applicationServerId", getDetailRow["application_server_id"]);
                      ptProcess->i("delay", getDetailRow["delay"]);
                      ptProcess->i("minProcesses", getDetailRow["min_processes"]);
                      ptProcess->i("maxProcesses", getDetailRow["max_processes"]);
                      ptProcess->i("minImage", getDetailRow["min_image"]);
                      ptProcess->i("maxImage", getDetailRow["max_image"]);
                      ptProcess->i("minResident", getDetailRow["min_resident"]);
                      ptProcess->i("maxResident", getDetailRow["max_resident"]);
                      ptProcess->i("owner", getDetailRow["owner"]);
                      ptProcess->i("script", getDetailRow["script"]);
                      ptProcess->i("serverId", getDetailRow["server_id"]);
                      ptConfig->m["processes"]->m[getDetailRow["daemon"]] = ptProcess;
                    }
                  }
                }
                else
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Interface::dbquery(central_r," << ssQuery.str() << ") error [" << server.first << "]:  " << strError;
                  log(ssMessage.str());
                }
                dbfree(getDetail);
                if (!storageAdd({"centralmon", "servers", server.first, "config"}, ptConfig, strError))
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Interface::storageAdd() error [centralmon,servers," << server.first << ",conf]:  " << strError;
                  log(ssMessage.str());
                }
                delete ptConfig;
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
        // }}}
        // {{{ analyze
        ptJson = new Json;
        if (storageRetrieve({"centralmon", "servers"}, ptJson, strError))
        {
          time(&CNow);
          for (auto &server : ptJson->m)
          {
            stringstream ssAlarmsSystem;
            Json *ptAlarms = server.second->m["alarms"];
            if (!exist(server.second, "alarms"))
            {
              server.second->m["alarms"] = new Json;
            }
            ptAlarms = server.second->m["alarms"];
            if (!exist(ptAlarms, "system"))
            {
              ptAlarms->m["system"] = new Json;
            }
            if (!exist(ptAlarms, "processes"))
            {
              ptAlarms->m["processes"] = new Json;
            }
            if (exist(server.second, "config"))
            {
              Json *ptConfig = server.second->m["config"];
              if (exist(ptConfig, "system"))
              {
                Json *ptConfigSystem = ptConfig->m["system"];
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
                        Json *ptDataSystem = ptData->m["system"];
                        if (!empty(ptConfigSystem, "maxProcesses") && atoi(ptConfigSystem->m["maxProcesses"]->v.c_str()) > 0 && !empty(ptDataSystem, "processes") && atoi(ptDataSystem->m["processes"]->v.c_str()) > atoi(ptConfigSystem->m["maxProcesses"]->v.c_str()))
                        {
                          ssAlarmsSystem << ((!ssAlarmsSystem.str().empty())?"  ":"") << ptDataSystem->m["processes"]->v << " processes are running which is more than the maximum " << ptConfigSystem->m["maxProcesses"]->v << " processes.";
                        }
                        if (!empty(ptConfigSystem, "maxCpuUsage") && atoi(ptConfigSystem->m["maxCpuUsage"]->v.c_str()) > 0 && !empty(ptDataSystem, "cpuUsage") && atoi(ptDataSystem->m["cpuUsage"]->v.c_str()) > atoi(ptConfigSystem->m["maxCpuUsage"]->v.c_str()))
                        {
                          ssAlarmsSystem << ((!ssAlarmsSystem.str().empty())?"  ":"") << "Using " << ptDataSystem->m["cpuUsage"]->v << "% CPU which is more than the maximum " << ptConfigSystem->m["maxCpuUsage"]->v << "%";
                          if (!empty(ptDataSystem, "cpuProcessUsage"))
                          {
                            ssAlarmsSystem << " (" << ptDataSystem->m["cpuProcessUsage"]->v << ")";
                          }
                          ssAlarmsSystem << ".";
                        }
                        if (!empty(ptConfigSystem, "maxMainUsage") && atoi(ptConfigSystem->m["maxMainUsage"]->v.c_str()) > 0 && !empty(ptDataSystem, "mainTotal") && !empty(ptDataSystem, "mainUsed") && (atoi(ptDataSystem->m["mainUsed"]->v.c_str()) * 100 / atoi(ptDataSystem->m["mainTotal"]->v.c_str())) > atoi(ptConfigSystem->m["maxMainUsage"]->v.c_str()))
                        {
                          ssAlarmsSystem << ((!ssAlarmsSystem.str().empty())?"  ":"") << "Using " << (atoi(ptDataSystem->m["mainUsed"]->v.c_str()) * 100 / atoi(ptDataSystem->m["mainTotal"]->v.c_str())) << "% main memory which is more than the maximum " << ptConfigSystem->m["maxMainUsage"]->v << "%.";
                        }
                        if (exist(ptData, "processes"))
                        {
                          for (auto &process : ptData->m["processes"]->m)
                          {
                            Json *ptDataProcess = process.second;
                            if (exist(ptConfig, "processes") && exist(ptConfig->m["processes"], process.first))
                            {
                              stringstream ssAlarmsProcess;
                              Json *ptConfigProcess = ptConfig->m["processes"]->m[process.first];
                              if (!exist(ptAlarms->m["processes"], process.first))
                              {
                                ptAlarms->m["processes"]->i(process.first, "");
                              }
                              if (!empty(ptDataProcess, "processes") && atoi(ptDataProcess->m["processes"]->v.c_str()) <= 0)
                              {
                                if (empty(ptConfigProcess, "delay") || atoi(ptConfigProcess->m["delay"]->v.c_str()) <= 0 || empty(ptDataProcess, "time") || atoi(ptDataProcess->m["time"]->v.c_str()) <= 0 || (CNow > atoi(ptDataProcess->m["time"]->v.c_str()) && (CNow - atoi(ptDataProcess->m["time"]->v.c_str())) >= atoi(ptConfigProcess->m["delay"]->v.c_str())))
                                {
                                  ssAlarmsProcess << ((!ssAlarmsProcess.str().empty())?"  ":"") << process.first << " is not currently running.";
                                }
                              }
                              else
                              {
                                if (!empty(ptConfigProcess, "owner"))
                                {
                                  bool bFound = false;
                                  if (exist(ptDataProcess, "owners"))
                                  {
                                    for (auto ownerIter = ptDataProcess->m["owners"]->m.begin(); !bFound && ownerIter != ptDataProcess->m["owners"]->m.end(); ownerIter++)
                                    {
                                      if (ptConfigProcess->m["owner"]->v == ownerIter->second->v)
                                      {
                                        bFound = true;
                                      }
                                    }
                                  }
                                  if (!bFound)
                                  {
                                    ssAlarmsProcess << ((!ssAlarmsProcess.str().empty())?"  ":"") << process.first << " is not running under the required " << ptConfigProcess->m["owner"]->v << " account.";
                                  }
                                }
                                if (!empty(ptDataProcess, "processes"))
                                {
                                  if (!empty(ptConfigProcess, "minProcesses") && atoi(ptConfigProcess->m["minProcesses"]->v.c_str()) > 0 && atoi(ptDataProcess->m["processes"]->v.c_str()) < atoi(ptConfigProcess->m["minProcesses"]->v.c_str()))
                                  {
                                    ssAlarmsProcess << ((!ssAlarmsProcess.str().empty())?"  ":"") << process.first << " has " << ptDataProcess->m["processes"]->v << " processes running which is less than the minimum " << ptConfigProcess->m["minProcesses"]->v << " processes.";
                                  }
                                  else if (!empty(ptConfigProcess, "maxProcesses") && atoi(ptConfigProcess->m["maxProcesses"]->v.c_str()) > 0 && atoi(ptDataProcess->m["processes"]->v.c_str()) > atoi(ptConfigProcess->m["maxProcesses"]->v.c_str()))
                                  {
                                    ssAlarmsProcess << ((!ssAlarmsProcess.str().empty())?"  ":"") << process.first << " has " << ptDataProcess->m["processes"]->v << " processes running which is more than the maximum " << ptConfigProcess->m["maxProcesses"]->v << " processes.";
                                  }
                                }
                                if (!empty(ptDataProcess, "image"))
                                {
                                  if (!empty(ptConfigProcess, "minImage") && atoi(ptConfigProcess->m["minImage"]->v.c_str()) > 0 && atoi(ptDataProcess->m["image"]->v.c_str()) < atoi(ptConfigProcess->m["minImage"]->v.c_str()))
                                  {
                                    ssAlarmsProcess << ((!ssAlarmsProcess.str().empty())?"  ":"") << process.first << " has an image size of " << m_manip.toShortByte((atof(ptDataProcess->m["image"]->v.c_str()) * 1024), strValue) << " which is less than the minimum " << m_manip.toShortByte((atof(ptConfigProcess->m["minImage"]->v.c_str()) * 1024), strValue) << ".";
                                  }
                                  else if (!empty(ptConfigProcess, "minImage") && atoi(ptConfigProcess->m["minImage"]->v.c_str()) > 0 && atoi(ptDataProcess->m["image"]->v.c_str()) < atoi(ptConfigProcess->m["minImage"]->v.c_str()))
                                  {
                                    ssAlarmsProcess << ((!ssAlarmsProcess.str().empty())?"  ":"") << process.first << " has an image size of " << m_manip.toShortByte((atof(ptDataProcess->m["image"]->v.c_str()) * 1024), strValue) << " which is less than the minimum " << m_manip.toShortByte((atof(ptConfigProcess->m["minImage"]->v.c_str()) * 1024), strValue) << ".";
                                  }
                                }
                                if (!empty(ptDataProcess, "resident"))
                                {
                                  if (!empty(ptConfigProcess, "minResident") && atoi(ptConfigProcess->m["minResident"]->v.c_str()) > 0 && atoi(ptDataProcess->m["resident"]->v.c_str()) < atoi(ptConfigProcess->m["minResident"]->v.c_str()))
                                  {
                                    ssAlarmsProcess << ((!ssAlarmsProcess.str().empty())?"  ":"") << process.first << " has an resident size of " << m_manip.toShortByte((atof(ptDataProcess->m["resident"]->v.c_str()) * 1024), strValue) << " which is less than the minimum " << m_manip.toShortByte((atof(ptConfigProcess->m["minResident"]->v.c_str()) * 1024), strValue) << ".";
                                  }
                                  else if (!empty(ptConfigProcess, "minResident") && atoi(ptConfigProcess->m["minResident"]->v.c_str()) > 0 && atoi(ptDataProcess->m["resident"]->v.c_str()) < atoi(ptConfigProcess->m["minResident"]->v.c_str()))
                                  {
                                    ssAlarmsProcess << ((!ssAlarmsProcess.str().empty())?"  ":"") << process.first << " has an resident size of " << m_manip.toShortByte((atof(ptDataProcess->m["resident"]->v.c_str()) * 1024), strValue) << " which is less than the minimum " << m_manip.toShortByte((atof(ptConfigProcess->m["minResident"]->v.c_str()) * 1024), strValue) << ".";
                                  }
                                }
                              }
                              if (ptAlarms->m["processes"]->m[process.first]->v != ssAlarmsProcess.str())
                              {
                                ptAlarms->m["processes"]->i(process.first, ssAlarmsProcess.str());
                                if (!ssAlarmsProcess.str().empty())
                                {
                                  if (!storageAdd({"centralmon", "servers", server.first, "alarms", "processes", process.first}, ptAlarms->m["processes"]->m[process.first], strError))
                                  {
                                    ssMessage.str("");
                                    ssMessage << strPrefix << "->Interface::storageAdd() error [centralmon,servers," << server.first << ",alarms,processes," << process.first << "]:  " << strError;
                                    log(ssMessage.str());
                                  }
                                  if (!empty(ptConfigProcess, "applicationId"))
                                  {
                                    ssQuery.str("");
                                    ssQuery << "select c.userid from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id where b.type in ('Primary Developer', 'Backup Developer') and a.application_id = '" << esc(ptConfigProcess->m["applicationId"]->v) << "'";
                                    auto getPerson = dbquery("central_r", ssQuery.str(), strError);
                                    if (getPerson != NULL)
                                    {
                                      for (auto &getPersonRow : *getPerson)
                                      {
                                        alert(getPersonRow["userid"], ssAlarmsProcess.str(), strError);
                                      }
                                    }
                                    dbfree(getPerson);
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                      else
                      {
                        ssAlarmsSystem << ((!ssAlarmsSystem.str().empty())?"  ":"") << "System data is missing.";
                      }
                    }
                    else
                    {
                      ssAlarmsSystem << ((!ssAlarmsSystem.str().empty())?"  ":"") << "Data is stale.";
                    }
                  }
                  else
                  {
                    ssAlarmsSystem << ((!ssAlarmsSystem.str().empty())?"  ":"") << "Data missing time stamp.";
                  }
                }
                else
                {
                  ssAlarmsSystem << ((!ssAlarmsSystem.str().empty())?"  ":"") << "Server appears to be offline.";
                }
              }
              else
              {
                ssAlarmsSystem << ((!ssAlarmsSystem.str().empty())?"  ":"") << "System configuration is missing.";
                storageRemove({"centralmon", "servers", server.first}, strError);
              }
            }
            else
            {
              ssAlarmsSystem << ((!ssAlarmsSystem.str().empty())?"  ":"") << "Configuration is missing.";
              storageRemove({"centralmon", "servers", server.first}, strError);
            }
            if (ptAlarms->m["system"]->v != ssAlarmsSystem.str())
            {
              ptAlarms->i("systems", ssAlarmsSystem.str());
              if (!ssAlarmsSystem.str().empty())
              {
                if (!storageAdd({"centralmon", "servers", server.first, "alarms", "system"}, ptAlarms->m["system"], strError))
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Interface::storageAdd() error [centralmon,servers," << server.first << ",alarms,system]:  " << strError;
                  log(ssMessage.str());
                }
                ssQuery.str("");
                ssQuery << "select d.userid from `server` a, server_contact b, contact_type c, person d where a.id = b.server_id and b.type_id = c.id and b.contact_id = d.id where c.type in ('Primary Admin', 'Backup Admin') and a.name = '" << esc(server.first) << "'";
                auto getPerson = dbquery("central_r", ssQuery.str(), strError);
                if (getPerson != NULL)
                {
                  for (auto &getPersonRow : *getPerson)
                  {
                    alert(getPersonRow["userid"], ssAlarmsSystem.str(), strError);
                  }
                }
                dbfree(getPerson);
              }
            }
          }
        }
        else
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->Interface::storageRetrieve() error [centralmon,servers]:  " << strError;
          log(ssMessage.str());
        }
        delete ptJson;
        // }}}
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
