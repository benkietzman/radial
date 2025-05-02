// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Session.cpp
// author     : Ben Kietzman
// begin      : 2023-07-11
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Session"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Session()
Session::Session(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "session", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Session()
Session::~Session()
{
}
// }}}
// {{{ callback()
void Session::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage, ssQuery;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Session::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    if (exist(ptJson, "Request"))
    {
      if (!empty(ptJson->m["Request"], "ID"))
      {
        // {{{ destroy
        if (ptJson->m["Function"]->v == "destroy")
        {
          bResult = db("dbCentralPhpSessionRemove", ptJson->m["Request"], strError);
        }
        // }}}
        // {{{ read
        else if (ptJson->m["Function"]->v == "read")
        {
          map<string, string> getSessionRow;
          if (db("dbCentralPhpSession", ptJson->m["Request"], getSessionRow, strError))
          {
            bResult = true;
            if (!getSessionRow.empty())
            {
              ptJson->insert("Response", getSessionRow);
            }
          }
        }
        // }}}
        // {{{ write
        else if (ptJson->m["Function"]->v == "write")
        {
          if (exist(ptJson->m["Request"], "Data"))
          {
            if (exist(ptJson->m["Request"], "Json"))
            {
              bResult = db("dbCentralPhpSessionAdd", ptJson->m["Request"], strError);
            }
            else
            {
              strError = "Please provide the Json in the Request.";
            }
          }
          else
          {
            strError = "Please provide the Data in the Request.";
          }
        }
        // }}}
        // {{{ invalid
        else
        {
          strError = "Please provide a valid Function:  destroy, read, write.";
        }
        // }}}
      }
      else
      {
        strError = "Please provide the ID in the Request.";
      }
    }
    else
    {
      strError = "Please provide the Request.";
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
}
}
