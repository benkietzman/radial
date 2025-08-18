// -*- C++ -*-
// Radial
// -------------------------------------
// file       : sqlite.cpp
// author     : Ben Kietzman
// begin      : 2024-10-24
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
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
// }}}
// {{{ prototypes
void display(list<map<string, string> > resultSet);
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  string d = ((argc >= 2)?argv[1]:"/data/warden/socket"), e;
  Warden w("Radial", d, e);

  if (e.empty())
  {
    Json *c = new Json;
    if (w.vaultRetrieve({"radial", "radial", "Password"}, c, e))
    {
      bool b = false;
      string f, l, p, strDatabase, v;
      Radial r(e);
      StringManip manip;
      if (!c->v.empty())
      {
        p = c->v;
      }
      else
      {
        for (auto i = c->l.begin(); p.empty() && i != c->l.end(); i++)
        {
          if (!(*i)->v.empty())
          {
            p = (*i)->v;
          }
        }
      }
      r.setCredentials("radial", p);
      r.useSingleSocket(true);
      cout << endl;
      cout << "sqlite> " << flush;
      while (!b && getline(cin, l))
      {
        stringstream ssLine(l);
        ssLine >> v;
        manip.toLower(f, v);
        cout << endl;
        // {{{ .create
        if (f == ".create")
        {
          string strDatabase;
          ssLine >> strDatabase;
          if (!strDatabase.empty())
          {
            string strNode;
            ssLine >> strNode;
            if (r.sqliteCreate(strDatabase, strNode, e))
            {
              cout << "Database created." << endl;
            }
            else
            {
              cerr << "Radial::sqliteCreate() error:  " << e << endl;
            }
          }
          else
          {
            cerr << "Please provide the database followed by an optional node." << endl;
          }
        }
        // }}}
        // {{{ .drop
        else if (f == ".drop")
        {
          string strDatabase;
          ssLine >> strDatabase;
          if (!strDatabase.empty())
          {
            string strNode;
            ssLine >> strNode;
            if (r.sqliteDrop(strDatabase, strNode, e))
            {
              cout << "Database dropped." << endl;
            }
            else
            {
              cerr << "Radial::sqliteDrop() error:  " << e << endl;
            }
          }
          else
          {
            cerr << "Please provide the database followed by an optional node.";
          }
        }
        // }}}
        // {{{ .database
        else if (f == ".database")
        {
          ssLine >> strDatabase;
          if (!strDatabase.empty())
          {
            cout << "Database:  " << strDatabase << endl;
          }
          else
          {
            cerr << "Please provide the database." << endl;
          }
        }
        // }}}
        // {{{ .exit
        else if (f == ".exit")
        {
          b = true;
        }
        // }}}
        // {{{ .databases
        else if (f == ".databases")
        {
          map<string, map<string, string> > databases;
          getline(ssLine, l);
          if (r.sqliteDatabases(databases, e))
          {
            list<map<string, string> > resultSet;
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
        // }}}
        // {{{ .help
        else if (f == ".help" || strDatabase.empty())
        {
          getline(ssLine, l);
          cout << "Help:  .help" << endl;
          cout << "Exit:  .exit" << endl;
          cout << endl;
          cout << "DATABASE FUNCTIONS" << endl;
          cout << "  Create database:  .create [name] <node>" << endl;
          cout << "  Drop database:  .drop [name] <node>" << endl;
          cout << "  Attach database:  .database [name]" << endl;
          cout << "  List databases:  .databases" << endl;
          cout << endl;
          cout << "TABLE FUNCTIONS (must be attached to a database)" << endl;
          cout << "  List tables:  .tables" << endl;
          cout << "  Describe table:  .desc [name]" << endl;
          cout << "  Export table:  .export [name] [file]" << endl;
          cout << "  Import table:  .import [name] [file]" << endl;
          cout << "  SQL statement: [statement]" << endl;
        }
        // }}}
        // {{{ .desc
        else if (f == ".desc")
        {
          string t;
          ssLine >> t;
          if (!t.empty())
          {
            list<map<string, string> > resultSet;
            size_t unID = 0, unRows = 0;
            string v;
            stringstream ssStatement;
            getline(ssLine, l);
            ssStatement << "select sql from sqlite_master where name = '" << manip.escape(t, v) << "'";
            if (r.sqliteQuery(strDatabase, ssStatement.str(), resultSet, unID, unRows, e))
            {
              if (unRows == 1 && resultSet.front().find("sql") != resultSet.front().end())
              {
                cout << resultSet.front()["sql"] << endl;
              }
              else if (unRows == 0)
              {
                cerr << "Table does not exist." << endl;
              }
              else
              {
                cout << "# of rows returned:  " << unRows << endl;
                display(resultSet);
              }
            }
            else
            {
              cerr << "Radial::sqliteQuery() error:  " << e << endl;
            }
          }
          else
          {
            cerr << "Please provide the table." << endl;
          }
        }
        // }}}
        // {{{ .export
        else if (f == ".export")
        {
          string t;
          ssLine >> t;
          if (!t.empty())
          {
            string strFile;
            ssLine >> strFile;
            if (!strFile.empty())
            {
              ofstream outFile;
              outFile.open(strFile);
              if (outFile)
              {
                list<map<string, string> > resultSet;
                size_t unID = 0, unRows = 0;
                string v;
                stringstream ssStatement;
                getline(ssLine, l);
                ssStatement << "select sql from sqlite_master where name = '" << manip.escape(t, v) << "'";
                if (r.sqliteQuery(strDatabase, ssStatement.str(), resultSet, unID, unRows, e))
                {
                  if (resultSet.size() == 1 && resultSet.front().find("sql") != resultSet.front().end())
                  {
                    list<map<string, string> > subResultSet;
                    size_t unSubID = 0, unSubRows = 0;
                    stringstream ssSubStatement;
                    outFile << resultSet.front()["sql"] << endl;
                    ssSubStatement << "select * from `" << t << "`";
                    if (r.sqliteQuery(strDatabase, ssSubStatement.str(), subResultSet, unSubID, unSubRows, e))
                    {
                      cout << "Table exported." << endl;
                      if (!subResultSet.empty())
                      {
                        outFile << "insert into `" << t << "` (";
                        for (auto col = subResultSet.front().begin(); col != subResultSet.front().end(); col++)
                        {
                          if (col != subResultSet.front().begin())
                          {
                            outFile << ", ";
                          }
                          outFile << "`" << col->first << "`";
                        }
                        outFile << ") values";
                        for (auto row : subResultSet)
                        {
                          outFile << " (";
                          for (auto col = row.begin(); col != row.end(); col++)
                          {
                            if (col != row.begin())
                            {
                              outFile << ", ";
                            }
                            if (!col->second.empty())
                            {
                              outFile << "`" << col->second << "`";
                            }
                            else
                            {
                              outFile << "null";
                            }
                          }
                          outFile << ")";
                        }
                        outFile << endl;
                      }
                    }
                    else
                    {
                      cerr << "Radial::sqliteQuery() error:  " << e << endl;
                    }
                  }
                  else if (resultSet.size() == 0)
                  {
                    cerr << "Table does not exist." << endl;
                  }
                  else
                  {
                    cerr << "Radial::sqliteQuery() error:  Invalid number of rows returned." << endl;
                  }
                }
                else
                {
                  cerr << "Radial::sqliteQuery() error:  " << e << endl;
                }
              }
              else
              {
                cerr << "ofstream::open(" << errno << ") error [" << strFile << "]:  " << strerror(errno) << endl;
              }
              outFile.close();
            }
            else
            {
              cerr << "Please provide the file." << endl;
            }
          }
          else
          {
            cerr << "Please provide the table." << endl;
          }
        }
        // }}}
        // {{{ .import
        else if (f == ".import")
        {
          string t;
          ssLine >> t;
          if (!t.empty())
          {
            string strFile;
            ssLine >> strFile;
            if (!strFile.empty())
            {
              ifstream inFile;
              inFile.open(strFile);
              if (inFile)
              {
                bool bExit = false;
                string l;
                while (!bExit && getline(inFile, l))
                {
                  if (!r.sqliteQuery(strDatabase, l, e))
                  {
                    bExit = true;
                  }
                }
                if (!bExit)
                {
                  cout << "Table imported." << endl;
                }
                else
                {
                  cerr << "Radial::sqliteQuery() error:  " << e << endl;
                }
              }
              else
              {
                cerr << "ifstream::open(" << errno << ") error [" << strFile << "]:  " << strerror(errno) << endl;
              }
              inFile.close();
            }
            else
            {
              cerr << "Please provide the file." << endl;
            }
          }
          else
          {
            cerr << "Please provide the table." << endl;
          }
        }
        // }}}
        // {{{ .tables
        else if (f == ".tables")
        {
          list<map<string, string> > resultSet;
          size_t unID = 0, unRows = 0;
          getline(ssLine, l);
          if (r.sqliteQuery(strDatabase, "select name from sqlite_master where type='table' order by name", resultSet, unID, unRows, e))
          {
            cout << "# of rows returned:  " << unRows << endl;
            display(resultSet);
          }
          else
          {
            cerr << "Radial::sqliteQuery() error:  " << e << endl;
          }
        }
        // }}}
        // {{{ sql
        else if (!f.empty())
        {
          list<map<string, string> > resultSet;
          size_t unID = 0, unRows = 0;
          stringstream ssStatement;
          getline(ssLine, l);
          ssStatement << v << l;
          if (r.sqliteQuery(strDatabase, ssStatement.str(), resultSet, unID, unRows, e))
          {
            if (f == "select")
            {
              cout << "# of rows returned:  " << unRows << endl;
              display(resultSet);
            }
            else if (f == "delete" || f == "insert" || f == "update")
            {
              cout << "# of rows " << ((f == "delete")?"deleted":((f == "insert")?"inserted":"updated")) << ":  " << unRows << endl;
              if (f == "insert")
              {
                cout << endl << "Lastest ID:  " << unID << endl;
              }
            }
            else
            {
              cout << "Operation completed." << endl;
            }
          }
          else
          {
            cerr << "Radial::sqliteQuery() error:  " << e << endl;
          }
        }
        // }}}
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
// }}}
// {{{ display()
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
// }}}
