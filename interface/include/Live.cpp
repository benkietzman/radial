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
Live::Live(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "live", argc, argv, pCallback)
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
void Live::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;
  time_t CTime;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Live::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
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
  if (!empty(ptJson, "Function"))
  {
    if (ptJson->m["Function"]->v == "connect")
    {
      if (!empty(ptJson, "wsRequestID"))
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
      if (!empty(ptJson, "wsRequestID"))
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
      if (exist(ptJson, "Request"))
      {
        if (exist(ptJson->m["Request"], "Message"))
        {
          string strApplication, strUser;
          bResult = true;
          if (!empty(ptJson->m["Request"], "Application"))
          {
            strApplication = ptJson->m["Request"]->m["Application"]->v;
          }
          if (!empty(ptJson->m["Request"], "User"))
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
          for (auto &link : m_l)
          {
            if (link->interfaces.find("live") != link->interfaces.end() && link->interfaces.find("websocket") != link->interfaces.end())
            {
              string strSubError;
              Json *ptSubJson = new Json;
              ptSubJson->i("Interface", "live");
              ptSubJson->i("Node", link->strNode);
              ptSubJson->i("Function", "list");
              if (hub("link", ptSubJson, strSubError) && exist(ptSubJson, "Response"))
              {
                for (auto &conn : ptSubJson->m["Response"]->m)
                {
                  if ((strApplication.empty() || (exist(conn.second, "Application") && conn.second->m["Application"]->v == strApplication)) && (strUser.empty() || (exist(conn.second, "User") && conn.second->m["User"]->v == strUser)))
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
    ptJson->j(p.p);
    hub(p, false);
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

  if (m_i.find("websocket") != m_i.end())
  {
    Json *ptJson = new Json;
    ptJson->i("Function", "list");
    if (hub("websocket", ptJson, strError) && exist(ptJson, "Response") && exist(ptJson->m["Response"], strWsRequestID))
    {
      bResult = true;
      if (!empty(ptJson->m["Response"]->m[strWsRequestID], "Application"))
      {
        strApplication = ptJson->m["Response"]->m[strWsRequestID]->m["Application"]->v;
      }
      if (!empty(ptJson->m["Response"]->m[strWsRequestID], "User"))
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
