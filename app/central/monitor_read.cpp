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
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
using namespace std;
#include <Json>
#include <StringManip>
using namespace common;
// }}}
// {{{ prototypes
void display(Json *ptData, string strIndent);
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
      string strLine;
      stringstream ssJson;
      Json *ptData;
      while (getline(inFile, strLine))
      {
        ssJson << strLine;
      }
      ptData = new Json(ssJson.str());
      display(ptData, "");
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
void display(Json *ptData, string strIndent)
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
  if (!data["comm"].empty() && data["comm"][0] == '(')
  {
    data["comm"].erase(0, 1);
  }
  if (!data["comm"].empty() && data["comm"][data["comm"].size()-1] == ')')
  {
    data["comm"].erase((data["comm"].size()-1), 1);
  }
  ssStartTime.str(data["starttime"]);
  ssStartTime >> CTime;
  time(&CTime);
  localtime_r(&CTime, &tTime);
  cout << strIndent;
  cout << data["comm"] << " [pid=" << data["pid"] << "] ";
  cout << "starttime=" << put_time(&tTime, "%Y-%m-%d %H:%M:%S");
  cout << ", state=" << data["state"];
  ssImage.str(data["image"]);
  ssImage >> fImage;
  fImage *= 1024;
  cout << " image=" << manip.toShortByte(fImage, strValue, 2);
  ssResident.str(data["resident"]);
  ssResident >> fResident;
  fResident *= 1024;
  cout << ", resident=" << manip.toShortByte(fResident, strValue, 2);
  cout << endl;
  if (ptChildren != NULL)
  {
    for (auto &i : ptChildren->l)
    {
      display(i, strIndent + "  ");
    }
    delete ptChildren;
  }
}
// }}}
