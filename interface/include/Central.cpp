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
Central::Central(int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "central", argc, argv, pCallback)
{
  string e;

  m_ptCred = new Json;
  if (m_pWarden != NULL)
  {
    Json *ptAes = new Json, *ptJwt = new Json;
    m_pWarden->vaultRetrieve({"radial", "radial"}, m_ptCred, e);
    if (m_pWarden->vaultRetrieve({"aes"}, ptAes, strError))
    {
      if (ptAes->m.find("Secret") != ptAes->m.end() && !ptAes->m["Secret"]->v.empty())
      {
        m_strAesSecret = ptAes->m["Secret"]->v;
      }
    }
    delete ptAes;
    if (m_pWarden->vaultRetrieve({"jwt"}, ptJwt, strError))
    {
      if (ptJwt->m.find("Secret") != ptJwt->m.end() && !ptJwt->m["Secret"]->v.empty())
      {
        m_strJwtSecret = ptJwt->m["Secret"]->v;
      }
      if (ptJwt->m.find("Signer") != ptJwt->m.end() && !ptJwt->m["Signer"]->v.empty())
      {
        m_strJwtSigner = ptJwt->m["Signer"]->v;
      }
    }
    delete ptJwt;
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
  m_functions["applicationBotLink"] = &Central::applicationBotLink;
  m_functions["applicationBotLinkAdd"] = &Central::applicationBotLinkAdd;
  m_functions["applicationBotLinkRemove"] = &Central::applicationBotLinkRemove;
  m_functions["applicationBotLinksByApplicationID"] = &Central::applicationBotLinksByApplicationID;
  m_functions["applicationDepend"] = &Central::applicationDepend;
  m_functions["applicationDependAdd"] = &Central::applicationDependAdd;
  m_functions["applicationDependRemove"] = &Central::applicationDependRemove;
  m_functions["applicationEdit"] = &Central::applicationEdit;
  m_functions["applicationIssue"] = &Central::applicationIssue;
  m_functions["applicationIssueAdd"] = &Central::applicationIssueAdd;
  m_functions["applicationIssueClose"] = &Central::applicationISsueClose;
  m_functions["applicationIssueCommentAdd"] = &Central::applicationIssueCommentAdd;
  m_functions["applicationIssueCommentEdit"] = &Central::applicationISsueCommentEdit;
  m_functions["applicationIssueComments"] = &Central::applicationIssueComments;
  m_functions["applicationIssueEdit"] = &Central::applicationIssueEdit;
  m_functions["applicationIssueEmail"] = &Central::applicationIssueEmail;
  m_functions["applicationIssues"] = &Central::applicationIssues;
  m_functions["applicationIssuesByApplicationID"] = &Central::applicationIssuesByApplicationID;
  m_functions["appplicationNotify"] = &Central::appplicationNotify;
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
  m_functions["botLinkRemoteSystem"] = &Central::botLinkRemoteSystem;
  m_functions["botLinkRemoveSystems"] = &Central::botLinkRemoveSystems;
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
  m_functions["ny"] = &Central::ny;
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
  m_functions["serversByUserID"] = &Central::serverByUserID;
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
bool Central::accountType(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "id"))
  {
    q << "select id, type, description from account_type where id = " << i->m["id"]->v;
    auto g = dbq(q, e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
        o = d.p->m["o"];
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
bool Central::accountTypes(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  q << "select id, type, description from account_type order by type";
  auto g = dbq(q, e);
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
// {{{ application()
bool Central::application(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

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
    auto g = dbq(q, e);
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
            data l;
            init(d, l);
            l.p->m["i"]->i("id", j->m["login_type_id"]->v);
            if (loginType(l, e))
            {
              j->i("login_type", l.p->m["o"]);
            }
            deinit(l);
          }
        }
        if (!empty(j, "menu_id"))
        {
          size_t unValue;
          stringstream ssValue(j->m["menu_id"]->v);
          ssValue >> unValue;
          if (unValue > 0)
          {
            data m;
            init(d, m);
            m.p->m["i"]->i("id", j->m["menu_id"]->v);
            if (menuAccess(m, e))
            {
              j->i("menu_access", m.p->m["o"]);
            }
            deinit(m);
          }
        }
        if (!empty(j, "notify_priority_id"))
        {
          size_t unValue;
          stringstream ssValue(j->m["notify_priority_id"]->v);
          ssValue >> unValue;
          if (unValue > 0)
          {
            data n;
            init(d, n);
            n.p->m["i"]->i("id", j->m["notify_priority_id"]->v);
            if (notifyPriority(n, e))
            {
              j->i("notify_priority", n.p->m["o"]);
            }
            deinit(n);
          }
        }
        if (!empty(j, "package_type_id"))
        {
          size_t unValue;
          stringstream ssValue(d->m["package_type_id"]->v);
          ssValue >> unValue;
          if (unValue > 0)
          {
            data p;
            init(d, n);
            p.p->m["i"]->i("id", j->m["package_type_id"]->v);
            if (packageType(p, e))
            {
              j->i("package_type", p.p->m["o"]);
            }
            deinit(p);
          }
        }
        ny(d, "secure_port");
        ny(d, "wiki");
        d.p->i("o", j);
        o = d.p->m["o"];
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
bool Central::applicationAccount(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "id"))
  {
    q << "select id, application_id, user_id, encrypt, aes, password, ";
    if (!empty(i, "aes") && i->m["aes"]->v != "0")
    {
      q << "aes_decrypt(from_base64(password), sha2('" << esc(i->m["aes"]->v) << "', 512)) decrypted_password, ";
    }
    q << "type_id, description from application_account where id = " << i->m["id"]->v;
    auto g = dbq(q, e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        data a;
        Json *j = new Json(g->front());
        init(d, a);
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
          o = d.p->m["o"];
        }
        else
        {
          e = "You are not authorized to perform this action.";
        }
        deinit(a);
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
bool Central::applicationAccountAdd(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    data a;
    init(d, a);
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
              else if (!empty(i, "aes") && i->m["aes"]->v != "0")
              {
                q << "1, to_base64(aes_encrypt('" << esc(i->m["password"]->v) << "', sha2('" << esc(i->m["aes"]->v) << "', 512)))";
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
              if (dbu(q, e))
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
    else
    {
      e = "You are not authorized to perform this action.";
    }
    deinit(a);
  }
  else
  {
    e = "Please provide the application_id.";
  }

  return b;
}
// }}}
// {{{ applicationAccountEdit()
bool Central::applicationAccountEdit(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "id"))
  {
    data a;
    init(d, a);
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
              else if (!empty(i, "aes") && i->m["aes"]->v != "0")
              {
                q << "1, `password` = to_base64(aes_encrypt('" << esc(i->m["password"]->v) << "', sha2('" << esc(i->m["aes"]->v) << "', 512)))";
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
              q << ") where id = " << i->m["id"]->v;
              if (dbu(q, e))
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
    deinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationAccountRemove()
bool Central::applicationAccountRemove(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "id"))
  {
    data a;
    init(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (applicationAccount(a, e))
    {
      q << "delete from application_account where id = " << i->m["id"]->v;
      if (dbu(q, e))
      {
        b = true;
      }
    }
    deinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationAccountsByApplicationID()
bool Central::applicationAccountsByApplicationID(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    data a;
    init(d, a);
    a.p->m["i"]->i("id", i->m["application_id"]->v);
    if (d.g || isApplicationDeveloper(a, e))
    {
      q << "select id, application_id, user_id, encrypt, aes, password, ";
      if (!empty(i, "aes") && i->m["aes"]->v != "0")
      {
        q << "aes_decrypt(from_base64(password), sha2('" << esc(i->m["aes"]->v) << "', 512)) decrypted_password, ";
      }
      q << "type_id, description from application_account where application_id = " << i->m["application_id"]->v << " order by user_id";
      if (exist(i, "page"))
      {
        size_t unNumPerPage, unOffset, unPage;
        stringstring ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
        ssNumPerPage >> unNumPerPage;
        ssPage >> unPage;
        unOffset = unPage * unNumPerPage;
        q << " limit " << unNumPerPage << " offset " << unOffset;
      }
      auto g = dbq(q, e);
      if (g != NULL)
      {
        b = true;
        for (auto &r : *g)
        {
          Json *j = new Json(r);
          if (!empty(j, "encrypt") && j->m["encrypt"]->v == 1 && exist(j, "password"))
          {
            rm(j, "password");
          }
          else if (!empty(j, "aes") && j->m["aes"]->v == 1)
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
            data t;
            init(d, t);
            t.p->m["i"]->i("id", j->m["type_id"]->v);
            if (accountType(t, e))
            {
              j->i("type", t.p->m["o"]);
            }
            deinit(t);
          }
          o->pb(j);
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
      e = "You are not authorized to perform this action.";
    }
    deinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationAdd()
bool Central::applicationAdd(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!d.u.empty() && (d.g || d.auth.find("Central") != d.auth.end()))
  {
    if (!empty(i, "name"))
    {
      data a;
      init(d, a);
      a.p->m["i"]->i("name", i->m["name"]->v);
      if (!application(a, e) && e == "No results returned.")
      {
        q << "insert into application (name, creation_date) values ('" << esc(i->m["name"]->v) << "', now())";
        if (dbu(q, e))
        {
          data n;
          init(d, n);
          n.p->m["i"]->i("name", i->m["name"]->v);
          if (application(n, e) && !empty(n.p->m["o"], "id"))
          {
            data u;
            init(d, u);
            u.p->m["i"]->i("userid", d.u);
            if (user(u, e) && !empty(u.p->m["o"], "id"))
            {
              data c;
              init(d, c);
              c.p->m["i"]->i("type", "Primary Developer");
              if (contactType(c, e) && !empty(c.p->m["o"], "id"))
              {
                q.str("");
                q << "insert into application_contact (application_id, contact_id, type_id, admin, locked, notify) values (" << n.p->m["o"]->m["id"]->v << ", " << u.p->m["o"]->m["id"]->v << ", " << c.p->m["o"]->m["id"]->v << ", 1, 0, 1)";
                if (dbu(q, e))
                {
                  b = true;
                }
              }
              deinit(c);
            }
            deinit(u);
          }
          deinit(n);
        }
      }
      else
      {
        e = "Application already exists.";
      }
      deinit(a);
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
bool Central::applicationDepend(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "id"))
  {
    q << "select id, application_id, dependant_id from application_dependant where id = " << i->m["id"]->v;
    auto g = dbq(q, e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
        o = d.p->m["o"];
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
bool Central::applicationDependAdd(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "application_id"))
  {
    if (!empty(i, "dependant_id"))
    {
      data a;
      init(d, a);
      a.p->m["i"]->i("id", i->m["id"]->v);
      if (d.g || isApplicationDeveloper(a, e))
      {
        q << "insert into application_dependant (application_id, dependant_id) values (" << i->m["application_id"]->v << ", " << i->m["dependant_id"]->v << ")";
        if (dbu(q, e))
        {
          b = true;
        }
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      deinit(a);
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
bool Central::applicationDependRemove(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "id"))
  {
    data a;
    init(d, a);
    a.p->m["i"]->i("id", i->m["id"]->v);
    if (applicationDepend(a, e) && !empty(a.p->m["o"], "application_id"))
    {
      data c;
      init(d, c);
      c.p->m["i"]->i("id", a.p->m["o"]->m["application_id"]->v);
      if (d.g || isApplicationDeveloper(c, e))
      {
        q << "delete from application_dependant where id = " << i->m["id"]->v;
        if (dbu(q, e))
        {
          b = true;
        }
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
      deinit(c);
    }
    deinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationEdit()
bool Central::applicationEdit(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "id"))
  {
    data a;
    init(d, a);
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
        if (dbu(q, e))
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
    deinit(a);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ applicationIssue()
bool Central::applicationIssue(data &d, string &e)
{
  bool b = false, bComments = (exist(i, "comments") && i->m["comments"]->v == "1");
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "id"))
  {
    q << "select id, application_id, date_format(open_date, '%Y-%m-%d') open_date, date_format(due_date, '%Y-%m-%d') due_date, date_format(close_date, '%Y-%m-%d') close_date, hold, priority from application_issue where id = " << i->m["id"]->v;
    auto g = dbq(q, e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        Json *j = new Json(g->front());
        b = true;
        if (bComments && !empty(j, "id"))
        {
          data a;
          init(d, a);
          a.p->m["i"]->i("issue_id", j->m["id"]->v);
          if (applicationIssueComments(a, e))
          {
            j->i("comments", a.p->m["o"]);
          }
          deinit(a);
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
bool Central::applicationIssueAdd(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!d.u.empty())
  {
    if (!empty(i, "application_id"))
    {
      q << "insert into application_issue (application_id, open_date";
      if (!empty(i, "due_date"))
      {
        q << ", due_date";
      }
      if (!empty(i, "priority"))
      {
        q << ", priority";
      }
      q << ") values (" << i->m["application_id"]->v << ", now()";
      if (!empty(i, "due_date"))
      {
        q << ", '" << esc(i->m["due_date"]->v) << "'";
      }
      if (!empty(i, "priority"))
      {
        q << ", '" << esc(i->m["priority"]->v) << "'";
      }
      q << ")";
      if (dbu(q, e))
      {
        data a;
        init(d, a);
        a.p->m["i"]->i("application_id", i->m["application_id"]->v);
        a.p->m["i"]->i("open", "1", 'n');
        if (applicationIssuesByApplicationID(a, e) && !a.p->m["o"]->l.empty())
        {
          b = true;
          d.p->i("o", a.p->m["o"]->l.back());
          o = d.p->m["o"];
        }
        deinit(a);
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
bool Central::applicationIssueClose(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!d.u.empty())
  {
    if (!empty(i, "id"))
    {
      q << "update application_issue set close_date = now() where id = " << i->m["id"]->v;
      if (dbu(q, e))
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
bool Central::applicationIssueCommentAdd(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!d.u.empty())
  {
    if (!empty(i, "issue_id"))
    {
      if (!empty(i, "comments"))
      {
        q << "select id from person where userid = '" << d.u << "'";
        auto g = dbq(q, e);
        if (g != NULL)
        {
          if (!g->empty())
          {
            auto r = g->front();
            q.str("");
            q << "insert issue_comment (issue_id, entry_date, user_id, comments) values (" << i->m["issue_id"]->v << ", now(), " << r["id"] << ", '" << esc(i->m["comments"]->v) << "')";
            if (dbu(q, e))
            {
              b = true;
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
bool Central::applicationIssueCommentEdit(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!d.u.empty())
  {
    if (!empty(i, "id"))
    {
      if (!empty(i, "comments"))
      {
        q << "select id from person where userid = '" << d.u << "'";
        auto g = dbq(q, e);
        if (g != NULL)
        {
          if (!g->empty())
          {
            auto r = g->front();
            q.str("");
            q << "select user_id from issue_comment where id = " << i->m["id"]->v;
            auto h = dbq(q, e);
            if (h != NULL)
            {
              if (!h->empty())
              {
                auto s = h->front();
                if (r["id"] == s["user_id"])
                {
                  q.str("");
                  q << "update issue_comment set comments = '" << esc(i->m["comments"]->v) << "' where id = " << i->m["id"]->v;
                  if (dbu(q, e))
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
bool Central::applicationIssueComments(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationIssueEdit()
bool Central::applicationIssueEdit(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationIssueEmail()
bool Central::applicationIssueEmail(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationIssues()
bool Central::applicationIssues(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationIssuesByApplicationID()
bool Central::applicationIssuesByApplicationID(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationNotify()
bool Central::applicationNotify(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationRemove()
bool Central::applicationRemove(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applications()
bool Central::applications(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationsByServerID()
bool Central::applicationsByServerID(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationsByUserID()
bool Central::applicationsByUserID(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationServer()
bool Central::applicationServer(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationServerAdd()
bool Central::applicationServerAdd(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationServerDetail()
bool Central::applicationServerDetail(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationServerDetailAdd()
bool Central::applicationServerDetailAdd(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationServerDetailEdit()
bool Central::applicationServerDetailEdit(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationServerDetailRemove()
bool Central::applicationServerDetailRemove(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationServerDetails()
bool Central::applicationServerDetails(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationServerRemove()
bool Central::applicationServerRemove(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationUser()
bool Central::applicationUser(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationUserAdd()
bool Central::applicationUserAdd(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationUserEdit()
bool Central::applicationUserEdit(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationUserRemove()
bool Central::applicationUserRemove(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ applicationUsersByApplicationID()
bool Central::applicationUsersByApplicationID(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ callback()
void Central::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  string strPrefix  bool bResult = false;
  string strError;

  threadIncrement();
  strPrefix += "->Central::callback()";
  if (ptJson->m.find("Request") != ptJson->m.end())
  {
    if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
    {
      string strFunction = ptJson->m["Function"]->v;
      if (m_functions.find(ptJson->m["Function"]->v) != m_functions.end())
      {
        string strJwt;
        data tData;
        tData.g = false;
        tData.p = new Json;
        tData.p->m["i"] = new Json;
        tData.p->m["o"] = new Json;
        if (ptJson->m.find("Jwt") != ptJson->m.end() && !ptJson->m["Jwt"]->v.empty())
        {
          strJwt = ptJson->m["Jwt"]->v;
        }
        else if (ptJson->m.find("wsJwt") != ptJson->m.end() && !ptJson->m["wsJwt"]->v.empty())
        {
          strJwt = ptJson->m["wsJwt"]->v;
        }
        if (!strJwt.empty() && !m_strJwtSecret.empty() && !m_strJwtSigner.empty())
        {
          string strBase64 = ptJson->m["wsJwt"]->v, strPayload, strValue;
          Json *ptJwt = new Json;
          m_manip.decryptAes(m_manip.decodeBase64(strBase64, strValue), m_strJwtSecret, strPayload, strError);
          if (strPayload.empty())
          {
            strPayload = strBase64;
          }
          if (jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
          {
            if (!empty(ptJwt, "sl_admin") && ptJwt->m["sl_admin"]->v == "1")
            {
              tData.g = true;
            }
            if (exist(ptJwt, "sl_auth"))
            {
              for (auto &auth : ptJwt->m["sl_auth"]->m)
              {
                d.auth[auth.first] = (auth.second->v == "1");
              }
            }
            if (!empty(ptJwt, "sl_login"))
            {
              d.u = ptJwt->m["sl_login"]->v;
            }
          }
          delete ptJwt;
        }
        if ((this->*m_functions[ptJson->m["Function"]->v])(tData, strError))
        {
          bResult = true;
          if (ptJson->m.find("Response") != ptJson->m.end())
          {
            delete ptJson->m["Response"];
          }
          ptJson->m["Response"] = new Json(tData.p->m["o"]);
        }
        deinit(tData);
      }
      else
      {
        strError = "Please provide a valid Function.";
      }
    }
    else
    {
      strError = "Please provide the Function.";
    }
  }
  else
  {
    strError = "Please provide the Request.";
  }
  ptJson->i("Status", ((bResult)?"okay":"error"));
  if (!e.empty())
  {
    ptJson->i("Error", strError);
  }
  if (bResponse)
  {
    hub(ptJson, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
// {{{ contactType()
bool Central::contactType(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


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
list<map<string, string> > *Central::dbq(const stringstream ssQuery, string &e)
{
  return dbq(ssQuery.str(), e);
}
// }}}
// {{{ dbu()
bool Central::dbu(const string strQuery, string &e)
{
  return dbupdate("central", strQuery, e);
}
bool Central::dbu(const stringstream ssQuery, string &e)
{
  return dbu(ssQuery.str(), e);
}
// }}}
// {{{ deinit()
void Central::deinit(data &d)
{
  delete d.p;
}
// }}}
// {{{ dependentsByApplicationID()
bool Central::dependentsByApplicationID(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ empty()
bool Central::empty(Json *ptJson, const string strField)
{
  return (!exist(ptJson, strField) || ptJson->m[strField]->v.empty());
}
// }}}
// {{{ esc()
string Central::esc(const string strValue)
{
  string strEscaped;

  return m_manip.esc(strValue, strEscaped);
}
// }}}
// {{{ exist()
bool Central::exist(Json *ptJson, const string strField)
{
  return (ptJson->m.find(strField) != ptJson->m.end());
}
// }}}
// {{{ init()
void Central::init(data &i, data &o)
{
  o.auth = i.auth;
  o.g = i.g;
  o.u = i.u;
  o.p = new Json;
  o.p->m["i"] = new Json;
  o.p->m["o"] = new Json;
}
// }}}
// {{{ isApplicationDeveloper()
bool Central::isApplicationDeveloper(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "id"))
  {
    if (!d.u.empty())
    {
      q << "select a.id from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.application_id = " << i->m["id"]->v << " and b.type in ('Primary Developer', 'Backup Developer') and c.userid = '" << d.u << "'";
      auto g = dbq(q, e);
      if (g != NULL)
      {
        if (!g->empty())
        {
          b = true;
        }
        else
        {
          e = "You are not a developer for this application.";
        }
      }
      dbf(g);
    }
    else
    {
      e = "You are not authorized to run this request.";
    }
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ isServerAdmin()
bool Central::isServerAdmin(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "id"))
  {
    if (!d.u.empty())
    {
      q << "select a.id from server_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.server_id = " << i->m["id"]->v << " and b.type in ('Primary Admin', 'Backup Admin') and c.userid = '" << d.u << "'";
      auto g = dbq(q, e);
      if (g != NULL)
      {
        if (!g->empty())
        {
          b = true;
        }
        else
        {
          e = "You are not an admin for this server.";
        }
      }
      dbf(g);
    }
    else
    {
      e = "You are not authorized to run this request.";
    }
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ loginType()
bool Central::loginType(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ loginTypes()
bool Central::loginTypes(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


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
bool Central::menuAccess(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ menuAccesses()
bool Central::menuAccesses(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ notifyPriorities()
bool Central::notifyPriorities(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ notifyPriority()
bool Central::notifyPriority(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ ny()
void Central::ny(Json *ptJson, const string strField)
{
  if (ptJson != NULL)
  {
    if (ptJson->m.find(strField) != ptJson->m.end())
    {
      if (ptJson->m[strField]->v == "1")
      {
        ptJson->m[strField]->i("name", "Yes");
        ptJson->m[strField]->i("value", "1", 1);
      }
      else
      {
        ptJson->m[strField]->i("name", "No");
        ptJson->m[strField]->i("value", "0", 0);
      }
    }
    else
    {
      ptJson->m[strField] = new Json;
      ptJson->m[strField]->i("name", "No");
      ptJson->m[strField]->i("value", "0", 0);
    }
  }
}
// }}}
// {{{ packageType()
bool Central::packageType(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ packageTypes()
bool Central::packageTypes(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ rm()
void Central::rm(Json *ptJson, const string strField)
{
  if (exist(ptJson, strField)
  {
    delete ptJson->m[strField];
    ptJson->m.erase(strField);
  }
}
// }}}
// {{{ server()
bool Central::server(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serverAdd()
bool Central::serverAdd(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serverDetailsByApplicationID()
bool Central::serverDetailsByApplicationID(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serverEdit()
bool Central::serverEdit(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serverNotify()
bool Central::serverNotify(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serverRemove()
bool Central::serverRemove(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ servers()
bool Central::servers(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serversByApplicationID()
bool Central::serversByApplicationID(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serversByUserID()
bool Central::serversByUserID(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serverUser()
bool Central::serverUser(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serverUserAdd()
bool Central::serverUserAdd(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serverUserEdit()
bool Central::serverUserEdit(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serverUserRemove()
bool Central::serverUserRemove(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ serverUsersByServerID()
bool Central::serverUsersByServerID(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ user()
bool Central::user(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ userAdd()
bool Central::userAdd(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ userEdit()
bool Central::userEdit(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ userRemove()
bool Central::userRemove(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
// {{{ users()
bool Central::users(data &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];


  return b;
}
// }}}
}
}
