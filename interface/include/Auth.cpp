// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Auth.cpp
// author     : Ben Kietzman
// begin      : 2022-05-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Auth"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Auth()
Auth::Auth(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "auth", argc, argv, pCallback)
{
  m_pAnalyzeCallback = NULL;
}
// }}}
// {{{ ~Auth()
Auth::~Auth()
{
}
// }}}
// {{{ callback()
void Auth::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Auth::callback()";
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "User"))
  {
    if (!empty(ptJson, "Password"))
    {
      if (exist(ptJson, "Request"))
      {
        if (!empty(ptJson->m["Request"], "Interface"))
        {
          Json *ptData = new Json(ptJson);
          if (m_pWarden != NULL && m_pWarden->authz(ptData, strError))
          {
            if (exist(ptData, "radial") && exist(ptData->m["radial"], "Access") && exist(ptData->m["radial"]->m["Access"], ptJson->m["Request"]->m["Interface"]->v))
            {
              string strAccessFunction = "Function";
              if (m_accessFunctions.find(ptJson->m["Request"]->m["Interface"]->v) != m_accessFunctions.end() && m_accessFunctions[ptJson->m["Request"]->m["Interface"]->v] != "Function")
              {
                strAccessFunction = m_accessFunctions[ptJson->m["Request"]->m["Interface"]->v];
              }
              if (ptData->m["radial"]->m["Access"]->m[ptJson->m["Request"]->m["Interface"]->v]->v == "all")
              {
                bResult = true;
              }
              else if (!empty(ptJson, strAccessFunction))
              {
                if (ptData->m["radial"]->m["Access"]->m[ptJson->m["Request"]->m["Interface"]->v]->v == ptJson->m[strAccessFunction]->v)
                {
                  bResult = true;
                }
                else
                {
                  for (auto &access : ptData->m["radial"]->m["Access"]->m[ptJson->m["Request"]->m["Interface"]->v]->l)
                  {
                    if (access->v == ptJson->m[strAccessFunction]->v)
                    {
                      bResult = true;
                    }
                  }
                }
              }
            }
            if (!bResult)
            {
              if (m_pAnalyzeCallback != NULL)
              {
                bResult = m_pAnalyzeCallback(strPrefix, ptJson, ptData, strError);
              }
              else
              {
                strError = "Authorization denied.";
              }
            }
          }
          else if (m_pWarden == NULL)
          {
            strError = "Please initialize Warden.";
          }
          delete ptData;
        }
        else
        {
          strError = "Please provide the Interface within the Request.";
        }
      }
      else
      {
        strError = "Please provide the Request.";
      }
    }
    else
    {
      strError = "Please provide the Password.";
    }
  }
  else
  {
    strError = "Please provide the User.";
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
// {{{ init()
bool Auth::init()
{
  bool bResult = false;
  string strError;
  Json *ptJson = new Json;

  ptJson->i("Function", "list");
  if (hub(ptJson, strError))
  {
    if (exist(ptJson, "Response"))
    {
      bResult = true;
      for (auto &interface : ptJson->m["Response"]->m)
      {
        m_accessFunctions[interface.first] = ((!empty(interface.second, "AccessFunction"))?interface.second->m["AccessFunction"]->v:"Function");
      }
    }
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ setAnalyze()
void Auth::setAnalyze(bool (*pCallback)(string, Json *, Json *, string &))
{
  m_pAnalyzeCallback = pCallback;
}
// }}}
}
}
