// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Jwt.cpp
// author     : Ben Kietzman
// begin      : 2023-02-17
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Jwt"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Jwt()
Jwt::Jwt(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "jwt", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Jwt()
Jwt::~Jwt()
{
}
// }}}
// {{{ callback()
void Jwt::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;

  threadIncrement();
  strPrefix += "->Jwt::callback()";
  if (ptJson->m.find("Signer") != ptJson->m.end() && !ptJson->m["Signer"]->v.empty())
  {
    MessageSigner *pSigner = nullptr;
    if (ptJson->m["Signer"]->v == "HS256" || ptJson->m["Signer"]->v == "HS384" || ptJson->m["Signer"]->v == "HS512")
    {
      if (ptJson->m.find("Secret") != ptJson->m.end() && !ptJson->m["Secret"]->v.empty())
      {
        if (ptJson->m["Signer"]->v == "HS256")
        {
          pSigner = new HS256Validator(ptJson->m["Secret"]->v);
        }
        else if (ptJson->m["Signer"]->v == "HS384")
        {
          pSigner = new HS384Validator(ptJson->m["Secret"]->v);
        }
        else if (ptJson->m["Signer"]->v == "HS512")
        {
          pSigner = new HS512Validator(ptJson->m["Secret"]->v);
        }
      }
    }
    else if (ptJson->m["Signer"]->v == "RS256" || ptJson->m["Signer"]->v == "RS384" || ptJson->m["Signer"]->v == "RS512")
    {
      if (ptJson->m.find("Public Key") != ptJson->m.end() && !ptJson->m["Public Key"]->v.empty())
      {
        if (ptJson->m.find("Private Key") != ptJson->m.end() && !ptJson->m["Private Key"]->v.empty())
        {
          if (ptJson->m["Signer"]->v == "RS256")
          {
            pSigner = new RS256Validator(ptJson->m["Public Key"]->v, ptJson->m["Private Key"]->v);
          }
          else if (ptJson->m["Signer"]->v == "RS384")
          {
            pSigner = new RS384Validator(ptJson->m["Public Key"]->v, ptJson->m["Private Key"]->v);
          }
          else if (ptJson->m["Signer"]->v == "RS512")
          {
            pSigner = new RS512Validator(ptJson->m["Public Key"]->v, ptJson->m["Private Key"]->v);
          }
        }
        else
        {
          if (ptJson->m["Signer"]->v == "RS256")
          {
            pSigner = new RS256Validator(ptJson->m["Public Key"]->v);
          }
          else if (ptJson->m["Signer"]->v == "RS384")
          {
            pSigner = new RS384Validator(ptJson->m["Public Key"]->v);
          }
          else if (ptJson->m["Signer"]->v == "RS512")
          {
            pSigner = new RS512Validator(ptJson->m["Public Key"]->v);
          }
        }
      }
      else
      {
        strError = "Please provide the Public Key.";
      }
    }
    if (ptJson->m.find("Payload") != ptJson->m.end())
    {
      if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
      {
        if (ptJson->m["Function"]->v == "decode")
        {
          ExpValidator exp;
          json header, payload;
          try
          {
            stringstream ssHeader, ssPayload;
            ptJson->m["Response"] = new Json;
            tie(header, payload) = JWT::Decode(ptJson->m["Payload"]->v, pSigner, &exp);
            ssHeader << header;
            ptJson->m["Response"]->m["Header"] = new Json(ssHeader.str());
            ssPayload << payload;
            ptJson->m["Response"]->m["Payload"] = new Json(ssPayload.str());
            bResult = true;
          }
          catch (InvalidTokenError &tfe)
          {
            strError = tfe.what();
          }
          catch (exception &e)
          {
            strError = e.what();
          }
        }
        else if (ptJson->m["Function"]->v == "encode")
        {
          json data;
          try
          {
            string strValue;
            stringstream ssJson(ptJson->m["Payload"]->j(strValue));
            ptJson->m["Response"] = new Json;
            ssJson >> data;
            ptJson->m["Response"]->i("Payload", JWT::Encode((*pSigner), data));
            bResult = true;
          }
          catch (exception &e)
          {
            strError = e.what();
          }
        }
        else
        {
          strError = "Please provide a valid Function:  decode, encode.";
        }
      }
      else
      {
        strError = "Please provide the Function.";
      }
    }
    else
    {
      strError = "Please provide the Payload.";
    }
    if (pSigner != nullptr)
    {
      delete pSigner;
    }
  }
  else
  {
    strError = "Please provide the Signer.";
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
