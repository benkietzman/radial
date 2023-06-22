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
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select id, type, description from account_type where id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
      }
      else
      {
        e = "No results returned.";
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ accountTypes()
bool Central::accountTypes(radialUser &d, string &e)
{
  bool b = false;
  string k = "account_type";
  stringstream q;
  Json *o = d.p->m["o"], *s = new Json;

  if (sr(k, s, e))
  {
    b = true;
    d.p->i("o", s);
  }
  else
  {
    q << "select id, type, description from account_type order by type";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
      sa(k, o, e);
    }
    dbf(g);
  }
  delete s;

  return b;
}
// }}}
// {{{ application()
bool Central::application(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "name"))
  {
    q << "select id, name, date_format(creation_date, '%Y-%m-%d') creation_date, notify_priority_id, website, login_type_id, secure_port, auto_register, account_check, dependable, date_format(retirement_date, '%Y-%m-%d') retirement_date, menu_id, package_type_id, wiki, highlight, description from application where ";
    if (!empty(i, "id"))
    {
      q << "id = " << i->m["id"]->v;
    }
    else
    {
      q << "name = '" << esc(i->m["name"]->v) << "'";
    }
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        Json *j = new Json(g->front());
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
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationAccount()
bool Central::applicationAccount(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select id, application_id, user_id, encrypt, aes, password, ";
    if (!m_strAesSecret.empty())
    {
      q << "aes_decrypt(from_base64(password), sha2('" << esc(m_strAesSecret) << "', 512)) decrypted_password, ";
    }
    q << "type_id, description from application_account where id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        radialUser a;
        Json *j = new Json(g->front());
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
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationAccountAdd()
bool Central::applicationAccountAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["application_id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      if (!empty(i, "user_id"))
      {
        if (exist(i, "encrypt") && !empty(i->m["encrypt"], "value"))
        {
          if (!empty(i, "password"))
          {
            if (exist(i, "type") && !empty(i->m["type"], "id"))
            {
              string strID;
              q << "insert into application_account (application_id, user_id, encrypt, aes, `password`, type_id";
              if (!empty(i, "description"))
              {
                q << ", description";
              }
              q << ") values (" << i->m["application_id"]->v << ", '" << i->m["user_id"]->v << "', " << i->m["encrypt"]->m["value"]->v << ", ";
              if (i->m["encrypt"]->m["value"]->v == "1")
              {
                q << "0, concat('*',upper(sha1(unhex(sha1('" << esc(i->m["password"]->v) << "')))))";
              }
              else if (!m_strAesSecret.empty())
              {
                q << "1, to_base64(aes_encrypt('" << esc(i->m["password"]->v) << "', sha2('" << esc(m_strAesSecret) << "', 512)))";
              }
              else
              {
                q << "0, '" << esc(i->m["password"]->v) << "'";
              }
              q << ", " << i->m["type"]->m["id"]->v;
              if (!empty(i, "description"))
              {
                q << ", '" << esc(i->m["description"]->v) << "'";
              }
              q << ")";
              if (dbu(q.str(), strID, e))
              {
                b = true;
                o->i("id", strID);
              }
            }
            else
            {
              e = "Please provide the type.";
            }
          }
          else
          {
            e = "Please provide the password.";
          }
        }
        else
        {
          e = "Please provide the encrypt.";
        }
      }
      else
      {
        e = "Please provide the user_id.";
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the application_id.";
  }

  return b;
}
// }}}
// {{{ applicationAccountEdit()
bool Central::applicationAccountEdit(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (applicationAccount(a, e))
    {
      if (!empty(i, "user_id"))
      {
        if (exist(i, "encrypt") && !empty(i->m["encrypt"], "value"))
        {
          if (!empty(i, "password"))
          {
            if (exist(i, "type") && !empty(i->m["type"], "id"))
            {
              q << "update application_account set user_id = '" << i->m["user_id"]->v << "', encrypt = " << i->m["encrypt"]->m["value"]->v << ", aes = ";
              if (i->m["encrypt"]->m["value"]->v == "1")
              {
                q << "0, `password` = concat('*',upper(sha1(unhex(sha1('" << esc(i->m["password"]->v) << "')))))";
              }
              else if (!m_strAesSecret.empty())
              {
                q << "1, `password` = to_base64(aes_encrypt('" << esc(i->m["password"]->v) << "', sha2('" << esc(m_strAesSecret) << "', 512)))";
              }
              else
              {
                q << "0, `password` = '" << esc(i->m["password"]->v) << "'";
              }
              q << ", type_id = " << i->m["type"]->m["id"]->v;
              if (exist(i, "description"))
              {
                q << ", description = ";
                if (!empty(i, "description"))
                {
                  q << "'" << esc(i->m["description"]->v) << "'";
                }
                else
                {
                  q << "null";
                }
              }
              q << " where id = " << i->m["id"]->v;
              if (dbu(q.str(), e))
              {
                b = true;
              }
            }
            else
            {
              e = "Please provide the type.";
            }
          }
          else
          {
            e = "Please provide the password.";
          }
        }
        else
        {
          e = "Please provide the encrypt.";
        }
      }
      else
      {
        e = "Please provide the user_id.";
      }
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationAccountRemove()
bool Central::applicationAccountRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (applicationAccount(a, e))
    {
      q << "delete from application_account where id = " << i->m["id"]->v;
      if (dbu(q.str(), e))
      {
        b = true;
      }
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationAccountsByApplicationID()
bool Central::applicationAccountsByApplicationID(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["application_id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      q << "select id, application_id, user_id, encrypt, aes, password, ";
      if (!m_strAesSecret.empty())
      {
        q << "aes_decrypt(from_base64(password), sha2('" << esc(m_strAesSecret) << "', 512)) decrypted_password, ";
      }
      q << "type_id, description from application_account where application_id = " << i->m["application_id"]->v << " order by user_id";
      if (exist(i, "page"))
      {
        size_t unNumPerPage, unOffset, unPage;
        stringstream ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
        ssNumPerPage >> unNumPerPage;
        ssPage >> unPage;
        unOffset = unPage * unNumPerPage;
        q << " limit " << unNumPerPage << " offset " << unOffset;
      }
      auto g = dbq(q.str(), e);
      if (g != NULL)
      {
        b = true;
        for (auto &r : *g)
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
      dbf(g);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationAdd()
bool Central::applicationAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (d.g || d.auth.find("Central") != d.auth.end())
  {
    if (!empty(i, "name"))
    {
      radialUser a;
      userInit(d, a);
      a.p->m["i"]->i("name", i->m["name"]->v);
      if (!application(a, e) && e == "No results returned.")
      {
        string strID;
        e.clear();
        q << "insert into application (name, creation_date) values ('" << esc(i->m["name"]->v) << "', now())";
        if (dbu(q.str(), strID, e))
        {
          radialUser u;
          o->i("id", strID);
          userInit(d, u);
          u.p->m["i"]->i("userid", d.u);
          if (user(u, e) && !empty(u.p->m["o"], "id"))
          {
            radialUser c;
            userInit(d, c);
            c.p->m["i"]->i("type", "Primary Developer");
            if (contactType(c, e) && !empty(c.p->m["o"], "id"))
            {
              q.str("");
              q << "insert into application_contact (application_id, contact_id, type_id, admin, locked, notify) values (" << strID << ", " << u.p->m["o"]->m["id"]->v << ", " << c.p->m["o"]->m["id"]->v << ", 1, 0, 1)";
              if (dbu(q.str(), e))
              {
                b = true;
              }
            }
            userDeinit(c);
          }
          userDeinit(u);
        }
      }
      else if (e.empty())
      {
        e = "Application already exists.";
      }
      userDeinit(a);
    }
    else
    {
      e = "Please provide the name.";
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
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select id, application_id, dependant_id from application_dependant where id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
      }
      else
      {
        e = "No results returned.";
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationDependAdd()
bool Central::applicationDependAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    if (!empty(i, "dependant_id"))
    {
      radialUser a;
      userInit(d, a);
      a.p->m["i"]->i("id", i->m["application_id"]->v);
      if (d.g || isApplicationDeveloper(a, e))
      {
        string strID;
        q << "insert into application_dependant (application_id, dependant_id) values (" << i->m["application_id"]->v << ", " << i->m["dependant_id"]->v << ")";
        if (dbu(q.str(), strID, e))
        {
          b = true;
          o->i("id", strID);
        }
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      userDeinit(a);
    }
    else
    {
      e = "Please provide the dependant_id.";
    }
  }
  else
  {
    e = "Please provide the application_id.";
  }

  return b;
}
// }}}
// {{{ applicationDependRemove()
bool Central::applicationDependRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
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
        q << "delete from application_dependant where id = " << i->m["id"]->v;
        if (dbu(q.str(), e))
        {
          b = true;
        }
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      userDeinit(c);
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
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

  if (!empty(i, "id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      if (!empty(i, "name"))
      {
        q << "update application set name = '" << esc(i->m["name"]->v) << "'";
        q << ", notify_priority_id = ";
        if (exist(i, "notify_priority") && !empty(i->m["notify_priority"], "id"))
        {
          q << i->m["notify_priority"]->m["id"]->v;
        }
        else
        {
          q << "null";
        }
        q << ", website = ";
        if (!empty(i, "website"))
        {
          q << "'" << esc(i->m["website"]->v) << "'";
        }
        else
        {
          q << "null";
        }
        q << ", login_type_id = ";
        if (exist(i, "login_type") && !empty(i->m["login_type"], "id"))
        {
          q << i->m["login_type"]->m["id"]->v;
        }
        else
        {
          q << "null";
        }
        if (exist(i, "secure_port") && !empty(i->m["secure_port"], "value"))
        {
          q << ", secure_port = " << i->m["secure_port"]->m["value"]->v;
        }
        if (exist(i, "auto_register") && !empty(i->m["auto_register"], "value"))
        {
          q << ", auto_register = " << i->m["auto_register"]->m["value"]->v;
        }
        if (exist(i, "account_check") && !empty(i->m["account_check"], "value"))
        {
          q << ", account_check = " << i->m["account_check"]->m["value"]->v;
        }
        if (exist(i, "dependable") && !empty(i->m["dependable"], "value"))
        {
          q << ", dependable = " << i->m["dependable"]->m["value"]->v;
        }
        q << ", menu_id = ";
        if (exist(i, "menu_access") && !empty(i->m["menu_access"], "id"))
        {
          q << i->m["menu_access"]->m["id"]->v;
        }
        else
        {
          q << "null";
        }
        if (exist(i, "wiki") && !empty(i->m["wiki"], "value"))
        {
          q << ", wiki = " << i->m["wiki"]->m["value"]->v;
        }
        q << ", highlight = ";
        if (!empty(i, "highlight"))
        {
          q << "'" << esc(i->m["highlight"]->v) << "'";
        }
        else
        {
          q << "null";
        }
        q << ", description = ";
        if (!empty(i, "description"))
        {
          q << "'" << esc(i->m["description"]->v) << "'";
        }
        else
        {
          q << "null";
        }
        q << ", retirement_date = ";
        if (!empty(i, "retirement_date"))
        {
          q << "'" << esc(i->m["retirement_date"]->v) << "'";
        }
        else
        {
          q << "null";
        }
        q << " where id = " << i->m["id"]->v;
        if (dbu(q.str(), e))
        {
          b = true;
        }
      }
      else
      {
        e = "Please provide the name";
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationIssue()
bool Central::applicationIssue(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select id, application_id, summary, date_format(open_date, '%Y-%m-%d') open_date, date_format(due_date, '%Y-%m-%d') due_date, date_format(close_date, '%Y-%m-%d') close_date, hold, priority, date_format(release_date, '%Y-%m-%d') release_date from application_issue where id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        Json *j = new Json(g->front());
        b = true;
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
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
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
    if (!empty(i, "application_id"))
    {
      string strID;
      q << "insert into application_issue (application_id, open_date";
      if (!empty(i, "summary"))
      {
        q << ", summary";
      }
      if (!empty(i, "due_date"))
      {
        q << ", due_date";
      }
      if (!empty(i, "priority"))
      {
        q << ", priority";
      }
      q << ") values (" << i->m["application_id"]->v << ", now()";
      if (!empty(i, "summary"))
      {
        q << ", '" << esc(i->m["summary"]->v) << "'";
      }
      if (!empty(i, "due_date"))
      {
        q << ", '" << esc(i->m["due_date"]->v) << "'";
      }
      if (!empty(i, "priority"))
      {
        q << ", '" << esc(i->m["priority"]->v) << "'";
      }
      q << ")";
      if (dbu(q.str(), strID, e))
      {
        b = true;
        o->i("id", strID);
        if (!empty(i, "comments"))
        {
          radialUser a;
          userInit(d, a);
          a.p->m["i"]->i("issue_id", strID);
          a.p->m["i"]->i("comments", i->m["comments"]->v);
          applicationIssueCommentAdd(a, e);
          userDeinit(a);
        }
        if (!empty(i, "server"))
        {
          radialUser a;
          userInit(d, a);
          a.p->m["i"]->i("id", strID);
          a.p->m["i"]->i("action", "add");
          a.p->m["i"]->i("application_id", i->m["application_id"]->v);
          a.p->m["i"]->i("server", i->m["server"]->v);
          applicationIssueEmail(a, e);
          userDeinit(a);
        }
      }
    }
    else
    {
      e = "Please provide the application_id.";
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
  stringstream q;
  Json *i = d.p->m["i"];

  if (!d.u.empty())
  {
    if (!empty(i, "id"))
    {
      q << "update application_issue set close_date = now() where id = " << i->m["id"]->v;
      if (dbu(q.str(), e))
      {
        b = true;
      }
    }
    else
    {
      e = "Please provide the id.";
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ applicationIssueCommentAdd()
bool Central::applicationIssueCommentAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["i"];

  if (!d.u.empty())
  {
    if (!empty(i, "issue_id"))
    {
      if (!empty(i, "comments"))
      {
        q << "select id from person where userid = '" << d.u << "'";
        auto g = dbq(q.str(), e);
        if (g != NULL)
        {
          if (!g->empty())
          {
            auto r = g->front();
            string strID;
            q.str("");
            q << "insert issue_comment (issue_id, entry_date, user_id, comments) values (" << i->m["issue_id"]->v << ", now(), " << r["id"] << ", '" << esc(i->m["comments"]->v) << "')";
            if (dbu(q.str(), strID, e))
            {
              b = true;
              o->i("id", strID);
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
          else
          {
            e = "No results returned.";
          }
        }
        dbf(g);
      }
      else
      {
        e = "Please provide the comments.";
      }
    }
    else
    {
      e = "Please provide the issue_id.";
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
  stringstream q;
  Json *i = d.p->m["i"];

  if (!d.u.empty())
  {
    if (!empty(i, "id"))
    {
      if (!empty(i, "comments"))
      {
        q << "select id from person where userid = '" << d.u << "'";
        auto g = dbq(q.str(), e);
        if (g != NULL)
        {
          if (!g->empty())
          {
            auto r = g->front();
            q.str("");
            q << "select user_id from issue_comment where id = " << i->m["id"]->v;
            auto h = dbq(q.str(), e);
            if (h != NULL)
            {
              if (!h->empty())
              {
                auto s = h->front();
                if (r["id"] == s["user_id"])
                {
                  q.str("");
                  q << "update issue_comment set comments = '" << esc(i->m["comments"]->v) << "' where id = " << i->m["id"]->v;
                  if (dbu(q.str(), e))
                  {
                    b = true;
                  }
                }
                else
                {
                  e = "You are not authorized to perform this action.";
                }
              }
              else
              {
                e = "No results returned.";
              }
            }
            dbf(h);
          }
          else
          {
            e = "No results returned.";
          }
        }
        dbf(g);
      }
      else
      {
        e = "Please provide the comments.";
      }
    }
    else
    {
      e = "Please provide the id.";
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
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "issue_id"))
  {
    q << "select a.id, date_format(a.entry_date, '%Y-%m-%d %H:%i:%s') entry_date, a.user_id, a.comments, b.last_name, b.first_name, b.userid, b.email from issue_comment a, person b where a.user_id = b.id and a.issue_id = " << i->m["issue_id"]->v << " order by entry_date, id";
    if (!empty(i, "limit"))
    {
      q << " limit " << i->m["limit"]->v;
    }
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the issue_id.";
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

  if (!empty(i, "application_id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("application_id", i->m["application_id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      if (!empty(i, "id"))
      {
        q << "update application_issue set ";
        q << "application_id = ";
        if (exist(i, "transfer") && !empty(i->m["transfer"], "id") && i->m["transfer"]->m["id"]->v != i->m["application_id"]->v)
        {
          q << i->m["transfer"]->m["id"]->v;
        }
        else
        {
          q << i->m["application_id"]->v;
        }
        q << ", close_date = ";
        if (!empty(i, "close_date"))
        {
          q << "'" << i->m["close_date"]->v << "'";
        }
        else
        {
          q << "null";
        }
        q << ", summary = ";
        if (!empty(i, "summary"))
        {
          q << "'" << esc(i->m["summary"]->v) << "'";
        }
        else
        {
          q << "null";
        }
        q << ", due_date = ";
        if (!empty(i, "due_date"))
        {
          q << "'" << i->m["due_date"]->v << "'";
        }
        else
        {
          q << "null";
        }
        q << ", hold = ";
        if (!empty(i, "hold"))
        {
          q << i->m["hold"]->v;
        }
        else
        {
          q << "0";
        }
        q << ", priority = ";
        if (!empty(i, "priority"))
        {
          q << i->m["priority"]->v;
        }
        else
        {
          q << "null";
        }
        q << ", release_date = ";
        if (!empty(i, "release_date"))
        {
          q << "'" << i->m["release_date"]->v << "'";
        }
        else
        {
          q << "null";
        }
        q << " where id = " << i->m["id"]->v;
        if (dbu(q.str(), e))
        {
          b = true;
        }
      }
      else
      {
        e = "Please provide the id.";
      }
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the application_id.";
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
    if (!empty(i, "action"))
    {
      if (i->m["action"]->v == "add" || i->m["action"]->v == "close" || i->m["action"]->v == "transfer" || i->m["action"]->v == "update")
      {
        if (!empty(i, "server"))
        {
          if (!empty(i, "id"))
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
                  for (auto &contact : f.p->m["o"]->l)
                  {
                    if (!empty(contact, "email"))
                    {
                      to.push_back(contact->m["email"]->v);
                    }
                  }
                  if (i->m["action"]->v == "transfer" && !empty(i, "application_id"))
                  {
                    radialUser h;
                    userInit(d, h);
                    h.p->m["i"]->i("id", i->m["application_id"]->v);
                    if (application(h, e) && !empty(h.p->m["o"], "name"))
                    {
                      radialUser k;
                      strApplication = h.p->m["o"]->m["name"]->v;
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
            e = "Please provide the id.";
          }
        }
        else
        {
          e = "Please provide the server.";
        }
      }
      else
      {
        e = "Please provide a valid action:  add, close, transfer, update.";
      }
    }
    else
    {
      e = "Please provide the action.";
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
  bool b = false, bApplication, bBackupDeveloper, bComments, bContact, bOpen, bOwner, bPrimaryDeveloper, bPrimaryContact;
  string strCloseDateEnd, strCloseDateStart, strCommenter, strDisplay, strOpenDateEnd, strOpenDateStart;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  bApplication = (!empty(i, "application") && i->m["application"]->v == "1");
  bBackupDeveloper = (!empty(i, "Backup Developer") && i->m["Backup Developer"]->v == "1");
  bComments = (!empty(i, "comments") && i->m["comments"]->v == "1");
  bContact = (!empty(i, "Contact") && i->m["Contact"]->v == "1");
  bOpen = (!empty(i, "open") && i->m["open"]->v == "1");
  bOwner = (!empty(i, "owner") && i->m["owner"]->v == "1");
  bPrimaryContact = (!empty(i, "Primary Contact") && i->m["Primary Contact"]->v == "1");
  bPrimaryDeveloper = (!empty(i, "Primary Developer") && i->m["Primary Developer"]->v == "1");
  strCloseDateEnd = ((!empty(i, "close_date_end"))?i->m["close_date_end"]->v:"");
  strCloseDateStart = ((!empty(i, "close_date_start"))?i->m["close_date_start"]->v:"");
  strCommenter = ((!empty(i, "commenter"))?i->m["commenter"]->v:"");
  strDisplay = ((!empty(i, "display"))?i->m["display"]->v:"");
  strOpenDateEnd = ((!empty(i, "open_date_end"))?i->m["open_date_end"]->v:"");
  strOpenDateStart = ((!empty(i, "open_date_start"))?i->m["open_date_start"]->v:"");
  q << "select id, application_id, summary, date_format(open_date, '%Y-%m-%d') open_date, date_format(due_date, '%Y-%m-%d') due_date, date_format(close_date, '%Y-%m-%d') close_date, hold, priority, date_format(release_date, '%Y-%m-%d') release_date from application_issue";
  if (bOpen || !strOpenDateStart.empty() || !strOpenDateEnd.empty() || !strCloseDateStart.empty() || !strCloseDateEnd.empty())
  {
    bool bFirst = true;
    if (bOpen && strDisplay != "all")
    {
      q << ((bFirst)?" where":" and") << " close_date is null";
      bFirst = false;
    }
    if (!strOpenDateStart.empty())
    {
      q << ((bFirst)?" where":" and") << " date_format(open_date, '%Y-%m-%d') >= '" << strOpenDateStart << "'";
      bFirst = false;
    }
    if (!strOpenDateEnd.empty())
    {
      q << ((bFirst)?" where":" and") << " date_format(open_date, '%Y-%m-%d') < '" << strOpenDateEnd << "'";
      bFirst = false;
    }
    if (!strCloseDateStart.empty())
    {
      q << ((bFirst)?" where":" and") << " date_format(close_date, '%Y-%m-%d') >= '" << strCloseDateStart << "'";
      bFirst = false;
    }
    if (!strCloseDateEnd.empty())
    {
      q << ((bFirst)?" where":" and") << " date_format(close_date, '%Y-%m-%d') < '" << strCloseDateEnd << "'";
      bFirst = false;
    }
    q << " order by due_date, priority desc, id";
  }
  auto g = dbq(q.str(), e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
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
  dbf(g);

  return b;
}
// }}}
// {{{ applicationIssuesByApplicationID()
bool Central::applicationIssuesByApplicationID(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    q << "select id, summary, date_format(open_date, '%Y-%m-%d') open_date, date_format(close_date, '%Y-%m-%d') close_date, date_format(due_date, '%Y-%m-%d') due_date, hold, priority, date_format(release_date, '%Y-%m-%d') release_date from application_issue where application_id = " << i->m["application_id"]->v;
    if (!empty(i, "open") && i->m["open"]->v == "1")
    {
      q << " and close_date is null";
    }
    q << " order by close_date, open_date, id";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        Json *j = new Json(r);
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
    dbf(g);
  }
  else
  {
    e = "Please provide the application_id.";
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
// {{{ applicationRemove()
bool Central::applicationRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      q << "delete from application where id = " << i->m["id"]->v;
      if (dbu(q.str(), e))
      {
        b = true;
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applications()
bool Central::applications(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  q << "select id, highlight, menu_id, name, package_type_id, date_format(retirement_date, '%Y-%m-%d %H:%i:%s') retirement_date, website from application where 1";
  if (!empty(i, "dependable") && i->m["dependable"]->v == "1")
  {
    q << " and dependable = 1";
  }
  if (!empty(i, "letter"))
  {
    q << " and";
    if (i->m["letter"]->v == "#")
    {
      q << " name regexp '^[ -@[-`{-~]'";
    }
    else
    {
      q << " upper(name) like '" << i->m["letter"]->v << "%'";
    }
  }
  q << " order by name";
  if (!empty(i, "page"))
  {
    size_t unNumPerPage, unOffset, unPage;
    stringstream ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
    ssNumPerPage >> unNumPerPage;
    ssPage >> unPage;
    unOffset = unPage * unNumPerPage;
    q << " limit " << unNumPerPage << " offset " << unOffset;
  }
  auto g = dbq(q.str(), e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      Json *j = new Json(r);
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
      o->pb(j);
      delete j;
    }
  }
  dbf(g);

  return b;
}
// }}}
// {{{ applicationsByServerID()
bool Central::applicationsByServerID(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "server_id"))
  {
    q << "select a.id, b.id application_id, b.name from application_server a, application b where a.application_id = b.id and a.server_id = " << i->m["server_id"]->v;
    if (!empty(i, "retired") && i->m["retired"]->v == "1")
    {
      q << " and b.retirement_date is null";
    }
    q << " order by b.name";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the server_id.";
  }

  return b;
}
// }}}
// {{{ applicationsByUserID()
bool Central::applicationsByUserID(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "contact_id"))
  {
    q << "select distinct a.id, b.id application_id, b.name, c.type from application_contact a, application b, contact_type c where a.application_id = b.id and a.type_id = c.id and a.contact_id = " << i->m["contact_id"]->v << " order by b.name";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the contact_id.";
  }

  return b;
}
// }}}
// {{{ applicationServer()
bool Central::applicationServer(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select id, application_id, server_id from application_server where id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
      }
      else
      {
        e = "No results returned.";
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationServerAdd()
bool Central::applicationServerAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["application_id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      if (!empty(i, "server_id"))
      {
        string strID;
        q << "insert into application_server (application_id, server_id) values (" << i->m["application_id"]->v << ", " << i->m["server_id"]->v << ")";
        if (dbu(q.str(), strID, e))
        {
          b = true;
          o->i("id", strID);
        }
      }
      else
      {
        e = "Please provide the server_id.";
      }
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the application_id.";
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

  if (!empty(i, "id"))
  {
    q << "select id, application_server_id, version, daemon, owner, script, delay, min_processes max_processes, min_image, max_image, min_resident, max_resident from application_server_detail where id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
      }
      else
      {
        e = "No results returned.";
      }
    }
    dbf(g);
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
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_server_id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["application_server_id"]->v);
    if (d.g || applicationServer(a, e))
    {
      string strID;
      q << "insert into application_server_detail (application_server_id, version, daemon, owner, script, delay, min_processes, max_processes, min_image, max_image, min_resident, max_resident) values (" << i->m["application_server_id"]->v;
      if (!empty(i, "version"))
      {
        q << ", '" << esc(i->m["version"]->v) << "'";
      }
      else
      {
        q << ", null";
      }
      if (!empty(i, "daemon"))
      {
        q << ", '" << esc(i->m["daemon"]->v) << "'";
      }
      else
      {
        q << ", null";
      }
      if (!empty(i, "owner"))
      {
        q << ", '" << esc(i->m["owner"]->v) << "'";
      }
      else
      {
        q << ", null";
      }
      if (!empty(i, "script"))
      {
        q << ", '" << esc(i->m["script"]->v) << "'";
      }
      else
      {
        q << ", null";
      }
      q << ", " << ((!empty(i, "delay"))?i->m["delay"]->v:"0");
      q << ", " << ((!empty(i, "min_processes"))?i->m["min_processes"]->v:"0");
      q << ", " << ((!empty(i, "max_processes"))?i->m["max_processes"]->v:"0");
      q << ", " << ((!empty(i, "min_image"))?i->m["min_image"]->v:"0");
      q << ", " << ((!empty(i, "max_image"))?i->m["max_image"]->v:"0");
      q << ", " << ((!empty(i, "min_resident"))?i->m["min_resident"]->v:"0");
      q << ", " << ((!empty(i, "max_resident"))?i->m["max_resident"]->v:"0");
      q << ")";
      if (dbu(q.str(), strID, e))
      {
        b = true;
        o->i("id", strID);
      }
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the application_server_id.";
  }

  return b;
}
// }}}
// {{{ applicationServerDetailEdit()
bool Central::applicationServerDetailEdit(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
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
          q << "update application_server_detail set";
          q << " version = ";
          if (!empty(i, "verson"))
          {
            q << "'" << esc(i->m["version"]->v) << "'";
          }
          else
          {
            q << "null";
          }
          q << ", daemon = ";
          if (!empty(i, "daemon"))
          {
            q << "'" << esc(i->m["daemon"]->v) << "'";
          }
          else
          {
            q << "null";
          }
          q << ", owner = ";
          if (!empty(i, "owner"))
          {
            q << "'" << esc(i->m["owner"]->v) << "'";
          }
          else
          {
            q << "null";
          }
          q << ", script = ";
          if (!empty(i, "script"))
          {
            q << "'" << esc(i->m["script"]->v) << "'";
          }
          else
          {
            q << "null";
          }
          q << ", delay = " << ((!empty(i, "delay"))?i->m["delay"]->v:"null");
          q << ", min_processes = " << ((!empty(i, "min_processes"))?i->m["min_processes"]->v:"null");
          q << ", max_processes = " << ((!empty(i, "max_processes"))?i->m["max_processes"]->v:"null");
          q << ", min_image = " << ((!empty(i, "min_image"))?i->m["min_image"]->v:"null");
          q << ", max_image = " << ((!empty(i, "max_image"))?i->m["max_image"]->v:"null");
          q << ", min_resident = " << ((!empty(i, "min_resident"))?i->m["min_resident"]->v:"null");
          q << ", max_resident = " << ((!empty(i, "max_resident"))?i->m["max_resident"]->v:"null");
          q << " where id = " << i->m["id"]->v;
          if (dbu(q.str(), e))
          {
            b = true;
          }
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
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationServerDetailRemove()
bool Central::applicationServerDetailRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
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
          q << "delete from application_server_detail where id = " << i->m["id"]->v;
          if (dbu(q.str(), e))
          {
            b = true;
          }
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
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationServerDetails()
bool Central::applicationServerDetails(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_server_id"))
  {
    q << "select id, application_server_id, version, daemon, owner, script, delay, min_processes, max_processes, min_image, max_image, min_resident, max_resident from application_server_detail where application_server_id = " << i->m["application_server_id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the application_server_id.";
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

  if (!empty(i, "id"))
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
        q << "delete from application_server where id = " << i->m["id"]->v;
        if (dbu(q.str(), e))
        {
          b = true;
        }
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      userDeinit(c);
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationUser()
bool Central::applicationUser(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select a.id, a.application_id, a.admin, a.locked, a.notify, a.description, b.type, c.last_name, c.first_name, c.userid, c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        Json *j = new Json(g->front());
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
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
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

  if (!empty(i, "application_id"))
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
        if (!empty(i, "userid"))
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
                      string strID;
                      if (!exist(i, "description"))
                      {
                        i->m["description"] = new Json;
                      }
                      q << "insert into application_contact (application_id, contact_id, type_id, admin, locked, notify, description) values (" << i->m["application_id"]->v << ", " << f.p->m["o"]->m["id"]->v << ", " << h.p->m["o"]->m["id"]->v << ", " << i->m["admin"]->m["value"]->v << ", " << i->m["locked"]->m["value"]->v << ", " << i->m["notify"]->m["value"]->v << ", '" << esc(i->m["description"]->v) << "')";
                      if (dbu(q.str(), strID, e))
                      {
                        b = true;
                        o->i("id", strID);
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
          e = "Please provide the userid.";
        }
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      userDeinit(c);
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the application_id.";
  }

  return b;
}
// }}}
// {{{ applicationUserEdit()
bool Central::applicationUserEdit(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
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
          if (!empty(i, "userid"))
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
                        if (!exist(i, "description"))
                        {
                          i->m["description"] = new Json;
                        }
                        q << "update application_contact set contact_id = " << h.p->m["o"]->m["id"]->v << ", type_id = " << k.p->m["o"]->m["id"]->v << ", admin = " << i->m["admin"]->m["value"]->v << ", locked = " << i->m["locked"]->m["value"]->v << ", notify = " << i->m["notify"]->m["value"]->v << ", description = '" << esc(i->m["description"]->v) << "' where id = " << i->m["id"]->v;
                        if (dbu(q.str(), e))
                        {
                          b = true;
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
            e = "Please provide the userid.";
          }
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
  else
  {
    e = "Please provide the id.";
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

  if (!empty(i, "id"))
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
          q << "delete from application_contact where id = " << i->m["id"]->v;
          if (dbu(q.str(), e))
          {
            b = true;
          }
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
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationUsersByApplicationID()
bool Central::applicationUsersByApplicationID(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    q << "select a.id, a.application_id, a.admin, a.locked, a.notify, a.description, a.type_id, c.id user_id, c.last_name, c.first_name, c.userid, c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.application_id = " << i->m["application_id"]->v;
    if ((!empty(i, "Primary Developer") && i->m["Primary Developer"]->v == "1") || (!empty(i, "Backup Developer") && i->m["Backup Developer"]->v == "1") || (!empty(i, "Primary Contact") && i->m["Primary Contact"]->v == "1") || (!empty(i, "Contact") && i->m["Contact"]->v == "1"))
    {
      bool bFirst = true;
      q << " and b.type in (";
      if (!empty(i, "Primary Developer") && i->m["Primary Developer"]->v == "1")
      {
        q << ((!bFirst)?", ":"") << "'Primary Developer'";
        bFirst = false;
      }
      if (!empty(i, "Backup Developer") && i->m["Backup Developer"]->v == "1")
      {
        q << ((!bFirst)?", ":"") << "'Backup Developer'";
        bFirst = false;
      }
      if (!empty(i, "Primary Contact") && i->m["Primary Contact"]->v == "1")
      {
        q << ((!bFirst)?", ":"") << "'Primary Contact'";
        bFirst = false;
      }
      if (!empty(i, "Contact") && i->m["Contact"]->v == "1")
      {
        q << ((!bFirst)?", ":"") << "'Contact'";
        bFirst = false;
      }
      q << ")";
    }
    q << " order by c.last_name, c.first_name, c.userid";
    if (exist(i, "page"))
    {
      size_t unNumPerPage, unOffset, unPage;
      stringstream ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
      ssNumPerPage >> unNumPerPage;
      ssPage >> unPage;
      unOffset = unPage * unNumPerPage;
      q << " limit " << unNumPerPage << " offset " << unOffset;
    }
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
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
    dbf(g);
  }
  else
  {
    e = "Please provide the application_id.";
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
  bool bInvalid = true, bResult = false;
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
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "type"))
  {
    q << "select id, type from contact_type where ";
    if (!empty(i, "id"))
    {
      q << "id = " << i->m["id"]->v;
    }
    else
    {
      q << "type = '" << esc(i->m["type"]->v) << "'";
    }
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
      }
      else
      {
        e = "No results returned.";
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the id or type.";
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
// {{{ dependentsByApplicationID()
bool Central::dependentsByApplicationID(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    q << "select a.id, b.id application_id, b.name from application_dependant a, application b where a.dependant_id = b.id and a.application_id = " << i->m["application_id"]->v << " order by b.name";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      o->m["depends"] = new Json;
      for (auto &r : *g)
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
      q.str("");
      q << "select a.id, b.id application_id, b.name from application_dependant a, application b where a.application_id = b.id and a.dependant_id = " << i->m["application_id"]->v << " order by b.name";
      auto h = dbq(q.str(), e);
      if (h != NULL)
      {
        b = true;
        o->m["dependents"] = new Json;
        for (auto &r : *h)
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
      dbf(h);
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the application_id.";
  }

  return b;
}
// }}}
// {{{ loginType()
bool Central::loginType(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select id, type from login_type where id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
      }
      else
      {
        e = "No results returned.";
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ loginTypes()
bool Central::loginTypes(radialUser &d, string &e)
{
  bool b = false;
  string k = "login_type";
  stringstream q;
  Json *o = d.p->m["o"], *s = new Json;

  if (sr(k, s, e))
  {
    b = true;
    d.p->i("o", s);
  }
  else
  {
    q << "select id, type from login_type order by type";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
      sa(k, o, e);
    }
    dbf(g);
  }
  delete s;

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
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select id, type from menu_access where id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
      }
      else
      {
        e = "No results returned.";
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ menuAccesses()
bool Central::menuAccesses(radialUser &d, string &e)
{
  bool b = false;
  string k = "menu_access";
  stringstream q;
  Json *o = d.p->m["o"], *s = new Json;

  if (sr(k, s, e))
  {
    b = true;
    d.p->i("o", s);
  }
  else
  {
    q << "select id, type from menu_access order by type";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
      sa(k, o, e);
    }
    dbf(g);
  }
  delete s;

  return b;
}
// }}}
// {{{ notifyPriorities()
bool Central::notifyPriorities(radialUser &d, string &e)
{
  bool b = false;
  string k = "notify_priority";
  stringstream q;
  Json *o = d.p->m["o"], *s = new Json;

  if (sr(k, s, e))
  {
    b = true;
    d.p->i("o", s);
  }
  else
  {
    q << "select id, priority from notify_priority order by priority";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
      sa(k, o, e);
    }
    dbf(g);
  }
  delete s;

  return b;
}
// }}}
// {{{ notifyPriority()
bool Central::notifyPriority(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select id, priority from notify_priority where id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
      }
      else
      {
        e = "No results returned.";
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ packageType()
bool Central::packageType(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select id, type from package_type where id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
      }
      else
      {
        e = "No results returned.";
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ packageTypes()
bool Central::packageTypes(radialUser &d, string &e)
{
  bool b = false;
  string k = "package_type";
  stringstream q;
  Json *o = d.p->m["o"], *s = new Json;

  if (sr(k, s, e))
  {
    b = true;
    d.p->i("o", s);
  }
  else
  {
    q << "select id, type from package_type order by type";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
      sa(k, o, e);
    }
    dbf(g);
  }
  delete s;

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
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "name"))
  {
    q << "select id, name, description, processes, cpu_usage, main_memory, swap_memory, disk_size from server where ";
    if (!empty(i, "id"))
    {
      q << "id = " << i->m["id"]->v;
    }
    else
    {
      q << "name = '" << esc(i->m["name"]->v) << "'";
    }
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
      }
      else
      {
        e = "No results returned.";
      }
    }
    dbf(g);
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
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (d.g || d.auth.find("Central") != d.auth.end())
  {
    if (!empty(i, "name"))
    {
      radialUser a;
      userInit(d, a);
      a.p->m["i"]->i("name", i->m["name"]->v);
      if (!server(a, e) && e == "No results returned.")
      {
        string strID;
        e.clear();
        q << "insert into server (name) values ('" << esc(i->m["name"]->v) << "')";
        if (dbu(q.str(), strID, e))
        {
          b = true;
          o->i("id", strID);
        }
      }
      else if (e.empty())
      {
        e = "Server already exists.";
      }
    }
    else
    {
      e = "Please provide the name.";
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
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    q << "select a.id application_server_id, b.id server_id, b.name, c.id application_server_detail_id, c.daemon from application_server a, server b, application_server_detail c where a.server_id = b.id and a.id = c.application_server_id and a.application_id = " << i->m["application_id"]->v << " order by b.name, c.daemon";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the application_id.";
  }

  return b;
}
// }}}
// {{{ serverEdit()
bool Central::serverEdit(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || isServerAdmin(a, e))
    {
      if (!empty(i, "name"))
      {
        q << "update server set";
        q << " name = '" << esc(i->m["id"]->v) << "'";
        q << ", description = ";
        if (!empty(i, "description"))
        {
          q << "'" << esc(i->m["description"]->v) << "'";
        }
        else
        {
          q << "null";
        }
        q << ", processes = ";
        if (!empty(i, "processes"))
        {
          q << i->m["processes"]->v;
        }
        else
        {
          q << "0";
        }
        q << ", cpu_usage = ";
        if (!empty(i, "cpu_usage"))
        {
          q << i->m["cpu_usage"]->v;
        }
        else
        {
          q << "0";
        }
        q << ", main_memory = ";
        if (!empty(i, "main_memory"))
        {
          q << i->m["main_memory"]->v;
        }
        else
        {
          q << "0";
        }
        q << ", swap_memory = ";
        if (!empty(i, "swap_memory"))
        {
          q << i->m["swap_memory"]->v;
        }
        else
        {
          q << "0";
        }
        q << ", disk_size = ";
        if (!empty(i, "disk_size"))
        {
          q << i->m["disk_size"]->v;
        }
        else
        {
          q << "0";
        }
        q << " where id = " << i->m["id"]->v;
        if (dbu(q.str(), e))
        {
          b = true;
        }
      }
      else
      {
        e = "Please provide the name.";
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
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

  if (!empty(i, "id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || isServerAdmin(a, e))
    {
      q << "delete from server where id = " << i->m["id"]->v;
      if (dbu(q.str(), e))
      {
        b = true;
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ servers()
bool Central::servers(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  q << "select id, name from server";
  if (!empty(i, "letter"))
  {
    q << " where";
    if (i->m["letter"]->v == "#")
    {
      q << " name regexp '^[ -@[-`{-~]'";
    }
    else
    {
      q << " upper(name) like '" << i->m["letter"]->v << "%'";
    }
  }
  q << " order by name";
  if (!empty(i, "page"))
  {
    size_t unNumPerPage, unOffset, unPage;
    stringstream ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
    ssNumPerPage >> unNumPerPage;
    ssPage >> unPage;
    unOffset = unPage * unNumPerPage;
    q << " limit " << unNumPerPage << " offset " << unOffset;
  }
  auto g = dbq(q.str(), e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
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
  dbf(g);

  return b;
}
// }}}
// {{{ serversByApplicationID()
bool Central::serversByApplicationID(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    q << "select a.id, b.id server_id, b.name from application_server a, server b where a.server_id = b.id and a.application_id = " << i->m["application_id"]->v << " order by b.name";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the application_id.";
  }

  return b;
}
// }}}
// {{{ serversByUserID()
bool Central::serversByUserID(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "contact_id"))
  {
    q << "select distinct a.id, b.id server_id, b.name, c.type from server_contact a, server b, contact_type c where a.server_id = b.id and a.type_id = c.id and a.contact_id = " << i->m["contact_id"]->v << " order by b.name";
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the contact_id.";
  }

  return b;
}
// }}}
// {{{ serverUser()
bool Central::serverUser(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select a.id, a.server_id, a.notify, b.type, c.last_name, c.first_name, c.userid, c.email from server_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.id = " << i->m["id"]->v;
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        Json *j = new Json(g->front());
        b = true;
        ny(j, "notify");
        d.p->i("o", j);
        delete j;
      }
      else
      {
        e = "No results returned.";
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ serverUserAdd()
bool Central::serverUserAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "server_id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["server_id"]->v);
    if (d.g || isServerAdmin(a, e))
    {
      if (!empty(i, "userid"))
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
                string strID;
                q << "insert into server_contact (server_id, contact_id, type_id, notify) values (" << i->m["server_id"]->v << ", " << c.p->m["o"]->m["id"]->v << ", " << f.p->m["o"]->m["id"]->v << ", " << i->m["notify"]->m["value"]->v << ")";
                if (dbu(q.str(), strID, e))
                {
                  b = true;
                  o->i("id", strID);
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
        e = "Please provide the userid.";
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the server_id.";
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

  if (!empty(i, "id"))
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
        if (!empty(i, "userid"))
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
                  q << "update server_contact set contact_id = " << f.p->m["o"]->m["id"]->v << ", type_id = " << h.p->m["o"]->m["id"]->v << ", notify = " << i->m["notify"]->m["value"]->v << ", where id = " << i->m["id"]->v;
                  if (dbu(q.str(), e))
                  {
                    b = true;
                  }
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
          e = "Please provide the userid.";
        }
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      userDeinit(c);
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
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

  if (!empty(i, "id"))
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
        q << "delete from server_contact where id = " << i->m["id"]->v;
        if (dbu(q.str(), e))
        {
          b = true;
        }
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      userDeinit(c);
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ serverUsersByServerID()
bool Central::serverUsersByServerID(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "server_id"))
  {
    q << "select a.id, a.server_id, a.notify, a.type_id, c.id user_id, c.last_name, c.first_name, c.userid, c.email from server_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.server_id = " << i->m["server_id"]->v;
    if ((!empty(i, "Primary Admin") && i->m["Primary Admin"]->v == "1") || (!empty(i, "Backup Admin") && i->m["Backup Admin"]->v == "1") || (!empty(i, "Primary Contact") && i->m["Primary Contact"]->v == "1") || (!empty(i, "Contact") && i->m["Contact"]->v == "1"))
    {
      bool bFirst = true;
      q << " and b.type in (";
      if (!empty(i, "Primary Admin") && i->m["Primary Admin"]->v == "1")
      {
        q << ((!bFirst)?", ":"") << "'Primary Admin'";
        bFirst = false;
      }
      if (!empty(i, "Backup Admin") && i->m["Backup Admin"]->v == "1")
      {
        q << ((!bFirst)?", ":"") << "'Backup Admin'";
        bFirst = false;
      }
      if (!empty(i, "Primary Contact") && i->m["Primary Contact"]->v == "1")
      {
        q << ((!bFirst)?", ":"") << "'Primary Contact'";
        bFirst = false;
      }
      if (!empty(i, "Contact") && i->m["Contact"]->v == "1")
      {
        q << ((!bFirst)?", ":"") << "'Contact'";
        bFirst = false;
      }
      q << ")";
    }
    q << " order by c.last_name, c.first_name, c.userid";
    if (exist(i, "page"))
    {
      size_t unNumPerPage, unOffset, unPage;
      stringstream ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
      ssNumPerPage >> unNumPerPage;
      ssPage >> unPage;
      unOffset = unPage * unNumPerPage;
      q << " limit " << unNumPerPage << " offset " << unOffset;
    }
    auto g = dbq(q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
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
        o->pb(j);
        delete j;
      }
    }
    dbf(g);
  }
  else
  {
    e = "Please provide the server_id.";
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
// {{{ userAdd()
bool Central::userAdd(radialUser &d, string &e)
{
  bool b = false, bApplication = false, bCentral = false, bServer = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

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
    if (!empty(i, "userid"))
    {
      radialUser a;
      userInit(d, a);
      a.p->m["i"]->i("userid", i->m["userid"]->v);
      if (!user(a, e) && e == "No results returned.")
      {
        string strID;
        q << "insert into person (userid, active, admin, locked) values ('" << i->m["userid"]->v << "', 1, 0, 0)";
        if (dbu(q.str(), strID, e))
        {
          b = true;
          o->i("id", strID);
        }
      }
      userDeinit(a);
    }
    else
    {
      e = "Please provide the userid.";
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ userEdit()
bool Central::userEdit(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || (user(a, e) && !empty(a.p->m["o"], "userid") && d.u == a.p->m["o"]->m["userid"]->v))
    {
      if (!empty(i, "userid"))
      {
        q << "update person set";
        q << " userid = '" << esc(i->m["userid"]->v) << "'";
        q << ", first_name = ";
        if (!empty(i, "first_name"))
        {
          q << "'" << esc(i->m["first_name"]->v) << "'";
        }
        else
        {
          q << "null";
        }
        q << ", last_name = ";
        if (!empty(i, "last_name"))
        {
          q << "'" << esc(i->m["last_name"]->v) << "'";
        }
        else
        {
          q << "null";
        }
        q << ", email = ";
        if (!empty(i, "email"))
        {
          q << "'" << esc(i->m["email"]->v) << "'";
        }
        else
        {
          q << "null";
        }
        q << ", pager = ";
        if (!empty(i, "pager"))
        {
          q << "'" << esc(i->m["pager"]->v) << "'";
        }
        else
        {
          q << "null";
        }
        q << ", active = ";
        if (exist(i, "active") && !empty(i->m["active"], "value") && m_manip.isNumeric(i->m["active"]->m["value"]->v))
        {
          q << i->m["active"]->m["value"]->v;
        }
        else
        {
          q << "null";
        }
        q << ", admin = ";
        if (exist(i, "admin") && !empty(i->m["admin"], "value") && m_manip.isNumeric(i->m["admin"]->m["value"]->v))
        {
          q << i->m["admin"]->m["value"]->v;
        }
        else
        {
          q << "null";
        }
        q << ", locked = ";
        if (exist(i, "locked") && !empty(i->m["locked"], "value") && m_manip.isNumeric(i->m["locked"]->m["value"]->v))
        {
          q << i->m["locked"]->m["value"]->v;
        }
        else
        {
          q << "null";
        }
        q << ", `password` = ";
        if (!empty(i, "password"))
        {
          q << "concat('*',upper(sha1(unhex(sha1('" << esc(i->m["password"]->v) << "')))))";
        }
        else
        {
          q << "null";
        }
        q << " where id = " << i->m["id"]->v;
        if (dbu(q.str(), e))
        {
          b = true;
        }
      }
      else
      {
        e = "Please provide the userid.";
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ userRemove()
bool Central::userRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    radialUser a;
    userInit(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (d.g || (user(a, e) && !empty(a.p->m["o"], "userid") && d.u == a.p->m["o"]->m["userid"]->v))
    {
      q << "delete from person where id = " << i->m["id"]->v;
      if (dbu(q.str(), e))
      {
        b = true;
      }
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
    userDeinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ users()
bool Central::users(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  q << "select id, last_name, first_name, userid, email, pager, active, admin, locked from person";
  if (!empty(i, "letter"))
  {
    q << " where";
    if (i->m["letter"]->v == "#")
    {
      q << " last_name regexp '^[ -@[-`{-~]'";
    }
    else
    {
      q << " upper(last_name) like '" << i->m["letter"]->v << "%'";
    }
  }
  q << " order by last_name, first_name, userid";
  if (!empty(i, "page"))
  {
    size_t unNumPerPage, unOffset, unPage;
    stringstream ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
    ssNumPerPage >> unNumPerPage;
    ssPage >> unPage;
    unOffset = unPage * unNumPerPage;
    q << " limit " << unNumPerPage << " offset " << unOffset;
  }
  auto g = dbq(q.str(), e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      o->pb(r);
    }
  }
  dbf(g);

  return b;
}
// }}}
}
}
