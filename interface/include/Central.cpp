// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Central.cpp
// author     : Ben Kietzman
// begin      : 2023-02-18
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Central"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Central()
Central::Central(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "central", argc, argv, pCallback)
{
  string strError;

  m_functions["accountType"] = &Central::accountType;
  m_functions["accountTypes"] = &Central::accountTypes;
  m_ptCred = new Json;
  if (m_pWarden != NULL)
  {
    m_pWarden->vaultRetrieve({"radial", "radial"}, m_ptCred, strError);
  }
}
// }}}
// {{{ ~Central()
Central::~Central()
{
  delete m_ptCred;
}
// }}}
// {{{ accountType()
bool Central::accountType(string strPrefix, Json *ptRequest, Json *ptResponse, string &strError)
{
  bool bResult = false;
  stringstream ssQuery;

  strPrefix += "->Central::accountType()";
  if (ptRequest->m.find("id") != ptRequest->m.end() && !ptRequest->m["id"]->v.empty())
  {
    ssQuery << "select id, type, description from account_type where id = " << ptRequest->m["id"]->v;
    auto getAccountType = dbquery("central_r", ssQuery.str(), strError);
    if (getAccountType != NULL)
    {
      if (!getAccountType->empty())
      {
        Json *ptGetAccountType = new Json(getAccountType->front());
        bResult = true;
        ptResponse->merge(ptGetAccountType, true, false);
        delete ptGetAccountType;
      }
    }
    dbfree(getAccountType);
  }
  else
  {
    strError = "Please provide the id.";
  }

  return bResult;
}
// }}}
// {{{ accountTypes()
bool Central::accountTypes(string strPrefix, Json *ptRequest, Json *ptResponse, string &strError)
{
  bool bResult = false;
  stringstream ssQuery;

  strPrefix += "->Central::accountTypes()";
  ssQuery << "select id, type, description from account_type order by type";
  auto getAccountType = dbquery("central_r", ssQuery.str(), strError);
  if (getAccountType != NULL)
  {
    bResult = true;
    for (auto &getAccountTypeRow : *getAccountType)
    {
      ptResponse->pb(getAccountTypeRow);
    }
  }
  dbfree(getAccountType);

  return bResult;
}
// }}}
// {{{ callback()
void Central::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssQuery;

  threadIncrement();
  strPrefix += "->Central::callback()";
  if (ptJson->m.find("Request") != ptJson->m.end())
  {
    if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
    {
      string strFunction = ptJson->m["Function"]->v;
      if (m_functions.find(ptJson->m["Function"]->v) != m_functions.end())
      {
        if (ptJson->m.find("Response") != ptJson->m.end())
        {
          delete ptJson->m["Response"];
        }
        ptJson->m["Response"] = new Json;
        if ((this->*m_functions[ptJson->m["Function"]->v])(strPrefix, ptJson->m["Request"], ptJson->m["Response"], strError))
        {
          bResult = true;
        }
      }
      else
      {
        strError = "Please provide a valid Function.";
      }
    }
    else
    {
      strError = "Please provide the Function.";
    }
  }
  else
  {
    strError = "Please provide the Request.";
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
