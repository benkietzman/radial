// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Secure.cpp
// author     : Ben Kietzman
// begin      : 2022-06-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Secure"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Secure()
Secure::Secure(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "secure", argc, argv, pCallback)
{
  string strError;
  Json *ptAes = new Json, *ptJwt = new Json;

  m_pLoginCallback = NULL;
  m_pLoginTitleCallback = NULL;
  m_pLogoutCallback = NULL;
  m_pProcessJwtCallback = NULL;
  m_pProcessPostAuthzCallback = NULL;
  m_pProcessPreAuthzCallback = NULL;
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"aes"}, ptAes, strError))
  { 
    if (!empty(ptAes, "Secret"))
    {
      m_strAesSecret = ptAes->m["Secret"]->v;
    }
  }
  delete ptAes; 
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"jwt"}, ptJwt, strError))
  {
    if (!empty(ptJwt, "Secret"))
    {
      m_strJwtSecret = ptJwt->m["Secret"]->v;
    }
    if (!empty(ptJwt, "Signer"))
    {
      m_strJwtSigner = ptJwt->m["Signer"]->v;
    }
  }
  delete ptJwt;
}
// }}}
// {{{ ~Secure()
Secure::~Secure()
{
}
// }}}
// {{{ callback()
void Secure::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError, strValue;
  stringstream ssMessage;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Secure::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    // {{{ auth
    if (ptJson->m["Function"]->v == "auth")
    {
      if (!empty(ptJson, "wsJwt"))
      {
        string strBase64 = ptJson->m["wsJwt"]->v, strPayload;
        Json *ptJwt = new Json;
        m_manip.decryptAes(m_manip.decodeBase64(strBase64, strValue), m_strJwtSecret, strPayload, strError);
        if (strPayload.empty())
        {
          strPayload = strBase64;
        }
        if (jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
        {
          bResult = true;
          ptJson->m["Response"] = new Json;
          if (exist(ptJwt, "sl_admin"))
          {
            ptJson->m["Response"]->m["admin"] = new Json(ptJwt->m["sl_admin"]);
          }
          if (exist(ptJwt, "sl_auth"))
          {
            ptJson->m["Response"]->m["apps"] = new Json(ptJwt->m["sl_auth"]);
          }
          if (exist(ptJwt, "sl_email"))
          {
            ptJson->m["Response"]->m["email"] = new Json(ptJwt->m["sl_email"]);
          }
          if (exist(ptJwt, "sl_first_name"))
          {
            ptJson->m["Response"]->m["first_name"] = new Json(ptJwt->m["sl_first_name"]);
          }
          if (exist(ptJwt, "sl_last_name"))
          {
            ptJson->m["Response"]->m["last_name"] = new Json(ptJwt->m["sl_last_name"]);
          }
          if (exist(ptJwt, "sl_login"))
          {
            ptJson->m["Response"]->m["userid"] = new Json(ptJwt->m["sl_login"]);
          }
        }
        delete ptJwt;
      }
      else
      {
        strError = "Please provide the wsJwt.";
      }
    }
    // }}}
    // {{{ getSecurityModule
    else if (ptJson->m["Function"]->v == "getSecurityModule")
    {
      if (!empty(ptJson, "reqApp"))
      {
        stringstream ssQuery;
        ssQuery << "select b.type from application a, login_type b where a.login_type_id = b.id and a.name = '" << m_manip.escape(ptJson->m["reqApp"]->v, strValue) << "'";
        auto getLoginType = dbquery("central_r", ssQuery.str(), strError);
        if (getLoginType != NULL)
        {
          if (!getLoginType->empty())
          {
            string strModule = getLoginType->front()["type"];
            bResult = true;
            if (!strModule.empty() && strModule[0] >= 'A' && strModule[0] <= 'Z')
            {
              strModule[0] = tolower(strModule[0]);
            }
            ptJson->m["Response"] = new Json;
            ptJson->m["Response"]->i("Module", strModule);
            ptJson->m["Response"]->i("Type", getLoginType->front()["type"]);
          }
          else
          {
            strError = "Security module not configured for this application.";
          }
        }
        dbfree(getLoginType);
      }
      else
      {
        strError = "Please provide the reqApp.";
      }
    }
    // }}}
    // {{{ login
    else if (ptJson->m["Function"]->v == "login")
    {
      if (exist(ptJson, "Request"))
      {
        if (!empty(ptJson->m["Request"], "Type"))
        {
          // {{{ password
          if (ptJson->m["Request"]->m["Type"]->v == "password")
          {
            bResult = true;
          } 
          // }}}
          // {{{ windows
          else if (ptJson->m["Request"]->m["Type"]->v == "windows")
          {
            bResult = true;
          } 
          // }}}
          // {{{ callback
          else if (m_pLoginCallback != NULL)
          {
            bResult = m_pLoginCallback(strPrefix, ptJson, strError);
          }
          // }}}
          // {{{ invalid
          else
          {
            strError = "Please provide a valid Type within the Request:  password, windows.";
          }
          // }}}
        }
        else
        {
          strError = "Please provide the Type within the Request.";
        }
      }
      else
      {
        strError = "Please provide the Request.";
      }
    }
    // }}}
    // {{{ logout
    else if (ptJson->m["Function"]->v == "logout")
    {
      if (exist(ptJson, "Request"))
      {
        if (!empty(ptJson->m["Request"], "Type"))
        {
          // {{{ password
          if (ptJson->m["Request"]->m["Type"]->v == "password")
          {
            if (!empty(ptJson->m["Request"], "Return"))
            {
              bResult = true;
              ptJson->m["Response"] = new Json;
              ptJson->m["Response"]->i("Redirect", ptJson->m["Request"]->m["Return"]->v);
            }
            else
            {
              strError = "Please provide the Return.";
            }
          }
          // }}}
          // {{{ windows
          else if (ptJson->m["Request"]->m["Type"]->v == "windows")
          {
            if (!empty(ptJson->m["Request"], "Return"))
            {
              bResult = true;
              ptJson->m["Response"] = new Json;
              ptJson->m["Response"]->i("Redirect", ptJson->m["Request"]->m["Return"]->v);
            }
            else
            {
              strError = "Please provide the Return.";
            }
          }
          // }}}
          // {{{ callback
          else if (m_pLogoutCallback != NULL)
          {
            bResult = m_pLogoutCallback(strPrefix, ptJson, strError);
          }
          // }}}
          // {{{ invalid
          else
          {
            strError = "Please provide a valid Type within the Request:  password, windows.";
          }
          // }}}
        }
        else
        {
          strError = "Please provide the Type within the Request.";
        }
      }
      else
      {
        strError = "Please provide the Request.";
      }
    }
    // }}}
    // {{{ process
    else if (ptJson->m["Function"]->v == "process")
    {
      bResult = true;
      ptJson->m["Response"] = new Json;
      ptJson->m["Response"]->m["auth"] = new Json;
      ptJson->m["Response"]->m["auth"]->i("login_title", "Login");
      if (exist(ptJson, "Request"))
      {
        if (exist(ptJson->m["Request"], "Type"))
        {
          if (ptJson->m["Request"]->m["Type"]->v == "password")
          {
            ptJson->m["Response"]->m["auth"]->i("login_title", "Login");
          }
          else if (ptJson->m["Request"]->m["Type"]->v == "windows")
          {
            ptJson->m["Response"]->m["auth"]->i("login_title", "Windows Login");
          }
          else if (m_pLoginTitleCallback != NULL)
          {
            ptJson->m["Response"]->m["auth"]->i("login_title", m_pLoginTitleCallback(ptJson->m["Request"]->m["Type"]->v));
          }
        }
        if (ptJson->m["Request"]->m.size() > 1)
        {
          Json *ptData = new Json(ptJson->m["Request"]);
          if (!empty(ptJson->m["Request"], "password"))
          {
            delete ptJson->m["Request"]->m["password"];
            ptJson->m["Request"]->m.erase("password");
          }
          if (!exist(ptData, "password") && !exist(ptData, "Password"))
          {
            ptData->i("Password", "");
          }
          if (m_pProcessPreAuthzCallback != NULL)
          {
            m_pProcessPreAuthzCallback(strPrefix, ptJson, ptData);
          }
          if (m_pWarden->authz(ptData, strError))
          {
            if (m_pProcessPostAuthzCallback != NULL)
            {
              m_pProcessPostAuthzCallback(strPrefix, ptJson, ptData);
            }
            if (exist(ptData, "central"))
            {
              map<string, string> getPersonRow;
              string strPayload, strValue;
              stringstream ssQuery, ssTime;
              time_t CTime;
              Json *ptJwt = new Json;
              ptData->m["central"]->flatten(getPersonRow, true, false);
              ptJwt->i("sl_email", getPersonRow["email"]);
              ptJson->m["Response"]->m["auth"]->i("email", getPersonRow["email"]);
              ptJwt->i("sl_first_name", getPersonRow["first_name"]);
              ptJson->m["Response"]->m["auth"]->i("first_name", getPersonRow["first_name"]);
              ptJwt->i("sl_last_name", getPersonRow["last_name"]);
              ptJson->m["Response"]->m["auth"]->i("last_name", getPersonRow["last_name"]);
              ptJwt->i("sl_login", getPersonRow["userid"]);
              ptJson->m["Response"]->m["auth"]->i("userid", getPersonRow["userid"]);
              time(&CTime);
              CTime += 1209600;
              ssTime << CTime;
              ptJwt->i("exp", ssTime.str(), 'n');
              ptJwt->i("sl_admin", getPersonRow["admin"], ((getPersonRow["admin"] == "1")?'1':'0'));
              ptJson->m["Response"]->m["auth"]->i("admin", getPersonRow["admin"], ((getPersonRow["admin"] == "1")?'1':'0'));
              ssQuery << "select a.name, b.aes, b.user_id, b.password";
              if (!m_strAesSecret.empty())
              {
                ssQuery << ", aes_decrypt(from_base64(b.password), sha2('" << m_manip.escape(m_strAesSecret, strValue) << "', 512)) decrypted_password";
              }
              ssQuery << " from application a, application_account b, account_type c where a.id = b.application_id and b.type_id = c.id and c.type = 'Radial - WebSocket'";
              auto getApplicationAccount = dbquery("central_r", ssQuery.str(), strError);
              if (getApplicationAccount != NULL)
              {
                ptJwt->m["RadialCredentials"] = new Json;
                for (auto &getApplicationAccountRow : *getApplicationAccount)
                {
                  if (!exist(ptJwt->m["RadialCredentials"], getApplicationAccountRow["name"]))
                  {
                    ptJwt->m["RadialCredentials"]->m[getApplicationAccountRow["name"]] = new Json;
                  }
                  ptJwt->m["RadialCredentials"]->m[getApplicationAccountRow["name"]]->i("User", getApplicationAccountRow["user_id"]);
                  if (getApplicationAccountRow["aes"] == "1")
                  {
                    if (getApplicationAccountRow.find("decrypted_password") != getApplicationAccountRow.end() && !getApplicationAccountRow["decrypted_password"].empty())
                    {
                      ptJwt->m["RadialCredentials"]->m[getApplicationAccountRow["name"]]->i("Password", getApplicationAccountRow["decrypted_password"]);
                    }
                  }
                  else
                  {
                    ptJwt->m["RadialCredentials"]->m[getApplicationAccountRow["name"]]->i("Password", getApplicationAccountRow["password"]);
                  }
                }
              }
              dbfree(getApplicationAccount);
              if (exist(ptData->m["central"], "apps"))
              {
                ptJwt->m["sl_auth"] = new Json(ptData->m["central"]->m["apps"]);
                ptJson->m["Response"]->m["auth"]->m["apps"] = new Json(ptData->m["central"]->m["apps"]);
              }
              if (m_pProcessJwtCallback != NULL)
              {
                m_pProcessJwtCallback(strPrefix, ptJson, ptData, ptJwt);
              }
              if (jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
              {
                ptJson->m["Response"]->i("jwt", m_manip.encodeBase64(m_manip.encryptAes(strPayload, m_strJwtSecret, strValue, strError), strValue));
              }
              delete ptJwt;
            }
            else
            {
              strError = "Failed to parse central authz data.";
            }
          }
          delete ptData;
        }
      }
    }
    // }}}
    // {{{ invalid
    else
    {
      strError = "Please provide a valid Function:  auth, getSecurityModule, login, logout, process.";
    }
    // }}}
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
// {{{ setLogin()
void Secure::setLogin(bool (*pCallback)(string, Json *, string &))
{
  m_pLoginCallback = pCallback;
}
// }}}
// {{{ setLoginTitle()
void Secure::setLoginTitle(string (*pCallback)(const string))
{
  m_pLoginTitleCallback = pCallback;
}
// }}}
// {{{ setLogout()
void Secure::setLogout(bool (*pCallback)(string, Json *, string &))
{
  m_pLogoutCallback = pCallback;
}
// }}}
// {{{ setProcessJwt()
void Secure::setProcessJwt(void (*pCallback)(string, Json *, Json *, Json *))
{
  m_pProcessJwtCallback = pCallback;
}
// }}}
// {{{ setProcessPostAuthz()
void Secure::setProcessPostAuthz(void (*pCallback)(string, Json *, Json *))
{
  m_pProcessPostAuthzCallback = pCallback;
}
// }}}
// {{{ setProcessPreAuthz()
void Secure::setProcessPreAuthz(void (*pCallback)(string, Json *, Json *))
{
  m_pProcessPreAuthzCallback = pCallback;
}
// }}}
}
}
