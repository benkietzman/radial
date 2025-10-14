// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Build.cpp
// author     : Ben Kietzman
// begin      : 2025-10-13
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Build"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Build()
Build::Build(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "build", argc, argv, pCallback)
{
  // {{{ functions
  m_functions["action"] = &Build::action;
  m_functions["construct"] = &Build::construct;
  m_functions["destruct"] = &Build::destruct;
  m_functions["install"] = &Build::install;
  m_functions["remove"] = &Build::remove;
  m_functions["status"] = &Build::status;
  // }}}
}
// }}}
// {{{ ~Build()
Build::~Build()
{
}
// }}}
// {{{ callback()
void Build::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Build::callback()";
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
// {{{ construct()
bool Build::construct(radialUser &d, string &e)
{
  bool b = false;
  stringstream m;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Server"}, i, e))
  {
  }

  return b;
}
// }}}
// {{{ destruct()
bool Build::destruct(radialUser &d, string &e)
{
  bool b = false;
  stringstream m;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Server"}, i, e))
  {
  }

  return b;
}
// }}}
// {{{ install()
bool Build::install(radialUser &d, string &e)
{
  bool b = false;
  stringstream m;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Package", "Server"}, i, e))
  {
  }

  return b;
}
// }}}
// {{{ remove()
bool Build::remove(radialUser &d, string &e)
{
  bool b = false;
  stringstream m;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Package", "Server"}, i, e))
  {
  }

  return b;
}
// }}}
}
}
