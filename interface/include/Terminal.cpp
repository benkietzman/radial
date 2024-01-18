// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Terminal.cpp
// author     : Ben Kietzman
// begin      : 2023-12-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Terminal"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Terminal()
Terminal::Terminal(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "terminal", argc, argv, pCallback)
{
  m_pCallbackAddon = NULL;
  m_functions["connect"] = &Terminal::connect;
  m_functions["ctrl"] = &Terminal::ctrl;
  m_functions["disconnect"] = &Terminal::disconnect;
  m_functions["down"] = &Terminal::down;
  m_functions["enter"] = &Terminal::enter;
  m_functions["escape"] = &Terminal::escape;
  m_functions["function"] = &Terminal::function;
  m_functions["getSocketTimeout"] = &Terminal::getSocketTimeout;
  m_functions["home"] = &Terminal::home;
  m_functions["key"] = &Terminal::key;
  m_functions["keypadEnter"] = &Terminal::keypadEnter;
  m_functions["left"] = &Terminal::left;
  m_functions["right"] = &Terminal::right;
  m_functions["screen"] = &Terminal::screen;
  m_functions["send"] = &Terminal::send;
  m_functions["setSocketTimeout"] = &Terminal::setSocketTimeout;
  m_functions["shiftFunction"] = &Terminal::shiftFunction;
  m_functions["tab"] = &Terminal::tab;
  m_functions["up"] = &Terminal::up;
  m_functions["wait"] = &Terminal::wait;
}
// }}}
// {{{ ~Terminal()
Terminal::~Terminal()
{
}
// }}}
// {{{ callback()
void Terminal::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bLocal = true, bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Terminal::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);

  if (exist(ptJson, "Request") && !empty(ptJson->m["Request"], "Session"))
  {
    string strNode;
    stringstream ssSession(ptJson->m["Request"]->m["Session"]->v);
    getline(ssSession, strNode, '_');
    if (!strNode.empty() && strNode != m_strNode)
    {
      Json *ptLink = new Json(ptJson);
      bLocal = false;
      ptLink->i("Interface", "terminal");
      ptLink->i("Node", strNode);
      if (hub("link", ptLink, strError))
      {
        bResult = true;
        if (exist(ptLink, "Response"))
        {
          ptJson->i("Response", ptLink->m["Response"]);
        }
      }
      delete ptLink;
    }
  }
  if (bLocal)
  {
    if (!empty(ptJson, "Function"))
    {
      bool bInvalid = true;
      string strFunction = ptJson->m["Function"]->v;
      radialUser d;
      userInit(ptJson, d);
      if (m_pCallbackAddon != NULL && m_pCallbackAddon(strFunction, d, strError, bInvalid))
      {
        bResult = true;
      }
      else if (bInvalid)
      {
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
      }
    }
    else
    {
      strError = "Please provide the Function.";
    }
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
// {{{ connect()
bool Terminal::connect(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "Server"))
  {
    if (!empty(i, "Port"))
    {
      bool bWait = (!empty(i, "Wait") && (i->m["Wait"]->t == '1' || i->m["Wait"]->v == "1" || i->m["Wait"]->v == "yes"));
      radialTerminal *t = new radialTerminal;
      if (!empty(i, "Cols"))
      {
        size_t unCols;
        stringstream ssCols(i->m["Cols"]->v);
        ssCols >> unCols;
        t->t.cols(unCols);
      }
      if (!empty(i, "Rows"))
      {
        size_t unRows;
        stringstream ssRows(i->m["Rows"]->v);
        ssRows >> unRows;
        t->t.rows(unRows);
      }
      if (!empty(i, "Type"))
      {
        t->t.type(i->m["Type"]->v);
      }
      if (t->t.connect(i->m["Server"]->v, i->m["Port"]->v) && t->t.wait(bWait))
      {
        stringstream ssSession, ssValue;
        vector<string> screen;
        b = true;
        ssSession << m_strNode << "_" << getpid() << "_" << syscall(SYS_gettid) << "_" << t;
        o->i("Session", ssSession.str());
        m_mutex.lock();
        m_sessions[ssSession.str()] = t;
        m_mutex.unlock();
        t->t.screen(screen);
        o->m["Screen"] = new Json;
        for (size_t i = 0; i < screen.size(); i++)
        {
          o->m["Screen"]->pb(screen[i]);
        }
        ssValue.str("");
        ssValue << t->t.col();
        o->insert("Col", ssValue.str(), 'n');
        ssValue.str("");
        ssValue << t->t.cols();
        o->insert("Cols", ssValue.str(), 'n');
        ssValue.str("");
        ssValue << t->t.row();
        o->insert("Row", ssValue.str(), 'n');
        ssValue.str("");
        ssValue << t->t.rows();
        o->insert("Rows", ssValue.str(), 'n');
      }
      else
      {
        e = t->t.error();
      }
      if (!b)
      {
        delete t;
      }
    }
    else
    {
      e = "Please provide the Port.";
    }
  }
  else
  {
    e = "Please provide the Server.";
  }

  return b;
}
// }}}
// {{{ ctrl()
bool Terminal::ctrl(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (k.size() == 1)
    {
      if (t->t.sendCtrl(k[0], w))
      {
        b = true;
      }
      else
      {
        e = t->t.error();
      }
    }
    else
    {
      e = "Data should contain a single character for this Function.";
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ disconnect()
bool Terminal::disconnect(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    b = true;
    t->t.disconnect();
    m_mutex.lock();
    delete t;
    t = NULL;
    m_sessions.erase(i->m["Session"]->v);
    m_mutex.unlock();
    delete o->m["Session"];
    o->m.erase("Session");
  }

  return b;
}
// }}}
// {{{ down()
bool Terminal::down(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (t->t.sendDown(c, w))
    {
      b = true;
    }
    else
    {
      e = t->t.error();
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ enter()
bool Terminal::enter(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (t->t.sendEnter(w))
    {
      b = true;
    }
    else
    {
      e = t->t.error();
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ escape()
bool Terminal::escape(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (t->t.sendEscape(w))
    {
      b = true;
    }
    else
    {
      e = t->t.error();
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ function()
bool Terminal::function(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    int nKey = 0;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    stringstream ssKey(k);
    ssKey >> nKey;
    if (nKey >= 1 && nKey <= 12)
    {
      if (t->t.sendFunction(nKey))
      {
        b = true;
      }
      else
      {
        e = t->t.error();
      }
    }
    else
    {
      e = "Please provide a Data value between 1 and 12.";
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ getSocketTimeout()
bool Terminal::getSocketTimeout(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    int nLong, nShort;
    size_t c;
    string k;
    stringstream ssLong, ssShort;
    pre(i, t, w, c, k);
    b = true;
    t->t.getSocketTimeout(nShort, nLong);
    ssLong << nLong;
    o->insert("Long", ssLong.str(), 'n');
    ssShort << nShort;
    o->insert("Short", ssShort.str(), 'n');
    post(o, t);
  }

  return b;
}
// }}}
// {{{ getTerminal()
bool Terminal::getTerminal(radialUser &d, radialTerminal *t, string &e)
{
  bool bResult = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "Session"))
  {
    m_mutex.lock();
    if (m_sessions.find(i->m["Session"]->v) != m_sessions.end())
    {
      bResult = true;
      t = m_sessions[i->m["Session"]->v];
      time(&(t->CTime));
      o->i("Session", i->m["Session"]->v);
    }
    else
    {
      e = "Please provide a valid Session.";
    }
    m_mutex.unlock();
  }
  else
  {
    e = "Please provide the Session.";
  }

  return bResult;
}
// }}}
// {{{ home()
bool Terminal::home(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (t->t.sendHome(w))
    {
      b = true;
    }
    else
    {
      e = t->t.error();
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ key()
bool Terminal::key(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (k.size() == 1)
    {
      if (t->t.sendKey(k[0], c, w))
      {
        b = true;
      }
      else
      {
        e = t->t.error();
      }
    }
    else
    {
      e = "Data should contain a single character for this Function.";
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ keypadEnter()
bool Terminal::keypadEnter(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (t->t.sendKeypadEnter(w))
    {
      b = true;
    }
    else
    {
      e = t->t.error();
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ left()
bool Terminal::left(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (t->t.sendLeft(c, w))
    {
      b = true;
    }
    else
    {
      e = t->t.error();
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ post()
void Terminal::post(Json *o, radialTerminal *t)
{
  stringstream ssValue;
  vector<string> screen;

  t->t.screen(screen);
  o->m["Screen"] = new Json;
  for (size_t i = 0; i < screen.size(); i++)
  {
    o->m["Screen"]->pb(screen[i]);
  }
  ssValue.str("");
  ssValue << t->t.col();
  o->insert("Col", ssValue.str(), 'n');
  ssValue.str("");
  ssValue << t->t.cols();
  o->insert("Cols", ssValue.str(), 'n');
  ssValue.str("");
  ssValue << t->t.row();
  o->insert("Row", ssValue.str(), 'n');
  ssValue.str("");
  ssValue << t->t.rows();
  o->insert("Rows", ssValue.str(), 'n');
  t->bActive = false;
  t->m.unlock();
}
// }}}
// {{{ pre()
void Terminal::pre(Json *i, radialTerminal *t, bool &w, size_t &c, string &k)
{
  t->bActive = true;
  t->m.lock();
  if (!empty(i, "Data"))
  {
    k = i->m["Data"]->v;
  }
  if (!empty(i, "Count"))
  {
    stringstream ssC(i->m["Count"]->v);
    ssC >> c;
  }
  else
  {
    c = 1;
  }
  w = (!empty(i, "Wait") && (i->t == '1' || i->m["Wait"]->v == "1" || i->m["Wait"]->v == "yes"));
}
// }}}
// {{{ right()
bool Terminal::right(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (t->t.sendRight(c, w))
    {
      b = true;
    }
    else
    {
      e = t->t.error();
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ schedule()
void Terminal::schedule(string strPrefix)
{
  list<string> removals;
  time_t CTime[2];

  threadIncrement();
  strPrefix += "->Terminal::schedule()";
  time(&(CTime[0]));
  while (!shutdown())
  {
    time(&(CTime[1]));
    if ((CTime[1] - CTime[0]) > 300)
    {
      CTime[0] = CTime[1];
      m_mutex.lock();
      for (auto &session : m_sessions)
      {
        if (!session.second->bActive && (CTime[1] - session.second->CTime) > 300)
        {
          removals.push_back(session.first);
        }
      }
      while (!removals.empty())
      {
        m_sessions[removals.front()]->m.lock();
        m_sessions[removals.front()]->t.disconnect();
        m_sessions[removals.front()]->m.unlock();
        delete m_sessions[removals.front()];
        removals.pop_front();
      }
      m_mutex.unlock();
    }
    msleep(1000);
  }
  threadDecrement();
}
// }}}
// {{{ screen()
bool Terminal::screen(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    b = true;
    post(o, t);
  }

  return b;
}
// }}}
// {{{ send()
bool Terminal::send(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if ((w && t->t.sendWait(k, c)) || (!w && t->t.send(k, c)))
    {
      b = true;
    }
    else
    {
      e = t->t.error();
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ setCallbackAddon()
void Terminal::setCallbackAddon(bool (*pCallback)(const string, radialUser &, string &, bool &))
{
  m_pCallbackAddon = pCallback;
}
// }}}
// {{{ setSocketTimeout()
bool Terminal::setSocketTimeout(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (!empty(i, "Long"))
    {
      int nLong;
      stringstream ssLong(i->m["Long"]->v);
      ssLong >> nLong;
      if (!empty(i, "Short"))
      {
        int nShort;
        stringstream ssShort(i->m["Short"]->v);
        ssShort >> nShort;
        b = true;
        t->t.setSocketTimeout(nShort, nLong);
      }
      else
      {
        e = "Please provide the Short within the Request.";
      }
    }
    else
    {
      e = "Please provide the Long within the Request.";
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ shiftFunction()
bool Terminal::shiftFunction(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    int nKey = 0;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    stringstream ssKey(k);
    ssKey >> nKey;
    if (nKey >= 1 && nKey <= 12)
    {
      if (t->t.sendShiftFunction(nKey))
      {
        b = true;
      }
      else
      {
        e = t->t.error();
      }
    }
    else
    {
      e = "Please provide a Data value between 1 and 12.";
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ tab()
bool Terminal::tab(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (t->t.sendTab(c, w))
    {
      b = true;
    }
    else
    {
      e = t->t.error();
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ up()
bool Terminal::up(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (t->t.sendUp(c, w))
    {
      b = true;
    }
    else
    {
      e = t->t.error();
    }
    post(o, t);
  }

  return b;
}
// }}}
// {{{ wait()
bool Terminal::wait(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];
  radialTerminal *t = NULL;

  if (getTerminal(d, t, e))
  {
    bool w;
    size_t c;
    string k;
    pre(i, t, w, c, k);
    if (t->t.wait(w))
    {
      b = true;
    }
    else
    {
      e = t->t.error();
    }
    post(o, t);
  }

  return b;
}
// }}}
}
}
