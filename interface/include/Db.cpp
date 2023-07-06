// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Db.cpp
// author     : Ben Kietzman
// begin      : 2023-05-18
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Db"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Db()
Db::Db(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "db", argc, argv, pCallback)
{
  m_functions["dbCentralAccountTypes"] = &Db::dbCentralAccountTypes;
  m_functions["dbCentralApplicationAccountAdd"] = &Db::dbCentralApplicationAccountAdd;
  m_functions["dbCentralApplicationAccountRemove"] = &Db::dbCentralApplicationAccountRemove;
  m_functions["dbCentralApplicationAccountUpdate"] = &Db::dbCentralApplicationAccountUpdate;
  m_functions["dbCentralApplicationAccounts"] = &Db::dbCentralApplicationAccounts;
  m_functions["dbCentralApplicationAdd"] = &Db::dbCentralApplicationAdd;
  m_functions["dbCentralApplicationDependAdd"] = &Db::dbCentralApplicationDependAdd;
  m_functions["dbCentralApplicationDependRemove"] = &Db::dbCentralApplicationDependRemove;
  m_functions["dbCentralApplicationDepends"] = &Db::dbCentralApplicationDepends;
  m_functions["dbCentralApplicationIssueAdd"] = &Db::dbCentralApplicationIssueAdd;
  m_functions["dbCentralApplicationIssueCommentAdd"] = &Db::dbCentralApplicationIssueCommentAdd;
  m_functions["dbCentralApplicationIssueComments"] = &Db::dbCentralApplicationIssueComments;
  m_functions["dbCentralApplicationIssueCommentUpdate"] = &Db::dbCentralApplicationIssueCommentUpdate;
  m_functions["dbCentralApplicationIssues"] = &Db::dbCentralApplicationIssues;
  m_functions["dbCentralApplicationIssueUpdate"] = &Db::dbCentralApplicationIssueUpdate;
  m_functions["dbCentralApplicationRemove"] = &Db::dbCentralApplicationRemove;
  m_functions["dbCentralApplications"] = &Db::dbCentralApplications;
  m_functions["dbCentralApplicationServerAdd"] = &Db::dbCentralApplicationServerAdd;
  m_functions["dbCentralApplicationServerDetailAdd"] = &Db::dbCentralApplicationServerDetailAdd;
  m_functions["dbCentralApplicationServerDetailRemove"] = &Db::dbCentralApplicationServerDetailRemove;
  m_functions["dbCentralApplicationServerDetails"] = &Db::dbCentralApplicationServerDetails;
  m_functions["dbCentralApplicationServerDetailUpdate"] = &Db::dbCentralApplicationServerDetailUpdate;
  m_functions["dbCentralApplicationServers"] = &Db::dbCentralApplicationServers;
  m_functions["dbCentralApplicationUpdate"] = &Db::dbCentralApplicationUpdate;
  m_functions["dbCentralApplicationUserAdd"] = &Db::dbCentralApplicationUserAdd;
  m_functions["dbCentralApplicationUserRemove"] = &Db::dbCentralApplicationUserRemove;
  m_functions["dbCentralApplicationUsers"] = &Db::dbCentralApplicationUsers;
  m_functions["dbCentralApplicationUserUpdate"] = &Db::dbCentralApplicationUserUpdate;
  m_functions["dbCentralContactTypes"] = &Db::dbCentralContactTypes;
  m_functions["dbCentralDependents"] = &Db::dbCentralDependents;
  m_functions["dbCentralLoginTypes"] = &Db::dbCentralLoginTypes;
  m_functions["dbCentralMenuAccesses"] = &Db::dbCentralMenuAccesses;
  m_functions["dbCentralNotifyPriorities"] = &Db::dbCentralNotifyPriorities;
  m_functions["dbCentralPackageTypes"] = &Db::dbCentralPackageTypes;
  m_functions["dbCentralServerAdd"] = &Db::dbCentralServerAdd;
  m_functions["dbCentralServerDetails"] = &Db::dbCentralServerDetails;
  m_functions["dbCentralServerRemove"] = &Db::dbCentralServerRemove;
  m_functions["dbCentralServers"] = &Db::dbCentralServers;
  m_functions["dbCentralServerUpdate"] = &Db::dbCentralServerUpdate;
  m_functions["dbCentralServerUserAdd"] = &Db::dbCentralServerUserAdd;
  m_functions["dbCentralServerUserRemove"] = &Db::dbCentralServerUserRemove;
  m_functions["dbCentralServerUsers"] = &Db::dbCentralServerUsers;
  m_functions["dbCentralServerUserUpdate"] = &Db::dbCentralServerUserUpdate;
  m_functions["dbCentralUserAdd"] = &Db::dbCentralUserAdd;
  m_functions["dbCentralUserRemove"] = &Db::dbCentralUserRemove;
  m_functions["dbCentralUserUsers"] = &Db::dbCentralUsers;
  m_functions["dbCentralUserUpdate"] = &Db::dbCentralUserUpdate;
}
// }}}
// {{{ ~Db()
Db::~Db()
{
}
// }}}
// {{{ callback()
void Db::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Db::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    if (ptJson->m["Function"]->v.size() > 2 && ptJson->m["Function"]->v.substr(0, 2) == "db")
    {
      if (exist(ptJson, "Request"))
      {
        bool bInvalid = false;
        string strID, strQuery;
        if (exist(ptJson, "Response"))
        {
          delete ptJson->m["Response"];
        }
        ptJson->m["Response"] = new Json;
        if (m_pCallbackAddon != NULL && m_pCallbackAddon(ptJson->m["Function"]->v, ptJson->m["Request"], ptJson->m["Response"], strID, strQuery, strError, bInvalid))
        {
          bResult = true;
        }
        else if (bInvalid)
        {
          if (m_functions.find(ptJson->m["Function"]->v) != m_functions.end())
          {
            if ((this->*m_functions[ptJson->m["Function"]->v])(ptJson->m["Request"], ptJson->m["Response"], strID, strQuery, strError))
            {
              bResult = true;
              if (!strID.empty())
              {
                ptJson->i("ID", strID, 'n');
              }
              if (!strQuery.empty())
              {
                ptJson->i("Query", strQuery);
              }
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
        strError = "Please provide the Request.";
      }
    }
    else
    {
      strError = "Please provide a db* style Function.";
    }
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
// {{{ db
// {{{ dbCentral
// {{{ dbCentralAccountTypes()
bool Db::dbCentralAccountTypes(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  list<string> k = {"database", "central", "account_type"};
  stringstream qs;

  qs << "select id, type, description from account_type order by type";
  q = qs.str();
  auto g = dbq("central_r", q, k, e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      if (!empty(i, "id"))
      {
        if (r["id"] == i->m["id"]->v)
        {
          o->pb(r);
        }
      }
      else
      {
        o->pb(r);
      }
    }
  }
  dbf(g);

  return b;
}
// }}}
// {{{ dbCentralApplicationAccountAdd()
bool Db::dbCentralApplicationAccountAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  stringstream qs;

  if (!empty(i, "application_id"))
  {
    qs << "insert into application_account (application_id";
    if (!empty(i, "description"))
    {
      qs << ", description";
    }
    if (!empty(i, "password"))
    {
      qs << ", encrypt";
      qs << ", aes";
      qs << ", `password`";
    }
    if (exist(i, "type") && !empty(i->m["type"], "id"))
    {
      qs << ", type_id";
    }
    if (!empty(i, "user_id"))
    {
      qs << ", user_id";
    }
    qs << ") values (" << i->m["application_id"]->v;
    if (!empty(i, "description"))
    {
      qs << ", " << v(i->m["description"]->v);
    }
    if (!empty(i, "password"))
    {
      qs << ", ";
      if (exist(i, "encrypt") && !empty(i->m["encrypt"], "value") && i->m["encrypt"]->m["value"]->v == "1")
      {
        qs << "1, 0, concat('!',upper(sha2(unhex(sha2(" << v(i->m["password"]->v) << ", 512)), 512)))";
      }
      else if (!m_strAesSecret.empty())
      {
        qs << "0, 1, to_base64(aes_encrypt('" << esc(i->m["password"]->v) << "', sha2(" << v(m_strAesSecret) << ", 512)))";
      }
      else
      {
        qs << "0, 0, " << v(i->m["password"]->v);
      }
    }
    if (exist(i, "type") && !empty(i->m["type"], "id"))
    {
      qs << ", " << v(i->m["type"]->m["id"]->v);
    }
    if (!empty(i, "user_id"))
    {
      qs << ", " << v(i->m["user_id"]->v);
    }
    qs << ")";
    q = qs.str();
    if (dbu("central", q, id, e))
    {
      b = true;
    }
  }
  else
  {
    e = "Please provide the application_id.";
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationAccountRemove()
bool Db::dbCentralApplicationAccountRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  stringstream qs;

  if (!empty(i, "id"))
  {
    qs << "delete from application_account where id = " << v(i->m["id"]->v);
    q = qs.str();
    if (dbu("central", q, e))
    {
      b = true;
    }
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationAccounts()
bool Db::dbCentralApplicationAccounts(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  stringstream qs;

  qs << "select id, application_id, user_id, encrypt, aes, password, ";
  if (!m_strAesSecret.empty())
  {
    qs << "aes_decrypt(from_base64(password), sha2(" << v(m_strAesSecret) << ", 512)) decrypted_password, ";
  }
  qs << "type_id, description from application_account where 1";
  if (!empty(i, "application_id"))
  {
    qs << " and application_id = " << v(i->m["application_id"]->v);
  }
  if (!empty(i, "id"))
  {
    qs << " and id = " << v(i->m["id"]->v);
  }
  qs << " order by user_id";
  if (exist(i, "page"))
  {
    size_t unNumPerPage, unOffset, unPage;
    stringstream ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
    ssNumPerPage >> unNumPerPage;
    ssPage >> unPage;
    unOffset = unPage * unNumPerPage;
    qs << " limit " << unNumPerPage << " offset " << unOffset;
  }
  q = qs.str();
  auto g = dbq("central_r", q, e);
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
// {{{ dbCentralApplicationAccountUpdate()
bool Db::dbCentralApplicationAccountUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  stringstream qs;

  if (!empty(i, "id"))
  {
    bool f = true;
    qs << "update application_account set" << u("user_id", i, f);
    if (exist(i, "password"))
    {
      if (f)
      {
        f = false;
      }
      else
      {
        qs << ",";
      }
      qs << " encrypt = ";
      if (!empty(i, "password"))
      {
        if (exist(i, "encrypt") && !empty(i->m["encrypt"], "value") && i->m["encrypt"]->m["value"]->v == "1")
        {
          qs << "1, aes = 0, `password` = concat('!',upper(sha2(unhex(sha2('" << esc(i->m["password"]->v) << "', 512)), 512)))";
        }
        else if (!m_strAesSecret.empty())
        {
          qs << "0, aes = 1, `password` = to_base64(aes_encrypt('" << esc(i->m["password"]->v) << "', sha2('" << esc(m_strAesSecret) << "', 512)))";
        }
        else
        {
          qs << "0, aes = 0, `password` = '" << esc(i->m["password"]->v) << "'";
        }
      }
      else
      {
        qs << "0, aes = 0, `password` = null";
      }
    }
    qs << u("type", i, f) << u("description", i, f) << " where id = " << v(i->m["id"]->v);
    q = qs.str();
    if (!f)
    {
      if (dbu("central", q, e))
      {
        b = true;
      }
    }
    else
    {
      e = "Please provide at least one field to update.";
    }
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationAdd()
bool Db::dbCentralApplicationAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  stringstream qs;

  if (!empty(i, "name"))
  {
    qs << "insert into application (name, creation_date) values (" << v(i->m["name"]->v) << ", now())";
    q = qs.str();
    if (dbu("central", q, id, e))
    {
      b = true;
    }
  }
  else
  {
    e = "Please provide the name.";
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationDependAdd()
bool Db::dbCentralApplicationDependAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  stringstream qs;
  
  if (!empty(i, "application_id"))
  {
    if (!empty(i, "dependant_id"))
    {
      qs << "insert into application_dependant (application_id, dependant_id) values (" << v(i->m["application_id"]->v) << ", " << v(i->m["dependant_id"]->v) << ")";
      q = qs.str();
      if (dbu("central", q, id, e))
      {
        b = true;
      } 
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
// {{{ dbCentralApplicationDependRemove()
bool Db::dbCentralApplicationDependRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  stringstream qs;
  
  if (!empty(i, "id"))
  { 
    qs << "delete from application_dependant where id = " << v(i->m["id"]->v);
    q = qs.str();
    if (dbu("central", q, e))
    {
      b = true;
    } 
  }
  else
  {
    e = "Please provide the id.";
  }
  return b;
}
// }}}
// {{{ dbCentralApplicationDepends()
bool Db::dbCentralApplicationDepends(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  stringstream qs;
  
  qs << "select id, application_id, dependant_id from application_dependant";
  if (!empty(i, "id"))
  {
    qs << " where id = " << v(i->m["id"]->v);
  }
  q = qs.str();
  auto g = dbq("central_r", q, e);
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
// {{{ dbCentralApplicationIssueAdd()
bool Db::dbCentralApplicationIssueAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationIssueCommentAdd()
bool Db::dbCentralApplicationIssueCommentAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationIssueComments()
bool Db::dbCentralApplicationIssueComments(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationIssueCommentUpdate()
bool Db::dbCentralApplicationIssueCommentUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationIssues()
bool Db::dbCentralApplicationIssues(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationIssueUpdate()
bool Db::dbCentralApplicationIssueUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationRemove()
bool Db::dbCentralApplicationRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplications()
bool Db::dbCentralApplications(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  stringstream qs;

  qs << "select id, account_check, auto_register, date_format(creation_date, '%Y-%m-%d') creation_date, dependable, description, highlight, login_type_id, menu_id, name, notify_priority_id, package_type_id, date_format(retirement_date, '%Y-%m-%d %H:%i:%s') retirement_date, secure_port, website, wiki from application where 1";
  if (!empty(i, "dependable") && i->m["dependable"]->v == "1")
  {
    qs << " and dependable = 1";
  }
  if (!empty(i, "id"))
  {
    qs << " and id = " << v(i->m["id"]->v);
  }
  if (!empty(i, "letter"))
  {
    qs << " and";
    if (i->m["letter"]->v == "#")
    {
      qs << " name regexp '^[ -@[-`{-~]'";
    }
    else
    {
      qs << " upper(name) like '" << i->m["letter"]->v << "%'";
    }
  }
  if (!empty(i, "name"))
  {
    qs << " and name = " << v(i->m["name"]->v);
  }
  qs << " order by name";
  if (!empty(i, "page"))
  {
    size_t unNumPerPage, unOffset, unPage;
    stringstream ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
    ssNumPerPage >> unNumPerPage;
    ssPage >> unPage;
    unOffset = unPage * unNumPerPage;
    qs << " limit " << unNumPerPage << " offset " << unOffset;
  }
  q = qs.str();
  auto g = dbq("central_r", q, e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      if (!empty(i, "id") || !empty(i, "name"))
      {
        if ((!empty(i, "id") && r["id"] == i->m["id"]->v) || (!empty(i, "name") && r["name"] == i->m["name"]->v))
        {
          o->pb(r);
        }
      }
      else
      {
        o->pb(r);
      }
    }
  }
  dbf(g);

  return b;
}
// }}}
// {{{ dbCentralApplicationServerAdd()
bool Db::dbCentralApplicationServerAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationServerDetailAdd()
bool Db::dbCentralApplicationServerDetailAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationServerDetailRemove()
bool Db::dbCentralApplicationServerDetailRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationServerDetails()
bool Db::dbCentralApplicationServerDetails(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationServerDetailUpdate()
bool Db::dbCentralApplicationServerDetailUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationServers()
bool Db::dbCentralApplicationServers(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationUpdate()
bool Db::dbCentralApplicationUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  stringstream qs;

  if (!empty(i, "id"))
  {
    if (!empty(i, "name"))
    {
      qs << "update application set" << u("name", i->m["name"]->v) << u("account_check", i) << u("auto_register", i) << u("dependable", i) << u("description", i) << u("highlight", i) << u("login_type_id", i) << u("menu_id", i) << u("notify_priority_id", i) << u("retirement_date", i) << u("secure_port", i) << u("website", i) << u("wiki", i) << " where id = " << i->m["id"]->v;
      q = qs.str();
      if (dbu("central", q, e))
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
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationUserAdd()
bool Db::dbCentralApplicationUserAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationUserRemove()
bool Db::dbCentralApplicationUserRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationUsers()
bool Db::dbCentralApplicationUsers(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralApplicationUserUpdate()
bool Db::dbCentralApplicationUserUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralContactTypes()
bool Db::dbCentralContactTypes(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralDependents()
bool Db::dbCentralDependents(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralLoginTypes()
bool Db::dbCentralLoginTypes(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralMenuAccesses()
bool Db::dbCentralMenuAccesses(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralNotifyPriorities()
bool Db::dbCentralNotifyPriorities(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralPackageTypes()
bool Db::dbCentralPackageTypes(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralServerAdd()
bool Db::dbCentralServerAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralServerDetails()
bool Db::dbCentralServerDetails(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralServerRemove()
bool Db::dbCentralServerRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralServers()
bool Db::dbCentralServers(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralServerUpdate()
bool Db::dbCentralServerUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralServerUserAdd()
bool Db::dbCentralServerUserAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralServerUserRemove()
bool Db::dbCentralServerUserRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralServerUsers()
bool Db::dbCentralServerUsers(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralServerUserUpdate()
bool Db::dbCentralServerUserUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralUserAdd()
bool Db::dbCentralUserAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralUserRemove()
bool Db::dbCentralUserRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralUsers()
bool Db::dbCentralUsers(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// {{{ dbCentralUserUpdate()
bool Db::dbCentralUserUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  return b;
}
// }}}
// }}}
// {{{ dbf()
void Db::dbf(list<map<string, string> > *g)
{
  dbfree(g);
}
// }}}
// {{{ dbq()
list<map<string, string> > *Db::dbq(const string d, const string q, string &e)
{
  return dbquery(d, q, e);
}
list<map<string, string> > *Db::dbq(const string d, const string q, const list<string> k, string &e)
{
  Json *s = new Json;
  list<map<string, string> > *g= NULL;

  if (storageRetrieve(k, s, e))
  {
    g = new list<map<string, string> >;
    for (auto &i : s->l)
    {
      map<string, string> r;
      i->flatten(r, true, false);
      g->push_back(r);
    }
  }
  else
  {
    g = dbquery(d, q, e);
    for (auto &r : *g)
    {
      s->pb(r);
    }
    storageAdd(k, s, e);
  }
  delete s;

  return g;
}
// }}}
// {{{ dbu()
bool Db::dbu(const string d, const string q, string &e)
{
  return dbupdate(d, q, e);
}
bool Db::dbu(const string d, const string q, string &id, string &e)
{
  return dbupdate(d, q, id, e);
}
// }}}
// }}}
// {{{ setCallbackAddon()
void Db::setCallbackAddon(bool (*pCallback)(const string, Json *, Json *, string &, string &, string &, bool &))
{
  m_pCallbackAddon = pCallback;
}
// }}}
// {{{ u()
string Db::u(const string k, Json *i)
{
  return u(k, k, i);
}
string Db::u(const string k, Json *i, bool &f)
{
  return u(k, k, i, f);
}
string Db::u(const string n, const string k, Json *i)
{
  bool f = false;

  return u(n, k, i, f);
}
string Db::u(const string n, const string k, Json *i, bool &f)
{
  string s;

  if (exist(i, k))
  {
    s = u(n, i->m[k]->v, f);
  }

  return s;
}
string Db::u(const string n, const string i)
{
  bool f = false;

  return u(n, i, f);
}
string Db::u(const string n, const string i, bool &f)
{
  string s;
  stringstream os;

  if (f)
  {
    f = false;
  }
  else
  {
    os << ",";
  }
  os << " `" << n << "` = " << v(i);

  s = os.str();
  return s;
}
// }}}
// {{{ v()
string Db::v(const string i)
{
  string o;

  if (!i.empty())
  {
    stringstream os;
    os << "'" << esc(i) << "'";
    o = os.str();
  }
  else
  {
    o = "null";
  }

  return o;
}
// }}}
}
}
