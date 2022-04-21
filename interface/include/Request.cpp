// vim600: fdm=marker
/* -*- c++ -*- */
///////////////////////////////////////////
// Radial
// -------------------------------------
// file       : Request.cpp
// author     : Ben Kietzman
// begin      : 2022-04-21
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
**************************************************************************/
/*! \file Request.cpp
* \brief Request Class
*
* Provides Request interface.
*/
// {{{ includes
#include "Request"
// }}}
extern "C++"
{ 
namespace radial
{
// {{{ Request()
Request::Request(int argc, char **argv) : Interface("request", argc, argv)
{
}
// }}}
// {{{ ~Request()
Request::~Request()
{
}
// }}}
// {{{ callback()
void Request::callback(string strPrefix, Json *ptJson, string &strError)
{
  bool bResult = false;
  stringstream ssMessage;

  strPrefix += "->Request::callback()";
  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
  {
    if (ptJson->m.find("Message") != ptJson->m.end() && !ptJson->m["Message"]->v.empty())
    {
      if (ptJson->m["Function"]->v == "alert")
      {
        if (m_pCentral->alert(ptJson->m["Message"]->v, strError))
        {
          bResult = true;
        }
      }
      else if (ptJson->m["Function"]->v == "log")
      {
        if (m_pCentral->log(ptJson->m["Message"]->v, strError))
        {
          bResult = true;
        }
      }
      else if (ptJson->m["Function"]->v == "notify")
      {
        if (m_pCentral->notify("", ptJson->m["Message"]->v, strError))
        {
          bResult = true;
        }
      }
      else
      {
        strError = "Please provide a valid Function:  alert, log, notify.";
      }
    }
    else
    {
      strError = "Please provide the Message.";
    }
  }
  else
  {
    strError = "Please provide the Function.";
  }
  ptJson->insert("Status", ((bResult)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->insert("Error", strError);
  }
  response(ptJson);
}
// }}}
// {{{ incoming()
void Request::incoming(string strPrefix)
{
  SSL_CTX *ctx = NULL:
  string strError;
  stringstream ssMessage;

  strPrefix += "->Request::incoming()";
  setlocale(LC_ALL, "");
  if ((ctx = m_pCentral->utility()->sslInitServer((m_strData + "/server.crt"), (m_strData + "/server.key"), strError)) != NULL)
  {
    addrinfo hints, *result;
    bool bBound[3] = {false, false, false};
    int fdSocket, nReturn;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((nReturn = getaddrinfo(NULL, "7234", &hints, &result)) == 0)
    {
      addrinfo *rp;
      bBound[0] = true;
      for (rp = result; !bBound[2] && rp != NULL; rp = rp->ai_next)
      {
        bBound[1] = false;
        if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
        {
          int nOn = 1;
          bBound[1] = true;
          setsockopt(fdSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&nOn, sizeof(nOn));
          if (bind(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
          {
            bBound[2] = true;
          }
          else
          {
            close(fdSocket);
          }
        }
      }
      freeaddrinfo(result);
    }
    if (bBound[2])
    {
      // continue logic
    }
    SSL_CTX_free(ctx);
    EVP_cleanup();
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->Central::utility()->sslInitServer() error:  " << strError;
    notify(ssMessage.str());
  }
  m_pCentral->utility()->sslDeinit();
}
// }}}
}
}
