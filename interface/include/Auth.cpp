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
  string strWarden;
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-w" || (strArg.size() > 9 && strArg.substr(0, 9) == "--warden="))
    {
      if (strArg == "-w" && i + 1 < argc && argv[i+1][0] != '-')
      {
        strWarden = argv[++i];
      }
      else
      {
        strWarden = strArg.substr(9, strArg.size() - 9);
      }
      m_manip.purgeChar(strWarden, strWarden, "'");
      m_manip.purgeChar(strWarden, strWarden, "\"");
    }
  }
  // }}}
  m_ptWarden = NULL;
  if (!strWarden.empty())
  {
    m_ptWarden = new Warden("Radial", strWarden, m_strError);
    if (!m_strError.empty())
    {
      delete m_ptWarden;
      m_ptWarden = NULL;
    }
  }
  else
  {
    m_strError = "Please provide the path to the Warden socket.";
  }
}
// }}}
// {{{ ~Auth()
Auth::~Auth()
{
  if (m_ptWarden != NULL)
  {
    delete m_ptWarden;
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
  if (m_ptWarden != NULL)
  {
    if (ptJson->m.find("User") != ptJson->m.end() && !ptJson->m["User"]->v.empty())
    {
      if (ptJson->m.find("Password") != ptJson->m.end() && !ptJson->m["Password"]->v.empty())
      {
        if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
        {
          if (m_ptWarden->authz(ptJson, strError))
          {
            if (ptJson->m.find("radial") != ptJson->m.end() && ptJson->m["radial"]->m.find("Access") != ptJson->m["radial"]->m.end() && ptJson->m["radial"]->m["Access"]->m.find(ptJson->m["Interface"]->v) != ptJson->m["radial"]->m["Access"]->m.end())
            {
              if (ptJson->m["radial"]->m["Access"]->m[ptJson->m["Interface"]->v]->v == "all")
              {
                bResult = true;
              }
              else if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
              {
                if (ptJson->m["radial"]->m["Access"]->m[ptJson->m["Interface"]->v]->v == ptJson->m["Function"]->v)
                {
                  bResult = true;
                }
                else
                {
                  bool bFound = false;
                  for (auto &access : ptJson->m["radial"]->m["Access"]->m[ptJson->m["Interface"]->v]->l)
                  {
                    if (access->v == ptJson->m["Function"]->v)
                    {
                      bFound = true;
                    }
                  }
                  if (bFound)
                  {
                    bResult = true;
                  }
                  else
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
  }
  else if (!m_strError.empty())
  {
    strError = m_strError;
  }
  else
  {
    strError = "Encountered an unknown error.";
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
}
}
