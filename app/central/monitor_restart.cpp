// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Central Monitor
// -------------------------------------
// file       : monitor_restart.cpp
// author     : Ben Kietzman
// begin      : 2025-06-30
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
// {{{ includes
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
using namespace std;
#include <File>
#include <Json>
#include <StringManip>
using namespace common;
// }}}
// {{{ prototypes
Json *getProcess(const string strPid, list<Json *> &processes);
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  if (argc >= 2)
  {
    string strJson;
    if (getline(cin, strJson))
    {
      bool bRestart = false;
      Json *ptJson = new Json(strJson);
      if (ptJson->m.find("image") != ptJson->m.end() && !ptJson->m["image"]->v.empty())
      {
        size_t unImage;
        stringstream ssImage(ptJson->m["image"]->v);
        ssImage >> unImage;
        if (ptJson->m.find("minImage") != ptJson->m.end() && !ptJson->m["minImage"]->v.empty())
        {
          size_t unMinImage;
          stringstream ssMinImage(ptJson->m["minImage"]->v);
          ssMinImage >> unMinImage;
          if (unMinImage > 0 && unImage > unMinImage)
          {
            bRestart = true;
          }
        }
        if (ptJson->m.find("maxImage") != ptJson->m.end() && !ptJson->m["maxImage"]->v.empty())
        {
          size_t unMaxImage;
          stringstream ssMaxImage(ptJson->m["maxImage"]->v);
          ssMaxImage >> unMaxImage;
          if (unMaxImage > 0 && unImage > unMaxImage)
          {
            bRestart = true;
          }
        }
      }
      if (ptJson->m.find("processes") != ptJson->m.end() && !ptJson->m["processes"]->v.empty())
      {
        size_t unProcesses;
        stringstream ssProcesses(ptJson->m["processes"]->v);
        ssProcesses >> unProcesses;
        if (ptJson->m.find("minProcesses") != ptJson->m.end() && !ptJson->m["minProcesses"]->v.empty())
        {
          size_t unMinProcesses;
          stringstream ssMinProcesses(ptJson->m["minProcesses"]->v);
          ssMinProcesses >> unMinProcesses;
          if (unMinProcesses > 0 && unProcesses < unMinProcesses)
          {
            bRestart = true;
          }
        }
        if (ptJson->m.find("maxProcesses") != ptJson->m.end() && !ptJson->m["maxProcesses"]->v.empty())
        {
          size_t unMaxProcesses;
          stringstream ssMaxProcesses(ptJson->m["maxProcesses"]->v);
          ssMaxProcesses >> unMaxProcesses;
          if (unMaxProcesses > 0 && unProcesses > unMaxProcesses)
          {
            bRestart = true;
          }
        }
      }
      if (ptJson->m.find("resident") != ptJson->m.end() && !ptJson->m["resident"]->v.empty())
      {
        size_t unResident;
        stringstream ssResident(ptJson->m["resident"]->v);
        ssResident >> unResident;
        if (ptJson->m.find("minResident") != ptJson->m.end() && !ptJson->m["minResident"]->v.empty())
        {
          size_t unMinResident;
          stringstream ssMinResident(ptJson->m["minResident"]->v);
          ssMinResident >> unMinResident;
          if (unMinResident > 0 && unResident < unMinResident)
          {
            bRestart = true;
          }
        }
        if (ptJson->m.find("maxResident") != ptJson->m.end() && !ptJson->m["maxResident"]->v.empty())
        {
          size_t unMaxResident;
          stringstream ssMaxResident(ptJson->m["maxResident"]->v);
          ssMaxResident >> unMaxResident;
          if (unMaxResident > 0 && unResident > unMaxResident)
          {
            bRestart = true;
          }
        }
      }
      if (bRestart)
      {
        if (argc >= 3 && ptJson->m.find("daemon") != ptJson->m.end() && !ptJson->m["daemon"]->v.empty())
        {
          ifstream inProc("/proc/stat");
          list<string> items;
          list<Json *> processes;
          long long llBootTime = 0;
          ofstream outData;
          string strDaemon = (string)"(" + ptJson->m["daemon"]->v + ")", strPid, strProc = "/proc";
          stringstream ssProc;
          File file;
          StringManip manip;
          if (inProc)
          {
            string strLine;
            while (llBootTime == 0 && getline(inProc, strLine))
            {
              string strField;
              stringstream ssLine(strLine);
              ssLine >> strField;
              if (strField == "btime")
              {
                ssLine >> llBootTime;
              }
            }
            file.directoryList(strProc.c_str(), items);
            for (auto &item : items)
            {
              if (manip.isNumeric(item))
              {
                ifstream inStat;
                stringstream ssStat;
                ssStat << "/proc/" << item << "/stat";
                inStat.open(ssStat.str().c_str());
                if (inStat)
                {
                  string strField;
                  vector<string> fields;
                  while (inStat >> strField)
                  {
                    fields.push_back(strField);
                  }
                  if (fields.size() >= 24)
                  {
                    char cState = fields[2][0];
                    ifstream inCmdLine;
                    long lJiffies = sysconf(_SC_CLK_TCK), lPageSize = sysconf(_SC_PAGE_SIZE) / 1024;
                    long long llStartTime;
                    string strState = "unknown";
                    stringstream ssCmdLine, ssImage[2], ssResident[2], ssStartTime[2];
                    time_t CStartTime;
                    unsigned long ulImage, ulResident;
                    Json *ptProc = new Json;
                    if (fields[1] == strDaemon && fields[3] == "1")
                    {
                      strPid = fields[0];
                    }
                    ptProc->i("pid", fields[0], 'n');
                    if (!fields[1].empty() && fields[1][0] == '(')
                    {
                      fields[1].erase(0, 1);
                    }
                    if (!fields[1].empty() && fields[1][fields[1].size()-1] == ')')
                    {
                      fields[1].erase((fields[1].size()-1), 1);
                    }
                    ptProc->i("comm", fields[1]);
                    ssCmdLine << "/proc/" << item << "/cmdline";
                    inCmdLine.open(ssCmdLine.str());
                    if (inCmdLine)
                    {
                      stringstream ssLine;
                      getline(inCmdLine, strLine);
                      if (!strLine.empty() && strLine[strLine.size()-1] == '\0')
                      {
                        strLine.erase((strLine.size()-1), 1);
                      }
                      for (size_t i = 0; i < strLine.size(); i++)
                      {
                        if (strLine[i] == '\0')
                        {
                          strLine[i] = ' ';
                        }
                      }
                      ptProc->i("cmdline", strLine);
                    }
                    inCmdLine.close();
                    switch (cState)
                    {
                      case 'R': strState = "running"; break;
                      case 'S': strState = "sleeping"; break;
                      case 'D': strState = "waiting"; break;
                      case 'Z': strState = "zombie"; break;
                      case 'T': strState = "stopped"; break;
                      case 't': strState = "tracing stop"; break;
                      case 'X': strState = "dead"; break;
                      case 'I': strState = "idle"; break;
                    }
                    ptProc->i("state", strState);
                    ptProc->i("ppid", fields[3], 'n');
                    ssStartTime[0].str(fields[21]);
                    ssStartTime[0] >> llStartTime;
                    CStartTime = llBootTime + (llStartTime / lJiffies);
                    ssStartTime[1] << CStartTime;
                    ptProc->i("starttime", ssStartTime[1].str(), 'n');
                    ssImage[0].str(fields[22]);
                    ssImage[0] >> ulImage;
                    ulImage /= 1024;
                    ssImage[1] << ulImage;
                    ptProc->i("image", ssImage[1].str(), 'n');
                    ssResident[0].str(fields[23]);
                    ssResident[0] >> ulResident;
                    ulResident *= lPageSize;
                    ssResident[1] << ulResident;
                    ptProc->i("resident", ssResident[1].str(), 'n');
                    processes.push_back(ptProc);
                  }
                  inStat.close();
                }
              }
            }
          }
          inProc.close(); 
          if (!strPid.empty())
          {
            Json *ptData = getProcess(strPid, processes);
            if (ptData != NULL)
            {
              outData.open(argv[2]);
              if (outData)
              {
                outData << ptData << endl;
              }
              outData.close();
              delete ptData;
            }
          }
          for (auto &process : processes)
          {
            delete process;
          }
        }
        system(((string)"systemctl restart " + argv[1]).c_str());
      }
      delete ptJson;
    }
  }
  else
  {
    cerr << "USAGE:  " << argv[0] << " [service]" << endl;
  }

  return 0;
}
// }}}
// {{{ getProcess()
Json *getProcess(const string strPid, list<Json *> &processes)
{
  Json *ptProcess = NULL;

  for (auto i = processes.begin(); ptProcess == NULL && i != processes.end(); i++)
  {
    if ((*i)->m["pid"]->v == strPid)
    {
      ptProcess = new Json(*i);
    }
  }
  if (ptProcess != NULL)
  {
    Json *ptChildren = new Json;
    for (auto i = processes.begin(); i != processes.end(); i++)
    {
      if ((*i)->m["ppid"]->v == strPid)
      {
        ptChildren->l.push_back(getProcess((*i)->m["pid"]->v, processes));
      }
    }
    if (!ptChildren->l.empty())
    {
      ptProcess->m["children"] = ptChildren;
    }
    else
    {
      delete ptChildren;
    }
  }

  return ptProcess;
}
// }}}
