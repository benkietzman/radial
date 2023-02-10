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
Secure::Secure(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "secure", argc, argv, pCallback)
{
  string strError;
  Json *ptAes = new Json, *ptJwt = new Json;

  m_pLoginCallback = NULL;
  m_pLoginTitleCallback = NULL;
  m_pLogoutCallback = NULL;
  m_pProcessJwtCallback = NULL;
  m_pProcessPostAuthzCallback = NULL;
  m_pProcessPreAuthzCallback = NULL;
  m_pJunction->useSingleSocket(true);
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"aes"}, ptAes, strError))
  { 
    if (ptAes->m.find("Secret") != ptAes->m.end() && !ptAes->m["Secret"]->v.empty())
    {
      m_strAesSecret = ptAes->m["Secret"]->v;
    }
  }
  delete ptAes; 
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"jwt"}, ptJwt, strError))
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
// }}}
// {{{ ~Secure()
Secure::~Secure()
{
}
// }}}
// {{{ callback()
void Secure::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError, strValue;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Secure::callback()";
  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
  {
    // {{{ auth
    if (ptJson->m["Function"]->v == "auth")
    {
      if (ptJson->m.find("wsJwt") != ptJson->m.end() && !ptJson->m["wsJwt"]->v.empty())
      {
        string strBase64 = ptJson->m["wsJwt"]->v, strPayload;
        Json *ptJwt = new Json;
        m_manip.decryptAes(m_manip.decodeBase64(strBase64, strValue), m_strJwtSecret, strPayload, strError);
        if (strPayload.empty())
        {
          strPayload = strBase64;
        }
        if (m_pJunction->jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
        {
          bResult = true;
          ptJson->m["Response"] = new Json;
          if (ptJwt->m.find("sl_admin") != ptJwt->m.end())
          {
            ptJson->m["Response"]->m["admin"] = new Json(ptJwt->m["sl_admin"]);
          }
          if (ptJwt->m.find("sl_auth") != ptJwt->m.end())
          {
            ptJson->m["Response"]->m["apps"] = new Json(ptJwt->m["sl_auth"]);
          }
          if (ptJwt->m.find("sl_email") != ptJwt->m.end())
          {
            ptJson->m["Response"]->m["email"] = new Json(ptJwt->m["sl_email"]);
          }
          if (ptJwt->m.find("sl_first_name") != ptJwt->m.end())
          {
            ptJson->m["Response"]->m["first_name"] = new Json(ptJwt->m["sl_first_name"]);
          }
          if (ptJwt->m.find("sl_last_name") != ptJwt->m.end())
          {
            ptJson->m["Response"]->m["last_name"] = new Json(ptJwt->m["sl_last_name"]);
          }
          if (ptJwt->m.find("sl_login") != ptJwt->m.end())
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
      if (ptJson->m.find("reqApp") != ptJson->m.end() && !ptJson->m["reqApp"]->v.empty())
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
      if (ptJson->m.find("Request") != ptJson->m.end())
      {
        if (ptJson->m["Request"]->m.find("Type") != ptJson->m["Request"]->m.end() && !ptJson->m["Request"]->m["Type"]->v.empty())
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
      if (ptJson->m.find("Request") != ptJson->m.end())
      {
        if (ptJson->m["Request"]->m.find("Type") != ptJson->m["Request"]->m.end() && !ptJson->m["Request"]->m["Type"]->v.empty())
        {
          // {{{ password
          if (ptJson->m["Request"]->m["Type"]->v == "password")
          {
            if (ptJson->m["Request"]->m.find("Return") != ptJson->m["Request"]->m.end() && !ptJson->m["Request"]->m["Return"]->v.empty())
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
            if (ptJson->m["Request"]->m.find("Return") != ptJson->m["Request"]->m.end() && !ptJson->m["Request"]->m["Return"]->v.empty())
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
ssMessage.str(""); ssMessage << strPrefix << ":  0"; log(ssMessage.str());
      bResult = true;
      ptJson->m["Response"] = new Json;
      ptJson->m["Response"]->m["auth"] = new Json;
      ptJson->m["Response"]->m["auth"]->i("login_title", "Login");
      if (ptJson->m["Request"]->m.find("Type") != ptJson->m["Request"]->m.end())
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
ssMessage.str(""); ssMessage << strPrefix << ":  1"; log(ssMessage.str());
      if (ptJson->m.find("Request") != ptJson->m.end())
      {
ssMessage.str(""); ssMessage << strPrefix << ":  1-0"; log(ssMessage.str());
        if (ptJson->m["Request"]->m.size() > 1)
        {
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-0"; log(ssMessage.str());
          Json *ptData = new Json(ptJson->m["Request"]);
          if (ptJson->m["Request"]->m.find("password") != ptJson->m["Request"]->m.end() && !ptJson->m["Request"]->m["password"]->v.empty())
          {
            delete ptJson->m["Request"]->m["password"];
            ptJson->m["Request"]->m.erase("password");
          }
          if (ptData->m.find("password") == ptData->m.end() && ptData->m.find("Password") == ptData->m.end())
          {
            ptData->i("Password", "");
          }
          if (m_pProcessPreAuthzCallback != NULL)
          {
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-0-0"; log(ssMessage.str());
            m_pProcessPreAuthzCallback(strPrefix, ptJson, ptData);
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-0-1"; log(ssMessage.str());
          }
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1 DATA " << ptData; log(ssMessage.str());
          if (m_pWarden->authz(ptData, strError))
          {
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1-0 DATA " << ptData; log(ssMessage.str());
            if (m_pProcessPostAuthzCallback != NULL)
            {
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1-0-0"; log(ssMessage.str());
              m_pProcessPostAuthzCallback(strPrefix, ptJson, ptData);
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1-0-1 DATA " << ptData; log(ssMessage.str());
            }
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1-1"; log(ssMessage.str());
            if (ptData->m.find("central") != ptData->m.end())
            {
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1-1-0"; log(ssMessage.str());
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
                  if (ptJwt->m["RadialCredentials"]->m.find(getApplicationAccountRow["name"]) == ptJwt->m["RadialCredentials"]->m.end())
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
              if (ptData->m["central"]->m.find("apps") != ptData->m["central"]->m.end())
              {
                ptJwt->m["sl_auth"] = new Json(ptData->m["central"]->m["apps"]);
                ptJson->m["Response"]->m["auth"]->m["apps"] = new Json(ptData->m["central"]->m["apps"]);
              }
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1-1-1"; log(ssMessage.str());
              if (m_pProcessJwtCallback != NULL)
              {
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1-1-1-0"; log(ssMessage.str());
                m_pProcessJwtCallback(strPrefix, ptJson, ptData, ptJwt);
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1-1-1-1"; log(ssMessage.str());
              }
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1-1-2 JWT " << ptJwt; log(ssMessage.str());
              if (m_pJunction->jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
              {
                ptJson->m["Response"]->i("jwt", m_manip.encodeBase64(m_manip.encryptAes(strPayload, m_strJwtSecret, strValue, strError), strValue));
              }
              delete ptJwt;
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1-1-3"; log(ssMessage.str());
            }
            else
            {
              strError = "Failed to parse central authz data.";
            }
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-1-2"; log(ssMessage.str());
          }
          delete ptData;
ssMessage.str(""); ssMessage << strPrefix << ":  1-0-2"; log(ssMessage.str());
        }
ssMessage.str(""); ssMessage << strPrefix << ":  1-1"; log(ssMessage.str());
      }
ssMessage.str(""); ssMessage << strPrefix << ":  2"; log(ssMessage.str());
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
    hub(ptJson, false);
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
