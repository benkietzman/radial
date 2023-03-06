// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Log.cpp
// author     : Ben Kietzman
// begin      : 2022-04-19
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Log"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Log()
Log::Log(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "log", argc, argv, pCallback)
{
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-e" || (strArg.size() > 8 && strArg.substr(0, 8) == "--email="))
    {
      if (strArg == "-e" && i + 1 < argc && argv[i+1][0] != '-')
      {
        m_strEmail = argv[++i];
      }
      else
      {
        m_strEmail = strArg.substr(8, strArg.size() - 8);
      }
      m_manip.purgeChar(m_strEmail, m_strEmail, "'");
      m_manip.purgeChar(m_strEmail, m_strEmail, "\"");
    }
  }
  // }}}
  m_pCentral->setEmail(m_strEmail);
  m_pCentral->setLog(m_strData, "radial_", "monthly", true, true);
}
// }}}
// {{{ ~Log()
Log::~Log()
{
}
// }}}
// {{{ callback()
void Log::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Log::callback()";
  if (!empty(ptJson, "Function"))
  {
    if (!empty(ptJson, "Message"))
    {
      if (ptJson->m["Function"]->v == "alert")
      {
        if (m_pCentral->alert(ptJson->m["Message"]->v, strError))
        {
          bResult = true;
        }
      }
      else if (ptJson->m["Function"]->v == "log")
      {
        if (m_pCentral->log(ptJson->m["Message"]->v, strError))
        {
          bResult = true;
        }
      }
      else if (ptJson->m["Function"]->v == "notify")
      {
        if (m_pCentral->notify("", ptJson->m["Message"]->v, strError))
        {
          bResult = true;
        }
      }
      else
      {
        strError = "Please provide a valid Function:  alert, log, notify.";
      }
    }
    else
    {
      strError = "Please provide the Message.";
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
