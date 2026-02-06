// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Emulator.cpp
// author     : Ben Kietzman
// begin      : 2026-02-05
// copyright  : Ben Emulator
// email      : ben@kietzman.org
// {{{ includes
#include "Emulator"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Emulator()
Emulator::Emulator(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "emulator", argc, argv, pCallback)
{
  string strError;

  m_functions["action"] = &Emulator::action;
  m_functions["data"] = &Emulator::data;
  m_functions["launch"] = &Emulator::launch;
  m_functions["status"] = &Emulator::status;
}
// }}}
// {{{ ~Emulator()
Emulator::~Emulator()
{
}
// }}}
// {{{ callback()
void Emulator::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Emulator::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    string strFunction = ptJson->m["Function"]->v;
    radialUser d;
    userInit(ptJson, d);
    if (m_functions.find(strFunction) != m_functions.end())
    {
      if ((this->*m_functions[strFunction])(d, strError))
      {
        bResult = true;
      }
    }
    else
    {
      strError = "Please provide a valid Function.";
    }
    if (bResult)
    {
      if (exist(ptJson, "Response"))
      {
        delete ptJson->m["Response"];
      }
      ptJson->m["Response"] = d.p->m["o"];
      d.p->m.erase("o");
    }
    userDeinit(d);
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
}
// }}}
// {{{ data()
bool Emulator::data(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (isValid(d, "Emulator"))
  {
    if (!empty(d.r, "wsRequestID"))
    {
      string w = d.r->m["wsRequestID"]->v;
      if (dep({"Data"}, i, e))
      {
        string v = i->m["Data"]->v;
        Json *m = new Json;
        m->i("Function", "data");
        m->i("Data", v);
        m->i("wsRequestID", w);
        if (application("Emulator", m, e))
        {
          b = true;
        }
        delete m;
      }
    }
    else
    {
      e = "Please provide the wsRequestID.";
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ launch()
bool Emulator::launch(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (isValid(d, "Emulator"))
  {
    if (!empty(d.r, "wsRequestID"))
    {
      string w = d.r->m["wsRequestID"]->v;
      if (dep({"Command"}, i, e))
      {
        string c = i->m["Command"]->v;
        Json *m = new Json;
        m->i("Function", "launch");
        m->i("Command", c);
        m->i("wsRequestID", w);
        if (application("Emulator", m, e))
        {
          b = true;
        }
        delete m;
      }
    }
    else
    {
      e = "Please provide the wsRequestID.";
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
}
}
