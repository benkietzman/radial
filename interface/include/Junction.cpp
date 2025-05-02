// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Junction.cpp
// author     : Ben Kietzman
// begin      : 2022-05-18
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
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
  sem_init(&m_sem, 0, 100);
  m_pJunction->useSingleSocket(true);
}
// }}}
// {{{ ~Junction()
Junction::~Junction()
{
  sem_destroy(&m_sem);
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
    sem_wait(&m_sem);
    if (m_pJunction->request(in, out, strError))
    {
      bResult = true;
      ptJson->m["Response"] = new Json;
      for (auto &i : out)
      {
        ptJson->m["Response"]->l.push_back(new Json(i));
      }
    }
    sem_post(&m_sem);
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
