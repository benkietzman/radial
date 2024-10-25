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

#include <iomanip>
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

void display(list<map<string, string> > resultSet);

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
      string f, l, strDatabase, v;
      Radial r(e);
      StringManip manip;
      r.setCredentials("radial", c);
      r.useSingleSocket(true);
      cout << endl;
      cout << "sqlite> " << flush;
      while (!b && cin >> v)
      {
        string strExtra;
        stringstream ssExtra;
        getline(cin, strExtra);
        ssExtra.str(strExtra);
        cout << endl;
        manip.toLower(f, v);
        if (f == ".create")
        {
          string strDatabase;
          ssExtra >> strDatabase;
          if (!strDatabase.empty())
          {
            string strNode;
            ssExtra >> strNode;
            if (r.sqliteCreate(strDatabase, strNode, e))
            {
              cout << "Radial::sqliteCreate():  okay" << endl;
            }
            else
            {
              cerr << "Radial::sqliteCreate() error:  " << e << endl;
            }
          }
          else
          {
            cerr << "Please provide the database followed by an option node.";
          }
        }
        else if (f == ".drop")
        {
          string strDatabase;
          ssExtra >> strDatabase;
          if (!strDatabase.empty())
          {
            string strNode;
            ssExtra >> strNode;
            if (r.sqliteDrop(strDatabase, strNode, e))
            {
              cout << "Radial::sqliteDrop():  okay" << endl;
            }
            else
            {
              cerr << "Radial::sqliteDrop() error:  " << e << endl;
            }
          }
          else
          {
            cerr << "Please provide the database followed by an option node.";
          }
        }
        else if (f == ".database")
        {
          ssExtra >> strDatabase;
          if (!strDatabase.empty())
          {
            cout << "Database:  " << strDatabase << endl;
          }
          else
          {
            cout << "Please provide the database." << endl;
          }
        }
        else if (f == ".exit" || f == ".quit")
        {
          b = true;
        }
        else if (f == ".databases")
        {
          map<string, map<string, string> > databases;
          getline(ssExtra, l);
          if (r.sqliteDatabases(databases, e))
          {
            list<map<string, string> > resultSet;
            cout << "Radial::sqliteDatabases():  okay" << endl;
            for (auto &database : databases)
            {
              map<string, string> row;
              stringstream ssNodes;
              row["name"] = database.first;
              for (auto node = database.second.begin(); node != database.second.end(); node++)
              {
                if (node != database.second.begin())
                {
                  ssNodes << ", ";
                }
                ssNodes << node->first;
                if (node->second == "master")
                {
                  ssNodes << " (master)";
                }
              }
              row["nodes"] = ssNodes.str();
              resultSet.push_back(row);
            }
            display(resultSet);
          }
          else
          {
            cerr << "Radial::sqliteDatabases() error:  " << e << endl;
          }
        }
        else if (f == ".help" || f == "help" || f == "?" || strDatabase.empty())
        {
          getline(ssExtra, l);
          cout << "Exit:  .exit or .quit" << endl;
          cout << "Create database:  .create <name> [node]" << endl;
          cout << "Drop database:  .drop <name> [node]" << endl;
          cout << "Attach database:  .database <name>" << endl;
          cout << "List databases:  .databases" << endl;
          cout << "List tables:  .tables" << endl;
          cout << "Describe table:  .desc <name>" << endl;
        }
        else if (f == ".desc")
        {
          string t;
          ssExtra >> t;
          if (!t.empty())
          {
            list<map<string, string> > resultSet;
            size_t unID = 0, unRows = 0;
            string v;
            stringstream ssStatement;
            getline(ssExtra, l);
            ssStatement << "select sql from sqlite_master where name = '" << manip.escape(t, v) << "'";
            if (r.sqliteQuery(strDatabase, ssStatement.str(), resultSet, unID, unRows, e))
            {
              cout << "Radial::sqliteQuery():  okay" << endl;
              cout << endl << "# of Rows Returned:  " << unRows << endl;
              display(resultSet);
            }
            else
            {
              cerr << "Radial::sqliteQuery() error:  " << e << endl;
            }
          }
          else
          {
            cout << "Please provide the table.";
          }
        }
        else if (f == ".tables")
        {
          list<map<string, string> > resultSet;
          size_t unID = 0, unRows = 0;
          getline(ssExtra, l);
          if (r.sqliteQuery(strDatabase, "select name from sqlite_master where type='table'", resultSet, unID, unRows, e))
          {
            cout << "Radial::sqliteQuery():  okay" << endl;
            cout << endl << "# of Rows Returned:  " << unRows << endl;
            display(resultSet);
          }
          else
          {
            cerr << "Radial::sqliteQuery() error:  " << e << endl;
          }
        }
        else if (!f.empty())
        {
          list<map<string, string> > resultSet;
          size_t unID = 0, unRows = 0;
          stringstream ssStatement;
          getline(ssExtra, l);
          ssStatement << v << l;
          if (r.sqliteQuery(strDatabase, ssStatement.str(), resultSet, unID, unRows, e))
          {
            cout << "Radial::sqliteQuery():  okay" << endl;
            if (f == "select")
            {
              cout << endl << "# of Rows Returned:  " << unRows << endl;
              display(resultSet);
            }
            else if (f == "delete" || f == "insert" || f == "update")
            {
              cout << endl << "# of Rows " << ((f == "delete")?"Deleted":((f == "insert")?"Inserted":"Updated")) << ":  " << unRows << endl;
              if (f == "insert")
              {
                cout << endl << "Lastest ID:  " << unID << endl;
              }
            }
          }
          else
          {
            cerr << "Radial::sqliteQuery() error:  " << e << endl;
          }
        }
        cout << endl;
        if (!b)
        {
          cout << "sqlite> " << flush;
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

void display(list<map<string, string> > resultSet)
{
  if (!resultSet.empty())
  {
    map<string, size_t> pad;
    for (auto &col : resultSet.front())
    {
      pad[col.first] = col.first.size();
    }
    for (auto &row : resultSet)
    {
      for (auto &col : row)
      {
        if (col.second.size() > pad[col.first])
        {
          pad[col.first] = col.second.size();
        }
      }
    }
    cout << endl;
    for (auto &col : resultSet.front())
    {
      cout << "  " << left << setw(pad[col.first]) << setfill(' ') << col.first;
    }
    cout << endl;
    for (auto &col : resultSet.front())
    {
      cout << "  " << left << setw(pad[col.first]) << setfill('-') << '-';;
    }
    cout << endl;
    for (auto &row : resultSet)
    {
      for (auto &col : row)
      {
        cout << "  " << left << setw(pad[col.first]) << setfill(' ') << col.second;
      }
      cout << endl;
    }
  }
}
