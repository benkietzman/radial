// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Live.cpp
// author     : Ben Kietzman
// begin      : 2023-01-18
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Live"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Live()
Live::Live(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "live", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Live()
Live::~Live()
{
  for (auto &conn : m_conns)
  {
    delete conn.second;
  }
}
// }}}
// {{{ callback()
void Live::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;
  time_t CTime;

  threadIncrement();
  strPrefix += "->Live::callback()";
  m_mutex.lock();
  time(&CTime);
  if ((CTime - m_CTime) > 600)
  {
    list<string> removals;
    string strApplication, strUser;
    m_CTime = CTime;
    for (auto &conn : m_conns)
    {
      if (!retrieve(conn.first, strApplication, strUser))
      {
        removals.push_back(conn.first);
      }
    }
    for (auto &removal : removals)
    {
      delete m_conns[removal];
      m_conns.erase(removal);
    }
  }
  m_mutex.unlock();
  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
  {
    if (ptJson->m["Function"]->v == "connect")
    {
      if (ptJson->m.find("wsRequestID") != ptJson->m.end() && !ptJson->m["wsRequestID"]->v.empty())
      {
        if (m_conns.find(ptJson->m["wsRequestID"]->v) == m_conns.end())
        {
          string strApplication, strNode, strUser;
          if (retrieve(ptJson->m["wsRequestID"]->v, strApplication, strUser))
          {
            data *ptData = new data;
            bResult = true;
            ptData->strApplication = strApplication;
            ptData->strUser = strUser;
            m_conns[ptJson->m["wsRequestID"]->v] = ptData;
          }
          else
          {
            strError = "Please provide a valid wsRequestID.";
          }
        }
        else
        {
          bResult = true;
          strError = "Already connected.";
        }
      }
      else
      {
        strError = "Please provide the wsRequestID.";
      }
    }
    else if (ptJson->m["Function"]->v == "disconnect")
    {
      if (ptJson->m.find("wsRequestID") != ptJson->m.end() && !ptJson->m["wsRequestID"]->v.empty())
      {
        bResult = true;
        if (m_conns.find(ptJson->m["wsRequestID"]->v) != m_conns.end())
        {
          delete m_conns[ptJson->m["wsRequestID"]->v];
          m_conns.erase(ptJson->m["wsRequestID"]->v);
        }
        else
        {
          strError = "Not connected.";
        }
      }
      else
      {
        strError = "Please provide the wsRequestID.";
      }
    }
    else if (ptJson->m["Function"]->v == "list")
    {
      bResult = true;
      ptJson->m["Response"] = new Json;
      for (auto &conn : m_conns)
      {
        Json *ptConn = new Json;
        ptConn->i("Application", conn.second->strApplication);
        ptConn->i("User", conn.second->strUser);
        ptJson->m["Response"]->m[conn.first] = ptConn;
      }
    }
    else if (ptJson->m["Function"]->v == "message")
    {
      if (ptJson->m.find("Request") != ptJson->m.end())
      {
        if (ptJson->m["Request"]->m.find("Message") != ptJson->m["Request"]->m.end())
        {
          string strApplication, strUser;
          bResult = true;
          if (ptJson->m["Request"]->m.find("Application") != ptJson->m["Request"]->m.end() && !ptJson->m["Request"]->m["Application"]->v.empty())
          {
            strApplication = ptJson->m["Request"]->m["Application"]->v;
          }
          if (ptJson->m["Request"]->m.find("User") != ptJson->m["Request"]->m.end() && !ptJson->m["Request"]->m["User"]->v.empty())
          {
            strUser = ptJson->m["Request"]->m["User"]->v;
          }
          for (auto &conn : m_conns)
          {
            if ((strApplication.empty() || conn.second->strApplication == strApplication) && (strUser.empty() || conn.second->strUser == strUser))
            {
              Json *ptSubJson = new Json(ptJson->m["Request"]->m["Message"]);
              ptSubJson->i("wsRequestID", conn.first);
              hub("websocket", ptSubJson, false);
              delete ptSubJson;
            }
          }
          for (auto &link : m_links)
          {
            if (link->interfaces.find("live") != link->interfaces.end() && link->interfaces.find("websocket") != link->interfaces.end())
            {
              string strSubError;
              Json *ptSubJson = new Json;
              ptSubJson->i("Interface", "live");
              ptSubJson->i("Node", link->strNode);
              ptSubJson->i("Function", "list");
              if (hub("link", ptSubJson, strSubError) && ptSubJson->m.find("Response") != ptSubJson->m.end())
              {
                for (auto &conn : ptSubJson->m["Response"]->m)
                {
                  if ((strApplication.empty() || (conn.second->m.find("Application") != conn.second->m.end() && conn.second->m["Application"]->v == strApplication)) && (strUser.empty() || (conn.second->m.find("User") != conn.second->m.end() && conn.second->m["User"]->v == strUser)))
                  {
                    Json *ptDeepJson = new Json(ptJson->m["Request"]->m["Message"]);
                    ptDeepJson->i("Interface", "websocket");
                    ptDeepJson->i("Node", link->strNode);
                    ptDeepJson->i("wsRequestID", conn.first);
                    hub("link", ptDeepJson, false);
                    delete ptDeepJson;
                  }
                }
              }
              delete ptSubJson;
            }
          }
        }
        else
        {
          strError = "Please provide the Message within the Request.";
        }
      }
      else
      {
        strError = "Please provide the Request.";
      }
    }
    else
    {
      strError = "Please provide a valid Function:  connect, disconnect, list, message.";
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
// {{{ retrieve()
bool Live::retrieve(const string strWsRequestID, string &strApplication, string &strUser)
{
  bool bResult = false;
  string strError;

  if (m_interfaces.find("websocket") != m_interfaces.end())
  {
    Json *ptJson = new Json;
    ptJson->i("Function", "list");
    if (hub("websocket", ptJson, strError) && ptJson->m.find("Response") != ptJson->m.end() && ptJson->m["Response"]->m.find(strWsRequestID) != ptJson->m["Response"]->m.end())
    {
      bResult = true;
      if (ptJson->m["Response"]->m[strWsRequestID]->m.find("Application") != ptJson->m["Response"]->m[strWsRequestID]->m.end() && !ptJson->m["Response"]->m[strWsRequestID]->m["Application"]->v.empty())
      {
        strApplication = ptJson->m["Response"]->m[strWsRequestID]->m["Application"]->v;
      }
      if (ptJson->m["Response"]->m[strWsRequestID]->m.find("User") != ptJson->m["Response"]->m[strWsRequestID]->m.end() && !ptJson->m["Response"]->m[strWsRequestID]->m["User"]->v.empty())
      {
        strUser = ptJson->m["Response"]->m[strWsRequestID]->m["User"]->v;
      }
    }
    delete ptJson;
  }

  return bResult;
}
// }}}
}
}
