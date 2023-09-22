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
  m_pAnalyzeCallback = NULL;
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
          if (m_pAnalyzeCallback != NULL)
          {
            if (!m_pAnalyzeCallback(strPrefix, strUser, ssName.str(), strMessage, user, strError))
            {
              errors.push_back(strError);
            }
          }
          else
          {
            if (!chat(ssName.str(), strMessage, strError))
            {
              errors.push_back((string)"Interface::chat() " + strError);
            }
            if (!user["email"].empty())
            {
              email(user["email"], user["email"], "Alert", strMessage, "");
            }
            if (!live("", strUser, {{"Action", "audio"}, {"Media", "/media/alert.mp3"}}, strError))
            {
              errors.push_back((string)"Interface::live(audio) " + strError);
            }
            if (!live("", strUser, {{"Action", "message"}, {"Class", "danger"}, {"Body", strMessage}}, strError))
            {
              errors.push_back((string)"Interface::live(message) " + strError);
            }
            if (!pageUser(strUser, strMessage, strError))
            {
              errors.push_back((string)"Interface::pageUser() " + strError);
            }
          }
          if (errors.empty())
          {
            bResult = true;
          }
          else
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
        }
        else
        {
          strError = (string)"db(centralUsers) " + strError;
        }
        delete ptUser;
      }
      else
      {
        strError = "Please provide the User within the Request.";
      }
    }
    else
    {
      strError = "Please provide the Message within the Request.";
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
// {{{ setAnalyze()
void Alert::setAnalyze(bool (*pCallback)(string, const string, const string, const string, map<string, string>, string &))
{
  m_pAnalyzeCallback = pCallback;
}
// }}}
}
}
