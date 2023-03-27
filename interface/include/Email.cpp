// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Email.cpp
// author     : Ben Kietzman
// begin      : 2022-03-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Email"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Email()
Email::Email(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "email", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Email()
Email::~Email()
{
}
// }}}
// {{{ callback()
void Email::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Email::callback()";
  if (!empty(ptJson, "From"))
  {
    list<string> to;
    if (!empty(ptJson, "To"))
    {
      to.push_back(ptJson->m["To"]->v);
    }
    else if (exist(ptJson, "To"))
    {
      for (auto &i : ptJson->m["To"]->l)
      {
        if (!i->v.empty())
        {
          to.push_back(i->v);
        }
      }
    }
    if (!to.empty())
    {
      list<string> bcc, cc, fileList;
      map<string, string> fileMap;
      string strHtml, strSubject, strText;
      if (!empty(ptJson, "Cc"))
      {
        cc.push_back(ptJson->m["Cc"]->v);
      }
      else if (exist(ptJson, "Cc"))
      {
        for (auto &i : ptJson->m["Cc"]->l)
        {
          if (!i->v.empty())
          {
            cc.push_back(i->v);
          }
        }
      }
      if (!empty(ptJson, "Bcc"))
      {
        bcc.push_back(ptJson->m["Bcc"]->v);
      }
      else if (exist(ptJson, "Bcc"))
      {
        for (auto &i : ptJson->m["Bcc"]->l)
        {
          if (!i->v.empty())
          {
            bcc.push_back(i->v);
          }
        }
      }
      if (!empty(ptJson, "File"))
      {
        fileList.push_back(ptJson->m["File"]->v);
      }
      else if (exist(ptJson, "File"))
      {
        if (!ptJson->m["File"]->l.empty())
        {
          for (auto &i : ptJson->m["File"]->l)
          {
            if (!i->v.empty())
            {
              fileList.push_back(i->v);
            }
          }
        }
        else if (!ptJson->m["File"]->m.empty())
        {
          for (auto &i : ptJson->m["File"]->m)
          {
            if (!i.second->v.empty())
            {
              fileMap[i.first] = i.second->v;
            }
          }
        }
      }
      if (!empty(ptJson, "Subject"))
      {
        strSubject = ptJson->m["Subject"]->v;
      }
      if (!empty(ptJson, "Html"))
      {
        strHtml = ptJson->m["Html"]->v;
      }
      if (!empty(ptJson, "Text"))
      {
        strText = ptJson->m["Text"]->v;
      }
      if (!fileList.empty())
      {
        if (m_pJunction->email(ptJson->m["From"]->v, to, cc, bcc, strSubject, strText, strHtml, fileList, strError))
        {
          bResult = true;
        }
      }
      else if (m_pJunction->email(ptJson->m["From"]->v, to, cc, bcc, strSubject, strText, strHtml, fileMap, strError))
      {
        bResult = true;
      }
    }
    else
    {
      strError = "Please provide the To.";
    }
  }
  else
  {
    strError = "Please provide the From.";
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
