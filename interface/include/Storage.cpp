// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Storage.cpp
// author     : Ben Kietzman
// begin      : 2022-04-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Storage"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Storage()
Storage::Storage(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "storage", argc, argv, pCallback)
{
  m_bInitialized = false;
}
// }}}
// {{{ ~Storage()
Storage::~Storage()
{
}
// }}}
// {{{ autoMode()
void Storage::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->Storage::autoMode()";
  if (!m_bInitialized)
  {
    mutexInitialize.lock();
    if (!m_bInitialized && !strNewMaster.empty() && m_strNode != strNewMaster)
    {
      string strError;
      stringstream ssMessage;
      Json *ptJson = new Json;
      ptJson->i("Interface", "storage");
      ptJson->i("Function", "retrieve");
      ptJson->i("Node", strNewMaster);
      ptJson->m["Keys"] = new Json;
      if (hub("link", ptJson, strError))
      {
        Json *ptData;
        ssMessage.str("");
        ssMessage << strPrefix << "->hub(link,storage,retrieve) [" << strNewMaster << "]:  Retrieved initial storage.";
        log(ssMessage.str());
        ptSubJson->i("Interface", "storage");
        ptSubJson->i("Function", "add");
        ptSubJson->m["Keys"] = new Json;
        if (ptJson->m.find("Response") != ptJson->m.end())
        {
          ptData = ptJson->m["Response"];
          ptJson->m.erase("Response");
        }
        else
        {
          ptData = new Json;
        }
        if (m_storage.add([], ptData, strError))
        {
          m_bInitialized = true;
          ssMessage.str("");
          ssMessage << strPrefix << "->Storage::add():  Initialized storage.";
          log(ssMessage.str());
        }
        else
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->Storage::add():  " << strError << " --- " << ptSubJson;
          log(ssMessage.str());
        }
        delete ptData;
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->hub(link,storage,retrieve) error [" << strNewMaster << "]:  " << strError;
        log(ssMessage.str());
      }
      delete ptJson;
    }
    mutexInitialize.unlock();
  }
  threadDecrement();
}
// }}}
// {{{ callback()
void Storage::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Storage::callback()";
  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
  {
    list<string> keys;
    Json *ptData = NULL;
    if (ptJson->m.find("Keys") != ptJson->m.end())
    {
      for (auto &ptKey : ptJson->m["Keys"]->l)
      {
        keys.push_back(ptKey->v);
      }
    }
    if (ptJson->m["Function"]->v == "add" || ptJson->m["Function"]->v == "update")
    {
      if (ptJson->m.find("Request") != ptJson->m.end())
      {
        ptData = new Json(ptJson->m["Request"]);
      }
      else
      {
        ptData = new Json;
      }
    }
    else if (ptJson->m["Function"]->v == "retrieve" || ptJson->m["Function"]->v == "retrieveKeys")
    {
      ptData = new Json;
    }
    if (m_storage.request(ptJson->m["Function"]->v, keys, ptData, strError))
    {
      bResult = true;
    }
    if (ptData != NULL)
    {
      if (ptJson->m["Function"]->v == "add" || ptJson->m["Function"]->v == "update")
      {
        delete ptData;
      }
      else
      {
        if (ptJson->m.find("Response") != ptJson->m.end())
        {
          delete ptJson->m["Response"];
        }
        ptJson->m["Response"] = ptData;
      }
    }
    if (bResult && (ptJson->m["Function"]->v == "add" || ptJson->m["Function"]->v == "remove" || ptJson->m["Function"]->v == "update") && (ptJson->m.find("Broadcast") == ptJson->m.end() || ptJson->m["Broadcast"]->v == "1"))
    {
      Json *ptLink = new Json;
      ptLink->i("Interface", "storage");
      ptLink->i("Function", ptJson->m["Function"]->v);
      ptLink->i("Broadcast", "0", '0');
      ptLink->i("Keys", keys);
      if (ptJson->m["Function"]->v != "remove" && ptJson->m.find("Request") != ptJson->m.end())
      {
        ptLink->m["Request"] = new Json(ptJson->m["Request"]);
      }
      hub("link", ptLink, false);
      delete ptLink;
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
