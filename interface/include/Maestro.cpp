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
Maestro::Maestro(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), void (*pCallbackInotify)(string, const string, const string)) : Interface(strPrefix, "maestro", argc, argv, pCallback)
{
  string strError;
  map<string, list<string> > watches;

  m_bLoaded = false;
  // {{{ functions
  m_functions["accesses"] = &Maestro::accesses;
  m_functions["accessRemove"] = &Maestro::accessRemove;
  m_functions["accessRequest"] = &Maestro::accessRequest;
  m_functions["accessRequestApprove"] = &Maestro::accessRequestApprove;
  m_functions["accessRequestCancel"] = &Maestro::accessRequestCancel;
  m_functions["accessRequestDeny"] = &Maestro::accessRequestDeny;
  m_functions["action"] = &Maestro::action;
  m_functions["cores"] = &Maestro::cores;
  m_functions["flow"] = &Maestro::flow;
  m_functions["flowAdd"] = &Maestro::flowAdd;
  m_functions["flowExec"] = &Maestro::flowExec;
  m_functions["flowRemove"] = &Maestro::flowRemove;
  m_functions["flows"] = &Maestro::flows;
  m_functions["flowUpdate"] = &Maestro::flowUpdate;
  m_functions["plan"] = &Maestro::plan;
  m_functions["planAdd"] = &Maestro::planAdd;
  m_functions["planRemove"] = &Maestro::planRemove;
  m_functions["plans"] = &Maestro::plans;
  m_functions["scopes"] = &Maestro::scopes;
  m_functions["status"] = &Maestro::status;
  // }}}
  m_strCipher = "AES-256 GCM";
  m_strHandle = "maestro";
  m_strPath = m_strData + (string)"/maestro";
  watches[m_strData] = {".cred"};
  m_pThreadInotify = new thread(&Maestro::inotify, this, strPrefix, watches, pCallbackInotify);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
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
  for (auto &i : m_javascript)
  {
    delete i.second;
  }
  m_javascript.clear();
}
// }}}
// {{{ access
// {{{ accesses()
bool Maestro::accesses(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Type"}, i, e))
  {
    string t = i->m["Type"]->v;
    if (t == "application" || t == "user")
    {
      b = true;
      m_mutex.lock();
      for (auto &p : m_p)
      {
        if (!p.second->f.empty() && ((t == "application" && p.second->t == 'a') || (t == "user" && p.second->t == 'u')))
        {
          o->pb(p.second->id);
        }
      }
      m_mutex.unlock();
    }
    else if (t == "core")
    {
      b = true;
      for (auto &j : m_javascript)
      {
        if (!j.second->scopes.empty())
        {
          o->pb(j.first);
        }
      }
    }
    else
    {
      e = "Please provide a valid Type:  application, core, user.";
    }
  }
  else if (dep({"Plan"}, i, e))
  {
    string p = i->m["Plan"]->v;
    e.clear();
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      b = true;
      if (m_p[p]->a != NULL)
      {
        o->merge(m_p[p]->a, true, false);
        for (auto &c : m_p)
        {
          if (c.second->a != NULL && c.second->a->exist({"r"}) && c.second->a->m["r"]->m.find(p) != c.second->a->m["r"]->m.end())
          {
            if (!o->exist({"p"}))
            {
              o->m["p"] = new Json;
            }
            o->m["p"]->i(c.first, c.second->a->m["r"]->m[p]);
          }
        }
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
  }
  else if (isLocalAdmin(d, "Maestro"))
  {
    b = true;
    e.clear();
    m_mutex.lock();
    for (auto &c : m_p)
    {
      if (c.second->a != NULL && c.second->a->exist({"r"}))
      {
        for (auto &g : c.second->a->m["r"]->m)
        {
          if (g.first.size() < 2 || (g.first.substr(0, 2) != "a_" && g.first.substr(0, 2) != "u_"))
          {
            if (!o->exist({c.first}))
            {
              o->m[c.first] = new Json;
            }
            o->m[c.first]->i(g.first, g.second);
          }
        }
      }
    }
    m_mutex.unlock();
  }

  return b;
}
// }}}
// {{{ accessRemove()
bool Maestro::accessRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Access", "Plan", "Scope"}, i, e))
  {
    string a = i->m["Access"]->v, p = i->m["Plan"]->v, s = i->m["Scope"]->v;
    if (isOwner(d, p))
    {
      string t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->a->exist({"a", a}))
        {
          list<Json *>::iterator scopeIter = m_p[p]->a->m["a"]->m[a]->l.end();
          for (auto c = m_p[p]->a->m["a"]->m[a]->l.begin(); scopeIter == m_p[p]->a->m["a"]->m[a]->l.end() && c != m_p[p]->a->m["a"]->m[a]->l.end(); c++)
          {
            if ((*c)->v == s)
            {
              scopeIter = c;
            }
          }
          if (scopeIter != m_p[p]->a->m["a"]->m[a]->l.end())
          {
            delete (*scopeIter);
            m_p[p]->a->m["a"]->m[a]->l.erase(scopeIter);
            if (m_p[p]->a->m["a"]->m[a]->l.empty())
            {
              delete m_p[p]->a->m["a"]->m[a];
              m_p[p]->a->m["a"]->m.erase(a);
            }
            if (m_p[p]->a->m["a"]->m.empty())
            {
              delete m_p[p]->a->m["a"];
              m_p[p]->a->m.erase("a");
            }
            if (m_p[p]->a->m.empty())
            {
              delete m_p[p]->a;
              m_p[p]->a = NULL;
            }
            if (m_p[p]->a != NULL)
            {
              string j;
              m_p[p]->a->j(j);
              if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
              {
                b = true;
              }
              else
              {
                ssChat.str("");
                ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                chat("#maestro", ssChat.str());
              }
            }
            else if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "a"}, e))
            {
              b = true;
            }
            else
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " | " << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            e = "Scope not found.";
          }
        }
        else
        {
          e = "Access not found.";
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
          ptLink->i("Function", "accessRemove");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink, false);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "03Access removed by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "accessRemove");
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

  return b;
}
// }}}
// {{{ accessRequest()
bool Maestro::accessRequest(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Access", "Plan", "Scope"}, i, e))
  {
    list<string> to;
    string a = i->m["Access"]->v, p = i->m["Plan"]->v, s = i->m["Scope"]->v, t = "core";
    stringstream ssHtml, ssText;
    if (a.size() > 2)
    {
      if (a.substr(0, 2) == "a_")
      {
        t = "application";
      }
      else if (a.substr(0, 2) == "u_")
      {
        t = "user";
      }
    } 
    m_mutex.lock();
    if (isOwner(d, p))
    {
      if (!haveAccess(p, a, s, e, false))
      {
        if (!haveAccessRequest(p, a, s))
        {
          if (t == "application" || t == "core" || t == "user")
          {
            stringstream ssLink, ssQuery;
            ssText << d.f << " " << d.l << " (" << d.u << ") has requested access to the \"" << s << "\" ";
            ssHtml << d.f << " " << d.l << " (" << d.u << ") has requested access to the \"" << s << "\" ";
            if (t == "application")
            {
              ssQuery << "select distinct c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and b.type in ('Primary Developer', 'Backup Developer') and a.application_id = '" << esc(a.substr(2, (a.size() - 2))) << "' order by c.email";
              ssText << "flow of the " << a << " plan";
              ssHtml << "flow of the " << a << " plan";
            }
            else if (t == "core")
            {
              ssQuery << "select distinct c.email from application a, application_contact b, person c where a.id = b.application_id and b.contact_id = c.id and b.admin = 1 and a.name = 'Maestro' order by c.email";
              ssText << "scope of the " << a << " core function";
              ssHtml << "scope of the " << a << " core function";
            }
            else if (t == "user")
            {
              ssQuery << "select email from person where id = '" << esc(a.substr(2, (a.size() - 2))) << "'";
              ssText << "flow of the " << a << " plan";
              ssHtml << "flow of the " << a << " plan";
            }
            ssText << " on behalf of the " << p << " plan within Maestro.";
            ssHtml << " on behalf of the " << p << " plan within Maestro.";
            ssText << endl << endl;
            ssHtml << "<br><br>";
            ssLink << "https://" << m_strServer << "/maestro/#";
            if (t == "core")
            {
              ssLink << "/Approvals";
            }
            else
            {
              ssLink << "/Plans/" << a;
            }
            ssText << ssLink.str();
            ssHtml << "<a href=\"" << ssLink.str() << "\" target=\"_blank\">" << ssLink.str() << "</a>";
            ssText << endl << endl;
            ssHtml << "<br><br>";
            ssText << "-- Maestro";
            ssHtml << "-- Maestro";
            auto getContact = dbquery("central_r", ssQuery.str(), e);
            if (getContact != NULL)
            {
              if (!getContact->empty())
              {
                string j;
                b = true;
                for (auto &getContactRow : *getContact)
                {
                  to.push_back(getContactRow["email"]);
                }
                if (m_p[p]->a == NULL)
                {
                  m_p[p]->a = new Json;
                }
                if (!m_p[p]->a->exist({"r"}))
                {
                  m_p[p]->a->m["r"] = new Json;
                }
                if (!m_p[p]->a->m["r"]->exist({a}))
                {
                  m_p[p]->a->m["r"]->m[a] = new Json;
                }
                m_p[p]->a->m["r"]->m[a]->pb(s);
                m_p[p]->a->j(j);
                if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
                {
                  b = true;
                }
                else
                {
                  ssChat.str("");
                  ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                  chat("#maestro", ssChat.str());
                }
              }
              else
              {
                e = "No approvers found.";
              }
            }
            dbfree(getContact);
          }
          else
          {
            e = "Please provide a valid Type:  application, core, user.";
          }
        }
        else
        {
          e = "You already have a pending request for access.";
        }
      }
      else
      {
        e = "You already have access.";
      }
    }
    else
    {
      e = "You are not the owner.";
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
        ptLink->i("Function", "accessRequest");
        ptLink->m["Request"]->i("_broadcast", "1", '1');
        hub("link", ptLink, false);
        delete ptLink;
        nodes.pop_front();
      }
      ssChat.str("");
      ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "03Access requested by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
      chat("#maestro", ssChat.str());
      ptLive->i("Action", "accessRequest");
      ptLive->i("Name", p);
      live("Maestro", "", ptLive);
      delete ptLive;
      email("m44310@att.com", to, "Maestro:  Access Request", ssText.str(), ssHtml.str());
    }
  }

  return b;
}
// }}}
// {{{ accessRequestApprove()
bool Maestro::accessRequestApprove(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Access", "Plan", "Scope"}, i, e))
  {
    list<string> to;
    string a = i->m["Access"]->v, p = i->m["Plan"]->v, s = i->m["Scope"]->v, t = "core";
    stringstream ssHtml, ssText;
    if (a.size() > 2)
    {
      if (a.substr(0, 2) == "a_")
      {
        t = "application";
      }
      else if (a.substr(0, 2) == "u_")
      {
        t = "user";
      }
    } 
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      if (((t == "application" || t == "user") && isOwner(d, a)) || (t == "core" && isLocalAdmin(d, "Maestro")))
      {
        if (haveAccessRequest(p, a, s))
        {
          list<Json *>::iterator scopeIter = m_p[p]->a->m["r"]->m[a]->l.end();
          for (auto c = m_p[p]->a->m["r"]->m[a]->l.begin(); scopeIter == m_p[p]->a->m["r"]->m[a]->l.end() && c != m_p[p]->a->m["r"]->m[a]->l.end(); c++)
          {
            if ((*c)->v == s)
            {
              scopeIter = c;
            }
          }
          if (scopeIter != m_p[p]->a->m["r"]->m[a]->l.end())
          {
            if (!haveAccess(p, a, s, e, false))
            {
              stringstream ssQuery;
              ssText << d.f << " " << d.l << " (" << d.u << ") has approved access to the \"" << s << "\" ";
              ssHtml << d.f << " " << d.l << " (" << d.u << ") has approved access to the \"" << s << "\" ";
              if (p.substr(0, 2) == "a_")
              {
                ssQuery << "select distinct c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and b.type in ('Primary Developer', 'Backup Developer') and a.application_id = '" << esc(p.substr(2, (p.size() - 2))) << "' order by c.email";
                ssText << "flow of the " << a << " plan";
                ssHtml << "flow of the " << a << " plan";
              }
              else if (p.substr(0, 2) == "u_")
              {
                ssQuery << "select email from person where id = '" << esc(p.substr(2, (p.size() - 2))) << "'";
                ssText << "flow of the " << a << " plan";
                ssHtml << "flow of the " << a << " plan";
              }
              ssText << " on behalf of the " << p << " plan within Maestro.";
              ssHtml << " on behalf of the " << p << " plan within Maestro.";
              ssText << endl << endl;
              ssHtml << "<br><br>";
              ssText << "-- Maestro";
              ssHtml << "-- Maestro";
              auto getContact = dbquery("central_r", ssQuery.str(), e);
              if (getContact != NULL)
              {
                if (!getContact->empty())
                {
                  string j;
                  for (auto &getContactRow : *getContact)
                  {
                    to.push_back(getContactRow["email"]);
                  }
                  delete (*scopeIter);
                  m_p[p]->a->m["r"]->m[a]->l.erase(scopeIter);
                  if (m_p[p]->a->m["r"]->m[a]->l.empty())
                  {
                    delete m_p[p]->a->m["r"]->m[a];
                    m_p[p]->a->m["r"]->m.erase(a);
                  }
                  if (m_p[p]->a->m["r"]->m.empty())
                  {
                    delete m_p[p]->a->m["r"];
                    m_p[p]->a->m.erase("r");
                  }
                  if (m_p[p]->a->m.find("a") == m_p[p]->a->m.end())
                  {
                    m_p[p]->a->m["a"] = new Json;
                  }
                  if (m_p[p]->a->m["a"]->m.find(a) == m_p[p]->a->m["a"]->m.end())
                  {
                    m_p[p]->a->m["a"]->m[a] = new Json;
                  }
                  m_p[p]->a->m["a"]->m[a]->pb(s);
                  m_p[p]->a->j(j);
                  if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
                  {
                    b = true;
                  }
                  else
                  {
                    ssChat.str("");
                    ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                    chat("#maestro", ssChat.str());
                  }
                }
                else
                {
                  e = "No approvers found.";
                }
              }
              dbfree(getContact);
            }
            else
            {
              delete (*scopeIter);
              m_p[p]->a->m["r"]->m[a]->l.erase(scopeIter);
              if (m_p[p]->a->m["r"]->m[a]->l.empty())
              {
                delete m_p[p]->a->m["r"]->m[a];
                m_p[p]->a->m["r"]->m.erase(a);
              }
              if (m_p[p]->a->m["r"]->m.empty())
              {
                delete m_p[p]->a->m["r"];
                m_p[p]->a->m.erase("r");
              }
              if (m_p[p]->a->m.empty())
              {
                delete m_p[p]->a;
                m_p[p]->a = NULL;
              }
              if (m_p[p]->a != NULL)
              {
                string j;
                m_p[p]->a->j(j);
                if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
                {
                  b = true;
                }
                else
                {
                  ssChat.str("");
                  ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                  chat("#maestro", ssChat.str());
                }
              }
              else if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "a"}, e))
              {
                b = true;
              }
              else
              {
                ssChat.str("");
                ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " | " << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                chat("#maestro", ssChat.str());
              }
            }
          }
          else
          {
            e = "Scope not found.";
          }
        }
        else
        {
          e = "Access request not found.";
        }
      }
      else
      {
        e = "You are not the owner.";
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
        ptLink->i("Function", "accessRequestApprove");
        ptLink->m["Request"]->i("_broadcast", "1", '1');
        hub("link", ptLink, false);
        delete ptLink;
        nodes.pop_front();
      }
      ssChat.str("");
      ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "03Access request approved by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
      chat("#maestro", ssChat.str());
      ptLive->i("Action", "accessRequestApprove");
      ptLive->i("Name", p);
      live("Maestro", "", ptLive);
      delete ptLive;
      email("m44310@att.com", to, "Maestro:  Access Request Approved", ssText.str(), ssHtml.str());
    }
  }

  return b;
}
// }}}
// {{{ accessRequestCancel()
bool Maestro::accessRequestCancel(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Access", "Plan", "Scope"}, i, e))
  {
    list<string> to;
    string a = i->m["Access"]->v, p = i->m["Plan"]->v, s = i->m["Scope"]->v, t = "core";
    stringstream ssHtml, ssText;
    if (a.size() > 2)
    {
      if (a.substr(0, 2) == "a_")
      {
        t = "application";
      }
      else if (a.substr(0, 2) == "u_")
      {
        t = "user";
      }
    } 
    m_mutex.lock();
    if (isOwner(d, p))
    {
      if (!haveAccess(p, a, s, e, false))
      {
        if (haveAccessRequest(p, a, s))
        {
          stringstream ssQuery;
          ssText << d.f << " " << d.l << " (" << d.u << ") has canceled their request for access to the \"" << s << "\" ";
          ssHtml << d.f << " " << d.l << " (" << d.u << ") has canceled their request for access to the \"" << s << "\" ";
          if (t == "application")
          {
            ssQuery << "select distinct c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and b.type in ('Primary Developer', 'Backup Developer') and a.application_id = '" << esc(a.substr(2, (a.size() - 2))) << "' order by c.email";
            ssText << "flow of the " << a << " plan";
            ssHtml << "flow of the " << a << " plan";
          }
          else if (t == "core")
          {
            ssQuery << "select distinct c.email from application a, application_contact b, person c where a.id = b.application_id and b.contact_id = c.id and b.admin = 1 and a.name = 'Maestro' order by c.email";
            ssText << "scope of the " << a << " core function";
            ssHtml << "scope of the " << a << " core function";
          }
          else if (t == "user")
          {
            ssQuery << "select email from person where id = '" << esc(a.substr(2, (a.size() - 2))) << "'";
            ssText << "flow of the " << a << " plan";
            ssHtml << "flow of the " << a << " plan";
          }
          ssText << " on behalf of the " << p << " plan within Maestro.";
          ssHtml << " on behalf of the " << p << " plan within Maestro.";
          ssText << endl << endl;
          ssHtml << "<br><br>";
          ssText << "-- Maestro";
          ssHtml << "-- Maestro";
          auto getContact = dbquery("central_r", ssQuery.str(), e);
          if (getContact != NULL)
          {
            if (!getContact->empty())
            {
              list<Json *>::iterator scopeIter = m_p[p]->a->m["r"]->m[a]->l.end();
              for (auto c = m_p[p]->a->m["r"]->m[a]->l.begin(); scopeIter == m_p[p]->a->m["r"]->m[a]->l.end() && c != m_p[p]->a->m["r"]->m[a]->l.end(); c++)
              {
                if ((*c)->v == s)
                {
                  scopeIter = c;
                }
              }
              if (scopeIter != m_p[p]->a->m["r"]->m[a]->l.end())
              {
                for (auto &getContactRow : *getContact)
                {
                  to.push_back(getContactRow["email"]);
                }
                delete (*scopeIter);
                m_p[p]->a->m["r"]->m[a]->l.erase(scopeIter);
                if (m_p[p]->a->m["r"]->m[a]->l.empty())
                {
                  delete m_p[p]->a->m["r"]->m[a];
                  m_p[p]->a->m["r"]->m.erase(a);
                }
                if (m_p[p]->a->m["r"]->m.empty())
                {
                  delete m_p[p]->a->m["r"];
                  m_p[p]->a->m.erase("r");
                }
                if (m_p[p]->a->m.empty())
                {
                  delete m_p[p]->a;
                  m_p[p]->a = NULL;
                }
                if (m_p[p]->a != NULL)
                {
                  string j;
                  m_p[p]->a->j(j);
                  if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
                  {
                    b = true;
                  }
                  else
                  {
                    ssChat.str("");
                    ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                    chat("#maestro", ssChat.str());
                  }
                }
                else if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "a"}, e))
                {
                  b = true;
                }
                else
                {
                  ssChat.str("");
                  ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " | " << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                  chat("#maestro", ssChat.str());
                }
              }
              else
              {
                e = "Scope not found.";
              }
            }
            else
            {
              e = "No approvers found.";
            }
          }
          dbfree(getContact);
        }
        else
        {
          e = "You already have a pending request for access.";
        }
      }
      else
      {
        e = "You already have access.";
      }
    }
    else
    {
      e = "You are not the owner.";
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
        ptLink->i("Function", "accessRequestCancel");
        ptLink->m["Request"]->i("_broadcast", "1", '1');
        hub("link", ptLink, false);
        delete ptLink;
        nodes.pop_front();
      }
      ssChat.str("");
      ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "03Access request canceled by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
      chat("#maestro", ssChat.str());
      ptLive->i("Action", "accessRequestCancel");
      ptLive->i("Name", p);
      live("Maestro", "", ptLive);
      delete ptLive;
      email("m44310@att.com", to, "Maestro:  Access Request Canceled", ssText.str(), ssHtml.str());
    }
  }

  return b;
}
// }}}
// {{{ accessRequestDeny()
bool Maestro::accessRequestDeny(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Access", "Plan", "Scope"}, i, e))
  {
    list<string> to;
    string a = i->m["Access"]->v, p = i->m["Plan"]->v, s = i->m["Scope"]->v, t = "core";
    stringstream ssHtml, ssText;
    if (a.size() > 2)
    {
      if (a.substr(0, 2) == "a_")
      {
        t = "application";
      }
      else if (a.substr(0, 2) == "u_")
      {
        t = "user";
      }
    } 
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      if (((t == "application" || t == "user") && isOwner(d, a)) || (t == "core" && isLocalAdmin(d, "Maestro")))
      {
        if (haveAccessRequest(p, a, s))
        {
          list<Json *>::iterator scopeIter = m_p[p]->a->m["r"]->m[a]->l.end();
          for (auto c = m_p[p]->a->m["r"]->m[a]->l.begin(); scopeIter == m_p[p]->a->m["r"]->m[a]->l.end() && c != m_p[p]->a->m["r"]->m[a]->l.end(); c++)
          {
            if ((*c)->v == s)
            {
              scopeIter = c;
            }
          }
          if (scopeIter != m_p[p]->a->m["r"]->m[a]->l.end())
          {
            stringstream ssQuery;
            ssText << d.f << " " << d.l << " (" << d.u << ") has denied access to the \"" << s << "\" ";
            ssHtml << d.f << " " << d.l << " (" << d.u << ") has denied access to the \"" << s << "\" ";
            if (p.substr(0, 2) == "a_")
            {
              ssQuery << "select distinct c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and b.type in ('Primary Developer', 'Backup Developer') and a.application_id = '" << esc(p.substr(2, (p.size() - 2))) << "' order by c.email";
              ssText << "flow of the " << a << " plan";
              ssHtml << "flow of the " << a << " plan";
            }
            else if (p.substr(0, 2) == "u_")
            {
              ssQuery << "select email from person where id = '" << esc(p.substr(2, (p.size() - 2))) << "'";
              ssText << "flow of the " << a << " plan";
              ssHtml << "flow of the " << a << " plan";
            }
            ssText << " on behalf of the " << p << " plan within Maestro.";
            ssHtml << " on behalf of the " << p << " plan within Maestro.";
            ssText << endl << endl;
            ssHtml << "<br><br>";
            ssText << "-- Maestro";
            ssHtml << "-- Maestro";
            auto getContact = dbquery("central_r", ssQuery.str(), e);
            if (getContact != NULL)
            {
              if (!getContact->empty())
              {
                for (auto &getContactRow : *getContact)
                {
                  to.push_back(getContactRow["email"]);
                }
                delete (*scopeIter);
                m_p[p]->a->m["r"]->m[a]->l.erase(scopeIter);
                if (m_p[p]->a->m["r"]->m[a]->l.empty())
                {
                  delete m_p[p]->a->m["r"]->m[a];
                  m_p[p]->a->m["r"]->m.erase(a);
                }
                if (m_p[p]->a->m["r"]->m.empty())
                {
                  delete m_p[p]->a->m["r"];
                  m_p[p]->a->m.erase("r");
                }
                if (m_p[p]->a->m.empty())
                {
                  delete m_p[p]->a;
                  m_p[p]->a = NULL;
                }
                if (m_p[p]->a != NULL)
                {
                  string j;
                  m_p[p]->a->j(j);
                  if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
                  {
                    b = true;
                  }
                  else
                  {
                    ssChat.str("");
                    ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                    chat("#maestro", ssChat.str());
                  }
                }
                else if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "a"}, e))
                {
                  b = true;
                }
                else
                {
                  ssChat.str("");
                  ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " | " << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                  chat("#maestro", ssChat.str());
                }
              }
              else
              {
                e = "No approvers found.";
              }
            }
            dbfree(getContact);
          }
          else
          {
            e = "Scope not found.";
          }
        }
        else
        {
          e = "Access request not found.";
        }
      }
      else
      {
        e = "You are not the owner.";
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
        ptLink->i("Function", "accessRequestDeny");
        ptLink->m["Request"]->i("_broadcast", "1", '1');
        hub("link", ptLink, false);
        delete ptLink;
        nodes.pop_front();
      }
      ssChat.str("");
      ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "03Access request denied by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
      chat("#maestro", ssChat.str());
      ptLive->i("Action", "accessRequestDeny");
      ptLive->i("Name", p);
      live("Maestro", "", ptLive);
      delete ptLive;
      email("m44310@att.com", to, "Maestro:  Access Request Denied", ssText.str(), ssHtml.str());
    }
  }

  return b;
}
// }}}
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
// {{{ callbackInotify()
void Maestro::callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Irc::callbackInotify()";
  if (strPath == m_strData && strFile == ".cred")
  {
    load(strPrefix);
  }
}
// }}}
// {{{ cores()
bool Maestro::cores(radialUser &d, string &e)
{
  bool b = true;
  Json *o = d.p->m["o"];

  for (auto &j : m_javascript)
  {
    Json *c = new Json;
    c->i("Function", j.first);
    c->i("Description", j.second->strDescription);
    c->i("Prototype", j.second->strPrototype);
    c->i("Scopes", j.second->scopes);
    o->l.push_back(c);
  }

  return b;
}
// }}}
// {{{ flow
// {{{ flow()
bool Maestro::flow(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Flow", "Plan"}, i, e))
  {
    string f = i->m["Flow"]->v, p = i->m["Plan"]->v;
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

  return b;
}
// }}}
// {{{ flowAdd()
bool Maestro::flowAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Flow", "Plan"}, i, e))
  {
    string f = i->m["Flow"]->v, p = i->m["Plan"]->v;
    if (isOwner(d, p))
    {
      string id, t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      getline(ssP, id);
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->f.find(f) == m_p[p]->f.end())
        {
          if (i->val({"_broadcast"}) == "1" || !m_p[p]->f.empty() || dataDirectoryAdd(m_strHandle, {p, "f"}, e))
          {
            string j;
            i->j(j);
            if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "f", f}, j, e))
            {
              b = true;
              if (m_p[p]->f.find(f) != m_p[p]->f.end())
              {
                delete m_p[p]->f[f];
              }
              m_p[p]->f[f] = new Json(i);
            }
            else
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | " << f << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | " << f << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryAdd() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Flow already exists.";
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
          ptLink->i("Function", "flowAdd");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink, false);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << p << " | " << f << " " << char(3) << " " << char(2) << char(3) << "03Flow added by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "flowAdd");
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

  return b;
}
// }}}
// {{{ flowExec()
bool Maestro::flowExec(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Flow", "Plan"}, i, e))
  {
    string f = i->m["Flow"]->v, p = i->m["Plan"]->v;
    if (isOwner(d, p))
    {
      Json *ptFlow = NULL;
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->f.find(f) != m_p[p]->f.end())
        {
          ptFlow = new Json(m_p[p]->f[f]);
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
      if (ptFlow != NULL)
      {
        if (!ptFlow->empty({"Script"}))
        {
          JSRuntime *jsrun = JS_NewRuntime();
          JSContext *jsctx = JS_NewContext(jsrun);
          JSValue jsglobal = JS_GetGlobalObject(jsctx);
          for (auto &i : m_javascript)
          {
            JS_SetPropertyStr(jsctx, jsglobal, i.first.c_str(), JS_NewCFunction(jsctx, i.second->func, i.first.c_str(), i.second->nLength));
          }
          if (!m_strSecret.empty())
          {
            string strPlan;
            if (!m_manip.encodeBase64(m_manip.encryptAes(p, m_strSecret, strPlan, e, m_strCipher), strPlan).empty())
            {
              JS_SetPropertyStr(jsctx, jsglobal, "g_js_plan", JS_NewStringLen(jsctx, strPlan.c_str(), strPlan.size()));
            }
          }
          if (i->exist({"Request"}))
          {
            string strRequest;
            i->m["Request"]->j(strRequest);
            JS_SetPropertyStr(jsctx, jsglobal, "g_js_request", JS_NewStringLen(jsctx, strRequest.c_str(), strRequest.size()));
          }
          JSValue jsres = JS_Eval(jsctx, ptFlow->m["Script"]->v.data(), ptFlow->m["Script"]->v.size(), "<embedded>", JS_EVAL_TYPE_GLOBAL);
          if (JS_IsException(jsres))
          {
            JSValue jsexc = JS_GetException(jsctx);
            if (JS_IsError(jsctx, jsexc))
            {
              JSValue jsmsg = JS_GetPropertyStr(jsctx, jsexc, "message");
              if (JS_IsException(jsmsg))
              {
                JSValue jssubexc = JS_GetException(jsctx);
                const char *pszMessage = JS_ToCString(jsctx, jssubexc);
                if (pszMessage != NULL)
                {
                  e = pszMessage;
                  JS_FreeCString(jsctx, pszMessage);
                }
                else
                {
                  e = "Failed to retrieve the exception from the message from the exception from the script.";
                }
                JS_FreeValue(jsctx, jssubexc);
              }
              else if (JS_IsString(jsmsg))
              {
                size_t unLength = 0;
                const char *pszMessage = JS_ToCStringLen(jsctx, &unLength, jsmsg);
                if (pszMessage != NULL)
                {
                  JSValue jsstk = JS_GetPropertyStr(jsctx, jsexc, "stack");
                  e.assign(pszMessage, unLength);
                  if (JS_IsString(jsstk))
                  {
                    const char *pszStk = JS_ToCString(jsctx, jsstk);
                    if (pszStk != NULL)
                    {
                      e += (string)" " + pszStk;
                      JS_FreeCString(jsctx, pszStk);
                    }
                  }
                  JS_FreeValue(jsctx, jsstk);
                  JS_FreeCString(jsctx, pszMessage);
                }
                else
                {
                  e = "Failed to retrieve the message within the exception from the script.";
                }
              }
              else
              {
                e = "Encountered a non-string message within the exception from the script.";
              }
              JS_FreeValue(jsctx, jsmsg);
            }
            else
            {
              e = "Encountered a non-error exception from the script.";
            }
            JS_FreeValue(jsctx, jsexc);
          }
          else
          {
            JSValue jserror = JS_GetPropertyStr(jsctx, jsglobal, "g_js_error"), jsresponse = JS_GetPropertyStr(jsctx, jsglobal, "g_js_response"), jsstatus = JS_GetPropertyStr(jsctx, jsglobal, "g_js_status");
            const char *pszError = JS_ToCString(jsctx, jserror), *pszResponse = JS_ToCString(jsctx, jsresponse), *pszStatus = JS_ToCString(jsctx, jsstatus);
            b = true;
            if (pszStatus != NULL)
            {
              o->i("Status", pszStatus);
              JS_FreeCString(jsctx, pszStatus);
            }
            JS_FreeValue(jsctx, jsstatus);
            if (pszError != NULL)
            {
              o->i("Error", pszError);
              JS_FreeCString(jsctx, pszError);
            }
            JS_FreeValue(jsctx, jserror);
            if (pszResponse != NULL)
            {
              Json *ptResponse = new Json(pszResponse);
              o->i("Response", ptResponse);
              delete ptResponse;
              JS_FreeCString(jsctx, pszResponse);
            }
            JS_FreeValue(jsctx, jsresponse);
          }
          JS_FreeValue(jsctx, jsglobal);
          JS_FreeValue(jsctx, jsres);
          JS_FreeContext(jsctx);
          JS_FreeRuntime(jsrun);
        }
        else
        {
          e = "Script not found.";
        }
        delete ptFlow;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// {{{ flowRemove()
bool Maestro::flowRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Flow", "Plan"}, i, e))
  {
    string f = i->m["Flow"]->v, p = i->m["Plan"]->v;
    if (isOwner(d, p))
    {
      string t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->f.find(f) != m_p[p]->f.end())
        {
          if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "f", f}, e))
          {
            b = true;
            delete m_p[p]->f[f];
            m_p[p]->f.erase(f);
            if (i->val({"_broadcast"}) != "1" && m_p[p]->f.empty() && !dataDirectoryRemove(m_strHandle, {p, "f"}, e))
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | " << f << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | " << f << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
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
          ptLink->i("Function", "flowRemove");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink, false);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << p << " | " << f << " " << char(3) << " " << char(2) << char(3) << "03Flow removed by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "flowRemove");
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

  return b;
}
// }}}
// {{{ flows()
bool Maestro::flows(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Plan"}, i, e))
  {
    string p = i->m["Plan"]->v;
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      b = true;
      for (auto &f : m_p[p]->f)
      {
        o->pb(f.first);
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
  }

  return b;
}
// }}}
// {{{ flowUpdate()
bool Maestro::flowUpdate(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Flow", "Plan", "Script"}, i, e))
  {
    string f = i->m["Flow"]->v, p = i->m["Plan"]->v;
    if (isOwner(d, p))
    {
      string id, t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      getline(ssP, id);
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->f.find(f) != m_p[p]->f.end())
        {
          string j;
          i->j(j);
          if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "f", f}, j, e))
          {
            b = true;
            if (m_p[p]->f.find(f) != m_p[p]->f.end())
            {
              delete m_p[p]->f[f];
            }
            m_p[p]->f[f] = new Json(i);
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | " << f << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
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
          ptLink->i("Function", "flowUpdate");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink, false);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << p << " | " << f << " " << char(3) << " " << char(2) << char(3) << "03Flow updated by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "flowUpdate");
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

  return b;
}
// }}}
// }}}
// {{{ haveAccess
// {{{ haveAccess()
bool Maestro::haveAccess(const string strPlan, const string strAccess, const string strScope, string &strError, const bool bMutex)
{
  bool bResult = false;

  if (!strPlan.empty() && !strAccess.empty() && !strScope.empty())
  {
    if (bMutex)
    {
      m_mutex.lock();
    }
    if (m_p.find(strPlan) != m_p.end() && m_p[strPlan]->a != NULL && m_p[strPlan]->a->exist({"a", strAccess}))
    {
      for (auto i = m_p[strPlan]->a->m["a"]->m[strAccess]->l.begin(); !bResult && i != m_p[strPlan]->a->m["a"]->m[strAccess]->l.end(); i++)
      {
        if ((*i)->v == strScope)
        {
          bResult = true;
        }
      }
    }
    if (bMutex)
    {
      m_mutex.unlock();
    }
  }
  if (!bResult)
  {
    strError = "Plan is not authorized to perform this action.";
  }

  return bResult;
}
bool Maestro::haveAccess(JSContext *jsctx, const string strAccess, const string strScope, string &strError)
{
  bool bResult = false;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  JSValue jsplan = JS_GetPropertyStr(jsctx, jsglobal, "g_js_plan");

  if (JS_IsString(jsplan))
  {
    const char *pszPlan = JS_ToCString(jsctx, jsplan);
    if (pszPlan != NULL)
    {
      string strPlan;
      if (!m_strSecret.empty() && !m_manip.decryptAes(m_manip.decodeBase64(pszPlan, strPlan), m_strSecret, strPlan, strError, m_strCipher).empty() && haveAccess(strPlan, strAccess, strScope, strError))
      {
        bResult = true;
      }
      JS_FreeCString(jsctx, pszPlan);
    }
  }
  JS_FreeValue(jsctx, jsplan);
  JS_FreeValue(jsctx, jsglobal);

  return bResult;
}
// }}}
// {{{ haveAccessRequest()
bool Maestro::haveAccessRequest(const string strPlan, const string strAccess, const string strScope)
{
  bool bResult = false;

  if (!strPlan.empty() && !strAccess.empty() && !strScope.empty())
  {
    if (m_p.find(strPlan) != m_p.end() && m_p[strPlan]->a != NULL && m_p[strPlan]->a->exist({"r", strAccess}))
    {
      for (auto i = m_p[strPlan]->a->m["r"]->m[strAccess]->l.begin(); !bResult && i != m_p[strPlan]->a->m["r"]->m[strAccess]->l.end(); i++)
      {
        if ((*i)->v == strScope)
        {
          bResult = true;
        }
      }
    }
  }

  return bResult;
}
// }}}
// }}}
// {{{ isOwner()
bool Maestro::isOwner(radialUser &d, const string p)
{
  bool b = false;
  string e;

  if (isValid(d))
  {
    radialUser u;
    userInit(d, u);
    u.p->m["i"]->i("userid", d.u);
    if (user(u, e) && !u.p->empty({"o", "id"}) && isOwner(u.p->m["o"]->m["id"]->v, p))
    {
      b = true;
    }
    userDeinit(u);
  }
  else if (auth(d.r, e) && !d.r->empty({"User"}) && m_applications.find(d.r->m["User"]->v) != m_applications.end())
  {
    map<string, string> getApplicationRow;
    Json *ptApplication = new Json;
    ptApplication->i("name", m_applications[d.r->m["User"]->v]);
    if (db("dbCentralApplications", ptApplication, getApplicationRow, e) && !getApplicationRow["id"].empty() && isOwner(getApplicationRow["id"], p, true))
    {
      b = true;
      d.l = m_applications[d.r->m["User"]->v];
    }
    delete ptApplication;
  }

  return b;
}
bool Maestro::isOwner(const string id, const string p, const bool a)
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
      if (t == "a")
      {
        if (!a)
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
      else if (!a && id == i)
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
// {{{ js_
// {{{ js_alert()
JSValue Maestro::js_alert(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc >= 2 && argc <= 3)
  {
    const char *pszUser = JS_ToCString(jsctx, argv[0]), *pszMessage = JS_ToCString(jsctx, argv[1]), *pszHtml = ((argc >= 3)?JS_ToCString(jsctx, argv[2]):NULL);
    if (pszUser != NULL && pszMessage != NULL)
    {
      if (alert(pszUser, pszMessage, ((pszHtml != NULL)?pszHtml:""), e))
      {
        b = true;
      }
    }
    else
    {
      e = "Please provide the user and message.";
    }
    if (pszUser != NULL)
    {
      JS_FreeCString(jsctx, pszUser);
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
    if (pszHtml != NULL)
    {
      JS_FreeCString(jsctx, pszHtml);
    }
  }
  else
  {
    e = "Please provide the user and message.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_application()
JSValue Maestro::js_application(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszApplication = JS_ToCString(jsctx, argv[0]), *pszMessage = JS_ToCString(jsctx, argv[1]);
    if (pszApplication != NULL && pszMessage != NULL)
    {
      Json *ptMessage = new Json(pszMessage);
      if (application(pszApplication, ptMessage, e))
      {
        b = true;
      }
      delete ptMessage;
    }
    else
    {
      e = "Please provide the application and message.";
    }
    if (pszApplication != NULL)
    {
      JS_FreeCString(jsctx, pszApplication);
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
  }
  else
  {
    e = "Please provide the appllication and message.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_centralmon()
JSValue Maestro::js_centralmon(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszServer = JS_ToCString(jsctx, argv[0]), *pszProcess = JS_ToCString(jsctx, argv[1]);
    if (pszServer != NULL && pszProcess != NULL)
    {
      string strResponse;
      Json *ptResponse = new Json;
      ptResponse->m["Data"] = new Json;
      if (centralmon(pszServer, pszProcess, ptResponse->m["Data"], e))
      {
        b = true;
      }
      ptResponse->j(strResponse);
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
      delete ptResponse;
    }
    else
    {
      e = "Please provide the server and process.";
    }
    if (pszServer != NULL)
    {
      JS_FreeCString(jsctx, pszServer);
    }
    if (pszProcess != NULL)
    {
      JS_FreeCString(jsctx, pszProcess);
    }
  }
  else
  {
    e = "Please provide the server and process.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_chat()
JSValue Maestro::js_chat(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;

  if (argc >= 2 && argc <= 3)
  {
    const char *pszTarget = JS_ToCString(jsctx, argv[0]), *pszMessage = JS_ToCString(jsctx, argv[1]), *pszSource = ((argc >= 3)?JS_ToCString(jsctx, argv[2]):NULL);
    if (pszTarget != NULL && pszMessage != NULL && (argc < 3 || pszSource != NULL))
    {
      b = true;
      chat(pszTarget, pszMessage, ((argc >= 3)?pszSource:""));
    }
    if (pszTarget != NULL)
    {
      JS_FreeCString(jsctx, pszTarget);
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
    if (pszSource != NULL)
    {
      JS_FreeCString(jsctx, pszSource);
    }
  }

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_command()
JSValue Maestro::js_command(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e, p;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 5)
  {
    const char *pszCommand = JS_ToCString(jsctx, argv[0]), *pszArguments = JS_ToCString(jsctx, argv[1]), *pszInput = JS_ToCString(jsctx, argv[2]), *pszTimeout = JS_ToCString(jsctx, argv[3]), *pszNode = JS_ToCString(jsctx, argv[4]);
    if (pszCommand != NULL && pszArguments != NULL && pszInput != NULL && pszTimeout != NULL && pszNode != NULL)
    {
      if (haveAccess(jsctx, "command", "all", e))
      {
        list<string> arguments;
        string strOutput, strResponse;
        stringstream ssDuration, ssTimeout(pszTimeout);
        size_t unDuration = 0;
        time_t CTimeout;
        Json *ptArguments = new Json(pszArguments), *ptResponse = new Json;
        for (auto &i : ptArguments->l)
        {
          arguments.push_back(i->v);
        }
        delete ptArguments;
        ssTimeout >> CTimeout;
        ptResponse->m["Output"] = new Json;
        if (command(pszCommand, arguments, pszInput, strOutput, unDuration, e, CTimeout, pszNode))
        {
          b = true;
        }
        ptResponse->i("Output", strOutput);
        ssDuration << unDuration;
        ptResponse->i("Duration", ssDuration.str(), 'n');
        ptResponse->j(strResponse);
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
        delete ptResponse;
      }
    }
    else
    {
      e = "Please provide the command, arguments, input, timeout, and node.";
    }
    if (pszCommand != NULL)
    {
      JS_FreeCString(jsctx, pszCommand);
    }
    if (pszArguments != NULL)
    {
      JS_FreeCString(jsctx, pszArguments);
    }
    if (pszInput != NULL)
    {
      JS_FreeCString(jsctx, pszInput);
    }
    if (pszTimeout != NULL)
    {
      JS_FreeCString(jsctx, pszTimeout);
    }
    if (pszNode != NULL)
    {
      JS_FreeCString(jsctx, pszNode);
    }
  }
  else
  {
    e = "Please provide the command, arguments, input, timeout, and node.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_db()
JSValue Maestro::js_db(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszFunction = JS_ToCString(jsctx, argv[0]), *pszInput = JS_ToCString(jsctx, argv[1]);
    if (pszFunction != NULL && pszInput != NULL)
    {
      if (haveAccess(jsctx, "db", pszFunction, e))
      {
        string strID, strQuery, strResponse;
        Json *ptInput = new Json(pszInput), *ptResponse = new Json;
        ptResponse->m["Output"] = new Json;
        if (db(pszFunction, ptInput, ptResponse->m["Output"], strID, strQuery, e))
        {
          b = true;
        }
        delete ptInput;
        ptResponse->i("ID", strID);
        ptResponse->i("Query", strQuery);
        ptResponse->j(strResponse);
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
        delete ptResponse;
      }
    }
    else
    {
      e = "Please provide the function and input.";
    }
    if (pszFunction != NULL)
    {
      JS_FreeCString(jsctx, pszFunction);
    }
    if (pszInput != NULL)
    {
      JS_FreeCString(jsctx, pszInput);
    }
  }
  else
  {
    e = "Please provide the function and input.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_dbquery()
JSValue Maestro::js_dbquery(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszDatabase = JS_ToCString(jsctx, argv[0]), *pszQuery = JS_ToCString(jsctx, argv[1]);
    if (pszDatabase != NULL && pszQuery != NULL)
    {
      if (haveAccess(jsctx, "dbquery", pszDatabase, e))
      {
        string strResponse;
        stringstream ssRows;
        unsigned long long ullRows = 0;
        Json *ptResponse = new Json;
        ptResponse->m["Response"] = new Json;
        auto get = dbquery(pszDatabase, pszQuery, ullRows, e);
        if (get != NULL)
        {
          b = true;
          for (auto &row : *get)
          {
            ptResponse->m["Response"]->pb(row);
          }
        }
        dbfree(get);
        ssRows << ullRows;
        ptResponse->i("Rows", ssRows.str(), 'n');
        ptResponse->j(strResponse);
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
        delete ptResponse;
      }
    }
    else
    {
      e = "Please provide the database and query.";
    }
    if (pszDatabase != NULL)
    {
      JS_FreeCString(jsctx, pszDatabase);
    }
    if (pszQuery != NULL)
    {
      JS_FreeCString(jsctx, pszQuery);
    }
  }
  else
  {
    e = "Please provide the database and query.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_dbupdate()
JSValue Maestro::js_dbupdate(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszDatabase = JS_ToCString(jsctx, argv[0]), *pszUpdate = JS_ToCString(jsctx, argv[1]);
    if (pszDatabase != NULL && pszUpdate != NULL)
    {
      if (haveAccess(jsctx, "dbupdate", pszDatabase, e))
      {
        string strResponse;
        stringstream ssID, ssRows;
        unsigned long long ullID = 0, ullRows = 0;
        Json *ptResponse = new Json;
        ptResponse->m["Response"] = new Json;
        if (dbupdate(pszDatabase, pszUpdate, ullID, ullRows, e))
        {
          b = true;
        }
        ssID << ullID;
        ptResponse->i("ID", ssID.str(), 'n');
        ssRows << ullRows;
        ptResponse->i("Rows", ssRows.str(), 'n');
        ptResponse->j(strResponse);
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
        delete ptResponse;
      }
    }
    else
    {
      e = "Please provide the database and update.";
    }
    if (pszDatabase != NULL)
    {
      JS_FreeCString(jsctx, pszDatabase);
    }
    if (pszUpdate != NULL)
    {
      JS_FreeCString(jsctx, pszUpdate);
    }
  }
  else
  {
    e = "Please provide the database and update.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_email()
JSValue Maestro::js_email(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 8)
  {
    const char *pszFrom = JS_ToCString(jsctx, argv[0]), *pszTo = JS_ToCString(jsctx, argv[1]), *pszCc = JS_ToCString(jsctx, argv[2]), *pszBcc = JS_ToCString(jsctx, argv[3]), *pszSubject = JS_ToCString(jsctx, argv[4]), *pszText = JS_ToCString(jsctx, argv[5]), *pszHtml = JS_ToCString(jsctx, argv[6]), *pszFile = JS_ToCString(jsctx, argv[7]);
    if (pszFrom != NULL && pszTo != NULL && pszCc != NULL && pszBcc != NULL && pszSubject != NULL && pszText != NULL && pszHtml != NULL && pszFile != NULL)
    {
      list<string> cc, bcc, to;
      map<string, string> file;
      Json *ptTo = new Json(pszTo), *ptCc = new Json(pszCc), *ptBcc = new Json(pszBcc), *ptFile = new Json(pszFile);
      b = true;
      for (auto &i : ptTo->l)
      {
        to.push_back(i->v);
      }
      delete ptTo;
      for (auto &i : ptCc->l)
      {
        cc.push_back(i->v);
      }
      delete ptCc;
      for (auto &i : ptBcc->l)
      {
        bcc.push_back(i->v);
      }
      delete ptBcc;
      ptFile->flatten(file, true, false);
      delete ptFile;
      email(pszFrom, to, cc, bcc, pszSubject, pszText, pszHtml, file);
    }
    else
    {
      e = "Please provide the from, to, cc, bcc, subject, text, html, file.";
    }
    if (pszFrom != NULL)
    {
      JS_FreeCString(jsctx, pszFrom);
    }
    if (pszTo != NULL)
    {
      JS_FreeCString(jsctx, pszTo);
    }
    if (pszCc != NULL)
    {
      JS_FreeCString(jsctx, pszCc);
    }
    if (pszBcc != NULL)
    {
      JS_FreeCString(jsctx, pszBcc);
    }
    if (pszSubject != NULL)
    {
      JS_FreeCString(jsctx, pszSubject);
    }
    if (pszText != NULL)
    {
      JS_FreeCString(jsctx, pszText);
    }
    if (pszHtml != NULL)
    {
      JS_FreeCString(jsctx, pszHtml);
    }
    if (pszFile != NULL)
    {
      JS_FreeCString(jsctx, pszFile);
    }
  }
  else
  {
    e = "Please provide the from, to, cc, bcc, subject, text, html, file.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_flow()
JSValue Maestro::js_flow(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e, sp;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  JSValue jsplan = JS_GetPropertyStr(jsctx, jsglobal, "g_js_plan");

  if (JS_IsString(jsplan))
  {
    const char *pszPlan = JS_ToCString(jsctx, jsplan);
    if (pszPlan != NULL)
    {
      if (!m_strSecret.empty())
      {
        m_manip.decryptAes(m_manip.decodeBase64(pszPlan, sp), m_strSecret, sp, e, m_strCipher);
      }
      JS_FreeCString(jsctx, pszPlan);
    }
  }
  JS_FreeValue(jsctx, jsplan);
	if (argc >= 2 && argc <= 3)
  {
    const char *pszPlan = JS_ToCString(jsctx, argv[0]), *pszFlow = JS_ToCString(jsctx, argv[1]), *pszRequest = ((argc >= 3)?JS_ToCString(jsctx, argv[2]):NULL);
    if (pszPlan != NULL && pszFlow != NULL)
    {
      string f = pszFlow, p = pszPlan;
      Json *ptFlow = NULL;
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->f.find(f) != m_p[p]->f.end())
        {
          ptFlow = new Json(m_p[p]->f[f]);
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
      if (ptFlow != NULL)
      {
        if (sp == pszPlan || haveAccess(jsctx, pszPlan, pszFlow, e))
        {
          if (!ptFlow->empty({"Script"}))
          {
            JSRuntime *jssubrun = JS_NewRuntime();
            JSContext *jssubctx = JS_NewContext(jssubrun);
            JSValue jssubglobal = JS_GetGlobalObject(jssubctx);
            for (auto &i : m_javascript)
            {
              JS_SetPropertyStr(jssubctx, jssubglobal, i.first.c_str(), JS_NewCFunction(jssubctx, i.second->func, i.first.c_str(), i.second->nLength));
            }
            if (!m_strSecret.empty())
            {
              string strPlan;
              if (!m_manip.encodeBase64(m_manip.encryptAes(p, m_strSecret, strPlan, e, m_strCipher), strPlan).empty())
              {
                JS_SetPropertyStr(jssubctx, jssubglobal, "g_js_plan", JS_NewStringLen(jssubctx, strPlan.c_str(), strPlan.size()));
              }
            }
            if (pszRequest != NULL)
            {
              JS_SetPropertyStr(jssubctx, jssubglobal, "g_js_request", JS_NewStringLen(jssubctx, pszRequest, strlen(pszRequest)));
            }
            JSValue jssubres = JS_Eval(jssubctx, ptFlow->m["Script"]->v.data(), ptFlow->m["Script"]->v.size(), "<embedded>", JS_EVAL_TYPE_GLOBAL);
            if (JS_IsException(jssubres))
            {
              JSValue jssubexc = JS_GetException(jssubctx);
              if (JS_IsError(jssubctx, jssubexc))
              {
                JSValue jssubmsg = JS_GetPropertyStr(jssubctx, jssubexc, "message");
                if (JS_IsException(jssubmsg))
                {
                  JSValue jsdeepexc = JS_GetException(jssubctx);
                  const char *pszMessage = JS_ToCString(jssubctx, jsdeepexc);
                  if (pszMessage != NULL)
                  {
                    e = pszMessage;
                    JS_FreeCString(jssubctx, pszMessage);
                  }
                  else
                  {
                    e = "Failed to retrieve the exception from the message from the exception from the script.";
                  }
                  JS_FreeValue(jssubctx, jsdeepexc);
                }
                else if (JS_IsString(jssubmsg))
                {
                  size_t unLength = 0;
                  const char *pszMessage = JS_ToCStringLen(jssubctx, &unLength, jssubmsg);
                  if (pszMessage != NULL)
                  {
                    JSValue jssubstk = JS_GetPropertyStr(jssubctx, jssubexc, "stack");
                    e.assign(pszMessage, unLength);
                    if (JS_IsString(jssubstk))
                    {
                      const char *pszStk = JS_ToCString(jssubctx, jssubstk);
                      if (pszStk != NULL)
                      {
                        e += (string)" " + pszStk;
                        JS_FreeCString(jssubctx, pszStk);
                      }
                    }
                    JS_FreeValue(jssubctx, jssubstk);
                    JS_FreeCString(jssubctx, pszMessage);
                  }
                  else
                  {
                    e = "Failed to retrieve the message within the exception from the script.";
                  }
                }
                else
                {
                  e = "Encountered a non-string message within the exception from the script.";
                }
              }
              else
              {
                e = "Encountered a non-error exception from the script.";
              }
              JS_FreeValue(jssubctx, jssubexc);
            }
            else
            {
              string strResponse;
              JSValue jssuberror = JS_GetPropertyStr(jssubctx, jssubglobal, "g_js_error"), jssubresponse = JS_GetPropertyStr(jssubctx, jssubglobal, "g_js_response"), jssubstatus = JS_GetPropertyStr(jssubctx, jssubglobal, "g_js_status");
              const char *pszError = JS_ToCString(jssubctx, jssuberror), *pszResponse = JS_ToCString(jssubctx, jssubresponse), *pszStatus = JS_ToCString(jssubctx, jssubstatus);
              Json *ptResponse = new Json;
              b = true;
              if (pszStatus != NULL)
              {
                ptResponse->i("Status", pszStatus);
                JS_FreeCString(jssubctx, pszStatus);
              }
              JS_FreeValue(jsctx, jssubstatus);
              if (pszError != NULL)
              {
                ptResponse->i("Error", pszError);
                JS_FreeCString(jssubctx, pszError);
              }
              JS_FreeValue(jsctx, jssuberror);
              if (pszResponse != NULL)
              {
                ptResponse->m["Response"] = new Json(pszResponse);
                JS_FreeCString(jssubctx, pszResponse);
              }
              JS_FreeValue(jsctx, jssubresponse);
              ptResponse->j(strResponse);
              delete ptResponse;
              JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
            }
            JS_FreeValue(jssubctx, jssubglobal);
            JS_FreeValue(jssubctx, jssubres);
            JS_FreeContext(jssubctx);
            JS_FreeRuntime(jssubrun);
          }
          else
          {
            e = "Script not found.";
          }
        }
        delete ptFlow;
      }
    }
    else
    {
      e = "Please provide the plan and flow.";
    }
    if (pszRequest != NULL)
    {
      JS_FreeCString(jsctx, pszRequest);
    }
    if (pszFlow != NULL)
    {
      JS_FreeCString(jsctx, pszFlow);
    }
    if (pszPlan != NULL)
    {
      JS_FreeCString(jsctx, pszPlan);
    }
  }
  else
  {
    e = "Please provide the plan and flow.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_getError()
JSValue Maestro::js_getError(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  JSValue jserror = JS_GetPropertyStr(jsctx, jsglobal, "g_js_error");

  JS_FreeValue(jsctx, jsglobal);

  return jserror;
}
// }}}
// {{{ js_getRequest()
JSValue Maestro::js_getRequest(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  JSValue jsrequest = JS_GetPropertyStr(jsctx, jsglobal, "g_js_request");

  JS_FreeValue(jsctx, jsglobal);

  return jsrequest;
}
// }}}
// {{{ js_getResponse()
JSValue Maestro::js_getResponse(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  JSValue jsresponse = JS_GetPropertyStr(jsctx, jsglobal, "g_js_internal_response");

  JS_FreeValue(jsctx, jsglobal);

  return jsresponse;
}
// }}}
// {{{ js_getStatus()
JSValue Maestro::js_getStatus(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  JSValue jsstatus = JS_GetPropertyStr(jsctx, jsglobal, "g_js_internal_status");

  JS_FreeValue(jsctx, jsglobal);

  return jsstatus;
}
// }}}
// {{{ js_live()
JSValue Maestro::js_live(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc >= 3 && argc <= 4)
  {
    const char *pszApplication = JS_ToCString(jsctx, argv[0]), *pszUser = JS_ToCString(jsctx, argv[1]), *pszMessage = JS_ToCString(jsctx, argv[2]), *pszWait = ((argc >= 4)?JS_ToCString(jsctx, argv[3]):NULL);
    if (pszApplication != NULL && pszUser != NULL && pszMessage != NULL && (argc < 4 || pszWait != NULL))
    {
      bool bWait = (pszWait != NULL && (string)pszWait == "true");
      Json *ptMessage = new Json(pszMessage);
      b = true;
      live(pszApplication, pszUser, ptMessage, bWait);
      delete ptMessage;
    }
    else
    {
      e = "Please provide the application, user, and message.";
    }
    if (pszApplication != NULL)
    {
      JS_FreeCString(jsctx, pszApplication);
    }
    if (pszUser != NULL)
    {
      JS_FreeCString(jsctx, pszUser);
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
    if (pszWait != NULL)
    {
      JS_FreeCString(jsctx, pszWait);
    }
  }
  else
  {
    e = "Please provide the application, user, and message.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_log()
JSValue Maestro::js_log(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszMessage = JS_ToCString(jsctx, argv[0]);
    if (pszMessage != NULL)
    {
      stringstream ssMessage;
      b = true;
      ssMessage << "maestro->main()->Maestro::js_log():  " << pszMessage;
      log(ssMessage.str());
    }
    else
    {
      e = "Please provide the message.";
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
  }
  else
  {
    e = "Please provide the message.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_setError()
JSValue Maestro::js_setError(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszError = JS_ToCString(jsctx, argv[0]);
    if (pszError != NULL)
    {
      b = true;
      JSValue jsglobal = JS_GetGlobalObject(jsctx);
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, pszError, strlen(pszError)));
      JS_FreeValue(jsctx, jsglobal);
    }
    else
    {
      e = "Please provide the error.";
    }
    if (pszError != NULL)
    {
      JS_FreeCString(jsctx, pszError);
    }
  }
  else
  {
    e = "Please provide the error.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_setRequest()
JSValue Maestro::js_setRequest(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszRequest = JS_ToCString(jsctx, argv[0]);
    if (pszRequest != NULL)
    {
      b = true;
      JSValue jsglobal = JS_GetGlobalObject(jsctx);
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_request", JS_NewStringLen(jsctx, pszRequest, strlen(pszRequest)));
      JS_FreeValue(jsctx, jsglobal);
    }
    else
    {
      e = "Please provide the nequest.";
    }
    if (pszRequest != NULL)
    {
      JS_FreeCString(jsctx, pszRequest);
    }
  }
  else
  {
    e = "Please provide the request.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_setResponse()
JSValue Maestro::js_setResponse(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszResponse = JS_ToCString(jsctx, argv[0]);
    if (pszResponse != NULL)
    {
      b = true;
      JSValue jsglobal = JS_GetGlobalObject(jsctx);
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_response", JS_NewStringLen(jsctx, pszResponse, strlen(pszResponse)));
      JS_FreeValue(jsctx, jsglobal);
    }
    else
    {
      e = "Please provide the response.";
    }
    if (pszResponse != NULL)
    {
      JS_FreeCString(jsctx, pszResponse);
    }
  }
  else
  {
    e = "Please provide the response.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_setStatus()
JSValue Maestro::js_setStatus(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszStatus = JS_ToCString(jsctx, argv[0]);
    if (pszStatus != NULL)
    {
      b = true;
      JSValue jsglobal = JS_GetGlobalObject(jsctx);
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_status", JS_NewStringLen(jsctx, pszStatus, strlen(pszStatus)));
      JS_FreeValue(jsctx, jsglobal);
    }
    else
    {
      e = "Please provide the status.";
    }
    if (pszStatus != NULL)
    {
      JS_FreeCString(jsctx, pszStatus);
    }
  }
  else
  {
    e = "Please provide the status.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// }}}
// {{{ jsreg()
void Maestro::jsreg(const char *prop, JSCFunction *func, const int nLength, const string strPrototype, const list<string> scopes, const string strDescription)
{
  radialMaestroJavascript *ptJavascript = new radialMaestroJavascript;

  ptJavascript->func = func;
  ptJavascript->nLength = nLength;
  ptJavascript->scopes = scopes;
  ptJavascript->strDescription = strDescription;
  ptJavascript->strPrototype = strPrototype;
  if (m_javascript.find(prop) != m_javascript.end())
  {
    delete m_javascript[prop];
  }
  m_javascript[prop] = ptJavascript;
}
// }}}
// {{{ load()
void Maestro::load(string strPrefix)
{
  string e;
  Json *ptCred = new Json;

  strPrefix += "->Maestro::load()";
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"maestro", "Secret"}, m_strSecret, e) && m_pWarden->vaultRetrieve({"radial"}, ptCred, e))
  {
    m_mutex.lock();
    m_applications.clear();
    for (auto &i : ptCred->m)
    {
      if (!i.second->empty({"Application"}))
      {
        m_applications[i.first] = i.second->m["Application"]->v;
      }
    }
    m_mutex.unlock();
  }
  delete ptCred;
  if (!m_bLoaded)
  {
    bool bLoaded = false;
    map<string, radialMaestroPlan *> p;
    stringstream ssChat;
    Json *l = NULL;
    if (dataDirectoryList(m_strHandle, {}, &l, e))
    {
      bLoaded = true;
      for (auto &i : l->m)
      {
        if (i.second->val({"Type"}) == "directory" && (i.first.substr(0, 2) == "a_" || i.first.substr(0, 2) == "u_"))
        {
          map<string, string> row;
          string id, t;
          stringstream ssP(i.first);
          Json *sl = NULL, *ptData = new Json;
          radialMaestroPlan *ptPlan = new radialMaestroPlan;
          getline(ssP, t, '_');
          getline(ssP, id);
          ptPlan->a = NULL;
          ptPlan->id = id;
          ptPlan->t = t[0];
          ptData->i("id", id);
          if (db(((ptPlan->t == 'a')?"dbCentralApplications":"dbCentralUsers"), ptData, row, e))
          {
            stringstream ssOwner;
            if (t == "a")
            {
              ssOwner << row["name"];
            }
            else
            {
              ssOwner << row["first_name"] << " " << row["last_name"];
            }
            ptPlan->owner = ssOwner.str();
            if (dataDirectoryList(m_strHandle, {i.first}, &sl, e))
            {
              for (auto &j : sl->m)
              {
                if (j.second->val({"Type"}) == "regular file" && j.first == "a")
                {
                  string b;
                  if (dataRead(m_strHandle, {i.first, j.first}, b, e))
                  {
                    ptPlan->a = new Json(b);
                  }
                  else
                  {
                    bLoaded = false;
                    ssChat.str("");
                    ssChat << char(2) << char(3) << "07Interface::dataRead() [" << m_strHandle << "," << i.first << "," << j.first << "] " << e << char(3) << char(2);
                    chat("#maestro", ssChat.str());
                  }
                }
                else if (j.second->val({"Type"}) == "directory" && j.first == "f")
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
            ssChat << char(2) << char(3) << "07Interface::db(dbCentral" << ((ptPlan->t == 'a')?"application":"user") << "s) [" << m_strHandle << "," << i.first << "] " << e << char(3) << char(2);
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
}
// }}}
// {{{ plan
// {{{ plan()
bool Maestro::plan(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Plan"}, i, e))
  {
    string p = i->m["Plan"]->v;
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      bool bEdit = isOwner(d, p);
      stringstream ssPlan, ssType;
      b = true;
      o->i("edit", ((bEdit)?"1":"0"), ((bEdit)?'1':'0'));
      o->i("ID", m_p[p]->id);
      o->i("NumAccesses", to_string((m_p[p]->a != NULL && m_p[p]->a->exist({"a"}))?m_p[p]->a->m["a"]->m.size():0), 'n');
      o->i("NumFlows", to_string(m_p[p]->f.size()), 'n');
      o->i("Owner", m_p[p]->owner);
      ssPlan << m_p[p]->t << "_" << m_p[p]->id;
      o->i("Plan", ssPlan.str());
      ssType << m_p[p]->t;
      o->i("Type", ssType.str());
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
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
            m_p[p]->a = NULL;
            m_p[p]->id = id;
            m_p[p]->t = t[0];
            if (m_p[p]->t == 'a')
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
            ssChat << char(3) << "00,06 " << p << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryAdd() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
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

  return b;
}
// }}}
// {{{ planRemove()
bool Maestro::planRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Plan"}, i, e))
  {
    string p = i->m["Plan"]->v;
    if (isOwner(d, p))
    {
      string t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (i->val({"_broadcast"}) == "1" || dataDirectoryRemove(m_strHandle, {p}, e))
        {
          b = true;
          if (m_p[p]->a != NULL)
          {
            delete m_p[p]->a;
          }
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
          ssChat << char(3) << "00,06 " << p << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
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
          ptLink->i("Function", "planRemove");
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

  return b;
}
// }}}
// {{{ plans()
bool Maestro::plans(radialUser &d, string &e)
{
  bool b = true;
  Json *o = d.p->m["o"];

  m_mutex.lock();
  for (auto &p : m_p)
  {
    bool bEdit = isOwner(d, p.first);
    stringstream ssType;
    Json *ptPlan = new Json;
    ptPlan->i("edit", ((bEdit)?"1":"0"), ((bEdit)?'1':'0'));
    ptPlan->i("ID", p.second->id);
    ptPlan->i("NumAccesses", to_string((p.second->a != NULL && p.second->a->exist({"a"}))?p.second->a->m["a"]->m.size():0), 'n');
    ptPlan->i("NumFlows", to_string(p.second->f.size()), 'n');
    ptPlan->i("Owner", p.second->owner);
    ptPlan->i("Plan", p.first);
    ssType << p.second->t;
    ptPlan->i("Type", ssType.str());
    o->l.push_back(ptPlan);
  }
  m_mutex.unlock();

  return b;
}
// }}}
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
    if ((CTime[1] - CTime[0]) > 30)
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
// {{{ scopes()
bool Maestro::scopes(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Access", "Type"}, i, e))
  {
    string a = i->m["Access"]->v, t = i->m["Type"]->v;
    if (t == "application" || t == "user")
    {
      string p = ((t == "application")?"a_":"u_") + a;
      m_mutex.lock();
      if (m_p.find(p) != m_p.end() && ((t == "application" && m_p[p]->t == 'a') || (t == "user" && m_p[p]->t == 'u')))
      {
        b = true;
        for (auto &f : m_p[p]->f)
        {
          o->pb(f.first);
        }
      }
      else
      {
        e = "Access not found.";
      }
      m_mutex.unlock();
    }
    else if (t == "core")
    {
      if (m_javascript.find(a) != m_javascript.end())
      {
        b = true;
        for (auto &s : m_javascript[a]->scopes)
        {
          o->pb(s);
        }
      }
      else
      {
        e = "Access not found.";
      }
    }
    else
    {
      e = "Please provide a valid Type:  application, core, user.";
    }
  }

  return b;
}
// }}}
}
}
