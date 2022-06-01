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
Auth::Auth(string strPrefix, int argc, char **argv, function<void(string, Json *, const bool)> callback) : Interface(strPrefix, "auth", argc, argv, callback)
{
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-w" || (strArg.size() > 9 && strArg.substr(0, 9) == "--warden="))
    {
      if (strArg == "-w" && i + 1 < argc && argv[i+1][0] != '-')
      {
        m_strWarden = argv[++i];
      }
      else
      {
        m_strWarden = strArg.substr(9, strArg.size() - 9);
      }
      m_manip.purgeChar(m_strWarden, m_strWarden, "'");
      m_manip.purgeChar(m_strWarden, m_strWarden, "\"");
    }
  }
  // }}}
  m_pWarden = NULL;
}
// }}}
// {{{ ~Auth()
Auth::~Auth()
{
  if (m_pWarden != NULL)
  {
    delete m_pWarden;
  }
}
// }}}
// {{{ callback()
void Auth::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Auth::callback()";
  if (ptJson->m.find("User") != ptJson->m.end() && !ptJson->m["User"]->v.empty())
  {
    if (ptJson->m.find("Password") != ptJson->m.end() && !ptJson->m["Password"]->v.empty())
    {
      if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
      {
        Json *ptData = new Json(ptJson);
        if (m_pWarden->authz(ptData, strError))
        {
          if (ptData->m.find("radial") != ptData->m.end() && ptData->m["radial"]->m.find("Access") != ptData->m["radial"]->m.end() && ptData->m["radial"]->m["Access"]->m.find(ptJson->m["Interface"]->v) != ptData->m["radial"]->m["Access"]->m.end())
          {
            string strAccessFunction = "Function";
            if (m_accessFunctions.find(ptJson->m["Interface"]->v) != m_accessFunctions.end() && m_accessFunctions[ptJson->m["Interface"]->v] != "Function")
            {
              strAccessFunction = m_accessFunctions[ptJson->m["Interface"]->v];
            }
            if (ptData->m["radial"]->m["Access"]->m[ptJson->m["Interface"]->v]->v == "all")
            {
              bResult = true;
            }
            else if (ptJson->m.find(strAccessFunction) != ptJson->m.end() && !ptJson->m[strAccessFunction]->v.empty())
            {
              if (ptData->m["radial"]->m["Access"]->m[ptJson->m["Interface"]->v]->v == ptJson->m[strAccessFunction]->v)
              {
                bResult = true;
              }
              else
              {
                for (auto &access : ptData->m["radial"]->m["Access"]->m[ptJson->m["Interface"]->v]->l)
                {
                  if (access->v == ptJson->m[strAccessFunction]->v)
                  {
                    bResult = true;
                  }
                }
                if (!bResult)
                {
                  strError = "Authorization denied.";
                }
              }
            }
            else
            {
              strError = "Authorization denied.";
            }
          }
          else
          {
            strError = "Authorization denied.";
          }
        }
        delete ptData;
      }
      else
      {
        strError = "Please provide the Interface.";
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
  ptJson->insert("Status", ((bResult)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->insert("Error", strError);
  }
  if (bResponse)
  {
    response(ptJson);
  }
}
// }}}
// {{{ init()
bool Auth::init()
{
  bool bResult = false;
  string strError;

  if (!m_strWarden.empty())
  {
    Json *ptJson = new Json;
    ptJson->insert("Function", "list");
    if (target(ptJson, strError))
    {
      if (ptJson->m.find("Response") != ptJson->m.end())
      {
        for (auto &interface : ptJson->m["Response"]->m)
        {
          m_accessFunctions[interface.first] = ((interface.second->m.find("AccessFunction") != interface.second->m.end() && !interface.second->m["AccessFunction"]->v.empty())?interface.second->m["AccessFunction"]->v:"Function");
        }
        m_pWarden = new Warden("Radial", m_strWarden, strError);
        if (strError.empty())
        {
          bResult = true;
        }
        else
        {
          delete m_pWarden;
          m_pWarden = NULL;
        }
      }
    }
    delete ptJson;
  }

  return bResult;
}
// }}}
}
}
