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
void setWidths(Json *ptData, map<string, size_t> &width);
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
      list<string> fields = {"cmdline", "comm", "pid", "ppid", "state"};
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
        width[i] = i.size();
      }
      setWidths(ptData, width);
      cout << setfill(' ') << setw(width["comm"]) << left << "comm";
      cout << " " << setfill(' ') << setw(width["pid"]) << right << "pid";
      cout << " " << setfill(' ') << setw(width["ppid"]) << right << "ppid";
      cout << " " << setfill(' ') << setw(6) << right << "img";
      cout << " " << setfill(' ') << setw(6) << right << "res";
      cout << " " << setfill(' ') << setw(19) << left << "starttime";
      cout << " cmdline" << endl;
      cout << setfill('-') << setw(width["comm"]) << left << "-";
      cout << " " << setfill('-') << setw(width["pid"]) << right << "-";
      cout << " " << setfill('-') << setw(width["ppid"]) << right << "-";
      cout << " " << setfill('-') << setw(6) << right << "-";
      cout << " " << setfill('-') << setw(6) << right << "-";
      cout << " " << setfill('-') << setw(19) << left << "-";
      cout << " -------" << endl;
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
  cout << setfill(' ') << setw(width["comm"]) << left << data["comm"];
  cout << " " << setfill(' ') << setw(width["pid"]) << right << data["pid"];
  cout << " " << setfill(' ') << setw(width["ppid"]) << right << data["ppid"];
  ssImage.str(data["image"]);
  ssImage >> fImage;
  fImage *= 1024;
  cout << " " << setfill(' ') << setw(6) << right << manip.toShortByte(fImage, strValue, 0);
  ssResident.str(data["resident"]);
  ssResident >> fResident;
  fResident *= 1024;
  cout << " " << setfill(' ') << setw(6) << right << manip.toShortByte(fResident, strValue, 0);
  cout << " " << left << put_time(&tTime, "%Y-%m-%d %H:%M:%S");
  cout << " " << strIndent << left << data["cmdline"] << endl;
  if (strIndent.size() >= 4 && strIndent.substr((strIndent.size()-4), 4) == " \\_ ")
  {
    strIndent.replace((strIndent.size()-4), 4, " |  ");
  }
  if (ptChildren != NULL)
  {
    bool bFirst = true;
    for (auto &i : ptChildren->l)
    {
      if (bFirst)
      {
        display(i, width, strIndent + " \\_ ");
      }
      else
      {
        display(i, width, strIndent + " |  ");
      }
    }
    delete ptChildren;
  }
}
// }}}
// {{{ setWidths()
void setWidths(Json *ptData, map<string, size_t> &width)
{
  for (auto &i : width)
  {
    if (ptData->m.find(i.first) != ptData->m.end() && ptData->m[i.first]->v.size() > i.second)
    {
      i.second = ptData->m[i.first]->v.size();
    }
  }
  if (ptData->m.find("children") != ptData->m.end())
  {
    for (auto &i : ptData->m["children"]->l)
    {
      setWidths(i, width);
    }
  }
}
// }}}
