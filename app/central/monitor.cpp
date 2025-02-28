// -*- C++ -*-
// Radial
// -------------------------------------
// file       : monitor.cpp
// author     : Ben Kietzman
// begin      : 2025-02-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include <cstdio>
#include <ctime>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>
using namespace std;
#include <Json>
#include <Radial>
#include <StringManip>
#include <Warden>
using namespace common;
// }}}
// {{{ defines
#ifdef VERSION
#undef VERSION
#endif
/*! \def VERSION
* \brief Contains the application version number.
*/
#define VERSION "0.1"
/*! \def mUSAGE(A)
* \brief Prints the usage statement.
*/
#define mUSAGE(A) cout << endl << "Usage:  "<< A << " [options]"  << endl << endl << " -c, --conf=[CONF]" << endl << "     Provides the configuration path." << endl << endl << " -h, --help" << endl << "     Displays this usage screen." << endl << endl << " -s, --server" << endl << "     Provides the server name." << endl << endl << " -u USER, --user=USER" << endl << "     Provides the Radial user." << endl << endl << " -w WARDEN, --warden=WARDEN" << endl << "     Provides the Warden socket path." << endl << endl << " -v, --version" << endl << "     Displays the current version of this software." << endl << endl
/*! \def mVER_USAGE(A,B)
* \brief Prints the version number.
*/
#define mVER_USAGE(A,B) cout << endl << A << " Version: " << B << endl << endl
// }}}
// {{{ global variables
bool gbShutdown = false;
// }}}
// {{{ prototypes
void sighandle(const int nSignal);
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  string strConf, strPrefix = "monitor->main()", strServer, strUser = "radial", strWarden = "/data/warden/socket";
  StringManip manip;

  // {{{ set signal handling
  sethandles(sighandle);
  signal(SIGBUS, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGCONT, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGSEGV, SIG_IGN);
  signal(SIGWINCH, SIG_IGN);
  // }}}
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-c" || (strArg.size() > 7 && strArg.substr(0, 7) == "--conf="))
    {
      if (strArg == "-c" && i + 1 < argc && argv[i+1][0] != '-')
      {
        strConf = argv[++i];
      }
      else
      {
        strConf = strArg.substr(7, strArg.size() - 7);
      }
      manip.purgeChar(strConf, strConf, "'");
      manip.purgeChar(strConf, strConf, "\"");
    }
    else if (strArg == "-h" || strArg == "--help")
    {
      mUSAGE(argv[0]);
      return 0;
    }
    else if (strArg == "-s" || (strArg.size() > 9 && strArg.substr(0, 9) == "--server="))
    {
      if (strArg == "-s" && i + 1 < argc && argv[i+1][0] != '-')
      {
        strServer = argv[++i];
      }
      else
      {
        strServer = strArg.substr(9, strArg.size() - 9);
      }
      manip.purgeChar(strServer, strServer, "'");
      manip.purgeChar(strServer, strServer, "\"");
    }
    else if (strArg == "-u" || (strArg.size() > 7 && strArg.substr(0, 7) == "--user="))
    {
      if (strArg == "-u" && i + 1 < argc && argv[i+1][0] != '-')
      {
        strUser = argv[++i];
      }
      else
      {
        strUser = strArg.substr(7, strArg.size() - 7);
      }
      manip.purgeChar(strUser, strUser, "'");
      manip.purgeChar(strUser, strUser, "\"");
    }
    else if (strArg == "-w" || (strArg.size() > 9 && strArg.substr(0, 9) == "--warden="))
    {
      if (strArg == "-w" && i + 1 < argc && argv[i+1][0] != '-')
      {
        strWarden = argv[++i];
      }
      else
      {
        strWarden = strArg.substr(9, strArg.size() - 9);
      }
      manip.purgeChar(strWarden, strWarden, "'");
      manip.purgeChar(strWarden, strWarden, "\"");
    }
    else
    {
      cout << endl << "Illegal option, '" << strArg << "'." << endl;
      mUSAGE(argv[0]);
      return 0;
    }
  }
  // }}}
  // {{{ normal run
  if (!strServer.empty())
  {
    Warden warden("Radial", strWarden, strError);
    setlocale(LC_ALL, "");
    if (strError.empty())
    {
      string strPassword;
      if (warden.vaultRetrieve({"radial", strUser, "Password"}, strPassword, strError))
      {
        Radial radial(strError);
        if (!strConf.empty())
        {
          strError.clear();
          radial.utility()->setConfPath(strConf, strError);
        }
        if (strError.empty())
        {
          time_t CConfig = 0, CNow;
          Json *ptConfig = new Json;
          radial.setCredentials(strUser, strPassword);
          radial.setTimeout("10");
          radial.utility()->sslInit();
          radial.useSingleSocket();
          while (!gbShutdown)
          {
            time(&CNow);
            // {{{ config
            if ((CNow - CConfig) > 60)
            {
              Json *ptReq = new Json, *ptRes = new Json;
              CConfig = CNow;
              ptReq->i("Interface", "central");
              ptReq->i("Function", "monitorConfig");
              ptReq->m["Request"] = new Json;
              ptReq->m["Request"]->i("server", strServer);
              if (radial.request(ptReq, ptRes, strError))
              {
                if (ptRes->m.find("Response") != ptRes->m.end())
                {
                  delete ptConfig;
                  ptConfig = new Json(ptRes->m["Response"]);
                }
                else
                {
                  cerr << strPrefix << "->Radial::request(central,monitorConfig) error [" << strServer << "]:  Failed to retrieve config." << endl;
                }
              }
              else
              {
                cerr << strPrefix << "->Radial::request(central,monitorConfig) error [" << strServer << "]:  " << strError << endl;
              }
              delete ptReq;
              delete ptRes;
            }
            // }}}
            // {{{ data
            if (ptConfig->m.find("system") != ptConfig->m.end())
            {
              map<string, bool> exclude;
              struct utsname server;
              Json *ptReq = new Json, *ptRes = new Json, *ptSystem;
              ptReq->i("Interface", "central");
              ptReq->i("Function", "monitorData");
              ptReq->m["Request"] = new Json;
              ptReq->m["Request"]->i("server", strServer);
              ptReq->m["Request"]->m["system"] = new Json;
              ptSystem = ptReq->m["Request"]->m["system"];
              if (uname(&server) != -1)
              {
                struct sysinfo sys;
                if (sysinfo(&sys) != -1)
                {
                  ifstream inCpuSpeed("/proc/cpuinfo");
                  if (inCpuSpeed.good())
                  {
                    if ((pfinPipe = popen("top -b -n 1 | sed -n '8,$p'| awk '{print $9, $12}'", "r")) != NULL)
                    {
                      char szProcess[32] = "\0";
                      float fCpu = 0, fCpuSpeed = 0, fCpuUsage = 0;
                      map<float, list<string> > load;
                      string strTemp;
                      while (fCpuSpeed == 0 && file.findLine(inCpuSpeed, false, false, "cpu MHz"))
                      {
                        inCpuSpeed >> strTemp >> strTemp >> strTemp >> fCpuSpeed;
                      }
                      ptSystem->i("operatingSystem"], server.sysname);
                      ptSystem->i("systemRelease", server.release);
                      ptSystem->i("processors", to_string(get_nprocs()), 'n');
                      ptSystem->i("cpuSpeed", to_string((get_nprocs() > 0)?(size_t)fCpuSeed:0), 'n');
                      ptSystem->i("processes", to_string(sys.procs), 'n');
                      while (fscanf(pfinPipe, "%f %s%*[^\n]", &fCpu, &szProcess[0]) != EOF)
                      {
                        fCpuUsage += fCpu;
                        if (load.find(fCpu) == load.end())
                        {
                          load[fCpu] = {};
                        }
                        if (load.find(fCpu) != load.end())
                        {
                          load[fCpu].push_back(szProcess);
                        }
                      }
                      ptSystem->i("cpuUsage", to_string((size_t)(fCpuUsage / ((get_nprocs() > 0)?get_ngrocs():1))), 'n');
                      while (load.size() > 5)
                      {
                        load.begin()->second.clear();
                        load.erase(load.begin()->first);
                      }
                      for (auto i = load.begin(); i != load.end(); i++)
                      {
                        for (auto j = i->second.begin(); j != i->second.end(); j++)
                        {
                          stringstream ssCpuProcessUsage;
                          ssCpuProcessUsage << (*j) << '=' << i->first;
                          if (ptSystem->m.find("cpuProcessUsage") == ptSystem->m.end())
                          {
                            ptSystem->i("cpuProcessUsage", "");
                          }
                          else if (!ptSystem->m["cpuProcessUsage"]->v.empty())
                          {
                            ssCpuProcessUsage << ',';
                          }
                          ptSystem->i("cpuProcessUsage", ssCpuProcessUsage.str() + ptSystem->m["cpuProcessUsage"]->v);
                        }
                        i->second.clear();
                      }
                      load.clear();
                      ptSystem->i("upTime", to_string(sys.uptime), 'n');
                      ptSystem->i("mainTotal", to_string(sys.totalram * sys.mem_unit), 'n');
                      ptSystem->i("mainUsed", to_string((sys.totalram - sys.freeram) * sys.mem_unit), 'n');
                      ptSystem->i("swapTotal", to_string(sys.totalswap * sys.mem_unit), 'n');
                      ptSystem->i("swapUsed", to_string((sys.totalswap - sys.freeswap) * sys.mem_unit), 'n');
                    }
                    pclose(pfinPipe);
                  }
                  inCpuSpeed.close();
                }
              }
              ptSystemn->m["partitions"] = new Json;
              if ((pfinPipe = popen("df -kl", "r")) != NULL)
              {
                char szField[3][128] = {"\0", "\0", "\0"};
                fscanf(pfinPipe, "%*s %s %*s %*s %s %s %*s", szField[0], szField[1], szField[2]);
                while (fscanf(pfinPipe, "%*s %s %*s %*s %s %s", szField[0], szField[1], szField[2]) != EOF)
                {
                  if (atoi(szField[0]) > 0 && exclude.find(szField[2]) == exclude.end())
                  {
                    string strUsage = szField[1];
                    strUsage.erase(strUsage.size() - 1, 1);
                    ptSystem->m["partitions"]->i(szField[2], strUsage);
                  }
                }
                pclose(pfinPipe);
              }
              if (ptConfig->m.find("processes") != ptConfig->m.end())
              {
                ptReq->m["Request"]->m["processes"] = new Json;
                for (auto &process : ptConfig->m["processes"]->m)
                {
                  Json *ptProcess = process->second;
                  // TODO - process
                }
              }
              radial.request(ptReq, ptRes, strError);
              delete ptReq;
              delete ptRes;
            }
            // }}}
            sleep(5);
          }
          radial.utility()->sslDeinit();
          delete ptConfig;
        }
        else
        {
          cerr << strPrefix << "->Radial::Radial() error:  " << strError << endl;
        }
      }
      else
      {
        cerr << strPrefix << "->Warden::vaultRetrieve() error [radial," << strUser << ",Password]:  " << strError << endl;
      }
    }
    else
    {
      cerr << strPrefix << "->Warden::Warden() error [" << strWarden << "]:  " << strError << endl;
    }
  }
  else
  {
    cerr << strPrefix << ":  Please provide the server." << endl;
  }
  // }}}
  // {{{ usage statement
  else
  {
    mUSAGE(argv[0]);
  }
  // }}}

  return 0;
}
// }}}
// {{{ sighandle()
void sighandle(const int nSignal)
{
  string strSignal;
  stringstream ssPrefix;
  sethandles(sigdummy);
  gbShutdown = true;
  ssPrefix << "monitor->sighandle(" << nSignal << ")";
  cerr << ssPrefix.str() << ":  " << sigstring(strSignal, nSignal) << endl;
}
// }}}
