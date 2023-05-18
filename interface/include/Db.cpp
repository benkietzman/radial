// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Db.cpp
// author     : Ben Kietzman
// begin      : 2023-05-18
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Db"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Db()
Db::Db(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "db", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Db()
Db::~Db()
{
}
// }}}
// {{{ callback()
void Db::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;

  threadIncrement();
  strPrefix += "->Db::callback()";
  if (!empty(ptJson, "Function"))
  {
    if (exist(ptJson, "Request"))
    {
      bool bInvalid = false;
      string strID, strQuery;
      if (exist(ptJson, "Response"))
      {
        delete ptJson->m["Response"];
      }
      ptJson->m["Response"] = new Json;
      if (m_pCallbackAddon != NULL && m_pCallbackAddon(ptJson->m["Function"]->v, ptJson->m["Request"], ptJson->m["Response"], strID, strQuery, strError, bInvalid))
      {
        bResult = true;
      }
      else if (bInvalid)
      {
        if (m_functions.find(ptJson->m["Function"]->v) != m_functions.end())
        {
          if ((this->*m_functions[ptJson->m["Function"]->v])(ptJson->m["Request"], ptJson->m["Response"], strID, strQuery, strError))
          {
            bResult = true;
            if (!strID.empty())
            {
              ptJson->i("ID", strID, 'n');
            }
            if (!strQuery.empty())
            {
              ptJson->i("Query", strQuery);
            }
          }
        }
        else
        {
          strError = "Please provide a valid Function.";
        }
      }
    }
    else
    {
      strError = "Please provide the Request.";
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
// {{{ setCallbackAddon()
void Db::setCallbackAddon(bool (*pCallback)(const string, Json *, Json *, string &, string &, string &, bool &))
{
  m_pCallbackAddon = pCallback;
}
// }}}
}
}
