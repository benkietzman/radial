// -*- C++ -*-
// Radial
// -------------------------------------
// file       : terminal.cpp
// author     : Ben Kietzman
// begin      : 2024-01-09
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/

#include <iostream>
#include <string>
using namespace std;
#include <Radial>
#include <Warden>
using namespace common;

int main(int argc, char *argv[])
{
  if (argc >= 2)
  {
    string d = "/data/warden/socket", e, p = "23", s = argv[1];
    if (argc >= 3)
    {
      p = argv[2];
      if (argc >= 4)
      {
        d = argv[3];
      }
    }
    Warden w("Radial", d, e);
    if (e.empty())
    {
      string c;
      if (w.vaultRetrieve({"radial", "radial", "Password"}, c, e))
      {
        bool b = false;
        Radial r(e);
        radialTerminal t;
        r.setCredentials("radial", c);
        r.useSingleSocket(true);
        if (r.terminalConnect(t, s, p, true, e))
        {
          string f;
          cout << "\033[32;40m";
          for (size_t i = 0; i < t.screen.size(); i++)
          {
            if (i == t.unRow && t.unCol < t.screen[i].size())
            {
              stringstream ssCursor;
              ssCursor << "\033[30;42m" << t.screen[i][t.unCol] << "\033[32;40m";
              t.screen[i].replace(t.unCol, 1, ssCursor.str());
            }
            cout << t.screen[i] << endl;
          }
          cout << "\033[0m> " << flush;
          while (!b && cin >> f)
          {
            if (f == "disconnect" || f == "exit" || f == "q" || f == "quit")
            {
              b = true;
            }
            else if (f == "d" || f == "down")
            {
              if (!r.terminalDown(t, 1, true, e))
              {
                cerr << "Radial::terminalDown() error:  " << e << endl;
              }
            }
            else if (f == "e" || f == "enter")
            {
              if (!r.terminalEnter(t, true, e))
              {
                cerr << "Radial::terminalEnter() error:  " << e << endl;
              }
            }
            else if (f == "f" || f == "function")
            {
              int k;
              cin >> k;
              if (!r.terminalFunction(t, k, e))
              {
                cerr << "Radial::terminalFunction() error:  " << e << endl;
              }
            }
            else if (f == "ke" || f == "keypadEnter")
            {
              if (!r.terminalKeypadEnter(t, true, e))
              {
                cerr << "Radial::terminalKeypadEnter() error:  " << e << endl;
              }
            }
            else if (f == "l" || f == "left")
            {
              if (!r.terminalLeft(t, 1, true, e))
              {
                cerr << "Radial::terminalLeft() error:  " << e << endl;
              }
            }
            else if (f == "r" || f == "right")
            {
              if (!r.terminalRight(t, 1, true, e))
              {
                cerr << "Radial::terminalRight() error:  " << e << endl;
              }
            }
            else if (f == "s" || f == "send")
            {
              string d;
              cin >> d;
              if (!r.terminalSend(t, d, 1, true, e))
              {
                cerr << "Radial::terminalSend() error:  " << e << endl;
              }
            }
            else if (f == "sf" || f == "shiftFunction")
            {
              int k;
              cin >> k;
              if (!r.terminalShiftFunction(t, k, e))
              {
                cerr << "Radial::terminalShiftFunction() error:  " << e << endl;
              }
            }
            else if (f == "t" || f == "tab")
            {
              if (!r.terminalTab(t, 1, true, e))
              {
                cerr << "Radial::terminalTab() error:  " << e << endl;
              }
            }
            else if (f == "u" || f == "up")
            {
              if (!r.terminalUp(t, 1, true, e))
              {
                cerr << "Radial::terminalUp() error:  " << e << endl;
              }
            }
            else if (f == "w" || f == "wait")
            {
              if (!r.terminalWait(t, true, e))
              {
                cerr << "Radial::terminalWait() error:  " << e << endl;
              }
            }
            else
            {
              cerr << "Please provide a valid Function:  disconnect (exit,q,quit), down (d), enter (e), function (f), keypadEnter (ke), left (l), right (r), shiftFunction (sf), send (s), tab (t), up (u), wait (w).";
            }
            if (!b)
            {
              cout << "\033[32;40m";
              for (size_t i = 0; i < t.screen.size(); i++)
              {
                if (i == t.unRow && t.unCol < t.screen[i].size())
                {
                  stringstream ssCursor;
                  ssCursor << "\033[30;42m" << t.screen[i][t.unCol] << "\033[32;40m";
                  t.screen[i].replace(t.unCol, 1, ssCursor.str());
                }
                cout << t.screen[i] << endl;
              }
              cout << "\033[0m> " << flush;
            }
          }
          if (!r.terminalDisconnect(t, e))
          {
            cerr << "Radial::terminalDisconnect() error:  " << e << endl;
          }
        }
        else
        {
          cerr << "Radial::terminalConnect() error [" << s << "," << p << "]:  " << e << endl;
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
  }
  else
  {
    cerr << "USAGE:  " << argv[0] << " [server] <port> <data>" << endl;
  }

  return 0;
}
