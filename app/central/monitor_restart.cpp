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
#include <iostream>
#include <sstream>
#include <string>
using namespace std;
#include <Json>
using namespace common;
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  if (argc == 2)
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
      delete ptJson;
      if (bRestart)
      {
        system(((string)"systemctl restart " + argv[1]).c_str());
      }
    }
  }
  else
  {
    cerr << "USAGE:  " << argv[0] << " [service]" << endl;
  }

  return 0;
}
// }}}
