// -*- C++ -*-
// Radial
// -------------------------------------
// file       : sqlite.cpp
// author     : Ben Kietzman
// begin      : 2024-10-24
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/

#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
using namespace std;
#include <Radial>
#include <StringManip>
#include <Warden>
using namespace common;

int main(int argc, char *argv[])
{
  string d = ((argc >= 2)?argv[1]:"/data/warden/socket"), e;
  Warden w("Radial", d, e);

  if (e.empty())
  {
    string c;
    if (w.vaultRetrieve({"radial", "radial", "Password"}, c, e))
    {
      bool b = false;
      string strDatabase, f, v;
      Radial r(e);
      StringManip manip;
      r.setCredentials("radial", c);
      r.useSingleSocket(true);
      cout << "> " << flush;
      while (!b && cin >> v)
      {
        manip.toLower(f, v);
        if (f == "database")
        {
          cin >> strDatabase;
          if (!strDatabase.empty())
          {
            cout << "Database has been set to \"" << strDatabase << "\"." << endl;
          }
          else
          {
            cout << "Please provide the database." << endl;
          }
        }
        else if (f == "exit" || f == "quit")
        {
          b = true;
        }
        else if (f == "list")
        {
          map<string, map<string, string> > databases;
          if (r.sqliteList(databases, e))
          {
            cout << "Radial::sqliteList():  okay" << endl;
            for (auto &database : databases)
            {
              cout << database.first << ":  ";
              for (auto node = database.second.begin(); node != database.second.end(); node++)
              {
                if (node != database.second.begin())
                {
                  cout << ", ";
                }
                cout << node->first << " (" << node->second << ")";
              }
              cout << endl;
            }
          }
          else
          {
            cerr << "Radial::sqliteList() error:  " << e << endl;
          }
        }
        else if (strDatabase.empty())
        {
          cout << "Please set the database using:  database <name>" << endl;
          cout << endl << "Obtain a list of databases using:  list" << endl;
        }
        else if (!f.empty())
        {
          list<map<string, string> > resultSet;
          size_t unID = 0, unRows = 0;
          string strLine;
          stringstream ssStatement;
          getline(cin, strLine);
          ssStatement << v << strLine;
          if (r.sqliteQuery(strDatabase, ssStatement.str(), resultSet, unID, unRows, e))
          {
            cout << "Radial::sqliteQuery():  okay" << endl;
            if (f == "select")
            {
              cout << "# of Rows Returned:  " << unRows << endl;
              if (!resultSet.empty())
              {
                for (auto &col : resultSet.front())
                {
                  cout << "  " << col.first;
                }
                cout << endl;
                for (auto &row : resultSet)
                {
                  for (auto &col : row)
                  {
                    cout << "  " << col.second;
                  }
                  cout << endl;
                }
              }
            }
            else if (f == "delete" || f == "insert" || f == "update")
            {
              cout << "# of Rows " << ((f == "delete")?"Deleted":((f == "insert")?"Inserted":"Updated")) << ":  " << unRows << endl;
              if (f == "insert")
              {
                cout << "Lastest ID:  " << unID << endl;
              }
            }
          }
          else
          {
            cerr << "Radial::sqliteQuery() error:  " << e << endl;
          }
        }
        if (!b)
        {
          cout << "> " << flush;
        }
      }
    }
    else
    {
      cerr << "Warden::vaultRetrieve() error [" << d << ",radial,radial,Password]:  " << e << endl;
    }
  }
  else
  {
    cerr << "Warden::Warden() error [" << d << "]:  " << e << endl;
  }

  return 0;
}
