// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Maestro.cpp
// author     : Ben Kietzman
// begin      : 2026-10-07
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Maestro"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Maestro()
Maestro::Maestro(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "maestro", argc, argv, pCallback)
{
  m_bLoaded = false;
  // {{{ functions
  m_functions["action"] = &Maestro::action;
  m_functions["flow"] = &Maestro::flow;
  m_functions["flows"] = &Maestro::flows;
  m_functions["plan"] = &Maestro::plan;
  m_functions["planAdd"] = &Maestro::planAdd;
  m_functions["plans"] = &Maestro::plans;
  m_functions["status"] = &Maestro::status;
  // }}}
  m_strHandle = "maestro";
  m_strPath = m_strData + (string)"/maestro";
  m_pThreadSchedule = new thread(&Maestro::schedule, this, strPrefix);
  pthread_setname_np(m_pThreadSchedule->native_handle(), "schedule");
}
// }}}
// {{{ ~Maestro()
Maestro::~Maestro()
{
  m_pThreadSchedule->join();
  delete m_pThreadSchedule;
  for (auto &p : m_p)
  {
    for (auto &f : p.second->f)
    {
      delete f.second;
    }
    p.second->f.clear();
    delete p.second;
  }
  m_p.clear();
}
// }}}
// {{{ callback()
void Maestro::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Maestro::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (dep({"Function"}, ptJson, strError))
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
      if (ptJson->exist({"Response"}))
      {
        delete ptJson->m["Response"];
      }
      ptJson->m["Response"] = d.p->m["o"];
      d.p->m.erase("o");
    }
    userDeinit(d);
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
// {{{ flow()
bool Maestro::flow(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (isValid(d))
  {
    if (dep({"Flow", "Plan"}, i, e))
    {
      string f = i->m["Flow"]->v, p = i->m["Plan"]->v;
      radialUser c;
      userInit(d, c);
      if (plan(c, e))
      {
        stringstream ssPath;
        b = true;
        ssPath << m_strPath << "/p/" << p << "/f/" << f;
        m_mutex.lock();
        if (m_p[p]->f.find(f) == m_p[p]->f.end())
        {
          ifstream inFlow(ssPath.str());
          stringstream ssJ;
          ssJ << inFlow.rdbuf();
          if (!ssJ.str().empty())
          {
            m_p[p]->f[f] = new Json(ssJ.str());
          }
          else
          {
            m_p[p]->f[f] = new Json;
          }
        }
        o->merge(m_p[p]->f[f], true, false);
        m_mutex.unlock();
      }
      userDeinit(c);
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ flows()
bool Maestro::flows(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (isValid(d))
  {
    if (dep({"Plan"}, i, e))
    {
      string p = i->m["Plan"]->v;
      radialUser c;
      userInit(d, c);
      if (plan(c, e))
      {
        list<string> l;
        stringstream ssPath;
        b = true;
        ssPath << m_strPath << "/p/" << p << "/f";
        m_file.directoryList(ssPath.str(), l);
        for (auto &f : l)
        {
          if (f != "." && f != "..")
          {
            o->pb(f);
          }
        }
      }
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ isOwner()
bool Maestro::isOwner(radialUser &d, const string p)
{
  bool b = false;
  string e;
  radialUser u;

  userInit(d, u);
  u.p->m["i"]->i("userid", d.u);
  if (user(u, e) && !u.p->empty({"o", "id"}) && isOwner(u.p->m["o"]->m["id"]->v, p))
  {
    b = true;
  }
  userDeinit(u);

  return b;
}
bool Maestro::isOwner(const string id, const string p)
{
  bool b = false;
  string e;

  if (p.size() > 2 && (p.substr(0, 2) == "a_" || p.substr(0, 2) == "u_"))
  {
    string i, t;
    stringstream ssP(p);
    getline(ssP, t, '_');
    getline(ssP, i);
    if (!t.empty() && !i.empty())
    {
      bool a = (t == "a");
      if (a)
      {
        stringstream q;
        q << "select a.id from application a, application_contact b, contact_type c, person d where a.id = b.application_id and b.type_id = c.id and b.contact_id = d.id and c.type in ('Primary Developer', 'Backup Developer') and a.id = '" << esc(i) << "' and d.id = '" << esc(id) << "' limit 1";
        auto g = dbquery("central_r", q.str(), e);
        if (g != NULL && !g->empty())
        {
          b = true;
        }
        dbfree(g);
      }
      else if (id == i)
      {
        b = true;
      }
    }
  }

  return b;
}
// }}}
// {{{ load()
void Maestro::load(string strPrefix)
{
  string e;
  stringstream ssChat;
  Json *l = NULL;

  strPrefix += "->Maestro::load()";
  if (dataDirectoryList(m_strHandle, {}, &l, e))
  {
    ssChat.str("");
    ssChat << l;
    chat("#maestro", ssChat.str());
  }
  else
  {
    ssChat.str("");
    ssChat << char(2) << char(3) << "07dataDirectoryList() " << e << char(3) << char(2);
    chat("#maestro", ssChat.str());
  }
  if (l != NULL)
  {
    delete l;
  }
}
// }}}
// {{{ plan()
bool Maestro::plan(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (isValid(d))
  {
    if (dep({"Plan"}, i, e))
    {
      string p = i->m["Plan"]->v;
      if (isOwner(d, p))
      {
        stringstream ssPath;
        ssPath << m_strPath << "/p";
        m_mutex.lock();
        if (m_p.empty() && !m_file.directoryExist(ssPath.str()))
        {
          m_file.makeDirectory(ssPath.str());
        }
        ssPath.str("");
        ssPath << m_strPath << "/p/" << p;
        if (m_p.find(p) == m_p.end() && p.size() > 2 && (p.substr(0, 2) == "a_" || p.substr(0, 2) == "u_") && m_file.directoryExist(ssPath.str()))
        {
          string i, t;
          stringstream ssP(p);
          radialMaestroPlan *ptPlan = new radialMaestroPlan;
          getline(ssP, t, '_');
          getline(ssP, i);
          ptPlan->a = (t == "a");
          ptPlan->id = i;
          m_p[p] = ptPlan;
        }
        if (m_p.find(p) != m_p.end())
        {
          b = true;
          o->i("Type", ((m_p[p]->a)?"application":"user"));
          o->i("ID", m_p[p]->id, 'n');
        }
        else
        {
          e = "Plan not found.";
        }
        m_mutex.unlock();
      }
      else
      {
        e = "You are not the owner.";
      }
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ planAdd()
bool Maestro::planAdd(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (isValid(d))
  {
    if (dep({"Plan"}, i, e))
    {
      string p = i->m["i"]->v;
      if (isOwner(d, p))
      {
        stringstream ssPath;
        ssPath << m_strPath << "/p/" << p;
        m_mutex.lock();
        if (m_p.find(p) == m_p.end() && !m_file.directoryExist(ssPath.str()))
        {
          b = true;
          m_file.makeDirectory(ssPath.str());
        }
        else
        {
          e = "Plan already exists.";
        }
        m_mutex.unlock();
      }
      else
      {
        e = "You are not the owner.";
      }
      if (i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        m_mutexShare.lock();
        for (auto &link : m_l)
        {
          if (link->interfaces.find("maestro") != link->interfaces.end())
          {
            nodes.push_back(link->strNode);
          }
        }
        m_mutexShare.unlock();
        while (!nodes.empty())
        {
          Json *ptLink = new Json(d.r);
          ptLink->i("Interface", "maestro");
          ptLink->i("Node", nodes.front());
          ptLink->i("Function", "planAdd");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink, e);
          delete ptLink;
          nodes.pop_front();
        }
        if (b)
        {
          stringstream ssChat;
          Json *ptLive = new Json;
          ssChat << char(3) << "00,06 " << i->m["Name"]->v << " " << char(3) << " " << char(2) << char(3) << "03Plan added by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
          chat("#maestro", ssChat.str());
          ptLive->i("Action", "planAdd");
          ptLive->i("Name", i->m["Name"]->v);
          live("Maestro", "", ptLive);
          delete ptLive;
        }
      }
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ plans()
bool Maestro::plans(radialUser &d, string &e)
{
  bool b = false;
  Json *o = d.p->m["o"];

  if (isValid(d))
  {
    radialUser u;
    userInit(d, u);
    u.p->m["i"]->i("userid", d.u);
    if (user(u, e) && !u.p->empty({"o", "id"}))
    {
      list<string> l;
      stringstream ssPath;
      b = true;
      ssPath << m_strPath << "/p";
      m_file.directoryList(ssPath.str(), l);
      for (auto &p : l)
      {
        if (isOwner(u.p->m["o"]->m["id"]->v, p))
        {
          o->pb(p);
        }
      }
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ schedule()
void Maestro::schedule(string strPrefix)
{
  list<string> removals;
  string e;
  time_t CTime[2];

  threadIncrement();
  strPrefix += "->Maestro::schedule()";
  time(&(CTime[0]));
  load(strPrefix);
  while (!shutdown())
  {
    time(&(CTime[1]));
    if ((CTime[1] - CTime[0]) > 300)
    {
      CTime[0] = CTime[1];
      if (!m_bLoaded)
      {
        load(strPrefix);
      }
    }
    msleep(1000);
  }
  threadDecrement();
}
// }}}
}
}
