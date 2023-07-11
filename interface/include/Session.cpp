// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Session.cpp
// author     : Ben Kietzman
// begin      : 2023-07-11
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Session"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Session()
Session::Session(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "session", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Session()
Session::~Session()
{
}
// }}}
// {{{ callback()
void Session::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage, strQuery;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Session::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    if (exist(ptJson, "Request"))
    {
      if (!empty(ptJson->m["Request"], "ID"))
      {
        // {{{ destroy
        if (ptJson->m["Function"]->v == "destroy")
        {
          ssQuery.str("");
          ssQuery << "delete from php_session where session_id = '" << esc(ptJson->m["Request"]->m["ID"]->v) << "'";
          if (dbupdate("central", ssQuery.str(), strError))
          {
            bResult = true;
          }
        }
        // }}}
        // {{{ read
        else if (ptJson->m["Function"]->v == "read")
        {
          ssQuery.str("");
          ssQuery << "select session_data Data, session_json Json from php_session where session_id = '" << esc(ptJson->m["Request"]->m["ID"]->v) << "' and session_data is not null and session_data != ''";
          auto getSession = dbquery("central_r", ssQuery.str(), strError);
          if (getSession != NULL)
          {
            bResult = true;
            if (!getSession->empty())
            {
              ptJson->insert("Response", getSession->front());
            }
          }
          dbfree(getSession);
        }
        // }}}
        // {{{ write
        else if (ptJson->m["Function"]->v == "write")
        {
          if (exist(ptJson->m["Request"], "Data"))
          {
            if (exist(ptJson->m["Request"], "Json"))
            {
              ssQuery.str("");
              ssQuery << "insert into php_session (session_id, last_updated, session_data, session_json) values ('" << esc(ptJson->m["Request"]->m["ID"]->v) << "', now(), '" << esc(ptJson->m["Request"]->m["Data"]->v) << "', '" << esc(ptJson->m["Request"]->m["Json"]->v) << "') on duplicate key update last_updated = now(), session_data = '" << esc(ptJson->m["Request"]->m["Data"]->v) << "', session_json = '" << esc(ptJson->m["Request"]->m["Json"]->v) << "'";
              if (dbupdate("central", ssQuery.str(), strError))
              {
                bResult = true;
              }
            }
            else
            {
              strError = "Please provide the Json in the Request.";
            }
          }
          else
          {
            strError = "Please provide the Data in the Request.";
          }
        }
        // }}}
        // {{{ invalid
        else
        {
          strError = "Please provide a valid Function:  destroy, read, write.";
        }
        // }}}
      }
      else
      {
        strError = "Please provide the ID in the Request.";
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
    ptJson->j(p.p);
    hub(p, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
}
}
