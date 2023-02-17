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
Auth::Auth(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "auth", argc, argv, pCallback)
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
void Auth::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;

  threadIncrement();
  strPrefix += "->Auth::callback()";
  if (ptJson->m.find("User") != ptJson->m.end() && !ptJson->m["User"]->v.empty())
  {
    if (ptJson->m.find("Password") != ptJson->m.end() && !ptJson->m["Password"]->v.empty())
    {
      if (ptJson->m.find("Request") != ptJson->m.end())
      {
        if (ptJson->m["Request"]->m.find("Interface") != ptJson->m["Request"]->m.end() && !ptJson->m["Request"]->m["Interface"]->v.empty())
        {
          Json *ptData = new Json(ptJson);
stringstream ssMessage;
ssMessage << strPrefix << "->Warden::authz():  " << ptData;
log(ssMessage.str());
          if (m_pWarden != NULL && m_pWarden->authz(ptData, strError))
          {
            if (ptData->m.find("radial") != ptData->m.end() && ptData->m["radial"]->m.find("Access") != ptData->m["radial"]->m.end() && ptData->m["radial"]->m["Access"]->m.find(ptJson->m["Request"]->m["Interface"]->v) != ptData->m["radial"]->m["Access"]->m.end())
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
              else if (ptJson->m.find(strAccessFunction) != ptJson->m.end() && !ptJson->m[strAccessFunction]->v.empty())
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
    hub(ptJson, false);
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
    if (ptJson->m.find("Response") != ptJson->m.end())
    {
      bResult = true;
      for (auto &interface : ptJson->m["Response"]->m)
      {
        m_accessFunctions[interface.first] = ((interface.second->m.find("AccessFunction") != interface.second->m.end() && !interface.second->m["AccessFunction"]->v.empty())?interface.second->m["AccessFunction"]->v:"Function");
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
