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
    // {{{ connect
    if (ptJson->m["Function"]->v == "connect")
    {
      if (!empty(ptJson, "wsRequestID"))
      {
        if (m_conns.find(ptJson->m["wsRequestID"]->v) == m_conns.end())
        {
          string strApplication, strNode, strUser;
          if (retrieve(ptJson->m["wsRequestID"]->v, strApplication, strUser))
          {
            map<string, string> getUser;
            Json *ptUser = new Json;
            data *ptData = new data;
            bResult = true;
            ptData->strApplication = strApplication;
            ptData->strUser = strUser;
            ptUser->i("userid", strUser);
            if (db("dbCentralUsers", ptUser, getUser, strError))
            {
              ptData->strFirstName = getUser["first_name"];
              ptData->strLastName = getUser["last_name"];
            }
            delete ptUser;
            m_conns[ptJson->m["wsRequestID"]->v] = ptData;
            message("", "", {{"Action", "connect"}, {"wsRequestID", ptJson->m["wsRequestID"]->v}, {"Application", strApplication}, {"User", strUser}, {"FirstName", ptData->strFirstName}, {"LastName", ptData->strLastName}});
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
    // }}}
    // {{{ disconnect
    else if (ptJson->m["Function"]->v == "disconnect")
    {
      if (!empty(ptJson, "wsRequestID"))
      {
        bResult = true;
        if (m_conns.find(ptJson->m["wsRequestID"]->v) != m_conns.end())
        {
          message("", "", {{"Action", "disconnect"}, {"wsRequestID", ptJson->m["wsRequestID"]->v}, {"Application", m_conns[ptJson->m["wsRequestID"]->v]->strApplication}, {"User", m_conns[ptJson->m["wsRequestID"]->v]->strUser}, {"FirstName", m_conns[ptJson->m["wsRequestID"]->v]->strFirstName}, {"LastName", m_conns[ptJson->m["wsRequestID"]->v]->strLastName}});
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
    // }}}
    // {{{ list
    else if (ptJson->m["Function"]->v == "list")
    {
      bResult = true;
      Json *ptResponse = new Json;
      for (auto &conn : m_conns)
      {
        map<string, string> getUser;
        Json *ptConn = new Json, *ptUser = new Json;
        ptConn->i("Application", conn.second->strApplication);
        ptConn->i("User", conn.second->strUser);
        ptUser->i("userid", conn.second->strUser);
        if (db("dbCentralUsers", ptUser, getUser, strError))
        {
          ptConn->i("FirstName", getUser["first_name"]);
          ptConn->i("LastName", getUser["last_name"]);
        }
        delete ptUser;
        ptResponse->m[conn.first] = ptConn;
      }
      if (exist(ptJson, "Request") && !empty(ptJson->m["Request"], "Scope") && ptJson->m["Request"]->m["Scope"]->v == "all")
      {
        list<string> nodes;
        m_mutexShare.lock();
        for (auto &link : m_l)
        {
          nodes.push_back(link->strNode);
        }
        m_mutexShare.unlock();
        for (auto &node : nodes)
        {
          Json *ptSubJson = new Json(ptJson);
          delete ptSubJson->m["Request"];
          ptSubJson->m.erase("Request");
          ptSubJson->i("Node", node);
          if (hub("link", ptSubJson, strError))
          {
            if (exist(ptSubJson, "Response"))
            {
              for (auto &item : ptSubJson->m["Response"]->m)
              {
                ptResponse->i(item.first, item.second);
              }
            }
          }
          delete ptSubJson;
        }
      }
      ptJson->m["Response"] = ptResponse;
    }
    // }}}
    // {{{ message
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
          message(strApplication, strUser, ptJson->m["Request"]->m["Message"]);
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
    // }}}
    // {{{ invalid
    else
    {
      strError = "Please provide a valid Function:  connect, disconnect, list, message.";
    }
    // }}}
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
// {{{ message()
void Live::message(const string strApplication, const string strUser, map<string, string> message)
{
  Json *ptMessage = new Json(message);

  Live::message(strApplication, strUser, ptMessage);
  delete ptMessage;
}
void Live::message(const string strApplication, const string strUser, Json *ptMessage)
{
  map<string, Json *> requests;
  for (auto &conn : m_conns)
  {
    if ((strApplication.empty() || conn.second->strApplication == strApplication) && (strUser.empty() || conn.second->strUser == strUser))
    {
      Json *ptSubJson = new Json(ptMessage);
      ptSubJson->i("wsRequestID", conn.first);
      hub("websocket", ptSubJson, false);
      delete ptSubJson;
    }
  }
  m_mutexShare.lock();
  for (auto &link : m_l)
  {
    if (link->interfaces.find("live") != link->interfaces.end() && link->interfaces.find("websocket") != link->interfaces.end())
    {
      Json *ptSubJson = new Json;
      ptSubJson->i("Interface", "live");
      ptSubJson->i("Node", link->strNode);
      ptSubJson->i("Function", "list");
      requests[link->strNode] = ptSubJson;
    }
  }
  m_mutexShare.unlock();
  for (auto &req : requests)
  {
    string strSubError;
    if (hub("link", req.second, strSubError) && exist(req.second, "Response"))
    {
      for (auto &conn : req.second->m["Response"]->m)
      {
        if ((strApplication.empty() || (exist(conn.second, "Application") && conn.second->m["Application"]->v == strApplication)) && (strUser.empty() || (exist(conn.second, "User") && conn.second->m["User"]->v == strUser)))
        {
          Json *ptDeepJson = new Json(ptMessage);
          ptDeepJson->i("Interface", "websocket");
          ptDeepJson->i("Node", req.first);
          ptDeepJson->i("wsRequestID", conn.first);
          hub("link", ptDeepJson, false);
          delete ptDeepJson;
        }
      }
    }
    delete req.second;
  }
}
// }}}
// {{{ retrieve()
bool Live::retrieve(const string strWsRequestID, string &strApplication, string &strUser)
{
  bool bWebSocket = false, bResult = false;
  string strError;

  m_mutexShare.lock();
  if (m_i.find("websocket") != m_i.end())
  {
    bWebSocket = true;
  }
  m_mutexShare.unlock();
  if (bWebSocket)
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
