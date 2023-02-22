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
Central::Central(string strP, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strP, "central", argc, argv, pCallback)
{
  string strE;

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
  m_functions["noyes"] = &Central::noyes;
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
  m_ptCred = new Json;
  if (m_pWarden != NULL)
  {
    m_pWarden->vaultRetrieve({"radial", "radial"}, m_ptCred, strE);
  }
}
// }}}
// {{{ ~Central()
Central::~Central()
{
  delete m_ptCred;
}
// }}}
// {{{ accountType()
bool Central::accountType(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::accountType()";
  if (ptI->m.find("id") != ptI->m.end() && !ptI->m["id"]->v.empty())
  {
    ssQuery << "select id, type, description from account_type where id = " << ptI->m["id"]->v;
    auto get = dbquery("central_r", ssQuery.str(), strE);
    if (get != NULL)
    {
      if (!get->empty())
      {
        Json *ptGet = new Json(get->front());
        bResult = true;
        ptO->merge(ptGet, true, false);
        delete ptGet;
      }
      else
      {
        strError = "No results returned.";
      }
    }
    dbfree(get);
  }
  else
  {
    strE = "Please provide the id.";
  }

  return bResult;
}
// }}}
// {{{ accountTypes()
bool Central::accountTypes(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::accountTypes()";
  ssQuery << "select id, type, description from account_type order by type";
  auto get = dbquery("central_r", ssQuery.str(), strE);
  if (get != NULL)
  {
    bResult = true;
    for (auto &row : *get)
    {
      ptO->pb(row);
    }
  }
  dbfree(get);

  return bResult;
}
// }}}
// {{{ application()
bool Central::application(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  string strValue;
  stringstream ssQuery;

  strP += "->Central::application()";
  if ((ptI->m.find("id") != ptI->m.end() && !ptI->m["id"]->v.empty()) || (ptI->m.find("name") != ptI->m.end() && !ptI->m["name"]->v.empty()))
  {
    ssQuery << "select id, name, date_format(creation_date, '%Y-%m-%d') creation_date, notify_priority_id, website, login_type_id, secure_port, auto_register, account_check, dependable, date_format(retirement_date, '%Y-%m-%d') retirement_date, menu_id, package_type_id, wiki, highlight, description from application where ";
    if (ptI->m.find("id") != ptI->m.end() && !ptI->m["id"]->v.empty())
    {
      ssQuery << "id = " << ptI->m["id"]->v;
    }
    else
    {
      ssQuery << "name = '" << m_manip.escape(ptI->m["name"]->v, strValue) << "'";
    }
    auto get = dbquery("central_r", ssQuery.str(), strE);
    if (get != NULL)
    {
      if (!get->empty())
      {
        Json *ptGet = new Json(get->front());
        bResult = true;
        noyes(ptGet, "account_check");
        noyes(ptGet, "auto_register");
        noyes(ptGet, "dependable");
        if (!ptGet->m["login_type_id"]->v.empty())
        {
          size_t unValue;
          stringstream ssValue(ptGet->m["login_type_id"]->v);
          ssValue >> unValue;
          if (unValue > 0)
          {
            Json *ptSubI = new Json;, *ptSubO = new Json;
            ptSubI->i("id", ptGet->m["login_type_id"]->v);
            if (loginType(strP, ptSubI, ptSubO, strE))
            {
              ptGet->insert("login_type", ptSubO);
            }
            delete ptSubI;
            delete ptSubO;
          }
        }
        if (!ptGet->m["menu_id"]->v.empty())
        {
          size_t unValue;
          stringstream ssValue(ptGet->m["menu_id"]->v);
          ssValue >> unValue;
          if (unValue > 0)
          {
            Json *ptSubI = new Json;, *ptSubO = new Json;
            ptSubI->i("id", ptGet->m["menu_id"]->v);
            if (menuAccess(strP, ptSubI, ptSubO, strE))
            {
              ptGet->insert("menu_access", ptSubO);
            }
            delete ptSubI;
            delete ptSubO;
          }
        }
        if (!ptGet->m["notify_priority_id"]->v.empty())
        {
          size_t unValue;
          stringstream ssValue(ptGet->m["notify_priority_id"]->v);
          ssValue >> unValue;
          if (unValue > 0)
          {
            Json *ptSubI = new Json;, *ptSubO = new Json;
            ptSubI->i("id", ptGet->m["notify_priority_id"]->v);
            if (notifyPriority(strP, ptSubI, ptSubO, strE))
            {
              ptGet->insert("notify_priority", ptSubO);
            }
            delete ptSubI;
            delete ptSubO;
          }
        }
        if (!ptGet->m["package_type_id"]->v.empty())
        {
          size_t unValue;
          stringstream ssValue(ptGet->m["package_type_id"]->v);
          ssValue >> unValue;
          if (unValue > 0)
          {
            Json *ptSubI = new Json;, *ptSubO = new Json;
            ptSubI->i("id", ptGet->m["package_type_id"]->v);
            if (packageType(strP, ptSubI, ptSubO, strE))
            {
              ptGet->insert("package_type", ptSubO);
            }
            delete ptSubI;
            delete ptSubO;
          }
        }
        noyes(ptGet, "secure_port");
        noyes(ptGet, "wiki");
        ptO->merge(ptGet, true, false);
        delete ptGet;
      }
      else
      {
        strError = "No results returned.";
      }
    }
    dbfree(get);
  }
  else
  {
    strE = "Please provide the id.";
  }

  return bResult;
}
// }}}
// {{{ applicationAccount()
bool Central::applicationAccount(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  string strValue;
  stringstream ssQuery;

  strP += "->Central::applicationAccount()";
  if (ptI->m.find("id") != ptI->m.end() && !ptI->m["id"]->v.empty())
  {
    ssQuery << "select id, application_id, user_id, encrypt, aes, password, ";
    if (ptI->m.find("aes") != ptI->m.end() && !ptI->m["aes"]->v.empty() && ptI->m["aes"]->v != "0")
    {
      ssQuery << "aes_decrypt(from_base64(password), sha2('" << m_manip.escape(ptI->m["aes"]->v, strValue) << "', 512)) decrypted_password, ";
    }
    ssQuery << "type_id, description from application_account where id = " << ptI->m["id"]->v;
    auto get = dbquery("central_r", ssQuery.str(), strE);
    if (get != NULL)
    {
      if (!get->empty())
      {
        Json *ptGet = new Json(get->front()), *ptSubI = new Json, *ptSubO = new Json;
        ptSubI->i("id", ptGet->m["application_id"]->v);
        if (isGlobalAdmin() || isApplicationDeveloper(strP, ptSubI, ptSubO, strError))
        {
          bResult = true;
          if (ptGet->m["encrypt"]->v == "1")
          {
            delete ptGet->m["password"];
            ptGet->m.erase("password");
          }
          else if (ptGet->m["aes"]->v == "1")
          {
            if (ptGet->m.find("decrypted_password") != ptGet->m.end() && !ptGet->m["decrypted_password"]->v.empty())
            {
              ptGet->i("password", ptGet->m["decrypted_password"]->v);
            }
            else
            {
              delete ptGet->m["password"];
              ptGet->m.erase("password");
            }
          }
          if (ptGet->m.find("decrypted_password") != ptGet->m.end())
          {
            delete ptGet->m["decrypted_password"];
            ptGet->m.erase("decrypted_password");
          }
          ptO->merge(ptGet, true, false);
        }
        else
        {
          strE = "You are not authorized to perform this action.";
        }
        delete ptGet;
        delete ptSubI;
        delete ptSubO;
      }
      else
      {
        strError = "No results returned.";
      }
    }
    dbfree(get);
  }
  else
  {
    strE = "Please provide the id.";
  }

  return bResult;
}
// }}}
// {{{ applicationAccountAdd()
bool Central::applicationAccountAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  string strValue;
  stringstream ssQuery;

  strP += "->Central::applicationAccountAdd()";
  if (ptI->m.find("application_id") != ptI->m.end() && !ptI->m["application_id"]->v.empty())
  {
    Json *ptSubI = new Json, *ptSubO = new Json;
    ptSubI->i("id", ptI->m["application_id"]->v);
    if (isGlobalAdmin() || isApplicationDeveloper(strP, ptSubI, ptSubO, strError))
    {
      if (ptI->m.find("user_id") != ptI->m.end() && !ptI->m["user_id"]->v.empty())
      {
        if (ptI->m.find("description") != ptI->m.end())
        {
          if (ptI->m.find("encrypt") != ptI->m.end() && ptI->m["encrypt"]->m.find("value") != ptI->m["encrypt"]->m.end() && !ptI->m["encrypt"]->m["value"]->v.empty())
          {
            if (ptI->m.find("password") != ptI->m.end() && !ptI->m["password"]->v.empty())
            {
              if (ptI->m.find("type") != ptI->m.end() && ptI->m["type"]->m.find("id") != ptI->m["type"]->m.end() && !ptI->m["type"]->m["id"]->v.empty())
              {
                ssQuery << "insert into application_account (application_id, user_id, encrypt, aes, `password`, type_id, description) values (" << ptI->m["application_id"]->v << ", '" << ptI->m["user_id"]->v << "', " << ptI->m["encrypt"]->m["value"]->v << ", ";
                if (ptI->m["encrypt"]->m["value"]->v == "1")
                {
                  ssQuery << "0, concat('*',upper(sha1(unhex(sha1('" << m_manip.escape(ptI->m["password"]->v, strValue) << "')))))";
                }
                else if (ptI->m.find("aes") != ptI->m.end() && !ptI->m["aes"]->v.empty() && ptI->m["aes"]->v != "0")
                {
                  ssQuery << "1, to_base64(aes_encrypt('" << m_manip.escape(ptI->m["password"]->v, strValue) << "', sha2('" << m_manip.escape(ptI->m["aes"]->v, strValue) << "', 512)))";
                }
                else
                {
                  ssQuery << "0, '" << m_manip.escape(ptI->m["password"]->v, strValue) << "'";
                }
                ssQuery << ", " << ptI->m["type"]->m["id"]->v << ", '" << m_manip.escape(ptI->m["description"]->v, strValue) << "')";
                if (dbUpdate("central_r", ssQuery.str(), strE))
                {
                  bResult = true;
                }
              }
              else
              {
                strE = "Please provide the type.";
              }
            }
            else
            {
              strE = "Please provide the password.";
            }
          }
          else
          {
            strE = "Please provide the encrypt.";
          }
        }
        else
        {
          strE = "Please provide the description.";
        }
      }
      else
      {
        strE = "Please provide the user_id.";
      }
    }
    else
    {
      strE = "You are not authorized to perform this action.";
    }
    delete ptSubI;
    delete ptSubO;
  }
  else
  {
    strE = "Please provide the id.";
  }

  return bResult;
}
// }}}
// {{{ applicationAccountEdit()
bool Central::applicationAccountEdit(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationAccountEdit()";

  return bResult;
}
// }}}
// {{{ applicationAccountRemove()
bool Central::applicationAccountRemove(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationAccountRemove()";

  return bResult;
}
// }}}
// {{{ applicationAccountsByApplicationID()
bool Central::applicationAccountsByApplicationID(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationAccountsByApplicationID()";

  return bResult;
}
// }}}
// {{{ applicationAdd()
bool Central::applicationAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationAdd()";

  return bResult;
}
// }}}
// {{{ applicationBotLink()
bool Central::applicationBotLink(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationBotLink()";

  return bResult;
}
// }}}
// {{{ applicationBotLinkAdd()
bool Central::applicationBotLinkAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationBotLinkAdd()";

  return bResult;
}
// }}}
// {{{ applicationBotLinkRemove()
bool Central::applicationBotLinkRemove(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationBotLinkRemove()";

  return bResult;
}
// }}}
// {{{ applicationBotLinksByApplicationID()
bool Central::applicationBotLinksByApplicationID(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationBotLinksByApplicationID()";

  return bResult;
}
// }}}
// {{{ applicationDepend()
bool Central::applicationDepend(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationDepend()";

  return bResult;
}
// }}}
// {{{ applicationDependAdd()
bool Central::applicationDependAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationDependAdd()";

  return bResult;
}
// }}}
// {{{ applicationDependRemove()
bool Central::applicationDependRemove(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationDependRemove()";

  return bResult;
}
// }}}
// {{{ applicationEdit()
bool Central::applicationEdit(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationEdit()";

  return bResult;
}
// }}}
// {{{ applicationIssue()
bool Central::applicationIssue(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationIssue()";

  return bResult;
}
// }}}
// {{{ applicationIssueAdd()
bool Central::applicationIssueAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationIssueAdd()";

  return bResult;
}
// }}}
// {{{ applicationIssueClose()
bool Central::applicationIssueClose(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationIssueClose()";

  return bResult;
}
// }}}
// {{{ applicationIssueCommentAdd()
bool Central::applicationIssueCommentAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationIssueCommentAdd()";

  return bResult;
}
// }}}
// {{{ applicationIssueCommentEdit()
bool Central::applicationIssueCommentEdit(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationIssueCommentEdit()";

  return bResult;
}
// }}}
// {{{ applicationIssueComments()
bool Central::applicationIssueComments(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationIssueComments()";

  return bResult;
}
// }}}
// {{{ applicationIssueEdit()
bool Central::applicationIssueEdit(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationIssueEdit()";

  return bResult;
}
// }}}
// {{{ applicationIssueEmail()
bool Central::applicationIssueEmail(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationIssueEmail()";

  return bResult;
}
// }}}
// {{{ applicationIssues()
bool Central::applicationIssues(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationIssues()";

  return bResult;
}
// }}}
// {{{ applicationIssuesByApplicationID()
bool Central::applicationIssuesByApplicationID(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationIssuesByApplicationID()";

  return bResult;
}
// }}}
// {{{ applicationNotify()
bool Central::applicationNotify(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationNotify()";

  return bResult;
}
// }}}
// {{{ applicationRemove()
bool Central::applicationRemove(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationRemove()";

  return bResult;
}
// }}}
// {{{ applications()
bool Central::applications(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applications()";

  return bResult;
}
// }}}
// {{{ applicationsByServerID()
bool Central::applicationsByServerID(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationsByServerID()";

  return bResult;
}
// }}}
// {{{ applicationsByUserID()
bool Central::applicationsByUserID(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationsByUserID()";

  return bResult;
}
// }}}
// {{{ applicationServer()
bool Central::applicationServer(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationServer()";

  return bResult;
}
// }}}
// {{{ applicationServerAdd()
bool Central::applicationServerAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationServerAdd()";

  return bResult;
}
// }}}
// {{{ applicationServerDetail()
bool Central::applicationServerDetail(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationServerDetail()";

  return bResult;
}
// }}}
// {{{ applciationServerDetailAdd()
bool Central::applciationServerDetailAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applciationServerDetailAdd()";

  return bResult;
}
// }}}
// {{{ applicationServerDetailEdit()
bool Central::applicationServerDetailEdit(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationServerDetailEdit()";

  return bResult;
}
// }}}
// {{{ applicationServerDetailRemove()
bool Central::applicationServerDetailRemove(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationServerDetailRemove()";

  return bResult;
}
// }}}
// {{{ applicationServerDetails()
bool Central::applicationServerDetails(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationServerDetails()";

  return bResult;
}
// }}}
// {{{ applicationServerRemove()
bool Central::applicationServerRemove(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationServerRemove()";

  return bResult;
}
// }}}
// {{{ applicationUser()
bool Central::applicationUser(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationUser()";

  return bResult;
}
// }}}
// {{{ applicationUserAdd()
bool Central::applicationUserAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationUserAdd()";

  return bResult;
}
// }}}
// {{{ applicationUserEdit()
bool Central::applicationUserEdit(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationUserEdit()";

  return bResult;
}
// }}}
// {{{ applicationUserRemove()
bool Central::applicationUserRemove(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationUserRemove()";

  return bResult;
}
// }}}
// {{{ applicationUsersByApplicationID()
bool Central::applicationUsersByApplicationID(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::applicationUsersByApplicationID()";

  return bResult;
}
// }}}
// {{{ botLinkRemoteSystem()
bool Central::botLinkRemoteSystem(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::botLinkRemoteSystem()";

  return bResult;
}
// }}}
// {{{ botLinkRemoteSystems()
bool Central::botLinkRemoteSystems(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::botLinkRemoteSystems()";

  return bResult;
}
// }}}
// {{{ contactType()
bool Central::contactType(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::contactType()";

  return bResult;
}
// }}}
// {{{ dependentsByApplicationID()
bool Central::dependentsByApplicationID(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::dependentsByApplicationID()";

  return bResult;
}
// }}}
// {{{ isApplicationDeveloper()
bool Central::isApplicationDeveloper(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::isApplicationDeveloper()";

  return bResult;
}
// }}}
// {{{ isServerAdmin()
bool Central::isServerAdmin(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::isServerAdmin()";

  return bResult;
}
// }}}
// {{{ loginType()
bool Central::loginType(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::loginType()";

  return bResult;
}
// }}}
// {{{ loginTypes()
bool Central::loginTypes(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::loginTypes()";

  return bResult;
}
// }}}
// {{{ menuAccess()
bool Central::menuAccess(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::menuAccess()";

  return bResult;
}
// }}}
// {{{ menuAccesses()
bool Central::menuAccesses(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::menuAccesses()";

  return bResult;
}
// }}}
// {{{ notifyPriorities()
bool Central::notifyPriorities(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::notifyPriorities()";

  return bResult;
}
// }}}
// {{{ notifyPriority()
bool Central::notifyPriority(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::notifyPriority()";

  return bResult;
}
// }}}
// {{{ noyes()
void Central::noyes(Json *ptJson, const string strField)
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
bool Central::packageType(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::packageType()";

  return bResult;
}
// }}}
// {{{ packageTypes()
bool Central::packageTypes(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::packageTypes()";

  return bResult;
}
// }}}
// {{{ server()
bool Central::server(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::server()";

  return bResult;
}
// }}}
// {{{ serverAdd()
bool Central::serverAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serverAdd()";

  return bResult;
}
// }}}
// {{{ serverDetailsByApplicationID()
bool Central::serverDetailsByApplicationID(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serverDetailsByApplicationID()";

  return bResult;
}
// }}}
// {{{ serverEdit()
bool Central::serverEdit(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serverEdit()";

  return bResult;
}
// }}}
// {{{ serverNotify()
bool Central::serverNotify(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serverNotify()";

  return bResult;
}
// }}}
// {{{ serverRemove()
bool Central::serverRemove(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serverRemove()";

  return bResult;
}
// }}}
// {{{ servers()
bool Central::servers(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::servers()";

  return bResult;
}
// }}}
// {{{ serversByApplicationID()
bool Central::serversByApplicationID(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serversByApplicationID()";

  return bResult;
}
// }}}
// {{{ serversByUserID()
bool Central::serversByUserID(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serversByUserID()";

  return bResult;
}
// }}}
// {{{ serverUser()
bool Central::serverUser(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serverUser()";

  return bResult;
}
// }}}
// {{{ serverUserAdd()
bool Central::serverUserAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serverUserAdd()";

  return bResult;
}
// }}}
// {{{ serverUserEdit()
bool Central::serverUserEdit(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serverUserEdit()";

  return bResult;
}
// }}}
// {{{ serverUserRemove()
bool Central::serverUserRemove(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serverUserRemove()";

  return bResult;
}
// }}}
// {{{ serverUsersByServerID()
bool Central::serverUsersByServerID(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::serverUsersByServerID()";

  return bResult;
}
// }}}
// {{{ user()
bool Central::user(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::user()";

  return bResult;
}
// }}}
// {{{ userAdd()
bool Central::userAdd(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::userAdd()";

  return bResult;
}
// }}}
// {{{ userEdit()
bool Central::userEdit(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::userEdit()";

  return bResult;
}
// }}}
// {{{ userRemove()
bool Central::userRemove(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::userRemove()";

  return bResult;
}
// }}}
// {{{ users()
bool Central::users(string strP, Json *ptI, Json *ptO, string &strE)
{
  bool bResult = false;
  stringstream ssQuery;

  strP += "->Central::users()";

  return bResult;
}
// }}}
// {{{ callback()
void Central::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
refix  bool bResult = false;
  string strError;
  stringstream ssQuery;

  threadIncrement();
  strPrefix += "->Central::callback()";
  if (ptJson->m.find("Request") != ptJson->m.end())
  {
    if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
    {
      string strFunction = ptJson->m["Function"]->v;
      if (m_functions.find(ptJson->m["Function"]->v) != m_functions.end())
      {
        if (ptJson->m.find("Response") != ptJson->m.end())
        {
          delete ptJson->m["Response"];
        }
        ptJson->m["Response"] = new Json;
        if ((this->*m_functions[ptJson->m["Function"]->v])(strPrefix, ptJson->m["Request"], ptJson->m["Response"], strError))
        {
          bResult = true;
        }
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
  if (!strError.empty())
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
}
}
