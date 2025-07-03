// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Central Monitor
// -------------------------------------
// file       : monitor_read.cpp
// author     : Ben Kietzman
// begin      : 2025-07-01
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
// {{{ includes
#include <cerrno>
#include <cstring>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
using namespace std;
#include <Json>
#include <StringManip>
using namespace common;
// }}}
// {{{ prototypes
void display(Json *ptData, map<string, size_t> width, string strIndent);
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  if (argc == 2)
  {
    ifstream inFile;
    inFile.open(argv[1]);
    if (inFile)
    {
      list<string> fields = {"comm", "pid", "ppid", "state"};
      map<string, size_t> width;
      string strLine;
      stringstream ssJson;
      Json *ptData;
      while (getline(inFile, strLine))
      {
        ssJson << strLine;
      }
      ptData = new Json(ssJson.str());
      for (auto &i : fields)
      {
        width[i] = 0;
      }
      for (auto &i : ptData->l)
      {
        for (auto &j : fields)
        {
          if (i->m.find(j) != i->m.end() && i->m[j]->v.size() > width[j])
          {
            width[j] = i->m[j]->v.size();
          }
        }
      }
      display(ptData, width, "");
      delete ptData;
    }
    else
    {
      cerr << "ifstream::open(" << errno << ") error [" << argv[1] << "]:  " << strerror(errno) << endl;
    }
    inFile.close();
  }
  else
  {
    cerr << "USAGE:  " << argv[0] << " [file]" << endl;
  }

  return 0;
}
// }}}
// {{{ display()
void display(Json *ptData, map<string, size_t> width, string strIndent)
{
  float fImage, fResident;
  map<string, string> data;
  string strValue;
  stringstream ssImage, ssResident, ssStartTime;
  struct tm tTime;
  time_t CTime;
  Json *ptChildren = NULL;
  StringManip manip;
  if (ptData->m.find("children") != ptData->m.end())
  {
    ptChildren = ptData->m["children"];
    ptData->m.erase("children");
  }
  ptData->flatten(data, true, false);
  ssStartTime.str(data["starttime"]);
  ssStartTime >> CTime;
  time(&CTime);
  localtime_r(&CTime, &tTime);

  cout << setfill(' ') << setw(width["comm"]) << data["comm"];
  cout << " " << setfill(' ') << setw(width["pid"]) << data["pid"];
  cout << " " << setfill(' ') << setw(width["ppid"]) << data["ppid"];
  cout << " " << setfill(' ') << setw(width["state"]) << data["state"];
  ssImage.str(data["image"]);
  ssImage >> fImage;
  fImage *= 1024;
  cout << " " << setfill(' ') << setw(6) << manip.toShortByte(fImage, strValue, 0);
  ssResident.str(data["resident"]);
  ssResident >> fResident;
  fResident *= 1024;
  cout << " " << setfill(' ') << setw(6) << manip.toShortByte(fResident, strValue, 0);
  cout << " " << put_time(&tTime, "%Y-%m-%d %H:%M:%S");
  cout << " " << strIndent;
  cout << data["cmdline"];
  cout << endl;
  if (ptChildren != NULL)
  {
    for (auto &i : ptChildren->l)
    {
      display(i, width, strIndent + "  ");
    }
    delete ptChildren;
  }
}
// }}}
