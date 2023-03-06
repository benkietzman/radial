// -*- C++ -*-
// Radial
// -------------------------------------
// file       : CentralMon.cpp
// author     : Ben Kietzman
// begin      : 2023-03-03
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "CentralMon"
// }}}
extern "C++"
{
namespace radial
{
// {{{ CentralMon()
CentralMon::CentralMon(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "centralmon", argc, argv, pCallback)
{
  m_strPort = "4636";
  m_strServer = "localhost";
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-p" || (strArg.size() > 7 && strArg.substr(0, 7) == "--port="))
    {
      if (strArg == "-p" && i + 1 < argc && argv[i+1][0] != '-')
      {
        m_strPort = argv[++i];
      }
      else
      {
        m_strPort = strArg.substr(7, strArg.size() - 7);
      }
      m_manip.purgeChar(m_strPort, m_strPort, "'");
      m_manip.purgeChar(m_strPort, m_strPort, "\"");
    }
    else if (strArg == "-s" || (strArg.size() > 9 && strArg.substr(0, 9) == "--server="))
    {
      if (strArg == "-s" && i + 1 < argc && argv[i+1][0] != '-')
      {
        m_strServer = argv[++i];
      }
      else
      {
        m_strServer = strArg.substr(9, strArg.size() - 9);
      }
      m_manip.purgeChar(m_strServer, m_strServer, "'");
      m_manip.purgeChar(m_strServer, m_strServer, "\"");
    }
  }
  // }}}
}
// }}}
// {{{ ~CentralMon()
CentralMon::~CentralMon()
{
}
// }}}
// {{{ callback()
void CentralMon::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->CentralMon::callback()";
  if (!empty(ptJson, "Function"))
  {
    bool bValid = false;
    string strFunction = ptJson->m["Function"]->v;
    stringstream ssRequest;
    ssRequest << ptJson->m["Function"]->v;
    if (strFunction == "process")
    {
      if (!empty(ptJson, "Server"))
      {
        ssRequest << " " << ptJson->m["Server"]->v;
        if (!empty(ptJson, "Process"))
        {
          bValid = true;
          ssRequest << " " << ptJson->m["Process"]->v << endl;
        }
        else
        {
          strError = "Please provide the Process.";
        }
      }
      else
      {
        strError = "Please provide the Server.";
      }
    }
    else if (strFunction == "system")
    {
      if (!empty(ptJson, "Server"))
      {
        bValid = true;
        ssRequest << " " << ptJson->m["Server"]->v;
      }
      else
      {
        strError = "Please provide the Server.";
      }
    }
    else if (strFunction == "update")
    {
      bValid = true;
    }
    else
    {
      strError = "Please provide a valid Function:  process, system, update.";
    }
    if (bValid)
    {
      int fdSocket = -1;
      if (m_pUtility->connect(m_strServer, m_strPort, fdSocket, strError))
      {
        bool bExit = false;
        int nReturn;
        size_t unPosition;
        string strBuffers[2];
        strBuffers[1] = ssRequest.str();
        while (!bExit && !shutdown())
        {
          pollfd fds[1];
          fds[0].fd = fdSocket;
          fds[0].events = POLLIN;
          if (!strBuffers[1].empty())
          {
            fds[0].events |= POLLOUT;
          }
          if ((nReturn = poll(fds, 1, 500)) > 0)
          {
            if (fds[0].revents & POLLIN)
            {
              if (m_pUtility->fdRead(fdSocket, strBuffers[0], nReturn))
              {
                if ((unPosition = strBuffers[0].find("\n")) != string::npos)
                {
                  bExit = true;
                  if (strFunction == "process")
                  {
                    string strItem;
                    stringstream ssLine(strBuffers[0].substr(0, unPosition));
                    vector<string> items;
                    while (getline(ssLine, strItem, ';'))
                    {
                      items.push_back(strItem);
                    }
                    if (items.size() == 10)
                    {
                      size_t unIndex = 0;
                      stringstream ssSubLine;
                      bResult = true;
                      ptJson->m["Response"] = new Json;
                      ptJson->m["Response"]->i("StartTime", items[unIndex++]);
                      ptJson->m["Response"]->m["Owners"] = new Json;
                      ssSubLine.str(items[unIndex++]);
                      while (getline(ssSubLine, strItem, ','))
                      {
                        ptJson->m["Response"]->m["Owners"]->pb(strItem);
                      }
                      ptJson->m["Response"]->i("NumberOfProcesses", items[unIndex++]);
                      ptJson->m["Response"]->i("ImageSize", items[unIndex++]);
                      ptJson->m["Response"]->i("MinImageSize", items[unIndex++]);
                      ptJson->m["Response"]->i("MaxImageSize", items[unIndex++]);
                      ptJson->m["Response"]->i("ResidentSize", items[unIndex++]);
                      ptJson->m["Response"]->i("MinResidentSize", items[unIndex++]);
                      ptJson->m["Response"]->i("MaxResidentSize", items[unIndex++]);
                      ptJson->m["Response"]->i("Alarms", items[unIndex++]);
                    }
                    else
                    {
                      strError = "Invalid number of fields returned.";
                    }
                  }
                  else if (strFunction == "system")
                  {
                    string strItem;
                    stringstream ssLine(strBuffers[0].substr(0, unPosition));
                    vector<string> items;
                    while (getline(ssLine, strItem, ';'))
                    {
                      items.push_back(strItem);
                    }
                    if (items.size() == 14)
                    {
                      size_t unIndex = 0;
                      stringstream ssSubLine;
                      bResult = true;
                      ptJson->m["Response"] = new Json;
                      ptJson->m["Response"]->i("Server", items[unIndex++]);
                      ptJson->m["Response"]->i("OperatingSystem", items[unIndex++]);
                      ptJson->m["Response"]->i("SystemRelease", items[unIndex++]);
                      ptJson->m["Response"]->i("NumberOfProcessors", items[unIndex++]);
                      ptJson->m["Response"]->i("CpuSpeed", items[unIndex++]);
                      ptJson->m["Response"]->i("NumberOfProcesses", items[unIndex++]);
                      ptJson->m["Response"]->i("CpuUsage", items[unIndex++]);
                      ptJson->m["Response"]->i("UpTime", items[unIndex++]);
                      ptJson->m["Response"]->i("MainUsed", items[unIndex++]);
                      ptJson->m["Response"]->i("MainTotal", items[unIndex++]);
                      ptJson->m["Response"]->i("SwapUsed", items[unIndex++]);
                      ptJson->m["Response"]->i("SwapTotal", items[unIndex++]);
                      ptJson->m["Response"]->m["Partitions"] = new Json;
                      ssSubLine.str(items[unIndex++]);
                      while (getline(ssSubLine, strItem, ','))
                      {
                        string strFirst, strSecond;
                        stringstream ssDeepLine(strItem);
                        getline(ssDeepLine, strFirst, '=');
                        getline(ssDeepLine, strSecond, '=');
                        ptJson->m["Response"]->m["Partitions"]->i(strFirst, strSecond);
                      }
                      ptJson->m["Response"]->i("Alarms", items[unIndex++]);
                    }
                    else
                    {
                      strError = "Invalid number of fields returned.";
                    }
                  }
                  else if (strBuffers[0].substr(0, unPosition) == "okay")
                  {
                    bResult = true;
                  }
                  else
                  {
                    strError = "Failed to update.";
                  }
                }
              }
              else
              {
                bExit = true;
                if (nReturn < 0)
                {
                  ssMessage.str("");
                  ssMessage << "Utility::fdRead(" << errno << ") " << strerror(errno);
                  strError = ssMessage.str();
                }
              }
            }
            if (fds[0].revents & POLLOUT)
            {
              if (!m_pUtility->fdWrite(fdSocket, strBuffers[1], nReturn))
              {
                bExit = true;
                if (nReturn < 0)
                {
                  ssMessage.str("");
                  ssMessage << "Utility::fdWrite(" << errno << ") " << strerror(errno);
                  strError = ssMessage.str();
                }
              }
            }
          }
          else
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << "poll(" << errno << ") " << strerror(errno);
            strError = ssMessage.str();
          }
        }
        close(fdSocket);
      }
    }
  }
  else
  {
    strError = "Please provide the Function.";
  }
  ptJson->i("Status", ((bResult)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->i("Error", strError);
  }
  if (bResponse)
  {
    hub(ptJson, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
}
}
