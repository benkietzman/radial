// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Alert.cpp
// author     : Ben Kietzman
// begin      : 2023-09-22
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Alert"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Alert()
Alert::Alert(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "alert", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Alert()
Alert::~Alert()
{
}
// }}}
// {{{ callback()
void Alert::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Alert::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (exist(ptJson, "Request"))
  {
    if (!empty(ptJson->m["Request"], "Message"))
    {
      string strMessage = ptJson->m["Request"]->m["Message"]->v;
      if (!empty(ptJson->m["Request"], "User"))
      {
        map<string, string> user;
        string strUser = ptJson->m["Request"]->m["User"]->v;
        Json *ptUser = new Json;
        ptUser->i("userid", strUser);
        if (db("dbCentralUsers", ptUser, user, strError))
        {
          if (!user.empty())
          {
            bool bAlerted = false;
            list<string> errors;
            string strFirstName, strLastName;
            stringstream ssFirstName(user["first_name"]), ssLastName(user["last_name"]), ssName;
            ssFirstName >> strFirstName;
            for (size_t i = 0; i < strFirstName.size(); i++)
            {
              if (i == 0)
              {
                strFirstName[i] = toupper(strFirstName[i]);
              }
              else
              {
                strFirstName[i] = tolower(strFirstName[i]);
              }
            }
            ssLastName >> strLastName;
            for (size_t i = 0; i < strLastName.size(); i++)
            {
              if (i == 0)
              {
                strLastName[i] = toupper(strLastName[i]);
              }
              else
              {
                strLastName[i] = tolower(strLastName[i]);
              }
            }
            ssName << strFirstName << strLastName;
            if (user["alert_chat"] == "1")
            {
              bAlerted = true;
              chat(ssName.str(), strMessage);
              chat(strUser, strMessage, "live");
            }
            if (user["alert_email"] == "1" && !user["email"].empty())
            {
              bAlerted = true;
              email(user["email"], user["email"], "Alert", strMessage, "");
            }
            if (user["alert_live_audio"] == "1")
            {
              bAlerted = true;
              live("", strUser, {{"Action", "audio"}, {"Media", "/radial/media/alert.mp3"}});
            }
            if (user["alert_live_message"] == "1")
            {
              bAlerted = true;
              live("", strUser, {{"Action", "message"}, {"Class", "danger"}, {"Body", strMessage}});
            }
            if (user["alert_pager"] == "1" && !user["pager"].empty())
            {
              bAlerted = true;
              email(user["pager"], user["pager"], "Alert", strMessage, "");
            }
            if (!user["alert_remote_url"].empty())
            {
              string strContent, strCookies, strHeader;
              stringstream ssProxy;
              Json *ptPost = new Json;
              if (!user["alert_remote_proxy"].empty())
              {
                ssProxy << user["alert_remote_proxy"];
                if (!user["alert_remote_proxy_user"].empty() && !user["alert_remote_proxy_password"].empty())
                {
                  ssProxy << endl << user["alert_remote_proxy_user"] << ":" << user["alert_remote_proxy_password"];
                }
              }
              ptPost->i("Interface", "alert");
              if (!user["alert_remote_auth_user"].empty())
              {
                ptPost->i("User", user["alert_remote_auth_user"]);
              }
              if (!user["alert_remote_auth_decrypted_password"].empty())
              {
                ptPost->i("Password", user["alert_remote_auth_decrypted_password"]);
              }
              ptPost->m["Request"] = new Json;
              if (!user["alert_remote_user"].empty())
              {
                ptPost->m["Request"]->i("User", user["alert_remote_user"]);
              }
              ptPost->m["Request"]->i("Message", strMessage);
              if (curl(user["alert_remote_url"], "json", NULL, NULL, ptPost, NULL, ssProxy.str(), strCookies, strHeader, strContent, strError))
              {
                Json *ptContent = new Json(strContent);
                if (ptContent->m.find("Status") != ptContent->m.end() && ptContent->m["Status"]->v == "okay")
                {
                  bAlerted = true;
                }
                else if (ptContent->m.find("Error") != ptContent->m.end())
                {
                  if (ptContent->m["Error"]->m.find("Message") != ptContent->m["Error"]->m.end() && !ptContent->m["Error"]->m["Message"]->v.empty())
                  {
                    errors.push_back((string)"Interface::curl() [" + ptContent->m["Error"]->m["Message"]->v + (string)"]");
                  }
                  else if (!ptContent->m["Error"]->v.empty())
                  {
                    errors.push_back((string)"Interface::curl() [" + ptContent->m["Error"]->v + (string)"]");
                  }
                  else
                  {
                    errors.push_back("Interface::curl() Encountered an unknown error.");
                  }
                }
                else
                {
                  errors.push_back("Interface::curl() Encountered an unknown error.");
                }
                delete ptContent;
              }
              else
              {
                errors.push_back((string)"Interface::curl() " + strError);
              }
              delete ptPost;
            }
            if (bAlerted)
            {
              bResult = true;
            }
            else if (!errors.empty())
            {
              strError.clear();
              for (auto error = errors.begin(); error != errors.end(); error++)
              {
                if (error != errors.begin())
                {
                  strError += ", ";
                }
                strError += (*error);
              }
            }
            else
            {
              strError = "User does not have any alerting enabled.";
            }
          }
          else
          {
            strError = "Please provide a valid User that is registered within Central.";
          }
        }
        else
        {
          strError = (string)"db(centralUsers) " + strError;
        }
        delete ptUser;
      }
      else
      {
        strError = "Please provide the User.";
      }
    }
    else
    {
      strError = "Please provide the Message.";
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
    ptJson->j(p.p);
    hub(p, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
}
}
