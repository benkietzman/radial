// -*- C++ -*-
// Radial
// -------------------------------------
// file       : manager.cpp
// author     : Ben Kietzman
// begin      : 2022-09-14
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
#include <cerrno>
#include <cstring>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
using namespace std;
#include <Json>
using namespace common;

#define UNIX_SOCKET "/data/radial/manager.sock"

bool request(const string strFunction, const string strInterface, string &strResponse, string &strError);

int main(int argc, char *argv[])
{
  if (argc == 2 || argc == 3)
  {
    string strError, strFunction = argv[1], strInterface = ((argc == 3)?argv[2]:""), strResponse;
    if (strFunction == "list")
    {
      if (request(strFunction, strInterface, strResponse, strError))
      {
        cout << strResponse << endl;
      }
      else
      {
        cerr << strError << endl;
      }
    }
    else if (strFunction == "restart")
    {
      if (request("stop", strInterface, strResponse, strError))
      {
        bool bGood;
        while ((bGood = request("status", strInterface, strResponse, strError)) && strResponse == "online")
        {
          usleep(250000);
        }
        if (bGood)
        {
          if (!request("start", strInterface, strResponse, strError))
          {
            cerr << strError << endl;
          }
        }
        else
        {
          cerr << strError << endl;
        }
      }
      else
      {
        cerr << strError << endl;
      }
    }
    else if (strFunction == "start")
    {
      if (!request(strFunction, strInterface, strResponse, strError))
      {
        cerr << strError << endl;
      }
    }
    else if (strFunction == "status")
    {
      if (request(strFunction, strInterface, strResponse, strError))
      {
        cout << strResponse << endl;
      }
      else
      {
        cerr << strError << endl;
      }
    }
    else if (strFunction == "stop")
    {
      if (!request(strFunction, strInterface, strResponse, strError))
      {
        cerr << strError << endl;
      }
    }
    else
    {
      cerr << "Please provide a valid function:  list, restart, start, status, stop." << endl;
    }
  }
  else
  {
    cout << "USAGE:  [function] [interface]" << endl;
  }

  return 0;
}

bool request(const string strFunction, const string strInterface, string &strResponse, string &strError)
{
  bool bResult = false;
  stringstream ssMessage;
  struct stat tStat;

  strError.clear();
  strResponse.clear();
  if (stat(UNIX_SOCKET, &tStat) == 0)
  {
    int fdUnix;
    if ((fdUnix = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0)
    {
      sockaddr_un addr;
      memset(&addr, 0, sizeof(sockaddr_un));
      addr.sun_family = AF_UNIX;
      strncpy(addr.sun_path, UNIX_SOCKET, sizeof(addr.sun_path) - 1);
      if (connect(fdUnix, (sockaddr *)&addr, sizeof(sockaddr)) == 0)
      {
        bool bExit = false;
        char szBuffer[65536];
        int nReturn;
        size_t unPosition;
        string strBuffers[2], strJson;
        Json *ptJson = new Json;
        ptJson->i("Function", strFunction);
        if (!strInterface.empty())
        {
          ptJson->i("Interface", strInterface);
        }
        strBuffers[1] = ptJson->j(strJson) + "\n";
        while (!bExit)
        {
          pollfd fds[1];
          fds[0].fd = fdUnix;
          fds[0].events = POLLIN;
          if (!strBuffers[1].empty())
          {
            fds[0].events |= POLLOUT;
          }
          if ((nReturn = poll(fds, 1, 250)) > 0)
          {
            if (fds[0].revents & POLLIN)
            {
              if ((nReturn = read(fds[0].fd, szBuffer, 65536)) > 0)
              {
                strBuffers[0].append(szBuffer, nReturn);
                if ((unPosition = strBuffers[0].find("\n")) != string::npos)
                {
                  Json *ptJson = new Json(strBuffers[0].substr(0, unPosition));
                  strBuffers[0].erase(0, (unPosition + 1));
                  bExit = true;
                  if (ptJson->m.find("Status") != ptJson->m.end() && ptJson->m["Status"]->v == "okay")
                  {
                    bResult = true;
                    if (ptJson->m.find("Response") != ptJson->m.end() && !ptJson->m["Response"]->v.empty())
                    {
                      strResponse = ptJson->m["Response"]->v;
                    }
                  }
                  else if (ptJson->m.find("Error") != ptJson->m.end() && !ptJson->m["Error"]->v.empty())
                  {
                    strError = ptJson->m["Error"]->v;
                  }
                  else
                  {
                    strError = "Encountered an unknown error.";
                  }
                }
              }
              else
              {
                bExit = true;
                if (nReturn < 0)
                {
                  ssMessage.str("");
                  ssMessage << "read(" << errno << ") error [" << UNIX_SOCKET << "]:  " << strerror(errno);
                  strError = ssMessage.str();
                }
              }
            }
            if (fds[0].revents & POLLOUT)
            {
              if ((nReturn = write(fds[0].fd, strBuffers[1].c_str(), strBuffers[1].size())) > 0)
              {
                strBuffers[1].erase(0, nReturn);
              }
              else
              {
                bExit = true;
                if (nReturn < 0)
                {
                  ssMessage.str("");
                  ssMessage << "write(" << errno << ") error [" << UNIX_SOCKET << "]:  " << strerror(errno);
                  strError = ssMessage.str();
                }
              }
            }
          }
          else if (nReturn < 0)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << "poll(" << errno << ") error [" << UNIX_SOCKET << "]:  " << strerror(errno);
            strError = ssMessage.str();
          }
        }
      }
      else
      {
        ssMessage.str("");
        ssMessage << "connect(" << errno << ") error [" << UNIX_SOCKET << "]:  " << strerror(errno);
        strError = ssMessage.str();
      }
      close(fdUnix);
    }
    else
    {
      ssMessage.str("");
      ssMessage << "socket(" << errno << ") error [" << UNIX_SOCKET << "]:  " << strerror(errno);
      strError = ssMessage.str();
    }
  }
  else
  {
    ssMessage.str("");
    ssMessage << "stat(" << errno << ") error [" << UNIX_SOCKET << "]:  " << strerror(errno);
    strError = ssMessage.str();
  }

  return bResult;
}
