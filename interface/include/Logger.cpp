// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Logger.cpp
// author     : Ben Kietzman
// begin      : 2022-05-17
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Logger"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Logger()
Logger::Logger(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "logger", argc, argv, pCallback)
{
  m_pLogger->useSingleSocket(true);
}
// }}}
// {{{ ~Logger()
Logger::~Logger()
{
}
// }}}
// {{{ callback()
void Logger::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Logger::callback()";
  if (!empty(ptJson, "Function"))
  {
    if (ptJson->m["Function"]->v == "log" || ptJson->m["Function"]->v == "message")
    {
      if (exist(ptJson, "Label"))
      {
        if (!empty(ptJson, "Message"))
        {
          map<string, string> label;
          ptJson->m["Label"]->flatten(label, true, false);
          if ((ptJson->m["Function"]->v == "log" && m_pLogger->log(label, ptJson->m["Message"]->v, strError)) || (ptJson->m["Function"]->v == "message" && m_pLogger->message(label, ptJson->m["Message"]->v, strError)))
          {
            bResult = true;
          }
        }
        else
        {
          string strJson;
          strError = "Please provide the Message.";
        }
      }
      else
      {
        strError = "Please provide the Label.";
      }
    }
    else
    {
      strError = "Please provide a valid Function:  log, message.";
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
