// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Junction.cpp
// author     : Ben Kietzman
// begin      : 2022-05-18
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Junction"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Junction()
Junction::Junction(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "junction", argc, argv, pCallback)
{
  m_pJunction->useSingleSocket(true);
}
// }}}
// {{{ ~Junction()
Junction::~Junction()
{
}
// }}}
// {{{ callback()
void Junction::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Junction::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (exist(ptJson, "Request"))
  {
    list<string> in, out;
    string strJson;
    for (auto &i : ptJson->m["Request"]->l)
    {
      in.push_back(i->j(strJson));
    }
    if (m_pJunction->request(in, out, strError))
    {
      bResult = true;
      ptJson->m["Response"] = new Json;
      for (auto &i : out)
      {
        ptJson->m["Response"]->l.push_back(new Json(i));
      }
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
    ptJson->j(p.p);
    hub(p, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
}
}
