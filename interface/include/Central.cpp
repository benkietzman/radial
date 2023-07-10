// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Central.cpp
// author     : Ben Kietzman
// begin      : 2023-02-18
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Central"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Central()
Central::Central(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "central", argc, argv, pCallback)
{
  string strError;

  m_pCallbackAddon = NULL;
  m_ptCred = new Json;
  if (m_pWarden != NULL)
  {
    m_pWarden->vaultRetrieve({"radial", "radial"}, m_ptCred, strError);
  }
  m_functions["accountType"] = &Central::accountType;
  m_functions["accountTypes"] = &Central::accountTypes;
  m_functions["application"] = &Central::application;
  m_functions["applicationAccount"] = &Central::applicationAccount;
  m_functions["applicationAccountAdd"] = &Central::applicationAccountAdd;
  m_functions["applicationAccountEdit"] = &Central::applicationAccountEdit;
  m_functions["applicationAccountRemove"] = &Central::applicationAccountRemove;
  m_functions["applicationAccountsByApplicationID"] = &Central::applicationAccountsByApplicationID;
  m_functions["applicationAdd"] = &Central::applicationAdd;
  m_functions["applicationDepend"] = &Central::applicationDepend;
  m_functions["applicationDependAdd"] = &Central::applicationDependAdd;
  m_functions["applicationDependRemove"] = &Central::applicationDependRemove;
  m_functions["applicationEdit"] = &Central::applicationEdit;
  m_functions["applicationIssue"] = &Central::applicationIssue;
  m_functions["applicationIssueAdd"] = &Central::applicationIssueAdd;
  m_functions["applicationIssueClose"] = &Central::applicationIssueClose;
  m_functions["applicationIssueCommentAdd"] = &Central::applicationIssueCommentAdd;
  m_functions["applicationIssueCommentEdit"] = &Central::applicationIssueCommentEdit;
  m_functions["applicationIssueComments"] = &Central::applicationIssueComments;
  m_functions["applicationIssueEdit"] = &Central::applicationIssueEdit;
  m_functions["applicationIssueEmail"] = &Central::applicationIssueEmail;
  m_functions["applicationIssues"] = &Central::applicationIssues;
  m_functions["applicationIssuesByApplicationID"] = &Central::applicationIssuesByApplicationID;
  m_functions["applicationNotify"] = &Central::applicationNotify;
  m_functions["applicationRemove"] = &Central::applicationRemove;
  m_functions["applications"] = &Central::applications;
  m_functions["applicationsByServerID"] = &Central::applicationsByServerID;
  m_functions["applicationsByUserID"] = &Central::applicationsByUserID;
  m_functions["applicationServer"] = &Central::applicationServer;
  m_functions["applicationServerAdd"] = &Central::applicationServerAdd;
  m_functions["applicationServerDetail"] = &Central::applicationServerDetail;
  m_functions["applicationServerDetailAdd"] = &Central::applicationServerDetailAdd;
  m_functions["applicationServerDetailEdit"] = &Central::applicationServerDetailEdit;
  m_functions["applicationServerDetailRemove"] = &Central::applicationServerDetailRemove;
  m_functions["applicationServerDetails"] = &Central::applicationServerDetails;
  m_functions["applicationServerRemove"] = &Central::applicationServerRemove;
  m_functions["applicationUser"] = &Central::applicationUser;
  m_functions["applicationUserAdd"] = &Central::applicationUserAdd;
  m_functions["applicationUserEdit"] = &Central::applicationUserEdit;
  m_functions["applicationUserRemove"] = &Central::applicationUserRemove;
  m_functions["applicationUsersByApplicationID"] = &Central::applicationUsersByApplicationID;
  m_functions["contactType"] = &Central::contactType;
  m_functions["dependentsByApplicationID"] = &Central::dependentsByApplicationID;
  m_functions["isApplicationDeveloper"] = &Central::isApplicationDeveloper;
  m_functions["isServerAdmin"] = &Central::isServerAdmin;
  m_functions["loginType"] = &Central::loginType;
  m_functions["loginTypes"] = &Central::loginTypes;
  m_functions["menuAccess"] = &Central::menuAccess;
  m_functions["menuAccesses"] = &Central::menuAccesses;
  m_functions["notifyPriorities"] = &Central::notifyPriorities;
  m_functions["notifyPriority"] = &Central::notifyPriority;
  m_functions["packageType"] = &Central::packageType;
  m_functions["packageTypes"] = &Central::packageTypes;
  m_functions["server"] = &Central::server;
  m_functions["serverAdd"] = &Central::serverAdd;
  m_functions["serverDetailsByApplicationID"] = &Central::serverDetailsByApplicationID;
  m_functions["serverEdit"] = &Central::serverEdit;
  m_functions["serverNotify"] = &Central::serverNotify;
  m_functions["serverRemove"] = &Central::serverRemove;
  m_functions["servers"] = &Central::servers;
  m_functions["serversByApplicationID"] = &Central::serversByApplicationID;
  m_functions["serversByUserID"] = &Central::serversByUserID;
  m_functions["serverUser"] = &Central::serverUser;
  m_functions["serverUserAdd"] = &Central::serverUserAdd;
  m_functions["serverUserEdit"] = &Central::serverUserEdit;
  m_functions["serverUserRemove"] = &Central::serverUserRemove;
  m_functions["serverUsersByServerID"] = &Central::serverUsersByServerID;
  m_functions["user"] = &Central::user;
  m_functions["userAdd"] = &Central::userAdd;
  m_functions["userEdit"] = &Central::userEdit;
  m_functions["userRemove"] = &Central::userRemove;
  m_functions["users"] = &Central::users;
}
// }}}
// {{{ ~Central()
Central::~Central()
{
  delete m_ptCred;
}
// }}}
// {{{ accountType()
bool Central::accountType(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "type"))
  {
    map<string, string> r;
    if (db("dbCentralAccountTypes", i, r, e))
    {
      if (!r.empty())
      {
        b = true;
        d.p->i("o", r);
      }
      else
      {
        e = "No results returned.";
      }
    }
  }
  else
  {
    e = "Please provide the id or type.";
  }

  return b;
}
// }}}
// {{{ accountTypes()
bool Central::accountTypes(radialUser &d, string &e)
{
  bool b = false;
  list<map<string, string> > rs;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (db("dbCentralAccountTypes", i, rs, e))
  {
    b = true;
    for (auto &r : rs)
    {
      o->pb(r);
    }
  }

  return b;
}
// }}}
// {{{ application()
bool Central::application(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "name"))
  {
    map<string, string> r;
    if (db("dbCentralApplications", i, r, e))
    {
      if (!r.empty())
      {
        Json *j = new Json(r);
        b = true;
        ny(j, "account_check");
        ny(j, "auto_register");
        ny(j, "dependable");
        if (!empty(j, "login_type_id"))
        {
          size_t unValue;
          stringstream ssValue(j->m["login_type_id"]->v);
          ssValue >> unValue;
          if (unValue > 0)
          {
            radialUser l;
            userInit(d, l);
            l.p->m["i"]->i("id", j->m["login_type_id"]->v);
            if (loginType(l, e))
            {
              j->i("login_type", l.p->m["o"]);
            }
            userDeinit(l);
          }
        }
        if (!empty(j, "menu_id"))
        {
          size_t unValue;
          stringstream ssValue(j->m["menu_id"]->v);
          ssValue >> unValue;
          if (unValue > 0)
          {
            radialUser m;
            userInit(d, m);
            m.p->m["i"]->i("id", j->m["menu_id"]->v);
            if (menuAccess(m, e))
            {
              j->i("menu_access", m.p->m["o"]);
            }
            userDeinit(m);
          }
        }
        if (!empty(j, "notify_priority_id"))
        {
          size_t unValue;
          stringstream ssValue(j->m["notify_priority_id"]->v);
          ssValue >> unValue;
          if (unValue > 0)
          {
            radialUser n;
            userInit(d, n);
            n.p->m["i"]->i("id", j->m["notify_priority_id"]->v);
            if (notifyPriority(n, e))
            {
              j->i("notify_priority", n.p->m["o"]);
            }
            userDeinit(n);
          }
        }
        if (!empty(j, "package_type_id"))
        {
          size_t unValue;
          stringstream ssValue(j->m["package_type_id"]->v);
          ssValue >> unValue;
          if (unValue > 0)
          {
            radialUser p;
            userInit(d, p);
            p.p->m["i"]->i("id", j->m["package_type_id"]->v);
            if (packageType(p, e))
            {
              j->i("package_type", p.p->m["o"]);
            }
            userDeinit(p);
          }
        }
        ny(j, "secure_port");
        ny(j, "wiki");
        d.p->i("o", j);
        delete j;
      }
      else
      {
        e = "No results returned.";
      }
    }
  }
  else
  {
    e = "Please provide the id or name.";
  }

  return b;
}
// }}}
// {{{ applicationAccount()
bool Central::applicationAccount(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    map<string, string> r;
    if (db("dbCentralApplicationAccounts", i, r, e))
    {
      if (!r.empty())
      {
        radialUser a;
        Json *j = new Json(r);
        userInit(d, a);
        a.p->m["i"]->i("id", j->m["application_id"]->v);
        if (d.g || isApplicationDeveloper(a, e))
        {
          b = true;
          if (j->m["encrypt"]->v == "1")
          {
            rm(j, "password");
          }
          else if (j->m["aes"]->v == "1")
          {
            if (!empty(j, "decrypted_password"))
            {
              j->i("password", j->m["decrypted_password"]->v);
            }
            else
            {
              rm(j, "password");
            }
          }
          if (exist(j, "decrypted_password"))
          {
            rm(j, "decrypted_password");
          }
          ny(j, "encrypt");
          if (!empty(j, "type_id"))
          {
            radialUser t;
            userInit(d, t);
            t.p->m["i"]->i("id", j->m["type_id"]->v);
            if (accountType(t, e))
            {
              j->i("type", t.p->m["o"]);
            }
            userDeinit(t);
          }
          d.p->i("o", j);
        }
        else
        {
          e = "You are not authorized to perform this action.";
        }
        userDeinit(a);
        delete j;
      }
      else
      {
        e = "No results returned.";
      }
    }
  }

  return b;
}
// }}}
// {{{ applicationAccountAdd()
bool Central::applicationAccountAdd(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (exist(i, "encrypt") && !empty(i->m["encrypt"], "value"))
  {
    i->i("encrypt", i->m["encrypt"]->m["value"]->v);
  }
  if (exist(i, "type") && !empty(i->m["type"], "id"))
  {
    i->i("type_id", i->m["type"]->m["id"]->v);
  }
  if (dep({"application_id", "user_id", "encrypt", "password", "type_id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["application_id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      string id, q;
      if (db("dbCentralApplicationAccountAdd", i, id, q, e))
      {
        b = true;
        o->i("id", id);
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationAccountEdit()
bool Central::applicationAccountEdit(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (exist(i, "encrypt") && !empty(i->m["encrypt"], "value"))
  {
    i->i("encrypt", i->m["encrypt"]->m["value"]->v);
  }
  if (exist(i, "type") && !empty(i->m["type"], "id"))
  {
    i->i("type_id", i->m["type"]->m["id"]->v);
  }
  if (dep({"id", "user_id", "encrypt", "password", "type_id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (applicationAccount(a, e))
    {
      b = db("dbCentralApplicationAccountUpdate", i, e);
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationAccountRemove()
bool Central::applicationAccountRemove(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (applicationAccount(a, e) && db("dbCentralApplicationAccountRemove", i, e))
    {
      b = true;
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationAccountsByApplicationID()
bool Central::applicationAccountsByApplicationID(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"application_id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["application_id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      list<map<string, string> > rs;
      if (db("dbCentralApplicationAccounts", i, rs, e))
      {
        b = true;
        for (auto &r : rs)
        {
          Json *j = new Json(r);
          if (!empty(j, "encrypt") && j->m["encrypt"]->v == "1" && exist(j, "password"))
          {
            rm(j, "password");
          }
          else if (!empty(j, "aes") && j->m["aes"]->v == "1")
          {
            if (!empty(j, "decrypted_password"))
            {
              j->i("password", j->m["decrypted_password"]->v);
            }
            else
            {
              rm(j, "password");
            }
          }
          if (exist(j, "decrypted_password"))
          {
            rm(j, "decrypted_password");
          }
          ny(j, "encrypt");
          if (!empty(j, "type_id"))
          {
            radialUser t;
            userInit(d, t);
            t.p->m["i"]->i("id", j->m["type_id"]->v);
            if (accountType(t, e))
            {
              j->i("type", t.p->m["o"]);
            }
            userDeinit(t);
          }
          o->pb(j);
          delete j;
        }
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationAdd()
bool Central::applicationAdd(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (d.g || d.auth.find("Central") != d.auth.end())
  {
    if (dep({"name"}, i, e))
    {
      radialUser a;
      userInit(d, a);
      a.p->m["i"]->i("name", i->m["name"]->v);
      if (!application(a, e) && e == "No results returned.")
      {
        string id, q;
        if (db("dbCentralApplicationAdd", i, id, q, e))
        {
          o->i("id", id);
          radialUser u;
          userInit(d, u);
          u.p->m["i"]->i("application_id", id);
          u.p->m["i"]->i("userid", d.u);
          u.p->m["i"]->m["type"] = new Json;
          u.p->m["i"]->m["type"]->i("type", "Primary Developer");
          u.p->m["i"]->m["admin"] = new Json;
          u.p->m["i"]->m["admin"]->i("value", "1", 'n');
          u.p->m["i"]->m["locked"] = new Json;
          u.p->m["i"]->m["locked"]->i("value", "0", 'n');
          u.p->m["i"]->m["notify"] = new Json;
          u.p->m["i"]->m["notify"]->i("value", "1", 'n');
          b = applicationUserAdd(u, e);
          userDeinit(u);
        }
      }
      else if (e.empty())
      {
        e = "Application already exists.";
      }
      userDeinit(a);
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ applicationDepend()
bool Central::applicationDepend(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    map<string, string> r;
    if (db("dbCentralApplicationDepends", i, r, e))
    {
      if (!r.empty())
      {
        b = true;
        d.p->i("o", r);
      }
      else
      {
        e = "No results returned.";
      }
    }
  }

  return b;
}
// }}}
// {{{ applicationDependAdd()
bool Central::applicationDependAdd(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"application_id", "dependant_id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["application_id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      string id, q;
      if (db("dbCentralApplicationDependAdd", i, id, q, e))
      {
        b = true;
        o->i("id", id);
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationDependRemove()
bool Central::applicationDependRemove(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (applicationDepend(a, e) && !empty(a.p->m["o"], "application_id"))
    {
      radialUser c;
      userInit(d, c);
      c.p->m["i"]->i("id", a.p->m["o"]->m["application_id"]->v);
      if (d.g || isApplicationDeveloper(c, e))
      {
        b = db("dbCentralApplicationDependRemove", i, e);
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      userDeinit(c);
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationEdit()
bool Central::applicationEdit(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (exist(i, "account_check") && !empty(i->m["account_check"], "value"))
  {
    i->i("account_check", i->m["account_check"]->m["value"]->v);
  }
  if (exist(i, "auto_register") && !empty(i->m["auto_register"], "value"))
  {
    i->i("auto_register", i->m["auto_register"]->m["value"]->v);
  }
  if (exist(i, "dependable") && !empty(i->m["dependable"], "value"))
  {
    i->i("dependable", i->m["dependable"]->m["value"]->v);
  }
  if (exist(i, "login_type") && !empty(i->m["login_type"], "id"))
  {
    i->i("login_type_id", i->m["login_type"]->m["id"]->v);
  }
  if (exist(i, "menu_access") && !empty(i->m["menu_access"], "id"))
  {
    i->i("menu_id", i->m["menu_access"]->m["id"]->v);
  }
  if (exist(i, "notify_priority") && !empty(i->m["notify_priority"], "id"))
  {
    i->i("notify_priority_id", i->m["notify_priority"]->m["id"]->v);
  }
  if (exist(i, "secure_port") && !empty(i->m["secure_port"], "value"))
  {
    i->i("secure_port", i->m["secure_port"]->m["value"]->v);
  }
  if (exist(i, "wiki") && !empty(i->m["wiki"], "value"))
  {
    i->i("wiki", i->m["wiki"]->m["value"]->v);
  }
  if (dep({"id", "name"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      b = db("dbCentralApplicationUpdate", i, e);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationIssue()
bool Central::applicationIssue(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    map<string, string> r;
    if (db("dbCentralApplicationIssues", i, r, e))
    {
      if (!r.empty())
      {
        Json *j = new Json(r);
        b = true;
        if (!empty(j, "assigned_id"))
        {
          radialUser a;
          userInit(d, a);
          a.p->m["i"]->i("id", j->m["assigned_id"]->v);
          if (user(a, e))
          {
            j->i("assigned", a.p->m["o"]);
          }
          userDeinit(a);
        }
        if (exist(i, "comments") && i->m["comments"]->v == "1" && !empty(j, "id"))
        {
          radialUser a;
          userInit(d, a);
          a.p->m["i"]->i("issue_id", j->m["id"]->v);
          if (applicationIssueComments(a, e))
          {
            j->i("comments", a.p->m["o"]);
          }
          userDeinit(a);
        }
        d.p->i("o", j);
        delete j;
      }
      else
      {
        e = "No results returned.";
      }
    }
  }

  return b;
}
// }}}
// {{{ applicationIssueAdd()
bool Central::applicationIssueAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!d.u.empty())
  {
    if (dep({"application_id"}, i, e))
    {
      string id, q;
      if (empty(i, "assigned_id") && !empty(i, "assigned_userid"))
      {
        radialUser a;
        userInit(d, a);
        a.p->m["i"]->i("userid", i->m["assigned_userid"]->v);
        if (user(a, e) && !empty(a.p->m["o"], "id"))
        {
          i->i("assigned_id", a.p->m["o"]->m["id"]->v);
        }
        userDeinit(a);
      }
      if (db("dbCentralApplicationIssueAdd", i, id, q, e))
      {
        b = true;
        o->i("id", id);
        if (!empty(i, "comments"))
        {
          radialUser a;
          userInit(d, a);
          a.p->m["i"]->i("issue_id", id);
          a.p->m["i"]->i("comments", i->m["comments"]->v);
          applicationIssueCommentAdd(a, e);
          userDeinit(a);
        }
        if (!empty(i, "server"))
        {
          radialUser a;
          userInit(d, a);
          a.p->m["i"]->i("id", id);
          a.p->m["i"]->i("action", "add");
          a.p->m["i"]->i("application_id", i->m["application_id"]->v);
          a.p->m["i"]->i("server", i->m["server"]->v);
          applicationIssueEmail(a, e);
          userDeinit(a);
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
// {{{ applicationIssueClose()
bool Central::applicationIssueClose(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (!d.u.empty())
  {
    if (dep({"id"}, i, e))
    {
      i->i("close_date", "now()");
      b = db("dbCentralApplicationIssueUpdate", i, e);
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ applicationIssueComment()
bool Central::applicationIssueComment(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    map<string, string> r;
    if (db("dbCentralApplicationIssues", i, r, e))
    {
      if (!r.empty())
      {
        d.p->i("o", r);
      }
      else
      {
        e = "No results returned.";
      }
    }
  }

  return b;
}
// }}}
// {{{ applicationIssueCommentAdd()
bool Central::applicationIssueCommentAdd(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["i"];

  if (!d.u.empty())
  {
    if (dep({"comments", "issue_id"}, i, e))
    {
      radialUser a;
      userInit(d, a);
      a.p->m["i"]->i("userid", d.u);
      if (user(a, e) && !empty(a.p->m["o"], "id"))
      {
        string id, q;
        i->i("user_id", a.p->m["o"]->m["id"]->v);
        if (db("dbCentalApplicationIssueCommentAdd", i, id, q, e))
        {
          b = true;
          o->i("id", id);
          if (!empty(i, "server"))
          {
            radialUser c;
            userInit(d, c);
            c.p->m["i"]->i("id", i->m["issue_id"]->v);
            c.p->m["i"]->i("action", "update");
            c.p->m["i"]->i("application_id", i->m["application_id"]->v);
            c.p->m["i"]->i("server", i->m["server"]->v);
            applicationIssueEmail(c, e);
            userDeinit(c);
          }
        }
      }
      userDeinit(a);
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ applicationIssueCommentEdit()
bool Central::applicationIssueCommentEdit(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (!d.u.empty())
  {
    if (dep({"comments", "id"}, i, e))
    {
      radialUser a;
      userInit(d, a);
      a.p->m["i"]->i("userid", d.u);
      if (user(a, e) && !empty(a.p->m["o"], "id"))
      {
        radialUser c;
        userInit(d, c);
        c.p->m["i"]->i("id", i->m["id"]->v);
        if (applicationIssueComment(c, e) && !empty(c.p->m["o"], "user_id"))
        {
          if (a.p->m["o"]->m["id"]->v == c.p->m["o"]->m["user_id"]->v)
          {
            b = db("dbCentralApplicationIssueCommentUpdate", i, e);
          }
          else
          {
            e = "You are not authorized to perform this action.";
          }
        }
        userDeinit(c);
      }
      userDeinit(a);
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ applicationIssueComments()
bool Central::applicationIssueComments(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"issue_id"}, i, e))
  {
    list<map<string, string> > rs;
    if (db("dbCentralApplicationIssueComments", i, rs, e))
    {
      b = true;
      for (auto &r : rs)
      {
        o->pb(r);
      }
    }
  }

  return b;
}
// }}}
// {{{ applicationIssueEdit()
bool Central::applicationIssueEdit(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (exist(i, "transfer") && !empty(i->m["transfer"], "id"))
  {
    i->i("transfer_id", i->m["transfer"]->m["id"]->v);
  }
  if (dep({"application_id", "id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("application_id", i->m["application_id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      if (empty(i, "assigned_id") && !empty(i, "assigned_userid"))
      {
        radialUser a;
        userInit(d, a);
        a.p->m["i"]->i("userid", i->m["assigned_userid"]->v);
        if (user(a, e) && !empty(a.p->m["o"], "id"))
        {
          i->i("assigned_id", a.p->m["o"]->m["id"]->v);
        }
        userDeinit(a);
      }
      b = db("dbCentralApplicationIssueUpdate", i, e);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationIssueEmail()
bool Central::applicationIssueEmail(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!d.u.empty())
  {
    if (dep({"action", "id", "server"}, i, e))
    {
      if (i->m["action"]->v == "add" || i->m["action"]->v == "close" || i->m["action"]->v == "transfer" || i->m["action"]->v == "update")
      {
        radialUser a;
        userInit(d, a);
        a.p->m["i"]->i("id", i->m["id"]->v);
        a.p->m["i"]->i("comments", "1", 'n');
        if (applicationIssue(a, e) && !empty(a.p->m["o"], "application_id"))
        {
          radialUser c;
          userInit(d, c);
          c.p->m["i"]->i("id", a.p->m["o"]->m["application_id"]->v);
          if (application(c, e) && !empty(c.p->m["o"], "id") && !empty(c.p->m["o"], "name"))
          {
            radialUser f;
            userInit(d, f);
            f.p->m["i"]->i("application_id", c.p->m["o"]->m["id"]->v);
            f.p->m["i"]->i("Primary Developer", "1", 'n');
            f.p->m["i"]->i("Backup Developer", "1", 'n');
            if (applicationUsersByApplicationID(f, e))
            {
              list<string> to;
              string strApplication, strName;
              stringstream m[2], s;
              s << c.p->m["o"]->m["name"]->v << " [" << i->m["action"]->v << "]:  Issue #" << i->m["id"]->v;
              if (!empty(a.p->m["o"], "summary"))
              {
                s << " - " << a.p->m["o"]->m["summary"]->v;
              }
              if (exist(a.p->m["o"], "assigned") && !empty(a.p->m["o"]->m["assigned"], "email"))
              {
                to.push_back(a.p->m["o"]->m["assigned"]->m["email"]->v);
              }
              else
              {
                for (auto &contact : f.p->m["o"]->l)
                {
                  if (!empty(contact, "email"))
                  {
                    to.push_back(contact->m["email"]->v);
                  }
                }
              }
              if (i->m["action"]->v == "transfer" && !empty(i, "application_id"))
              {
                radialUser h;
                userInit(d, h);
                h.p->m["i"]->i("id", i->m["application_id"]->v);
                if (application(h, e) && !empty(h.p->m["o"], "name"))
                {
                  strApplication = h.p->m["o"]->m["name"]->v;
                  if (!exist(a.p->m["o"], "assigned") || empty(a.p->m["o"]->m["assigned"], "email"))
                  {
                    radialUser k;
                    userInit(d, k);
                    k.p->m["i"]->i("application_id", i->m["application_id"]->v);
                    k.p->m["i"]->i("Primary Developer", "1", 'n');
                    k.p->m["i"]->i("Backup Developer", "1", 'n');
                    if (applicationUsersByApplicationID(k, e))
                    {
                      for (auto &contact : k.p->m["o"]->l)
                      {
                        if (!empty(contact, "email"))
                        {
                          to.push_back(contact->m["email"]->v);
                        }
                      }
                    }
                    userDeinit(k);
                  }
                }
                userDeinit(h);
              }
              if (exist(a.p->m["o"], "comments"))
              {
                for (auto &contact : a.p->m["o"]->m["comments"]->l)
                {
                  if (!empty(contact, "email"))
                  {
                    to.push_back(contact->m["email"]->v);
                  }
                }
              }
              strName = getUserName(d);
              m[0] << "<html><body style=\"background:#f3f3f3:padding:10px;\">";
              if (!empty(a.p->m["o"], "summary"))
              {
                m[0] << "<h3>" << a.p->m["o"]->m["summary"]->v << "</h3>";
              }
              if (i->m["action"]->v == "add")
              {
                m[0] << "<a href=\"https://" << i->m["server"]->v << "/central/#/Applications/Issues/" << i->m["id"]->v << "\" style=\"text-decoration:none;\">Issue #" << i->m["id"]->v << "</a> has been <b>created</b> by " << strName << ".";
                m[1] << "Issue #" << i->m["id"]->v << " has been <b>created</b> by " << strName << "." << endl << endl << "You can view this issue at:" << endl << "https://" << i->m["server"]->v << "/central/#/Applications/Issues/" << i->m["id"]->v;
              }
              else if (i->m["action"]->v == "close")
              {
                m[0] << "<a href=\"https://" << i->m["server"]->v << "/central/#/Applications/Issues/" << i->m["id"]->v << "\" style=\"text-decoration:none;\">Issue #" << i->m["id"]->v << "</a> has been <b>closed</b> by " << strName << ".";
                m[1] << "Issue #" << i->m["id"]->v << " has been <b>closed</b> by " << strName << "." << endl << endl << "You can view this issue at:" << endl << "https://" << i->m["server"]->v << "/central/#/Applications/Issues/" << i->m["id"]->v;
              }
              else if (i->m["action"]->v == "transfer")
              {
                m[0] << "<a href=\"https://" << i->m["server"]->v << "/central/#/Applications/Issues/" << i->m["id"]->v << "\" style=\"text-decoration:none;\">Issue #" << i->m["id"]->v << "</a> has been <b>transferred</b> from " << strApplication << " to " << c.p->m["o"]->m["name"]->v << " by " << strName << ".";
                m[1] << "Issue #" << i->m["id"]->v << " has been <b>transferred</b> from " << strApplication << " to " << c.p->m["o"]->m["name"]->v << " by " << strName << "." << endl << endl << "You can view this issue at:" << endl << "https://" << i->m["server"]->v << "/central/#/Applications/Issues/" << i->m["id"]->v;
              }
              else if (i->m["action"]->v == "update")
              {
                m[0] << "<a href=\"https://" << i->m["server"]->v << "/central/#/Applications/Issues/" << i->m["id"]->v << "\" style=\"text-decoration:none;\">Issue #" << i->m["id"]->v << "</a> has been <b>updated</b> by " << strName << ".";
                m[1] << "Issue #" << i->m["id"]->v << " has been <b>updated</b> by " << strName << "." << endl << endl << "You can view this issue at:" << endl << "https://" << i->m["server"]->v << "/central/#/Applications/Issues/" << i->m["id"]->v;
              }
              if (exist(a.p->m["o"], "comments") && !a.p->m["o"]->m["comments"]->l.empty())
              {
                m[0] << "<div style=\"margin:10px 5px;border-style:solid;border-width:1px;border-color:#cccccc;border-radius:10px;background:white;box-shadow: 3px 3px 4px #888888;padding:10px;\">";
                m[0] << "<small style=\"float:right;color:#999999;\"><i>";
                if (!empty(a.p->m["o"]->m["comments"]->l.back(), "entry_date"))
                {
                  m[0] << a.p->m["o"]->m["comments"]->l.back()->m["entry_date"]->v;
                }
                if (!empty(a.p->m["o"]->m["comments"]->l.back(), "first_name") && !empty(a.p->m["o"]->m["comments"]->l.back(), "last_name"))
                {
                  m[0] << " by " << a.p->m["o"]->m["comments"]->l.back()->m["first_name"]->v << " " << a.p->m["o"]->m["comments"]->l.back()->m["last_name"]->v;
                }
                m[0] << "</i></small>";
                if (!empty(a.p->m["o"]->m["comments"]->l.back(), "comments"))
                {
                  size_t unPosition;
                  string strComments = a.p->m["o"]->m["comments"]->l.back()->m["comments"]->v;
                  while ((unPosition = strComments.find("<")) != string::npos)
                  {
                    strComments.replace(unPosition, 1, "&lt;");
                  }
                  while ((unPosition = strComments.find(">")) != string::npos)
                  {
                    strComments.replace(unPosition, 1, "&gt;");
                  }
                  while ((unPosition = strComments.find("\n")) != string::npos)
                  {
                    strComments.replace(unPosition, 1, "<br>");
                  }
                  m[0] << "<pre style=\"white-space:pre-wrap;\">" << strComments << "</pre>";
                }
                m[0] << "</div>";
              }
              m[0] << "<p>Viewing your <a href=\"http://" << i->m["server"]->v << "/central/#/Applications/Workload\">Workload</a> provides you with your personalized list of open application issues.  The issues provided on the Workload are pulled from applications for which you are registered as either a primary or backup developer.  The issues are sorted according to priority, due date, and open date.</p>";
              m[0] << "<p>Please use the <a href=\"http://" << i->m["server"]->v << "/central/#/Home/FrontDoor\">Front Door</a> to create a new issue for an application.  The Front Door provides a comprehensive list of applications from which to choose.</p>";
              m[0] << "This message was sent by <a href=\"https://" << i->m["server"]->v << "/central\" style=\"text-decoration:none;\">Central</a>.";
              m[0] << "</body></html>";
              to.sort();
              to.unique();
              email(getUserEmail(d), to, s.str(), m[1].str(), m[0].str(), e);
              b = true;
            }
            userDeinit(f);
          }
          userDeinit(c);
        }
        userDeinit(a);
      }
      else
      {
        e = "Please provide a valid action:  add, close, transfer, update.";
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
// {{{ applicationIssues()
bool Central::applicationIssues(radialUser &d, string &e)
{
  bool b = false, bApplication, bBackupDeveloper, bComments, bContact, bOwner, bPrimaryDeveloper, bPrimaryContact;
  list<map<string, string> > rs;
  string strCommenter;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  bApplication = (!empty(i, "application") && i->m["application"]->v == "1");
  bBackupDeveloper = (!empty(i, "Backup Developer") && i->m["Backup Developer"]->v == "1");
  bComments = (!empty(i, "comments") && i->m["comments"]->v == "1");
  bContact = (!empty(i, "Contact") && i->m["Contact"]->v == "1");
  bOwner = (!empty(i, "owner") && i->m["owner"]->v == "1");
  bPrimaryContact = (!empty(i, "Primary Contact") && i->m["Primary Contact"]->v == "1");
  bPrimaryDeveloper = (!empty(i, "Primary Developer") && i->m["Primary Developer"]->v == "1");
  strCommenter = ((!empty(i, "commenter"))?i->m["commenter"]->v:"");
  if (db("dbCentralApplicationIssues", i, rs, e))
  {
    b = true;
    for (auto &r : rs)
    {
      bool bUse = ((bOwner || !strCommenter.empty())?false:true);
      Json *j = new Json(r);
      if (bApplication)
      {
        radialUser a;
        userInit(d, a);
        a.p->m["i"]->i("id", r["application_id"]);
        if (application(a, e))
        {
          radialUser c;
          j->i("application", a.p->m["o"]);
          userInit(d, c);
          c.p->m["i"]->i("application_id", r["application_id"]);
          if (bPrimaryDeveloper)
          {
            c.p->m["i"]->i("Primary Developer", "1", 'n');
          }
          if (bBackupDeveloper)
          {
            c.p->m["i"]->i("Backup Developer", "1", 'n');
          }
          if (bPrimaryContact)
          {
            c.p->m["i"]->i("Primary Contact", "1", 'n');
          }
          if (bContact)
          {
            c.p->m["i"]->i("Contact", "1", 'n');
          }
          if (applicationUsersByApplicationID(c, e))
          {
            j->m["application"]->i("contacts", c.p->m["o"]);
            if (bOwner)
            {
              for (auto contactIter = j->m["application"]->m["contacts"]->l.begin(); !bUse && contactIter != j->m["application"]->m["contacts"]->l.end(); contactIter++)
              {
                if (!empty((*contactIter), "userid") && (*contactIter)->m["userid"]->v == d.u)
                {
                  bUse = true;
                }
              }
            }
          }
          userDeinit(c);
        }
        userDeinit(a);
      }
      if (!r["assigned_id"].empty())
      {
        radialUser a;
        userInit(d, a);
        a.p->m["i"]->i("id", r["assigned_id"]);
        if (user(a, e))
        {
          j->i("assigned", a.p->m["o"]);
        }
        userDeinit(a);
      }
      if (bComments)
      {
        radialUser a;
        userInit(d, a);
        a.p->m["i"]->i("issue_id", r["id"]);
        if (applicationIssueComments(a, e))
        {
          j->i("comments", a.p->m["o"]);
          if (!strCommenter.empty())
          {
            for (auto commentIter = j->m["comments"]->l.begin(); !bUse && commentIter != j->m["comments"]->l.end(); commentIter++)
            {
              if (!empty((*commentIter), "userid") && (*commentIter)->m["userid"]->v == d.u)
              {
                bUse = true;
              }
            }
          }
        }
        userDeinit(a);
      }
      if (bUse)
      {
        o->pb(j);
      }
      delete j;
    }
  }

  return b;
}
// }}}
// {{{ applicationIssuesByApplicationID()
bool Central::applicationIssuesByApplicationID(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"application_id"}, i, e))
  {
    list<map<string, string> > rs;
    if (db("dbCentralApplicationIssues", i, rs, e))
    {
      b = true;
      for (auto &r : rs)
      {
        Json *j = new Json(r);
        if (!r["assigned_id"].empty())
        {
          radialUser a;
          userInit(d, a);
          a.p->m["i"]->i("id", r["assigned_id"]);
          if (user(a, e))
          {
            j->i("assigned", a.p->m["o"]);
          }
          userDeinit(a);
        }
        if (!empty(i, "comments") && i->m["comments"]->v == "1")
        {
          radialUser a;
          userInit(d, a);
          a.p->m["i"]->i("issue_id", r["id"]);
          if (applicationIssueComments(a, e))
          {
            j->i("comments", a.p->m["o"]);
          }
          userDeinit(a);
        }
        o->pb(j);
        delete j;
      }
    }
  }

  return b;
}
// }}}
// {{{ applicationNotify()
bool Central::applicationNotify(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"id", "notification", "server"}, i, e))
  {
    string strNotification = i->m["notification"]->v;
    size_t unPosition;
    radialUser a;
    while ((unPosition = strNotification.find("<")) != string::npos)
    {
      strNotification.replace(unPosition, 1, "&lt;");
    }
    while ((unPosition = strNotification.find(">")) != string::npos)
    {
      strNotification.replace(unPosition, 1, "&gt;");
    }
    while ((unPosition = strNotification.find("\n")) != string::npos)
    {
      strNotification.replace(unPosition, 1, "<br>");
    }
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      radialUser c;
      userInit(d, c);
      c.p->m["i"]->i("id", i->m["id"]->v);
      if (application(c, e) && !empty(c.p->m["o"], "name"))
      {
        radialUser f;
        userInit(d, f);
        f.p->m["i"]->i("application_id", i->m["id"]->v);
        f.p->m["i"]->i("Primary Developer", "1", 'n');
        f.p->m["i"]->i("Backup Developer", "1", 'n');
        f.p->m["i"]->i("Contact", "1", 'n');
        if (applicationUsersByApplicationID(f, e))
        {
          map<string, map<string, string> > developer = {{"primary", {}}, {"backup", {}} };
          b = true;
          for (auto &contact : f.p->m["o"]->l)
          {
            stringstream s;
            radialUser h;
            if (!empty(contact, "user_id") && !empty(contact, "userid") && exist(contact, "notify") && !empty(contact->m["notify"], "value") && contact->m["notify"]->m["value"]->v == "1" && !empty(contact, "email"))
            {
              if (!exist(o, contact->m["userid"]->v))
              {
                o->m[contact->m["userid"]->v] = new Json;
              }
              o->m[contact->m["userid"]->v]->i("sent", "0", '0');
              o->m[contact->m["userid"]->v]->i("email", contact->m["email"]->v);
              o->m[contact->m["userid"]->v]->i("name", (string)((!empty(contact, "last_name"))?contact->m["last_name"]->v:"") + (string)", " + (string)((!empty(contact, "first_name"))?contact->m["first_name"]->v:""));
              if (exist(contact, "type") && !empty(contact->m["type"], "type") && contact->m["type"]->m["type"]->v == "Primary Developer")
              {
                if (exist(contact, "notify") && !empty(contact->m["notify"], "value") && contact->m["notify"]->m["value"]->v == "1")
                {
                  o->m[contact->m["userid"]->v]->i("primary", "1", 1);
                }
                developer["primary"][contact->m["user_id"]->v] = (string)((!empty(contact, "last_name"))?contact->m["last_name"]->v:"") + (string)", " + (string)((!empty(contact, "first_name"))?contact->m["first_name"]->v:"");
              }
              else if (exist(contact, "type") && !empty(contact->m["type"], "type") && contact->m["type"]->m["type"]->v == "Backup Developer")
              {
                if (exist(contact, "notify") && !empty(contact->m["notify"], "value") && contact->m["notify"]->m["value"]->v == "1")
                {
                  o->m[contact->m["userid"]->v]->i("backup", "1", 1);
                }
                developer["backup"][contact->m["user_id"]->v] = (string)((!empty(contact, "last_name"))?contact->m["last_name"]->v:"") + (string)", " + (string)((!empty(contact, "first_name"))?contact->m["first_name"]->v:"");
              }
              else if (exist(contact, "notify") && !empty(contact->m["notify"], "value") && contact->m["notify"]->m["value"]->v == "1")
              {
                o->m[contact->m["userid"]->v]->i("contact", "1", 1);
              }
            }
            userInit(d, h);
            h.p->m["i"]->i("application_id", i->m["id"]->v);
            h.p->m["i"]->i("contacts", "1", 'n');
            if (dependentsByApplicationID(h, e) && exist(h.p->m["o"], "dependents"))
            {
              for (auto &dependent : h.p->m["o"]->m["dependents"]->l)
              {
                if (empty(dependent, "retirement_date") && exist(dependent, "contacts"))
                {
                  for (auto &contact : dependent->m["contacts"]->l)
                  {
                    if (!empty(contact, "userid") && !empty(contact, "type") && (contact->m["type"]->v == "Primary Developer" || contact->m["type"]->v == "Backup Developer") && exist(contact, "notify") && !empty(contact->m["notify"], "value") && contact->m["notify"]->m["value"]->v == "1" && !empty(contact, "email"))
                    {
                      if (!exist(o, contact->m["userid"]->v))
                      {
                        o->m[contact->m["userid"]->v] = new Json;
                      }
                      o->m[contact->m["userid"]->v]->i("sent", "0", '0');
                      o->m[contact->m["userid"]->v]->i("email", contact->m["email"]->v);
                      o->m[contact->m["userid"]->v]->i("name", (string)((!empty(contact, "last_name"))?contact->m["last_name"]->v:"") + (string)", " + (string)((!empty(contact, "first_name"))?contact->m["first_name"]->v:""));
                      if (!empty(dependent, "name"))
                      {
                        if (!exist(o->m[contact->m["userid"]->v], "depend"))
                        {
                          o->m[contact->m["userid"]->v]->m["depend"] = new Json;
                        }
                        o->m[contact->m["userid"]->v]->m["depend"]->pb(dependent->m["name"]->v);
                      }
                    }
                  }
                }
              }
            }
            userDeinit(h);
            s << "Application Notification:  " << c.p->m["o"]->m["name"]->v;
            for (auto &k : o->m)
            {
              list<string> to;
              stringstream m;
              to.push_back(k.second->m["email"]->v);
              m << "<html><body>";
              m << "<div style=\"font-family: arial, helvetica, sans-serif; font-size: 12px;\">";
              m << "<h3><b>Application Notification:  <a href=\"https://" << i->m["server"]->v << "/central/#/Applications/" << i->m["id"]->v << "\">" << c.p->m["o"]->m["name"]->v << "</a></b></h3>";
              m << strNotification;
              m << "<br><br>";
              m << "<b>You are receiving this application notification for the following reason(s):</b>";
              m << "<br><br>";
              m << "<ul>";
              if (exist(k.second, "primary"))
              {
                m << "<li>You are a Primary Developer for this application.</li>";
              }
              else if (exist(k.second, "backup"))
              {
                m << "<li>You are a Backup Developer for this application.</li>";
              }
              else if (exist(k.second, "contact"))
              {
                m << "<li>You are a Contact for this application.</li>";
              }
              if (exist(k.second, "depend"))
              {
                m << "<li>";
                m << "You are a developer for the following application(s) which depend on the " << c.p->m["o"]->m["name"]->v << ":";
                m << "<ul>";
                for (auto &depend : k.second->m["depend"]->l)
                {
                  m << "<li>" << depend->v << "</li>";
                }
                m << "</ul>";
                m << "</li>";
              }
              m << "</ul>";
              if (!developer["primary"].empty())
              {
                bool bFirst = true;
                m << "<br><br>";
                m << "<b>Primary Developer(s):</b><br>";
                for (auto &dev : developer["primary"])
                {
                  if (bFirst)
                  {
                    bFirst = false;
                  }
                  else
                  {
                    m << ", ";
                  }
                  m << "<a href=\"https://" << i->m["server"]->v << "/central/#/Users/" << dev.first << "\">" << dev.second << "</a>";
                }
              }
              if (!developer["backup"].empty())
              {
                bool bFirst = true;
                m << "<br><br>";
                m << "<b>Backup Developer(s):</b><br>";
                for (auto &dev : developer["backup"])
                {
                  if (bFirst)
                  {
                    bFirst = false;
                  }
                  else
                  {
                    m << ", ";
                  }
                  m << "<a href=\"https://" << i->m["server"]->v << "/central/#/Users/" << dev.first << "\">" << dev.second << "</a>";
                }
              }
              m << "<br><br>";
              m << "If you have any questions or concerns, please contact your application contacts.";
              m << "</div>";
              m << "</body></html>";
              email(getUserEmail(d), to, s.str(), "", m.str(), e);
              k.second->i("sent", "1", 'n');
            }
          }
        }
        userDeinit(f);
      }
      userDeinit(c);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationRemove()
bool Central::applicationRemove(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      b = db("dbCentralApplicationRemove", i, e);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applications()
bool Central::applications(radialUser &d, string &e)
{
  bool b = false;
  list<map<string, string> > rs;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (db("dbCentralApplications", i, rs, e))
  {
    b = true;
    for (auto &r : rs)
    {
      Json *j = new Json(r);
      ny(j, "account_check");
      ny(j, "auto_register");
      if (!empty(i, "contacts") && i->m["contacts"]->v == "1")
      {
        radialUser a;
        userInit(d, a);
        a.p->m["i"]->i("application_id", r["id"]);
        a.p->m["i"]->i("Primary Developer", "1", 'n');
        a.p->m["i"]->i("Backup Developer", "1", 'n');
        a.p->m["i"]->i("Primary Contact", "1", 'n');
        if (applicationUsersByApplicationID(a, e))
        {
          j->i("contacts", a.p->m["o"]);
        }
        userDeinit(a);
      }
      ny(j, "dependable");
      if (!empty(j, "login_type_id"))
      {
        size_t unValue;
        stringstream ssValue(j->m["login_type_id"]->v);
        ssValue >> unValue;
        if (unValue > 0)
        {
          radialUser l;
          userInit(d, l);
          l.p->m["i"]->i("id", j->m["login_type_id"]->v);
          if (loginType(l, e))
          {
            j->i("login_type", l.p->m["o"]);
          }
          userDeinit(l);
        }
      }
      if (!empty(j, "menu_id"))
      {
        size_t unValue;
        stringstream ssValue(j->m["menu_id"]->v);
        ssValue >> unValue;
        if (unValue > 0)
        {
          radialUser m;
          userInit(d, m);
          m.p->m["i"]->i("id", j->m["menu_id"]->v);
          if (menuAccess(m, e))
          {
            j->i("menu_access", m.p->m["o"]);
          }
          userDeinit(m);
        }
      }
      if (!empty(j, "notify_priority_id"))
      {
        size_t unValue;
        stringstream ssValue(j->m["notify_priority_id"]->v);
        ssValue >> unValue;
        if (unValue > 0)
        {
          radialUser n;
          userInit(d, n);
          n.p->m["i"]->i("id", j->m["notify_priority_id"]->v);
          if (notifyPriority(n, e))
          {
            j->i("notify_priority", n.p->m["o"]);
          }
          userDeinit(n);
        }
      }
      if (!empty(j, "package_type_id"))
      {
        size_t unValue;
        stringstream ssValue(j->m["package_type_id"]->v);
        ssValue >> unValue;
        if (unValue > 0)
        {
          radialUser p;
          userInit(d, p);
          p.p->m["i"]->i("id", j->m["package_type_id"]->v);
          if (packageType(p, e))
          {
            j->i("package_type", p.p->m["o"]);
          }
          userDeinit(p);
        }
      }
      ny(j, "secure_port");
      if (!empty(i, "servers") && i->m["servers"]->v == "1")
      {
        radialUser a;
        userInit(d, a);
        a.p->m["i"]->i("application_id", r["id"]);
        if (serversByApplicationID(a, e))
        {
          j->i("servers", a.p->m["o"]);
        }
        userDeinit(a);
      }
      ny(j, "wiki");
      o->pb(j);
      delete j;
    }
  }

  return b;
}
// }}}
// {{{ applicationsByServerID()
bool Central::applicationsByServerID(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"server_id"}, i, e))
  {
    list<map<string, string> > rs;
    if (db("dbCentralApplications", i, rs, e))
    {
      b = true;
      for (auto &r : rs)
      {
        o->pb(r);
      }
    }
  }

  return b;
}
// }}}
// {{{ applicationsByUserID()
bool Central::applicationsByUserID(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"contact_id"}, i, e))
  {
    list<map<string, string> > rs;
    if (db("dbCentralApplications", i, rs, e))
    {
      b = true;
      for (auto &r : rs)
      {
        o->pb(r);
      }
    }
  }

  return b;
}
// }}}
// {{{ applicationServer()
bool Central::applicationServer(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    map<string, string> r;
    if (db("dbCentralApplicationServers", i, r, e))
    {
      if (!r.empty())
      {
        b = true;
        d.p->i("o", r);
      }
      else
      {
        e = "No results returned.";
      }
    }
  }

  return b;
}
// }}}
// {{{ applicationServerAdd()
bool Central::applicationServerAdd(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"application_id", "server_id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["application_id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      string id, q;
      if (db("dbCentralApplicationServerAdd", i, id, q, e))
      {
        b = true;
        o->i("id", id);
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationServerDetail()
bool Central::applicationServerDetail(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    map<string, string> r;
    if (db("dbCentralApplicationServerDetails", i, r, e))
    {
      if (!r.empty())
      {
        b = true;
        d.p->i("o", r);
      }
      else
      {
        e = "No results returned.";
      }
    }
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationServerDetailAdd()
bool Central::applicationServerDetailAdd(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"application_server_id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["application_server_id"]->v);
    if (d.g || applicationServer(a, e))
    {
      string id, q;
      if (db("dbCentralApplicationServerDetailAdd", i, id, q, e))
      {
        b = true;
        o->i("id", id);
      }
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationServerDetailEdit()
bool Central::applicationServerDetailEdit(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || (applicationServerDetail(a, e) && !empty(a.p->m["o"], "application_server_id")))
    {
      radialUser c;
      userInit(d, c);
      if (!d.g)
      {
        c.p->m["i"]->i("id", a.p->m["o"]->m["application_server_id"]->v);
      }
      if (d.g || (applicationServer(c, e) && !empty(c.p->m["o"], "id")))
      {
        radialUser f;
        userInit(d, f);
        if (!d.g)
        {
          f.p->m["i"]->i("id", c.p->m["o"]->m["id"]->v);
        }
        if (d.g || isApplicationDeveloper(f, e))
        {
          b = db("dbCentralApplicationServerDetailUpdate", i, e);
        }
        else
        {
          e = "You are not authorized to perform this action.";
        }
        userDeinit(f);
      }
      userDeinit(c);
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationServerDetailRemove()
bool Central::applicationServerDetailRemove(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || (applicationServerDetail(a, e) && !empty(a.p->m["o"], "application_server_id")))
    {
      radialUser c;
      userInit(d, c);
      if (!d.g)
      {
        c.p->m["i"]->i("id", a.p->m["o"]->m["application_server_id"]->v);
      }
      if (d.g || (applicationServer(c, e) && !empty(c.p->m["o"], "id")))
      {
        radialUser f;
        userInit(d, f);
        if (!d.g)
        {
          f.p->m["i"]->i("id", c.p->m["o"]->m["id"]->v);
        }
        if (d.g || isApplicationDeveloper(f, e))
        {
          b = db("dbCentralApplicationServerDetailRemove", i, e);
        }
        else
        {
          e = "You are not authorized to perform this action.";
        }
        userDeinit(f);
      }
      userDeinit(c);
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationServerDetails()
bool Central::applicationServerDetails(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"application_server_id"}, i, e))
  {
    list<map<string, string> > rs;
    if (db("dbCentralApplicationServerDetails", i, rs, e))
    {
      b = true;
      for (auto &r : rs)
      {
        o->pb(r);
      }
    }
  }

  return b;
}
// }}}
// {{{ applicationServerRemove()
bool Central::applicationServerRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || (applicationServer(a, e) && !empty(a.p->m["o"], "application_id")))
    {
      radialUser c;
      userInit(d, c);
      if (!d.g)
      {
        c.p->m["i"]->i("id", a.p->m["o"]->m["application_id"]->v);
      }
      if (d.g || isApplicationDeveloper(c, e))
      {
        b = db("dbCentralApplicationServerRemove", i, e);
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      userDeinit(c);
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationUser()
bool Central::applicationUser(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    map<string, string> r;
    if (db("dbCentralApplicationUsers", i, r, e))
    {
      if (!r.empty())
      {
        Json *j = new Json(r);
        b = true;
        ny(j, "admin");
        ny(j, "locked");
        ny(j, "notify");
        d.p->i("o", j);
        delete j;
      }
      else
      {
        e = "No results returned.";
      }
    }
  }

  return b;
}
// }}}
// {{{ applicationUserAdd()
bool Central::applicationUserAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"application_id", "userid"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["application_id"]->v);
    if (d.g || (application(a, e) && !empty(a.p->m["o"], "name")))
    {
      radialUser c;
      userInit(d, c);
      if (!d.g)
      {
        c.p->m["i"]->i("id", i->m["application_id"]->v);
      }
      if (d.g || (d.auth.find(a.p->m["o"]->m["name"]->v) != d.auth.end() && d.auth[a.p->m["o"]->m["name"]->v]) || isApplicationDeveloper(c, e))
      {
        bool bReady = false;
        radialUser f;
        userInit(d, f);
        f.p->m["i"]->i("userid", i->m["userid"]->v);
        if (user(f, e) && !empty(f.p->m["o"], "id"))
        {
          bReady = true;
        }
        else if (e == "No results returned.")
        {
          userDeinit(f);
          userInit(d, f);
          f.p->m["i"]->i("userid", i->m["userid"]->v);
          f.p->m["i"]->i("application_id", i->m["application_id"]->v);
          if (userAdd(f, e))
          {
            userDeinit(f);
            userInit(d, f);
            f.p->m["i"]->i("userid", i->m["userid"]->v);
            if (user(f, e) && !empty(f.p->m["o"], "id"))
            {
              bReady = true;
            }
          }
        }
        if (bReady)
        {
          if (exist(i, "type") && !empty(i->m["type"], "type"))
          {
            radialUser h;
            userInit(d, h);
            h.p->m["i"]->i("type", i->m["type"]->m["type"]->v);
            if (contactType(h, e) && !empty(h.p->m["o"], "id"))
            {
              if (exist(i, "admin") && !empty(i->m["admin"], "value"))
              {
                if (exist(i, "locked") && !empty(i->m["locked"], "value"))
                {
                  if (exist(i, "notify") && !empty(i->m["notify"], "value"))
                  {
                    string id, q;
                    i->i("contact_id", f.p->m["o"]->m["id"]->v);
                    i->i("type_id", h.p->m["o"]->m["id"]->v);
                    if (db("dbCentralApplicationUserAdd", i, id, q, e))
                    {
                      b = true;
                      o->i("id", id);
                    }
                  }
                  else
                  {
                    e = "Please provide the notify.";
                  }
                }
                else
                {
                  e = "Please provide the locked.";
                }
              }
              else
              {
                e = "Please provide the admin.";
              }
            }
            userDeinit(h);
          }
          else
          {
            e = "Please provide the type.";
          }
        }
        userDeinit(f);
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      userDeinit(c);
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationUserEdit()
bool Central::applicationUserEdit(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id", "userid"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || (applicationUser(a, e) && !empty(a.p->m["o"], "application_id")))
    {
      radialUser c;
      userInit(d, c);
      if (!d.g)
      {
        c.p->m["i"]->i("id", a.p->m["o"]->m["application_id"]->v);
      }
      if (d.g || (application(c, e) && !empty(c.p->m["o"], "name")))
      {
        radialUser f;
        userInit(d, f);
        if (!d.g)
        {
          f.p->m["i"]->i("id", a.p->m["o"]->m["application_id"]->v);
        }
        if (d.g || (d.auth.find(c.p->m["o"]->m["name"]->v) != d.auth.end() && d.auth[c.p->m["o"]->m["name"]->v]) || isApplicationDeveloper(f, e))
        {
          bool bReady = false;
          radialUser h;
          userInit(d, h);
          h.p->m["i"]->i("userid", i->m["userid"]->v);
          if (user(h, e) && !empty(h.p->m["o"], "id"))
          {
            bReady = true;
          }
          else if (e == "No results returned.")
          {
            userDeinit(h);
            userInit(d, h);
            h.p->m["i"]->i("userid", i->m["userid"]->v);
            h.p->m["i"]->i("application_id", i->m["application_id"]->v);
            if (userAdd(h, e))
            {
              userDeinit(h);
              userInit(d, h);
              h.p->m["i"]->i("userid", i->m["userid"]->v);
              if (user(h, e) && !empty(h.p->m["o"], "id"))
              {
                bReady = true;
              }
            }
          }
          if (bReady)
          {
            if (exist(i, "type") && !empty(i->m["type"], "type"))
            {
              radialUser k;
              userInit(d, k);
              k.p->m["i"]->i("type", i->m["type"]->m["type"]->v);
              if (contactType(k, e) && !empty(k.p->m["o"], "id"))
              {
                if (exist(i, "admin") && !empty(i->m["admin"], "value"))
                {
                  if (exist(i, "locked") && !empty(i->m["locked"], "value"))
                  {
                    if (exist(i, "notify") && !empty(i->m["notify"], "value"))
                    {
                      i->i("contact_id", h.p->m["o"]->m["id"]->v);
                      i->i("type_id", k.p->m["o"]->m["id"]->v);
                      b = db("dbCentralApplicationUserUpdate", i, e);
                    }
                    else
                    {
                      e = "Please provide the notify.";
                    }
                  }
                  else
                  {
                    e = "Please provide the locked.";
                  }
                }
                else
                {
                  e = "Please provide the admin.";
                }
              }
              userDeinit(k);
            }
            else
            {
              e = "Please provide the type.";
            }
          }
          userDeinit(h);
        }
        else
        {
          e = "You are not authorized to perform this action.";
        }
        userDeinit(f);
      }
      userDeinit(c);
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationUserRemove()
bool Central::applicationUserRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || (applicationUser(a, e) && !empty(a.p->m["o"], "application_id")))
    {
      radialUser c;
      userInit(d, c);
      if (!d.g)
      {
        c.p->m["i"]->i("id", a.p->m["o"]->m["application_id"]->v);
      }
      if (d.g || (application(c, e) && !empty(c.p->m["o"], "name")))
      {
        radialUser f;
        userInit(d, f);
        if (!d.g)
        {
          f.p->m["i"]->i("id", a.p->m["o"]->m["application_id"]->v);
        }
        if (d.g || (d.auth.find(c.p->m["o"]->m["name"]->v) != d.auth.end() && d.auth[c.p->m["o"]->m["name"]->v]) || isApplicationDeveloper(f, e))
        {
          b = db("dbCentralApplicationUserRemove", i, e);
        }
        else
        {
          e = "You are not authorized to perform this action.";
        }
        userDeinit(f);
      }
      userDeinit(c);
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ applicationUsersByApplicationID()
bool Central::applicationUsersByApplicationID(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"application_id"}, i, e))
  {
    list<map<string, string> > rs;
    if (db("dbCentralApplicationUsers", i, rs, e))
    {
      b = true;
      for (auto &r : rs)
      {
        radialUser a;
        Json *j = new Json(r);
        ny(j, "admin");
        ny(j, "locked");
        ny(j, "notify");
        userInit(d, a);
        a.p->m["i"]->i("id", r["type_id"]);
        if (contactType(a, e))
        {
          j->i("type", a.p->m["o"]);
        }
        o->pb(j);
        delete j;
      }
    }
  }

  return b;
}
// }}}
// {{{ autoMode()
void Central::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->Central::autoMode()";
  if (strOldMaster != strNewMaster)
  {
    stringstream ssMessage;
    ssMessage << strPrefix << " [" << strNewMaster << "]:  Updated master.";
    log(ssMessage.str());
  }
  threadDecrement();
}
// }}}
// {{{ callback()
void Central::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Central::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    bool bInvalid = true;
    string strFunction = ptJson->m["Function"]->v, strJwt;
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
    if (bResult)
    {
      if (exist(ptJson, "Response"))
      {
        delete ptJson->m["Response"];
      }
      ptJson->m["Response"] = new Json(d.p->m["o"]);
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
  threadDecrement();
}
// }}}
// {{{ contactType()
bool Central::contactType(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "type"))
  {
    map<string, string> r;
    if (db("dbCentralContactTypes", i, r, e))
    {
      if (!r.empty())
      {
        b = true;
        d.p->i("o", r);
      }
      else
      {
        e = "No results returned.";
      }
    }
  }
  else
  {
    e = "Please provide the id or type.";
  }

  return b;
}
// }}}
// {{{ contactTypes()
bool Central::contactTypes(radialUser &d, string &e)
{
  bool b = false;
  list<map<string, string> > rs;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (db("dbCentralContactTypes", i, rs, e))
  {
    b = true;
    for (auto &r : rs)
    {
      o->pb(r);
    }
  }

  return b;
}
// }}}
// {{{ dbf()
void Central::dbf(list<map<string, string> > *g)
{
  dbfree(g);
}
// }}}
// {{{ dbq()
list<map<string, string> > *Central::dbq(const string strQuery, string &e)
{
  return dbquery("central_r", strQuery, e);
}
// }}}
// {{{ dbu()
bool Central::dbu(const string strQuery, string &e)
{
  return dbupdate("central", strQuery, e);
}
bool Central::dbu(const string strQuery, string &strID, string &e)
{
  return dbupdate("central", strQuery, strID, e);
}
// }}}
// {{{ dep()
bool Central::dep(const list<string> fs, Json *i, string &e)
{
  bool bResult = true;
  stringstream es;

  for (auto fi = fs.begin(); bResult && fi != fs.end(); fi++)
  {
    if (!exist(i, *fi) || empty(i, *fi))
    {
      bResult = false;
      es << "Please provide the " << *fi;
      e = es.str();
    }
  }

  return bResult;
}
// }}}
// {{{ dependentsByApplicationID()
bool Central::dependentsByApplicationID(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"application_id"}, i, e))
  {
    list<map<string, string> > rs;
    if (db("dbCentralDependents", i, rs, e))
    {
      o->m["depends"] = new Json;
      for (auto &r : rs)
      {
        Json *j = new Json(r);
        if (!empty(i, "contacts") && i->m["contacts"]->v == "1")
        {
          radialUser a;
          userInit(d, a);
          a.p->m["i"]->i("application_id", r["application_id"]);
          a.p->m["i"]->i("Primary Developer", "1", 'n');
          a.p->m["i"]->i("Backup Developer", "1", 'n');
          a.p->m["i"]->i("Primary Contact", "1", 'n');
          if (applicationUsersByApplicationID(a, e))
          {
            j->i("contacts", a.p->m["o"]);
          }
          userDeinit(a);
        }
        if (!empty(i, "servers") && i->m["servers"]->v == "1")
        {
          radialUser a;
          userInit(d, a);
          a.p->m["i"]->i("application_id", r["application_id"]);
          if (serversByApplicationID(a, e))
          {
            j->i("servers", a.p->m["o"]);
          }
          userDeinit(a);
        }
        o->m["depends"]->pb(j);
        delete j;
      }
      i->i("dependant_id", i->m["application_id"]->v);
      list<map<string, string> > hs;
      if (db("dbCentralDependents", i, hs, e))
      {
        b = true;
        o->m["dependents"] = new Json;
        for (auto &r : hs)
        {
          Json *j = new Json(r);
          if (!empty(i, "contacts") && i->m["contacts"]->v == "1")
          {
            radialUser a;
            userInit(d, a);
            a.p->m["i"]->i("application_id", r["application_id"]);
            a.p->m["i"]->i("Primary Developer", "1", 'n');
            a.p->m["i"]->i("Backup Developer", "1", 'n');
            a.p->m["i"]->i("Primary Contact", "1", 'n');
            if (applicationUsersByApplicationID(a, e))
            {
              j->i("contacts", a.p->m["o"]);
            }
            userDeinit(a);
          }
          if (!empty(i, "servers") && i->m["servers"]->v == "1")
          {
            radialUser a;
            userInit(d, a);
            a.p->m["i"]->i("application_id", r["application_id"]);
            if (serversByApplicationID(a, e))
            {
              j->i("servers", a.p->m["o"]);
            }
            userDeinit(a);
          }
          o->m["dependents"]->pb(j);
          delete j;
        }
      }
    }
  }

  return b;
}
// }}}
// {{{ loginType()
bool Central::loginType(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "type"))
  {
    map<string, string> r;
    if (db("dbCentralLoginTypes", i, r, e))
    {
      if (!r.empty())
      {
        b = true;
        d.p->i("o", r);
      }
      else
      {
        e = "No results returned.";
      }
    }
  }
  else
  {
    e = "Please provide the id or type.";
  }

  return b;
}
// }}}
// {{{ loginTypes()
bool Central::loginTypes(radialUser &d, string &e)
{
  bool b = false;
  list<map<string, string> > rs;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (db("dbCentralLoginTypes", i, rs, e))
  {
    b = true;
    for (auto &r : rs)
    {
      o->pb(r);
    }
  }

  return b;
}
// }}}
// {{{ merge()
void Central::merge(Json *ptOuter, Json *ptInner)
{
  ptOuter->merge(ptInner, true, false);
}
// }}}
// {{{ menuAccess()
bool Central::menuAccess(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "type"))
  {
    map<string, string> r;
    if (db("dbCentralMenuAccesses", i, r, e))
    {
      if (!r.empty())
      {
        b = true;
        d.p->i("o", r);
      }
      else
      {
        e = "No results returned.";
      }
    }
  }
  else
  {
    e = "Please provide the id or type.";
  }

  return b;
}
// }}}
// {{{ menuAccesses()
bool Central::menuAccesses(radialUser &d, string &e)
{
  bool b = false;
  list<map<string, string> > rs;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (db("dbCentralMenuAccesses", i, rs, e))
  {
    b = true;
    for (auto &r : rs)
    {
      o->pb(r);
    }
  }

  return b;
}
// }}}
// {{{ notifyPriorities()
bool Central::notifyPriorities(radialUser &d, string &e)
{
  bool b = false;
  list<map<string, string> > rs;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (db("dbCentralNotifyPriorities", i, rs, e))
  {
    b = true;
    for (auto &r : rs)
    {
      o->pb(r);
    }
  }

  return b;
}
// }}}
// {{{ notifyPriority()
bool Central::notifyPriority(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "priority"))
  {
    map<string, string> r;
    if (db("dbCentralNotifyPriorities", i, r, e))
    {
      if (!r.empty())
      {
        b = true;
        d.p->i("o", r);
      }
      else
      {
        e = "No results returned.";
      }
    }
  }
  else
  {
    e = "Please provide the id or priority.";
  }

  return b;
}
// }}}
// {{{ packageType()
bool Central::packageType(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "type"))
  {
    map<string, string> r;
    if (db("dbCentralPackageTypes", i, r, e))
    {
      if (!r.empty())
      {
        b = true;
        d.p->i("o", r);
      }
      else
      {
        e = "No results returned.";
      }
    }
  }
  else
  {
    e = "Please provide the id or type.";
  }

  return b;
}
// }}}
// {{{ packageTypes()
bool Central::packageTypes(radialUser &d, string &e)
{
  bool b = false;
  list<map<string, string> > rs;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (db("dbCentralPackageTypes", i, rs, e))
  {
    b = true;
    for (auto &r : rs)
    {
      o->pb(r);
    }
  }

  return b;
}
// }}}
// {{{ rm()
void Central::rm(Json *ptJson, const string strField)
{
  if (exist(ptJson, strField))
  {
    delete ptJson->m[strField];
    ptJson->m.erase(strField);
  }
}
// }}}
// {{{ sa()
bool Central::sa(const string strKey, Json *ptData, string &strError)
{
  bool bResult = false;
  list<string> keys = {"central", strKey};

  if (storageAdd(keys, ptData, strError))
  {
    bResult = true;
  }

  return bResult;
}
// }}}
// {{{ schedule()
void Central::schedule(string strPrefix)
{
  string strError;
  stringstream ssMessage;
  time_t CTime[3] = {0, 0, 0};
  Json *ptJson;

  threadIncrement();
  strPrefix += "->Central::schedule()";
  time(&(CTime[0]));
  while (!shutdown())
  {
    if (isMasterSettled() && isMaster())
    {
      time(&(CTime[1]));
      if ((CTime[1] - CTime[0]) > 600)
      {
        CTime[0] = CTime[1];
        ptJson = new Json;
        if (sr("_time", ptJson, strError))
        {
          stringstream ssTime(ptJson->v);
          ssTime >> CTime[2];
          if ((CTime[0] - CTime[2]) > 14400)
          {
            storageRemove({"central"}, strError);
          }
        }
        else if (strError == "Failed to find key.")
        {
          stringstream ssTime;
          ssTime << CTime[0];
          ptJson->i("_time", ssTime.str(), 'n');
          storageAdd({"central"}, ptJson, strError);
        }
        delete ptJson;
      }
    }
    msleep(2000);
  }
  threadDecrement();
}
// }}}
// {{{ server()
bool Central::server(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "name"))
  {
    map<string, string> r;
    if (db("dbCentralServers", i, r, e))
    {
      if (!r.empty())
      {
        b = true;
        d.p->i("o", r);
      }
      else
      {
        e = "No results returned.";
      }
    }
  }
  else
  {
    e = "Please provide the id or name.";
  }

  return b;
}
// }}}
// {{{ serverAdd()
bool Central::serverAdd(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (d.g || d.auth.find("Central") != d.auth.end())
  {
    if (dep({"name"}, i, e))
    {
      radialUser a;
      userInit(d, a);
      a.p->m["i"]->i("name", i->m["name"]->v);
      if (!server(a, e) && e == "No results returned.")
      {
        string id, q;
        if (db("dbCentralServerAdd", i, id, q, e))
        {
          b = true;
          o->i("id", id);
        }
      }
      else if (e.empty())
      {
        e = "Server already exists.";
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
// {{{ serverDetailsByApplicationID()
bool Central::serverDetailsByApplicationID(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"application_id"}, i, e))
  {
    list<map<string, string> > rs;
    if (db("dbCentralServerDetails", i, rs, e))
    {
      b = true;
      for (auto &r : rs)
      {
        o->pb(r);
      }
    }
  }

  return b;
}
// }}}
// {{{ serverEdit()
bool Central::serverEdit(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id", "name"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || isServerAdmin(a, e))
    {
      b = db("dbCentralServerUpdate", i, e);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ serverNotify()
bool Central::serverNotify(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "id"))
  {
    if (!empty(i, "notification"))
    {
      string strNotification = i->m["notification"]->v;
      size_t unPosition;
      while ((unPosition = strNotification.find("<")) != string::npos)
      {
        strNotification.replace(unPosition, 1, "&lt;");
      }
      while ((unPosition = strNotification.find(">")) != string::npos)
      {
        strNotification.replace(unPosition, 1, "&gt;");
      }
      while ((unPosition = strNotification.find("\n")) != string::npos)
      {
        strNotification.replace(unPosition, 1, "<br>");
      }
      if (!empty(i, "server"))
      {
        radialUser a;
        userInit(d, a);
        a.p->m["i"]->i("id", i->m["id"]->v);
        if (d.g || isServerAdmin(a, e))
        {
          radialUser c;
          userInit(d, c);
          c.p->m["i"]->i("id", i->m["id"]->v);
          if (server(c, e) && !empty(c.p->m["o"], "name"))
          {
            radialUser f;
            userInit(d, f);
            f.p->m["i"]->i("server_id", i->m["id"]->v);
            f.p->m["i"]->i("Primary Admin", "1", 'n');
            f.p->m["i"]->i("Backup Admin", "1", 'n');
            f.p->m["i"]->i("Primary Contact", "1", 'n');
            if (serverUsersByServerID(f, e))
            {
              map<string, map<string, string> > admin = {{"primary", {}}, {"backup", {}} };
              b = true;
              for (auto &contact : f.p->m["o"]->l)
              {
                radialUser h;
                stringstream s;
                if (!empty(contact, "user_id") && !empty(contact, "userid") && exist(contact, "notify") && !empty(contact->m["notify"], "value") && contact->m["notify"]->m["value"]->v == "1" && !empty(contact, "email"))
                {
                  if (!exist(o, contact->m["userid"]->v))
                  {
                    o->m[contact->m["userid"]->v] = new Json;
                  }
                  o->m[contact->m["userid"]->v]->i("sent", "0", '0');
                  o->m[contact->m["userid"]->v]->i("email", contact->m["email"]->v);
                  o->m[contact->m["userid"]->v]->i("name", (string)((!empty(contact, "last_name"))?contact->m["last_name"]->v:"") + (string)", " + (string)((!empty(contact, "first_name"))?contact->m["first_name"]->v:""));
                  if (exist(contact, "type") && !empty(contact->m["type"], "type") && contact->m["type"]->m["type"]->v == "Primary Admin")
                  {
                    if (exist(contact, "notify") && !empty(contact->m["notify"], "value") && contact->m["notify"]->m["value"]->v == "1")
                    {
                      o->m[contact->m["userid"]->v]->i("primary", "1", 1);
                    }
                    admin["primary"][contact->m["user_id"]->v] = (string)((!empty(contact, "last_name"))?contact->m["last_name"]->v:"") + (string)", " + (string)((!empty(contact, "first_name"))?contact->m["first_name"]->v:"");
                  }
                  else if (exist(contact, "type") && !empty(contact->m["type"], "type") && contact->m["type"]->m["type"]->v == "Backup Admin")
                  {
                    if (exist(contact, "notify") && !empty(contact->m["notify"], "value") && contact->m["notify"]->m["value"]->v == "1")
                    {
                      o->m[contact->m["userid"]->v]->i("backup", "1", 1);
                    }
                    admin["backup"][contact->m["user_id"]->v] = (string)((!empty(contact, "last_name"))?contact->m["last_name"]->v:"") + (string)", " + (string)((!empty(contact, "first_name"))?contact->m["first_name"]->v:"");
                  }
                  else if (exist(contact, "notify") && !empty(contact->m["notify"], "value") && contact->m["notify"]->m["value"]->v == "1")
                  {
                    o->m[contact->m["userid"]->v]->i("contact", "1", 1);
                  }
                }
                s << "Server Notification:  " << c.p->m["o"]->m["name"]->v;
                for (auto &k : o->m)
                {
                  list<string> to;
                  stringstream m;
                  to.push_back(k.second->m["email"]->v);
                  m << "<html><body>";
                  m << "<div style=\"font-family: arial, helvetica, sans-serif; font-size: 12px;\">";
                  m << "<h3><b>Server Notification:  <a href=\"https://" << i->m["server"]->v << "/central/#/Servers/" << i->m["id"]->v << "\">" << c.p->m["o"]->m["name"]->v << "</a></b></h3>";
                  m << strNotification;
                  m << "<br><br>";
                  m << "<b>You are receiving this server notification for the following reason(s):</b>";
                  m << "<br><br>";
                  m << "<ul>";
                  if (exist(k.second, "primary"))
                  {
                    m << "<li>You are a Primary Admin for this server.</li>";
                  }
                  else if (exist(k.second, "backup"))
                  {
                    m << "<li>You are a Backup Admin for this server.</li>";
                  }
                  else if (exist(k.second, "contact"))
                  {
                    m << "<li>You are a Contact for this server.</li>";
                  }
                  m << "</ul>";
                  if (!admin["primary"].empty())
                  {
                    bool bFirst = true;
                    m << "<br><br>";
                    m << "<b>Primary Admin(s):</b><br>";
                    for (auto &ad : admin["primary"])
                    {
                      if (bFirst)
                      {
                        bFirst = false;
                      }
                      else
                      {
                        m << ", ";
                      }
                      m << "<a href=\"https://" << i->m["server"]->v << "/central/#/Users/" << ad.first << "\">" << ad.second << "</a>";
                    }
                  }
                  if (!admin["backup"].empty())
                  {
                    bool bFirst = true;
                    m << "<br><br>";
                    m << "<b>Backup Admin(s):</b><br>";
                    for (auto &ad : admin["backup"])
                    {
                      if (bFirst)
                      {
                        bFirst = false;
                      }
                      else
                      {
                        m << ", ";
                      }
                      m << "<a href=\"https://" << i->m["server"]->v << "/central/#/Users/" << ad.first << "\">" << ad.second << "</a>";
                    }
                  }
                  m << "<br><br>";
                  m << "If you have any questions or concerns, please contact your server contacts.";
                  m << "</div>";
                  m << "</body></html>";
                  email(getUserEmail(d), to, s.str(), "", m.str(), e);
                  k.second->i("sent", "1", 'n');
                }
                userInit(d, h);
                h.p->m["i"]->i("server_id", i->m["id"]->v);
                if (applicationsByServerID(h, e))
                {
                  map<string, map<string, map<string, string> > > developer;
                  for (auto &app : h.p->m["o"]->l)
                  {
                    if (!empty(app, "application_id"))
                    {
                      radialUser k;
                      userInit(d, k);
                      k.p->m["i"]->i("id", app->m["application_id"]->v);
                      if (application(k, e) && !empty(k.p->m["o"], "name") && empty(k.p->m["o"], "retirement_date"))
                      {
                        radialUser l;
                        userInit(d, l);
                        l.p->m["i"]->i("application_id", app->m["application_id"]->v);
                        l.p->m["i"]->i("Primary Developer", "1", 'n');
                        l.p->m["i"]->i("Backup Developer", "1", 'n');
                        l.p->m["i"]->i("Contact", "1", 'n');
                        if (applicationUsersByApplicationID(l, e))
                        {
                          stringstream s;
                          for (auto &contact : l.p->m["o"]->l)
                          {
                            if (!empty(contact, "userid") && exist(contact, "notify") && !empty(contact->m["notify"], "value") && contact->m["notify"]->m["value"]->v == "1" && !empty(contact, "email"))
                            {
                              if (developer.find(contact->m["userid"]->v) == developer.end())
                              {
                                developer[contact->m["userid"]->v] = {};
                              }
                              if (developer[contact->m["userid"]->v].find(app->m["application_id"]->v) == developer[contact->m["userid"]->v].end())
                              {
                                developer[contact->m["userid"]->v][app->m["application_id"]->v] = {};
                                developer[contact->m["userid"]->v][app->m["application_id"]->v]["name"] = k.p->m["o"]->m["name"]->v;
                              }
                              developer[contact->m["userid"]->v][app->m["application_id"]->v]["email"] = contact->m["email"]->v;
                              developer[contact->m["userid"]->v][app->m["application_id"]->v]["name"] = (string)((!empty(contact, "last_name"))?contact->m["last_name"]->v:"") + (string)", " + (string)((!empty(contact, "first_name"))?contact->m["first_name"]->v:"");
                              if (exist(contact, "type") && !empty(contact->m["type"], "type"))
                              {
                                if (contact->m["type"]->m["type"]->v == "Primary Developer")
                                {
                                  developer[contact->m["userid"]->v][app->m["application_id"]->v]["primary"] = "1";
                                }
                                else if (contact->m["type"]->m["type"]->v == "Primary Developer")
                                {
                                  developer[contact->m["userid"]->v][app->m["application_id"]->v]["backup"] = "1";
                                }
                              }
                            }
                          }
                          s << "Server Notification:  " << c.p->m["o"]->m["name"]->v;
                          for (auto &n : developer)
                          {
                            list<string> to;
                            stringstream m;
                            m << "<html><body>";
                            m << "<div style=\"font-family: arial, helvetica, sans-serif; font-size: 12px;\">";
                            m << "<h3><b>Server Notification:  <a href=\"https://" << i->m["server"]->v << "/central/#/Servers/" << i->m["id"]->v << "\">" << c.p->m["o"]->m["name"]->v << "</a></b></h3>";
                            m << strNotification;
                            m << "<br><br>";
                            m << "<b>You are associated with the following applications that depend upon this server:</b>";
                            m << "<br><br>";
                            m << "<ul>";
                            for (auto &p : n.second)
                            {
                              to.push_back(p.second["email"]);
                              m << "<li><a href=\"https://" << i->m["server"]->v << "/central/#/Applications/" << p.first << "\">" << p.second["application"] << "</a>";
                              if (p.second.find("primary") != p.second.end())
                              {
                                m << ":  You are a Primary Developer for this application.";
                              }
                              else if (p.second.find("backup") != p.second.end())
                              {
                                m << ":  You are a Backup Developer for this application.";
                              }
                              m << "</li>";
                            }
                            m << "</ul>";
                            if (!admin["primary"].empty())
                            {
                              bool bFirst = true;
                              m << "<br><br>";
                              m << "<b>Primary Admin(s):</b><br>";
                              for (auto &ad : admin["primary"])
                              {
                                if (bFirst)
                                {
                                  bFirst = false;
                                }
                                else
                                {
                                  m << ", ";
                                }
                                m << "<a href=\"https://" << i->m["server"]->v << "/central/#/Users/" << ad.first << "\">" << ad.second << "</a>";
                              }
                            }
                            if (!admin["backup"].empty())
                            {
                              bool bFirst = true;
                              m << "<br><br>";
                              m << "<b>Backup Admin(s):</b><br>";
                              for (auto &ad : admin["backup"])
                              {
                                if (bFirst)
                                {
                                  bFirst = false;
                                }
                                else
                                {
                                  m << ", ";
                                }
                                m << "<a href=\"https://" << i->m["server"]->v << "/central/#/Users/" << ad.first << "\">" << ad.second << "</a>";
                              }
                            }
                            m << "<br><br>";
                            m << "If you have any questions or concerns, please contact your server contacts.";
                            m << "</div>";
                            m << "</body></html>";
                            to.sort();
                            to.unique();
                            email(getUserEmail(d), to, s.str(), "", m.str(), e);
                          }
                        }
                        userDeinit(l);
                      }
                      userDeinit(k);
                    }
                  }
                }
                userDeinit(h);
              }
            }
            userDeinit(f);
          }
          userDeinit(c);
        }
        else
        {
          e = "You are not authorized to perform this action.";
        }
        userDeinit(a);
      }
      else
      {
        e = "Please provide the server.";
      }
    }
    else
    {
      e = "Please provide the notification.";
    }
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ serverRemove()
bool Central::serverRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || isServerAdmin(a, e))
    {
      b = db("dbCentralServerRemove", i, e);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ servers()
bool Central::servers(radialUser &d, string &e)
{
  bool b = false;
  list<map<string, string> > rs;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (db("dbCentralServers", i, rs, e))
  {
    b = true;
    for (auto &r : rs)
    {
      Json *j = new Json(r);
      if (!empty(i, "contacts") && i->m["contacts"]->v == "1")
      {
        radialUser a;
        userInit(d, a);
        a.p->m["i"]->i("server_id", r["id"]);
        a.p->m["i"]->i("Primary Admin", "1", 'n');
        a.p->m["i"]->i("Backup Admin", "1", 'n');
        a.p->m["i"]->i("Primary Contact", "1", 'n');
        if (serverUsersByServerID(a, e))
        {
          j->i("contacts", a.p->m["o"]);
        }
        userDeinit(a);
      }
      o->pb(j);
      delete j;
    }
  }

  return b;
}
// }}}
// {{{ serversByApplicationID()
bool Central::serversByApplicationID(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"application_id"}, i, e))
  {
    list<map<string, string> > rs;
    if (db("dbCentralServers", i, rs, e))
    {
      b = true;
      for (auto &r : rs)
      {
        o->pb(r);
      }
    }
  }

  return b;
}
// }}}
// {{{ serversByUserID()
bool Central::serversByUserID(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"contact_id"}, i, e))
  {
    list<map<string, string> > rs;
    if (db("dbCentralServers", i, rs, e))
    {
      b = true;
      for (auto &r : rs)
      {
        o->pb(r);
      }
    }
  }

  return b;
}
// }}}
// {{{ serverUser()
bool Central::serverUser(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    map<string, string> r;
    if (db("dbCentralServerUsers", i, r, e))
    {
      if (!r.empty())
      {
        radialUser a;
        Json *j = new Json(r);
        b = true;
        ny(j, "notify");
        userInit(d, a);
        a.p->m["i"]->i("id", r["type_id"]);
        if (contactType(a, e))
        {
          j->i("type", a.p->m["o"]);
        }
        userDeinit(a);
        d.p->i("o", j);
        delete j;
      }
      else
      {
        e = "No results returned.";
      }
    }
  }

  return b;
}
// }}}
// {{{ serverUserAdd()
bool Central::serverUserAdd(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"server_id", "userid"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["server_id"]->v);
    if (d.g || isServerAdmin(a, e))
    {
      bool bReady = false;
      radialUser c;
      userInit(d, c);
      c.p->m["i"]->i("userid", i->m["userid"]->v);
      if (user(c, e) && !empty(c.p->m["o"], "id"))
      {
        bReady = true;
      }
      else if (e == "No results returned.")
      {
        userDeinit(c);
        userInit(d, c);
        c.p->m["i"]->i("userid", i->m["userid"]->v);
        c.p->m["i"]->i("server_id", i->m["server_id"]->v);
        if (userAdd(c, e))
        {
          userDeinit(c);
          userInit(d, c);
          c.p->m["i"]->i("userid", i->m["userid"]->v);
          if (user(c, e) && !empty(c.p->m["o"], "id"))
          {
            bReady = true;
          }
        }
      }
      if (bReady)
      {
        if (exist(i, "type") && !empty(i->m["type"], "type"))
        {
          radialUser f;
          userInit(d, f);
          f.p->m["i"]->i("type", i->m["type"]->m["type"]->v);
          if (contactType(f, e) && !empty(f.p->m["o"], "id"))
          {
            if (exist(i, "notify") && !empty(i->m["notify"], "value"))
            {
              string id, q;
              i->i("contact_id", c.p->m["o"]->m["id"]->v);
              i->i("notify", i->m["notify"]->m["value"]->v);
              i->i("type_id", f.p->m["o"]->m["id"]->v);
              if (db("dbCentralServerUserAdd", i, id, q, e))
              {
                b = true;
                o->i("id", id);
              }
            }
            else
            {
              e = "Please provide the notify.";
            }
          }
          userDeinit(f);
        }
        else
        {
          e = "Please provide the type.";
        }
      }
      userDeinit(c);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ serverUserEdit()
bool Central::serverUserEdit(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (dep({"id", "userid"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || (serverUser(a, e) && !empty(a.p->m["o"], "server_id")))
    {
      radialUser c;
      userInit(d, c);
      if (!d.g)
      {
        c.p->m["i"]->i("id", a.p->m["o"]->m["server_id"]->v);
      }
      if (d.g || isServerAdmin(c, e))
      {
        bool bReady = false;
        radialUser f;
        userInit(d, f);
        f.p->m["i"]->i("userid", i->m["userid"]->v);
        if (user(f, e) && !empty(f.p->m["o"], "id"))
        {
          bReady = true;
        }
        else if (e == "No results returned.")
        {
          userDeinit(f);
          userInit(d, f);
          f.p->m["i"]->i("userid", i->m["userid"]->v);
          f.p->m["i"]->i("server_id", i->m["server_id"]->v);
          if (userAdd(f, e))
          {
            userDeinit(f);
            userInit(d, f);
            f.p->m["i"]->i("userid", i->m["userid"]->v);
            if (user(f, e) && !empty(f.p->m["o"], "id"))
            {
              bReady = true;
            }
          }
        }
        if (bReady)
        {
          if (exist(i, "type") && !empty(i->m["type"], "type"))
          {
            radialUser h;
            userInit(d, h);
            h.p->m["i"]->i("type", i->m["type"]->m["type"]->v);
            if (contactType(h, e) && !empty(h.p->m["o"], "id"))
            {
              if (exist(i, "notify") && !empty(i->m["notify"], "value"))
              {
                i->i("contact_id", f.p->m["o"]->m["id"]->v);
                i->i("notify", i->m["notify"]->m["value"]->v);
                i->i("type_id", h.p->m["o"]->m["id"]->v);
                b = db("dbCentralServerUserUpdate", i, e);
              }
              else
              {
                e = "Please provide the notify.";
              }
            }
            userDeinit(h);
          }
          else
          {
            e = "Please provide the type.";
          }
        }
        userDeinit(f);
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      userDeinit(c);
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ serverUserRemove()
bool Central::serverUserRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || (serverUser(a, e) && !empty(a.p->m["o"], "server_id")))
    {
      radialUser c;
      userInit(d, c);
      if (!d.g)
      {
        c.p->m["i"]->i("id", a.p->m["o"]->m["server_id"]->v);
      }
      if (d.g || isServerAdmin(c, e))
      {
        b = db("dbCentralServerUserRemove", i, e);
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      userDeinit(c);
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ serverUsersByServerID()
bool Central::serverUsersByServerID(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"server_id"}, i, e))
  {
    list<map<string, string> > rs;
    if (db("dbCentralSserverUsers", i, rs, e))
    {
      b = true;
      for (auto &r : rs)
      {
        radialUser a;
        Json *j = new Json(r);
        ny(j, "notify");
        userInit(d, a);
        a.p->m["i"]->i("id", r["type_id"]);
        if (contactType(a, e))
        {
          j->i("type", a.p->m["o"]);
        }
        userDeinit(a);
        o->pb(j);
        delete j;
      }
    }
  }

  return b;
}
// }}}
// {{{ setCallbackAddon()
void Central::setCallbackAddon(bool (*pCallback)(const string, radialUser &, string &, bool &))
{
  m_pCallbackAddon = pCallback;
}
// }}}
// {{{ sr()
bool Central::sr(const string strKey, Json *ptData, string &strError)
{
  bool bResult = false;
  list<string> keys = {"central", strKey};

  if (storageRetrieve(keys, ptData, strError))
  {
    bResult = true;
  }

  return bResult;
}
// }}}
// {{{ user()
bool Central::user(radialUser &d, string &e)
{
  return Interface::user(d, e);
}
// }}}
// {{{ userAdd()
bool Central::userAdd(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"userid"}, i, e))
  {
    bool bApplication = false, bCentral = false, bServer = false;
    if (d.auth.find("Central") != d.auth.end())
    {
      bCentral = true;
    }
    else if (!empty(i, "application_id"))
    {
      radialUser a;
      userInit(d, a);
      a.p->m["i"]->i("id", i->m["application_id"]->v);
      if (application(a, e) && !empty(a.p->m["o"], "name"))
      {
        radialUser c;
        userInit(d, c);
        c.p->m["i"]->i("id", i->m["application_id"]->v);
        if ((d.auth.find(a.p->m["o"]->m["name"]->v) != d.auth.end() && d.auth[a.p->m["o"]->m["name"]->v]) || isApplicationDeveloper(c, e))
        {
          bApplication = true;
        }
        userDeinit(c);
      }
      userDeinit(a);
    }
    else if (!empty(i, "server_id"))
    {
      radialUser a;
      userInit(d, a);
      a.p->m["i"]->i("id", i->m["server_id"]->v);
      if (isServerAdmin(a, e))
      {
        bServer = true;
      }
      userDeinit(a);
    }
    if (d.g || bCentral || bApplication || bServer)
    {
      radialUser a;
      userInit(d, a);
      a.p->m["i"]->i("userid", i->m["userid"]->v);
      if (!user(a, e) && e == "No results returned.")
      {
        string id, q;
        i->i("active", "1", 'n');
        i->i("admin", "0", 'n');
        i->i("locked", "0", 'n');
        if (db("dbCentralUserAdd", i, id, q, e))
        {
          b = true;
          o->i("id", id);
        }
      }
      userDeinit(a);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
  }

  return b;
}
// }}}
// {{{ userEdit()
bool Central::userEdit(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || (user(a, e) && !empty(a.p->m["o"], "userid") && d.u == a.p->m["o"]->m["userid"]->v))
    {
      if (exist(i, "active") && !empty(i->m["active"], "value"))
      {
        i->i("active", i->m["active"]->m["value"]->v);
      }
      if (exist(i, "admin") && !empty(i->m["admin"], "value"))
      {
        i->i("admin", i->m["admin"]->m["value"]->v);
      }
      if (exist(i, "locked") && !empty(i->m["locked"], "value"))
      {
        i->i("locked", i->m["locked"]->m["value"]->v);
      }
      b = db("dbCentralUserUpdate", i, e);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ userRemove()
bool Central::userRemove(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"id"}, i, e))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || (user(a, e) && !empty(a.p->m["o"], "userid") && d.u == a.p->m["o"]->m["userid"]->v))
    {
      b = db("dbCentralUserRemove", i, e);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }

  return b;
}
// }}}
// {{{ users()
bool Central::users(radialUser &d, string &e)
{
  bool b = false;
  list<map<string, string> > rs;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (db("dbCentralUsers", i, rs, e))
  {
    b = true;
    for (auto &r : rs)
    {
      Json *j = new Json(r);
      b = true; 
      ny(j, "active");
      ny(j, "admin");
      ny(j, "locked");
      o->pb(j);
      delete j;
    }
  }

  return b;
}
// }}}
}
}
