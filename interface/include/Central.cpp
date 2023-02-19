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
// {{{ callback()
void Central::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;

  threadIncrement();
  strPrefix += "->Central::callback()";
  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
  {
    if (m_ptCred->m.find("Password") != m_ptCred->m.end() && !m_ptCred->m["Password"]->v.empty())
    {
      string strJwt, strSessionID;
      if (ptJson->m.find("wsJwt") != ptJson->m.end() && !ptJson->m["wsJwt"]->v.empty())
      {
        strJwt = ptJson->m["wsJwt"]->v;
      }
      else if (ptJson->m.find("wsSessionID") != ptJson->m.end() && !ptJson->m["wsSessionID"]->v.empty())
      {
        strSessionID = ptJson->m["wsSessionID"]->v;
      }
      ptJson->m["Response"] = new Json;
      if (m_pCentral->request("radial", m_ptCred->m["Password"]->v, "radial", "central", ptJson->m["Function"]->v, strJwt, strSessionID, ptJson, ptJson->m["Response"], strError))
      {
        bResult = true;
      }
    }
    else
    {
      strError = "Missing credentials.";
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
