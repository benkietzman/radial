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
  m_functions["planRemove"] = &Maestro::planRemove;
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
      if (isOwner(d, p))
      {
        m_mutex.lock();
        if (m_p.find(p) != m_p.end())
        {
          if (m_p[p]->f.find(f) != m_p[p]->f.end())
          {
            b = true;
            o->merge(m_p[p]->f[f], true, false);
          }
          else
          {
            e = "Flow not found.";
          }
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
      if (isOwner(d, p))
      {
        m_mutex.lock();
        if (m_p.find(p) != m_p.end())
        {
          b = true;
          for (auto &f : m_p[p]->f)
          {
            o->i(f.first, f.second);
          }
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

  if (p.size() > 2 && (p.substr(0, 2) == "a_" || p.substr(0, 2) == "p_" || p.substr(0, 2) == "u_"))
  {
    string i, t;
    stringstream ssP(p);
    getline(ssP, t, '_');
    getline(ssP, i);
    if (!t.empty() && !i.empty())
    {
      if (t == "a")
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
      else if (t == "p" || id = i)
      {
        b = true;
      }
    }
  }
  else
  {
    e = "Please provide a valid plan.";
  }

  return b;
}
// }}}
// {{{ load()
void Maestro::load(string strPrefix)
{
  bool bLoaded = false;
  string e;
  stringstream ssChat;
  Json *l = NULL;
  map<string, radialMaestroPlan *> p;

  strPrefix += "->Maestro::load()";
  if (dataDirectoryList(m_strHandle, {}, &l, e))
  {
    bLoaded = true;
    for (auto &i : l->m)
    {
      if (i.second->val({"Type"}) == "directory" && (i.first.substr(0, 2) == "a_" || i.first.substr(0, 2) == "p_" || i.first.substr(0, 2) == "u_"))
      {
        map<string, string> row;
        string id, t;
        stringstream ssP(i.first);
        Json *sl = NULL, *ptData = new Json;
        radialMaestroPlan *ptPlan = new radialMaestroPlan;
        getline(ssP, t, '_');
        getline(ssP, id);
        ptPlan->id = id;
        ptPlan->type = t[0];
        ptData->i("id", id);
        if (db(((ptPlan->a)?"dbCentralApplications":"dbCentralUsers"), ptData, row, e))
        {
          stringstream ssOwner;
          if (t == "a")
          {
            ssOwner << row["name"];
          }
          else if (t == "u")
          {
            ssOwner << row["first_name"] << " " << row["last_name"];
          }
          ptPlan->owner = ssOwner.str();
          if (dataDirectoryList(m_strHandle, {i.first}, &sl, e))
          {
            for (auto &j : sl->m)
            {
              if (j.second->val({"Type"}) == "directory" && j.first == "f")
              {
                Json *dl = NULL;
                if (dataDirectoryList(m_strHandle, {i.first, j.first}, &dl, e))
                {
                  for (auto &k : dl->m)
                  {
                    if (k.second->val({"Type"}) == "regular file")
                    {
                      string b;
                      if (dataRead(m_strHandle, {i.first, j.first, k.first}, b, e))
                      {
                        ptPlan->f[k.first] = new Json(b);
                      }
                      else
                      {
                        bLoaded = false;
                        ssChat.str("");
                        ssChat << char(2) << char(3) << "07Interface::dataRead() [" << m_strHandle << "," << i.first << "," << j.first << "," << k.first << "] " << e << char(3) << char(2);
                        chat("#maestro", ssChat.str());
                      }
                    }
                  }
                }
                else
                {
                  bLoaded = false;
                  ssChat.str("");
                  ssChat << char(2) << char(3) << "07Interface::dataDirectoryList() [" << m_strHandle << "," << i.first << "," << j.first << "] " << e << char(3) << char(2);
                  chat("#maestro", ssChat.str());
                }
              }
            }
            delete sl;
          }
          else
          {
            bLoaded = false;
            ssChat.str("");
            ssChat << char(2) << char(3) << "07Interface::dataDirectoryList() [" << m_strHandle << "," << i.first << "] " << e << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          bLoaded = false;
          ssChat.str("");
          ssChat << char(2) << char(3) << "07Interface::db(dbCentral" << ((ptPlan->a)?"application":"user") << "s) [" << m_strHandle << "," << i.first << "] " << e << char(3) << char(2);
          chat("#maestro", ssChat.str());
        }
        p[i.first] = ptPlan;
      }
    }
    delete l;
  }
  else
  {
    ssChat.str("");
    ssChat << char(2) << char(3) << "07Interface::dataDirectoryList() [" << m_strHandle << "] " << e << char(3) << char(2);
    chat("#maestro", ssChat.str());
  }
  if (bLoaded)
  {
    size_t unFlows = 0, unPlans = p.size();
    string v;
    m_mutex.lock();
    for (auto &i : p)
    {
      unFlows += i.second->f.size();
      m_p[i.first] = i.second;
    }
    m_mutex.unlock();
    m_bLoaded = true;
    ssChat.str("");
    ssChat << "Loaded " << m_manip.toShort(unPlans, v) << " plan" << ((unPlans != 1)?"s":"") << " containing " << m_manip.toShort(unFlows, v) << " flow" << ((unFlows != 1)?"s":"") << " from disk into memory.";
    chat("#maestro", ssChat.str());
  }
  else
  {
    bLoaded = false;
    ssChat.str("");
    ssChat << char(2) << char(3) << "04Failed to load plans." << char(3) << char(2);
    chat("#maestro", ssChat.str());
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
        m_mutex.lock();
        if (m_p.find(p) != m_p.end())
        {
          stringstream ssPlan, ssType;
          b = true;
          ssPlan << m_p[p]->type << "_" << m_p[p]->id;
          o->i("Plan", ssPlan.str());
          ssType << m_p[p]->type;
          o->i("Type", ssType.str());
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
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (isValid(d))
  {
    if (dep({"Plan"}, i, e))
    {
      string p = i->m["Plan"]->v;
      if (isOwner(d, p))
      {
        map<string, string> row;
        string id, t;
        stringstream ssP(p);
        Json *ptData = new Json;
        getline(ssP, t, '_');
        getline(ssP, id);
        ptData->i("id", id);
        if (db(((t == "a")?"dbCentralApplications":"dbCentralUsers"), ptData, row, e))
        {
          m_mutex.lock();
          if (m_p.find(p) == m_p.end())
          {
            if (i->val({"_broadcast"}) == "1" || dataDirectoryAdd(m_strHandle, {p}, e))
            {
              stringstream ssOwner;
              b = true;
              m_p[p] = new radialMaestroPlan;
              m_p[p]->type = t[0];
              m_p[p]->id = id;
              if (m_p[p]->a)
              {
                ssOwner << row["name"];
              }
              else
              {
                ssOwner << row["first_name"] << " " << row["last_name"];
              }
              m_p[p]->owner = ssOwner.str();
            }
            else
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryAdd() [" << m_strHandle << "," << p << "] " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            e = "Plan already exists.";
          }
          m_mutex.unlock();
          if (b && i->val({"_broadcast"}) != "1")
          {
            list<string> nodes;
            Json *ptLive = new Json;
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
              hub("link", ptLink, false);
              delete ptLink;
              nodes.pop_front();
            }
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " " << char(3) << " " << char(2) << char(3) << "03Plan added by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
            chat("#maestro", ssChat.str());
            ptLive->i("Action", "planAdd");
            ptLive->i("Name", p);
            live("Maestro", "", ptLive);
            delete ptLive;
          }
        }
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
// {{{ planRemove()
bool Maestro::planRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (isValid(d))
  {
    if (dep({"Plan"}, i, e))
    {
      string p = i->m["Plan"]->v;
      if (isOwner(d, p))
      {
        m_mutex.lock();
        if (m_p.find(p) != m_p.end())
        {
          if (dataDirectoryRemove(m_strHandle, {p}, e))
          {
            b = true;
            for (auto &f : m_p[p]->f)
            {
              delete f.second;
            }
            delete m_p[p];
            m_p.erase(p);
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryRemove() [" << m_strHandle << "," << p << "] " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Plan not found.";
        }
        m_mutex.unlock();
        if (b && i->val({"_broadcast"}) != "1")
        {
          list<string> nodes;
          Json *ptLive = new Json;
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
            hub("link", ptLink, false);
            delete ptLink;
            nodes.pop_front();
          }
          ssChat.str("");
          ssChat << char(3) << "00,06 " << p << " " << char(3) << " " << char(2) << char(3) << "03Plan removed by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
          chat("#maestro", ssChat.str());
          ptLive->i("Action", "planRemove");
          ptLive->i("Name", p);
          live("Maestro", "", ptLive);
          delete ptLive;
        }
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
      b = true;
      m_mutex.lock();
      for (auto &p : m_p)
      {
        if (isOwner(u.p->m["o"]->m["id"]->v, p.first))
        {
          stringstream ssType;
          Json *ptPlan = new Json;
          ptPlan->i("ID", p.second->id);
          ptPlan->i("NumFlows", to_string(p.second->f.size()), 'n');
          ptPlan->i("Owner", p.second->owner);
          ptPlan->i("Plan", p.first);
          ssType << p.second->type;
          ptPlan->i("Type", ssType.str());
          o->l.push_back(ptPlan);
        }
      }
      m_mutex.unlock();
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
  msleep(5000);
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
