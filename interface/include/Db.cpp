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
  m_functions["dbCentralApplicationServerRemove"] = &Db::dbCentralApplicationServerRemove;
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
  m_functions["dbCentralUsers"] = &Db::dbCentralUsers;
  m_functions["dbCentralUserUpdate"] = &Db::dbCentralUserUpdate;
}
// }}}
// {{{ ~Db()
Db::~Db()
{
}
// }}}
// {{{ autoMode()
void Db::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->Db::autoMode()";
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
      bool bInvalid = true;
      string strID, strQuery;
      if (!exist(ptJson, "Request"))
      {
        ptJson->m["Request"] = new Json;
      }
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
  list<string> k = {"db", "central", "account_type"};
  stringstream qs;

  qs << "select id, type, description from account_type order by type";
  auto g = dbq("central_r", qs, q, k, e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      if (!empty(i, "id") || !empty(i, "type"))
      {
        if ((!empty(i, "id") && r["id"] == i->m["id"]->v) || (!empty(i, "type") && r["type"] == i->m["type"]->v))
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

  if (dep({"application_id"}, i, e))
  {
    bool fa = true, fb = true;
    list<string> ks = {"application_id", "description", "type_id", "user_id"};
    stringstream qs;
    qs << "insert into application_account (" << ia(ks, i, fa);
    if (!empty(i, "password"))
    {
      qs << ((fa)?"":",") << " encrypt, aes, `password`";
      fa = false;
    }
    qs << ") values (" << ib(ks, i, fb);
    if (!empty(i, "password"))
    {
      qs << ((fb)?" ":", ");
      fb = false;
      if (!empty(i, "encrypt") && i->m["encrypt"]->v == "1")
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
    qs << ")";
    b = dbu("central", qs, q, id, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationAccountRemove()
bool Db::dbCentralApplicationAccountRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    stringstream qs;
    qs << "delete from application_account where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationAccounts()
bool Db::dbCentralApplicationAccounts(Json *i, Json *o, string &id, string &q, string &e)
{
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

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralApplicationAccountUpdate()
bool Db::dbCentralApplicationAccountUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    bool f = true;
    stringstream qs;
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
        if (!empty(i, "encrypt") && i->m["encrypt"]->v == "1")
        {
          qs << "1, aes = 0, `password` = concat('!',upper(sha2(unhex(sha2(" << v(i->m["password"]->v) << ", 512)), 512)))";
        }
        else if (!m_strAesSecret.empty())
        {
          qs << "0, aes = 1, `password` = to_base64(aes_encrypt(" << v(i->m["password"]->v) << ", sha2('" << esc(m_strAesSecret) << "', 512)))";
        }
        else
        {
          qs << "0, aes = 0, `password` = " << v(i->m["password"]->v);
        }
      }
      else
      {
        qs << "0, aes = 0, `password` = null";
      }
    }
    qs << u("type_id", i, f) << u("description", i, f) << " where id = " << v(i->m["id"]->v);
    if (!f)
    {
      b = dbu("central", qs, q, e);
    }
    else
    {
      e = "Please provide at least one field to update.";
    }
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationAdd()
bool Db::dbCentralApplicationAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"name"}, i, e))
  {
    bool fa = true, fb = true;
    list<string> ks = {"account_check", "auto_register", "dependable", "description", "highlight", "login_type_id", "menu_id", "name", "notify_priority_id", "package_type_id", "retirement_date", "secure_port", "website", "wiki"};
    stringstream qs;
    i->i("creation_date", "now()");
    qs << "insert into application (" << ia(ks, i, fa) << ") values (" << ib(ks, i, fb) << ")";
    b = dbu("central", qs, q, id, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationDependAdd()
bool Db::dbCentralApplicationDependAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  
  if (dep({"application_id", "dependant_id"}, i, e))
  {
    bool fa = true, fb = true;
    list<string> ks = {"application_id", "dependant_id"};
    stringstream qs;
    qs << "insert into application_dependant (" << ia(ks, i, fa) << ") values (" << ib(ks, i, fb) << ")";
    b = dbu("central", qs, q, id, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationDependRemove()
bool Db::dbCentralApplicationDependRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  
  if (dep({"id"}, i, e))
  { 
    stringstream qs;
    qs << "delete from application_dependant where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationDepends()
bool Db::dbCentralApplicationDepends(Json *i, Json *o, string &id, string &q, string &e)
{
  stringstream qs;
  
  qs << "select id, application_id, dependant_id from application_dependant";
  if (!empty(i, "id"))
  {
    qs << " where id = " << v(i->m["id"]->v);
  }

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralApplicationIssueAdd()
bool Db::dbCentralApplicationIssueAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"application_id"}, i, e))
  {
    bool fa = true, fb = true;
    list<string> ks = {"application_id", "assigned_id", "due_date", "open_date", "priority", "summary"};
    stringstream qs;
    i->i("open_date", "now()");
    qs << "insert into application_issue (" << ia(ks, i, fa) << ") values (" << ib(ks, i, fb) << ")";
    b = dbu("central", qs, q, id, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationIssueCommentAdd()
bool Db::dbCentralApplicationIssueCommentAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  
  if (dep({"issue_id", "comments", "user_id"}, i, e))
  {
    bool fa = true, fb = true;
    list<string> ks = {"comments", "entry_date", "issue_id", "user_id"};
    stringstream qs;
    i->i("entry_date", "now()");
    qs << "insert issue_comment (" << ia(ks, i, fa) << ") values (" << ib(ks, i, fb) << ")";
    b = dbu("central", qs, q, id, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationIssueComments()
bool Db::dbCentralApplicationIssueComments(Json *i, Json *o, string &id, string &q, string &e)
{
  stringstream qs;

  qs << "select a.id, a.comments, date_format(a.entry_date, '%Y-%m-%d %H:%i:%s') entry_date, b.email, b.first_name, b.last_name, a.user_id, b.userid from issue_comment a, person b where a.user_id = b.id";
  if (!empty(i, "issue_id"))
  {
    qs << " and a.issue_id = " << v(i->m["issue_id"]->v);
  }
  qs << " order by entry_date, id";
  if (!empty(i, "limit"))
  {
    qs << " limit " << i->m["limit"]->v;
  }

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralApplicationIssueCommentUpdate()
bool Db::dbCentralApplicationIssueCommentUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  
  if (dep({"comments", "id"}, i, e))
  {
    bool f = true;
    stringstream qs;
    qs << "update issue_comment set" << u("comments", i, f) << " where id = " << i->m["id"]->v;
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationIssues()
bool Db::dbCentralApplicationIssues(Json *i, Json *o, string &id, string &q, string &e)
{
  bool bOpen, bRelease;
  string strCloseDateEnd, strCloseDateStart, strDisplay, strOpenDateEnd, strOpenDateStart;
  stringstream qs;

  bOpen = (!empty(i, "open") && i->m["open"]->v == "1");
  bRelease = (!empty(i, "release") && i->m["release"]->v == "1");
  strCloseDateEnd = ((!empty(i, "close_date_end"))?i->m["close_date_end"]->v:"");
  strCloseDateStart = ((!empty(i, "close_date_start"))?i->m["close_date_start"]->v:"");
  strDisplay = ((!empty(i, "display"))?i->m["display"]->v:"");
  strOpenDateEnd = ((!empty(i, "open_date_end"))?i->m["open_date_end"]->v:"");
  strOpenDateStart = ((!empty(i, "open_date_start"))?i->m["open_date_start"]->v:"");
  qs << "select id, application_id, assigned_id, date_format(close_date, '%Y-%m-%d') close_date, date_format(due_date, '%Y-%m-%d') due_date, hold, date_format(open_date, '%Y-%m-%d') open_date, priority, date_format(release_date, '%Y-%m-%d') release_date, summary from application_issue where 1";
  if (!empty(i, "application_id"))
  {
    qs << " and application_id = " << v(i->m["application_id"]->v);
  }
  if ((bOpen || bRelease) && strDisplay != "all")
  {
    qs << " and close_date is null";
    if (bRelease)
    {
      qs << " and release_date is not null and date_format(release_date, '%Y-%m-%d') >= date_format(now(), '%Y-%m-%d')";
    }
  }
  if (!empty(i, "id"))
  {
    qs << " and id = " << v(i->m["id"]->v);
  }
  if (!strOpenDateStart.empty())
  {
    qs << " and date_format(open_date, '%Y-%m-%d') >= '" << strOpenDateStart << "'";
  }
  if (!strOpenDateEnd.empty())
  {
    qs << " and date_format(open_date, '%Y-%m-%d') < '" << strOpenDateEnd << "'";
  }
  if (!strCloseDateStart.empty())
  {
    qs << " and date_format(close_date, '%Y-%m-%d') >= '" << strCloseDateStart << "'";
  }
  if (!strCloseDateEnd.empty())
  {
    qs << " and date_format(close_date, '%Y-%m-%d') < '" << strCloseDateEnd << "'";
  }
  if (bRelease)
  {
    qs << " order by release_date, priority desc, due_date, open_date, id";
  }
  else
  {
    qs << " order by priority desc, due_date, open_date, id";
  }

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralApplicationIssueUpdate()
bool Db::dbCentralApplicationIssueUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    bool f = true;
    stringstream qs;
    qs << "update application_issue set";
    if (!empty(i, "transfer_id") && i->m["transfer_id"]->v != i->m["application_id"]->v)
    {
      f = false;
      qs << " application_id = " << v(i->m["transfer_id"]->v);
    }
    else if (!empty(i, "application_id"))
    {
      f = false;
      qs << " application_id = " << v(i->m["application_id"]->v);
    }
    qs << u({"assigned_id", "close_date", "due_date", "hold", "priority", "release_date", "summary"}, i, f) << " where id = " << i->m["id"]->v;
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationRemove()
bool Db::dbCentralApplicationRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    stringstream qs;
    qs << "delete from application where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplications()
bool Db::dbCentralApplications(Json *i, Json *o, string &id, string &q, string &e)
{
  stringstream qs;

  if (!empty(i, "server_id"))
  {
    qs << "select a.id, b.id application_id, b.name from application_server a, application b where a.application_id = b.id and a.server_id = " << v(i->m["server_id"]->v);
    if (!empty(i, "retired") && i->m["retired"]->v == "1")
    {
      qs << " and b.retirement_date is null";
    }
    qs << " order by b.name";
  }
  else if (!empty(i, "contact_id"))
  {
    qs << "select a.id, b.id application_id, b.name, c.type from application_contact a, application b, contact_type c where a.application_id = b.id and a.type_id = c.id and a.contact_id = " << v(i->m["contact_id"]->v) << " order by b.name";
  }
  else
  {
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
  }

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralApplicationServerAdd()
bool Db::dbCentralApplicationServerAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"application_id", "server_id"}, i, e))
  {
    stringstream qs;
    qs << "insert into application_server (application_id, server_id) values (" << v(i->m["application_id"]->v) << ", " << v(i->m["server_id"]->v) << ")";
    b = dbu("central", qs, q, id, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationServerDetailAdd()
bool Db::dbCentralApplicationServerDetailAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"application_server_id"}, i, e))
  {
    bool fa = true, fb = true;
    list<string> ks = {"application_server_id", "daemon", "delay", "max_image", "max_processes", "max_resident", "min_image", "min_processes", "min_resident", "owner", "script", "version"};
    stringstream qs;
    qs << "insert into application_server_detail (" << ia(ks, i, fa) << ") values (" << ib(ks, i, fb) << ")";
    b = dbu("central", qs, q, id, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationServerDetailRemove()
bool Db::dbCentralApplicationServerDetailRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    stringstream qs;
    qs << "delete from application_server_detail where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationServerDetails()
bool Db::dbCentralApplicationServerDetails(Json *i, Json *o, string &id, string &q, string &e)
{
  stringstream qs;

  qs << "select id, application_server_id, daemon, delay, max_image, max_processes, max_resident, min_image, min_processes, min_resident, owner, script, version from application_server_detail where 1";
  if (!empty(i, "id"))
  {
    qs << " and id = " << v(i->m["id"]->v);
  }
  if (!empty(i, "application_server_id"))
  {
    qs << " and application_server_id = " << v(i->m["application_server_id"]->v);
  }

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralApplicationServerDetailUpdate()
bool Db::dbCentralApplicationServerDetailUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    bool f = true;
    stringstream qs;
    qs << "update application_server_detail set" << u({"daemon", "delay", "max_image", "max_processes", "max_resident", "min_image", "max_processes", "max_resident", "owner", "script", "version"}, i, f) << " where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationServerRemove()
bool Db::dbCentralApplicationServerRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    stringstream qs;
    qs << "delete from application_server where id = (" << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationServers()
bool Db::dbCentralApplicationServers(Json *i, Json *o, string &id, string &q, string &e)
{
  stringstream qs;

  qs << "select id, application_id, server_id from application_server";
  if (!empty(i, "id"))
  {
    qs << " where id = " << v(i->m["id"]->v);
  }

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralApplicationUpdate()
bool Db::dbCentralApplicationUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id", "name"}, i, e))
  {
    bool f = true;
    stringstream qs;
    qs << "update application set" << u({"account_check", "auto_register", "dependable", "description", "highlight", "login_type_id", "menu_id", "name", "notify_priority_id", "retirement_date", "secure_port", "website", "wiki"}, i, f) << " where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationUserAdd()
bool Db::dbCentralApplicationUserAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"application_id", "contact_id"}, i, e))
  {
    bool fa = true, fb = true;
    list<string> ks = {"admin", "application_id", "contact_id", "description", "locked", "notify", "type_id"};
    stringstream qs;
    qs << "insert into application_contact (" << ia(ks, i, fa) << ") values (" << ib(ks, i, fb) << ")";
    b = dbu("central", qs, q, id, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationUserRemove()
bool Db::dbCentralApplicationUserRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    stringstream qs;
    qs << "delete from application_contact where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralApplicationUsers()
bool Db::dbCentralApplicationUsers(Json *i, Json *o, string &id, string &q, string &e)
{
  stringstream qs;

  qs << "select a.id, a.application_id, a.admin, a.description, c.email, c.first_name, c.last_name, a.locked, a.notify, b.type, a.type_id, c.id user_id, c.userid from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id";
  if (!empty(i, "id"))
  {
    qs << " and a.id = " << v(i->m["id"]->v);
  }
  if (!empty(i, "application_id"))
  {
    qs << " and a.application_id = " << v(i->m["application_id"]->v);
  }
  if ((!empty(i, "Primary Developer") && i->m["Primary Developer"]->v == "1") || (!empty(i, "Backup Developer") && i->m["Backup Developer"]->v == "1") || (!empty(i, "Primary Contact") && i->m["Primary Contact"]->v == "1") || (!empty(i, "Contact") && i->m["Contact"]->v == "1"))
  {
    bool f = true;
    qs << " and b.type in (";
    if (!empty(i, "Primary Developer") && i->m["Primary Developer"]->v == "1")
    {
      qs << ((!f)?", ":"") << "'Primary Developer'";
      f = false;
    }
    if (!empty(i, "Backup Developer") && i->m["Backup Developer"]->v == "1")
    {
      qs << ((!f)?", ":"") << "'Backup Developer'";
      f = false;
    }
    if (!empty(i, "Primary Contact") && i->m["Primary Contact"]->v == "1")
    {
      qs << ((!f)?", ":"") << "'Primary Contact'";
      f = false;
    }
    if (!empty(i, "Contact") && i->m["Contact"]->v == "1")
    {
      qs << ((!f)?", ":"") << "'Contact'";
      f = false;
    }
    qs << ")";
  }
  qs << " order by c.last_name, c.first_name, c.userid";
  if (exist(i, "page"))
  {
    size_t unNumPerPage, unOffset, unPage;
    stringstream ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
    ssNumPerPage >> unNumPerPage;
    ssPage >> unPage;
    unOffset = unPage * unNumPerPage;
    qs << " limit " << unNumPerPage << " offset " << unOffset;
  }

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralApplicationUserUpdate()
bool Db::dbCentralApplicationUserUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    bool f = true;
    stringstream qs;
    qs << "update application_contact set" << u({"admin", "contact_id", "description", "locked", "notifyf", "type_id"}, i, f) << " where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralContactTypes()
bool Db::dbCentralContactTypes(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  list<string> k = {"db", "central", "contact_type"};
  stringstream qs;

  qs << "select id, type from contact_type order by type";
  auto g = dbq("central_r", qs, q, k, e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      if (!empty(i, "id") || !empty(i, "type"))
      {
        if ((!empty(i, "id") && r["id"] == i->m["id"]->v) || (!empty(i, "type") && r["type"] == i->m["type"]->v))
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
// {{{ dbCentralDependents()
bool Db::dbCentralDependents(Json *i, Json *o, string &id, string &q, string &e)
{
  stringstream qs;

  qs << "select a.id, b.id application_id, b.name from application_dependant a, application b where a.dependant_id = b.id";
  if (!empty(i, "dependant_id"))
  {
    qs << " and a.dependant_id = " << v(i->m["dependant_id"]->v);
  }
  else if (!empty(i, "application_id"))
  {
    qs << " and a.application_id = " << v(i->m["application_id"]->v);
  }
  qs << " order by b.name";

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralLoginTypes()
bool Db::dbCentralLoginTypes(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  list<string> k = {"db", "central", "login_type"};
  stringstream qs;

  qs << "select id, type from login_type order by type";
  auto g = dbq("central_r", qs, q, k, e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      if (!empty(i, "id") || !empty(i, "type"))
      {
        if ((!empty(i, "id") && r["id"] == i->m["id"]->v) || (!empty(i, "type") && r["type"] == i->m["type"]->v))
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
// {{{ dbCentralMenuAccesses()
bool Db::dbCentralMenuAccesses(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  list<string> k = {"db", "central", "menu_access"};
  stringstream qs;

  qs << "select id, type from menu_access order by type";
  auto g = dbq("central_r", qs, q, k, e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      if (!empty(i, "id") || !empty(i, "type"))
      {
        if ((!empty(i, "id") && r["id"] == i->m["id"]->v) || (!empty(i, "type") && r["type"] == i->m["type"]->v))
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
// {{{ dbCentralNotifyPriorities()
bool Db::dbCentralNotifyPriorities(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  list<string> k = {"db", "central", "notify_priority"};
  stringstream qs;

  qs << "select id, priority from notify_priority order by priority";
  auto g = dbq("central_r", qs, q, k, e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      if (!empty(i, "id") || !empty(i, "priority"))
      {
        if ((!empty(i, "id") && r["id"] == i->m["id"]->v) || (!empty(i, "priority") && r["priority"] == i->m["priority"]->v))
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
// {{{ dbCentralPackageTypes()
bool Db::dbCentralPackageTypes(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;
  list<string> k = {"db", "central", "package_type"};
  stringstream qs;

  qs << "select id, type from package_type order by type";
  auto g = dbq("central_r", qs, q, k, e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      if (!empty(i, "id") || !empty(i, "type"))
      {
        if ((!empty(i, "id") && r["id"] == i->m["id"]->v) || (!empty(i, "type") && r["type"] == i->m["type"]->v))
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
// {{{ dbCentralServerAdd()
bool Db::dbCentralServerAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"name"}, i, e))
  {
    bool fa = true, fb = true;
    list<string> ks = {"cpu_usage", "description", "disk_size", "main_memory", "name", "processes", "swap_memory"};
    stringstream qs;
    qs << "insert into server (" << ia(ks, i, fa) << ") values (" << ib(ks, i, fb) << ")";
    b = dbu("central", qs, q, id, e);
  }

  return b;
}
// }}}
// {{{ dbCentralServerDetails()
bool Db::dbCentralServerDetails(Json *i, Json *o, string &id, string &q, string &e)
{
  stringstream qs;

  qs << "select a.id application_server_id, b.id server_id, b.name, c.id application_server_detail_id, c.daemon from application_server a, server b, application_server_detail c where a.server_id = b.id and a.id = c.application_server_id";
  if (!empty(i, "application_id"))
  {
    qs << " and a.application_id = " << v(i->m["application_id"]->v);
  }
  qs << " order by b.name, c.daemon";

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralServerRemove()
bool Db::dbCentralServerRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    stringstream qs;
    qs << "delete from server where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralServers()
bool Db::dbCentralServers(Json *i, Json *o, string &id, string &q, string &e)
{
  stringstream qs;

  if (!empty(i, "application_id"))
  {
    qs << "select a.id, b.id server_id, b.name from application_server a, server b where a.server_id = b.id and a.application_id = " << v(i->m["application_id"]->v) << " order by b.name";
  }
  else if (!empty(i, "contact_id"))
  {
    qs << "select distinct a.id, b.id server_id, b.name, c.type from server_contact a, server b, contact_type c where a.server_id = b.id and a.type_id = c.id and a.contact_id = " << v(i->m["contact_id"]->v) << " order by b.name";
  }
  else
  {
    qs << "select id, cpu_usage, description, disk_size, main_memory, name, processes, swap_memory from server where 1";
    if (!empty(i, "id"))
    {
      qs << " and id = " << v(i->m["id"]->v);
    } 
    if (!empty(i, "letter"))
    {
      if (i->m["letter"]->v == "#")
      {
        qs << " and name regexp '^[ -@[-`{-~]'";
      }
      else
      {
        qs << " and upper(name) like '" << i->m["letter"]->v << "%'";
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
  }

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralServerUpdate()
bool Db::dbCentralServerUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id", "name"}, i, e))
  {
    bool f = true;
    stringstream qs;
    qs << "update server set" << u({"cpu_usage", "description", "disk_size", "main_memory", "name", "processes", "swap_memory"}, i, f) << " where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralServerUserAdd()
bool Db::dbCentralServerUserAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"contact_id", "notify", "server_id", "type_id"}, i, e))
  {
    bool fa = true, fb = true;
    list<string> ks = {"contact_id", "notify", "server_id", "type_id"};
    stringstream qs;
    qs << "insert into server_contact (" << ia(ks, i, fa) << ") values (" << ib(ks, i, fb) << ")";
    b = dbu("central", qs, q, id, e);
  }

  return b;
}
// }}}
// {{{ dbCentralServerUserRemove()
bool Db::dbCentralServerUserRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    stringstream qs;
    qs << "delete from server_contact where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralServerUsers()
bool Db::dbCentralServerUsers(Json *i, Json *o, string &id, string &q, string &e)
{
  stringstream qs;

  qs << "select a.id, c.email, c.first-name, c.last_name, a.notify, a.server_id, a.type_id, c.id user_id, c.userid from server_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id";
  if (!empty(i, "id"))
  {
    qs << " and a.id = " << v(i->m["id"]->v);
  }
  if (!empty(i, "server_id"))
  {
    qs << " and a.server_id = " << v(i->m["server_id"]->v);
  }
  if ((!empty(i, "Primary Admin") && i->m["Primary Admin"]->v == "1") || (!empty(i, "Backup Admin") && i->m["Backup Admin"]->v == "1") || (!empty(i, "Primary Contact") && i->m["Primary Contact"]->v == "1") || (!empty(i, "Contact") && i->m["Contact"]->v == "1"))
  {
    bool f = true;
    qs << " and b.type in (";
    if (!empty(i, "Primary Admin") && i->m["Primary Admin"]->v == "1")
    {
      qs << ((!f)?", ":"") << "'Primary Admin'";
      f = false;
    }
    if (!empty(i, "Backup Admin") && i->m["Backup Admin"]->v == "1")
    {
      qs << ((!f)?", ":"") << "'Backup Admin'";
      f = false;
    }
    if (!empty(i, "Primary Contact") && i->m["Primary Contact"]->v == "1")
    {
      qs << ((!f)?", ":"") << "'Primary Contact'";
      f = false;
    }
    if (!empty(i, "Contact") && i->m["Contact"]->v == "1")
    {
      qs << ((!f)?", ":"") << "'Contact'";
      f = false;
    }
    qs << ")";
  }
  qs << " order by c.last_name, c.first_name, c.userid";
  if (exist(i, "page"))
  {
    size_t unNumPerPage, unOffset, unPage;
    stringstream ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
    ssNumPerPage >> unNumPerPage;
    ssPage >> unPage;
    unOffset = unPage * unNumPerPage;
    qs << " limit " << unNumPerPage << " offset " << unOffset;
  }

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralServerUserUpdate()
bool Db::dbCentralServerUserUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"contact_id", "id", "notify", "type_id"}, i, e))
  {
    bool f = true;
    stringstream qs;
    qs << "update server_contact set" << u({"contact_id", "notify", "type_id"}, i, f) << " where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralUserAdd()
bool Db::dbCentralUserAdd(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"userid"}, i, e))
  {
    bool fa = true, fb = true;
    list<string> ks = {"active", "admin", "email", "first_name", "last_name", "locked", "pager", "userid"};
    stringstream qs;
    qs << "insert into person (" << ia(ks, i, fa) << ") values (" << ib(ks, i, fb) << ")";
    b = dbu("central", qs, q, id, e);
  }

  return b;
}
// }}}
// {{{ dbCentralUserRemove()
bool Db::dbCentralUserRemove(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    stringstream qs;
    qs << "delete from person where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

  return b;
}
// }}}
// {{{ dbCentralUsers()
bool Db::dbCentralUsers(Json *i, Json *o, string &id, string &q, string &e)
{
  stringstream qs;

  qs << "select id, active, admin, email, first_name, last_name, locked, pager, userid from person where 1";
  if (!empty(i, "id"))
  {
    qs << " and id = " << v(i->m["id"]->v);
  }
  if (!empty(i, "letter"))
  {
    if (i->m["letter"]->v == "#")
    {
      qs << " and last_name regexp '^[ -@[-`{-~]'";
    }
    else
    {
      qs << " and upper(last_name) like '" << i->m["letter"]->v << "%'";
    }
  }
  if (!empty(i, "userid"))
  {
    qs << " and userid = " << v(i->m["userid"]->v);
  }
  qs << " order by last_name, first_name, userid";
  if (!empty(i, "page"))
  {
    size_t unNumPerPage, unOffset, unPage;
    stringstream ssNumPerPage((!empty(i, "numPerPage"))?i->m["numPerPage"]->v:"10"), ssPage(i->m["page"]->v);
    ssNumPerPage >> unNumPerPage;
    ssPage >> unPage;
    unOffset = unPage * unNumPerPage;
    qs << " limit " << unNumPerPage << " offset " << unOffset;
  }

  return dbq("central_r", qs, q, o, e);
}
// }}}
// {{{ dbCentralUserUpdate()
bool Db::dbCentralUserUpdate(Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (dep({"id"}, i, e))
  {
    bool f = true;
    stringstream qs;
    qs << "update person set" << u({"active", "admin", "email", "first_name", "last_name", "locked", "pager", "userid"}, i, f);
    if (exist(i, "password"))
    {
      qs << ((f)?"":",") << " `password` = ";
      f = false;
      if (!empty(i, "password"))
      {
        qs << "concat('!',upper(sha2(unhex(sha2('" << esc(i->m["password"]->v) << "', 512)), 512)))";
      }
      else
      {
        qs << "null";
      }
    }
    qs << " where id = " << v(i->m["id"]->v);
    b = dbu("central", qs, q, e);
  }

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
bool Db::dbq(const string d, stringstream &qs, string &q, Json *o, string &e)
{
  auto g = dbq(d, qs, q, e);
  bool b = false;

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
bool Db::dbq(const string d, stringstream &qs, string &q, const list<string> k, Json *o, string &e)
{
  auto g = dbq(d, qs, q, k, e);
  bool b = false;

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
list<map<string, string> > *Db::dbq(const string d, stringstream &qs, string &q, string &e)
{
  q = qs.str();

  return dbquery(d, q, e);
}
list<map<string, string> > *Db::dbq(const string d, stringstream &qs, string &q, const list<string> k, string &e)
{
  Json *s = new Json;
  list<map<string, string> > *g = NULL;

  q = qs.str();
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
  else if ((g = dbquery(d, q, e)) != NULL)
  {
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
bool Db::dbu(const string d, stringstream &qs, string &q, string &e)
{
  q = qs.str();

  return dbupdate(d, q, e);
}
bool Db::dbu(const string d, stringstream &qs, string &q, string &id, string &e)
{
  q = qs.str();

  return dbupdate(d, q, id, e);
}
// }}}
// }}}
// {{{ dep()
bool Db::dep(const list<string> fs, Json *i, string &e)
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
// {{{ ia()
string Db::ia(const list<string> ks, Json *i)
{
  bool f = false;

  return ia(ks, i, f);
}
string Db::ia(const list<string> ks, Json *i, bool &f)
{
  string o;
  stringstream os;

  for (auto &k : ks)
  {
    os << ia(k, i, f);
  }
  o = os.str();

  return o;
}
string Db::ia(const string k, Json *i)
{
  bool f = false;

  return ia(k, i, f);
}
string Db::ia(const string k, Json *i, bool &f)
{
  string s;

  if (exist(i, k))
  {
    s = ia(k, f);
  }

  return s;
}
string Db::ia(const string k)
{
  bool f = false;

  return ia(k, f);
}
string Db::ia(const string k, bool &f)
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
  os << " `" << k << "`";

  s = os.str();
  return s;
}
// }}}
// {{{ ib()
string Db::ib(const list<string> ks, Json *i)
{
  bool f = false;

  return ib(ks, i, f);
}
string Db::ib(const list<string> ks, Json *i, bool &f)
{
  string o;
  stringstream os;

  for (auto &k : ks)
  {
    os << ib(k, i, f);
  }
  o = os.str();

  return o;
}
string Db::ib(const string k, Json *i)
{
  bool f = false;

  return ib(k, i, f);
}
string Db::ib(const string k, Json *i, bool &f)
{
  string s;

  if (exist(i, k))
  {
    s = ib(k, i->m[k]->v, f);
  }

  return s;
}
string Db::ib(const string k, const string i)
{
  bool f = false;

  return ib(k, i, f);
}
string Db::ib(const string k, const string i, bool &f)
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
  os << " " << ((i == "now()")?i:v(i));

  s = os.str();
  return s;
}
// }}}
// {{{ schedule()
void Db::schedule(string strPrefix)
{
  string strError;
  stringstream ssMessage;
  time_t CTime[3] = {0, 0, 0};
  Json *ptJson;

  threadIncrement();
  strPrefix += "->Db::schedule()";
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
        if (storageRetrieve({"db", "_time"}, ptJson, strError))
        {
          stringstream ssTime(ptJson->v);
          ssTime >> CTime[2];
          if ((CTime[0] - CTime[2]) > 14400)
          {
            storageRemove({"db"}, strError);
          }
        }
        else if (strError == "Failed to find key.")
        {
          stringstream ssTime;
          ssTime << CTime[0];
          ptJson->i("_time", ssTime.str(), 'n');
          storageAdd({"db"}, ptJson, strError);
        }
        delete ptJson;
      }
    }
    msleep(2000);
  }
  threadDecrement();
}
// }}}
// {{{ setCallbackAddon()
void Db::setCallbackAddon(bool (*pCallback)(const string, Json *, Json *, string &, string &, string &, bool &))
{
  m_pCallbackAddon = pCallback;
}
// }}}
// {{{ u()
string Db::u(const list<string> ks, Json *i)
{
  bool f = false;

  return u(ks, i, f);
}
string Db::u(const list<string> ks, Json *i, bool &f)
{
  string o;
  stringstream os;

  for (auto &k : ks)
  {
    os << u(k, i, f);
  }
  o = os.str();

  return o;
}
string Db::u(const string k, Json *i)
{
  bool f = false;

  return u(k, i, f);
}
string Db::u(const string k, Json *i, bool &f)
{
  string s;

  if (exist(i, k))
  {
    s = u(k, i->m[k]->v, f);
  }

  return s;
}
string Db::u(const string k, const string i)
{
  bool f = false;

  return u(k, i, f);
}
string Db::u(const string k, const string i, bool &f)
{
  string s;
  stringstream os;

  os << ((f)?"":",") << " `" << k << "` = " << ((i == "now()")?i:v(i));
  f = false;

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
