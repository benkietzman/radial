// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Websocket.cpp
// author     : Ben Kietzman
// begin      : 2022-06-03
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Websocket"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Websocket()
Websocket::Websocket(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool), int (*pWebsocket)(lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)) : Interface(strPrefix, "websocket", argc, argv, pCallback)
{
  string strError;
  Json *ptJwt = new Json;

  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"jwt"}, ptJwt, strError))
  {
    if (ptJwt->m.find("Secret") != ptJwt->m.end() && !ptJwt->m["Secret"]->v.empty())
    {
      m_strSecret = ptJwt->m["Secret"]->v;
    }
    if (ptJwt->m.find("Signer") != ptJwt->m.end() && !ptJwt->m["Signer"]->v.empty())
    {
      m_strSigner = ptJwt->m["Signer"]->v;
    }
  }
  delete ptJwt;
  m_pWebsocket = pWebsocket;
}
// }}}
// {{{ ~Websocket()
Websocket::~Websocket()
{
}
// }}}
// {{{ callback()
void Websocket::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Websocket::callback()";
  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
  {
    if (ptJson->m["Function"]->v == "ping")
    {
      bResult = true;
    }
    else
    {
      strError = "Please provide a valid Function:  ping.";
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
  if (bResponse)
  {
    hub(ptJson, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
// {{{ request()
void Websocket::request(string strPrefix, data *ptConn, Json *ptJson)
{
  string strApplication, strError, strJson, strPassword, strUser, strUserID;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Websocket::request()";
  ptConn->mutexShare.lock();
  ptConn->unThreads++;
  ptConn->mutexShare.unlock();
  if (!ptConn->strApplication.empty())
  {
    strApplication = ptConn->strApplication;
  }
  else if (ptJson->m.find("reqApp") != ptJson->m.end() && !ptJson->m["reqApp"]->v.empty())
  {
    strApplication = ptJson->m["reqApp"]->v;
  }
  // {{{ existing
  if (!ptConn->strUser.empty() && !ptConn->strPassword.empty() && !ptConn->strUserID.empty())
  {
    strUser = ptConn->strUser;
    strPassword = ptConn->strPassword;
    strUserID = ptConn->strUserID;
  }
  // }}}
  // {{{ jwt
  if ((strUser.empty() || strPassword.empty()) && ptJson->m.find("wsJwt") != ptJson->m.end() && !ptJson->m["wsJwt"]->v.empty())
  {
    string strBase64 = ptJson->m["wsJwt"]->v;
    if (!m_strSecret.empty())
    {
      if (!m_strSigner.empty())
      {
        string strPayload, strValue;
        Json *ptJwt = new Json;
        m_manip.decryptAes(m_manip.decodeBase64(strBase64, strValue), m_strSecret, strPayload, strError);
        if (strPayload.empty())
        {
          strPayload = strBase64;
        }
        if (m_pJunction->jwt(m_strSigner, m_strSecret, strPayload, ptJwt, strError))
        {
          if (ptJwt->m.find("RadialCredentials") != ptJwt->m.end())
          {
            if (ptJwt->m["RadialCredentials"]->m.find(strApplication) != ptJwt->m["RadialCredentials"]->m.end())
            {
              if (ptJwt->m["RadialCredentials"]->m[strApplication]->m.find("User") != ptJwt->m["RadialCredentials"]->m[strApplication]->m.end() && !ptJwt->m["RadialCredentials"]->m[strApplication]->m["User"]->v.empty())
              {
                strUser = ptConn->strUser = ptJwt->m["RadialCredentials"]->m[strApplication]->m["User"]->v;
                if (ptJwt->m["RadialCredentials"]->m[strApplication]->m.find("Password") != ptJwt->m["RadialCredentials"]->m[strApplication]->m.end() && !ptJwt->m["RadialCredentials"]->m[strApplication]->m["Password"]->v.empty())
                {
                  strPassword = ptConn->strPassword = ptJwt->m["RadialCredentials"]->m[strApplication]->m["Password"]->v;
                }
                else
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << " error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to find Password in RadialCredentials in jwt.";
                  log(ssMessage.str());
                }
              }
              else
              {
                ssMessage.str("");
                ssMessage << strPrefix << " error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to find User in RadialCredentials in jwt.";
                log(ssMessage.str());
              }
            }
            else
            {
              ssMessage.str("");
              ssMessage << strPrefix << " error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to find Application in RadialCredentials in jwt.";
              log(ssMessage.str());
            }
          }
          else
          {
            ssMessage.str("");
            ssMessage << strPrefix << " error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to find RadialCredentials in jwt.";
            log(ssMessage.str());
          }
          if (ptJwt->m.find("sl_login") != ptJwt->m.end() && !ptJwt->m["sl_login"]->v.empty())
          {
            strUserID = ptConn->strUserID = ptJwt->m["sl_login"]->v;
          }
        }
        else if (strError != "Failed: exp")
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->ServiceJunction::jwt(decode) error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  " << strError;
          log(ssMessage.str());
        }
        delete ptJwt;
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << " error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to locate the JWT Signer.";
        log(ssMessage.str());
      }
    }
    else
    {
      ssMessage.str("");
      ssMessage << strPrefix << " error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to locate the JWT Secret.";
      log(ssMessage.str());
    }
  }
  // }}}
  // {{{ wsSessionID
  else if ((strUser.empty() || strPassword.empty()) && ptJson->m.find("wsSessionID") != ptJson->m.end() && !ptJson->m["wsSessionID"]->v.empty())
  {
    stringstream ssQuery;
    ssQuery << "select session_json from php_session where session_id = '" << ptJson->m["wsSessionID"]->v << "'";
    auto getSession = dbquery("central_r", ssQuery.str(), strError);
    if (getSession != NULL)
    {
      if (!getSession->empty())
      {
        Json *ptSessionData = new Json(getSession->front()["session_json"]);
        if (ptSessionData->m.find("RadialCredentials") != ptSessionData->m.end())
        {
          if (ptSessionData->m["RadialCredentials"]->m.find(strApplication) != ptSessionData->m["RadialCredentials"]->m.end())
          {
            if (ptSessionData->m["RadialCredentials"]->m[strApplication]->m.find("User") != ptSessionData->m["RadialCredentials"]->m[strApplication]->m.end() && !ptSessionData->m["RadialCredentials"]->m[strApplication]->m["User"]->v.empty())
            {
              strUser = ptConn->strUser = ptSessionData->m["RadialCredentials"]->m[strApplication]->m["User"]->v;
              if (ptSessionData->m["RadialCredentials"]->m[strApplication]->m.find("Password") != ptSessionData->m["RadialCredentials"]->m[strApplication]->m.end() && !ptSessionData->m["RadialCredentials"]->m[strApplication]->m["Password"]->v.empty())
              {
                strPassword = ptConn->strPassword = ptSessionData->m["RadialCredentials"]->m[strApplication]->m["Password"]->v;
              }
              else
              {
                ssMessage.str("");
                ssMessage << strPrefix << " error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to find Password in Application in RadialCredentials in session.";
                log(ssMessage.str());
              }
            }
            else
            {
              ssMessage.str("");
              ssMessage << strPrefix << " error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to find User in Application in RadialCredentials in session.";
              log(ssMessage.str());
            }
          }
          else
          {
            ssMessage.str("");
            ssMessage << strPrefix << " error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to find Application in RadialCredentials in session.";
            log(ssMessage.str());
          }
        }
        if (ptSessionData->m.find("sl_login") != ptSessionData->m.end() && !ptSessionData->m["sl_login"]->v.empty())
        {
          strUserID = ptConn->strUserID = ptSessionData->m["sl_login"]->v;
        }
        delete ptSessionData;
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->query(" << ssQuery.str() << ") error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  No results returned.";
        log(ssMessage.str());
      }
    }
    else
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->query(" << ssQuery.str() << ") error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  " << strError;
      log(ssMessage.str());
    }
    dbfree(getSession);
  }
  // }}}
  if (!strUser.empty() && !strPassword.empty())
  {
    ptJson->insert("User", strUser);
    ptJson->insert("Password", strPassword);
  }
  if (!strUserID.empty())
  {
    ptJson->insert("UserID", strUserID);
  }
  else if (ptJson->m.find("UserID") != ptJson->m.end())
  {
    delete ptJson->m["UserID"];
    ptJson->m.erase("UserID");
  }
  if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
  {
    if (ptJson->m["Interface"]->v == "hub")
    {
      if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
      {
        if (ptJson->m["Function"]->v == "list" || ptJson->m["Function"]->v == "ping")
        {
          hub(ptJson);
        }
        else
        {
          ptJson->insert("Status", "error");
          ptJson->insert("Error", "Please provide a valid Function:  list, ping.");
        }
      }
      else
      {
        ptJson->insert("Status", "error");
        ptJson->insert("Error", "Please provide the Function.");
      }
    }
    else
    {
      bool bRestricted;
      string strTarget;
      m_mutexShare.lock();
      if (m_interfaces.find(ptJson->m["Interface"]->v) != m_interfaces.end())
      {
        strTarget = ptJson->m["Interface"]->v;
        bRestricted = m_interfaces[ptJson->m["Interface"]->v]->bRestricted;
      }
      else
      {
        list<radialLink *>::iterator linkIter = m_links.end();
        for (auto i = m_links.begin(); linkIter == m_links.end() && i != m_links.end(); i++)
        {
          if ((*i)->interfaces.find(ptJson->m["Interface"]->v) != (*i)->interfaces.end())
          { 
            linkIter = i;
          }
        }
        if (linkIter != m_links.end() && m_interfaces.find("link") != m_interfaces.end())
        { 
          strTarget = "link";
          bRestricted = (*linkIter)->interfaces[ptJson->m["Interface"]->v]->bRestricted;
        }
      }
      m_mutexShare.unlock();
      if (!strTarget.empty())
      {
        if (!bRestricted || auth(ptJson, strError))
        {
          hub(strTarget, ptJson);
        }
        else
        {
          ptJson->insert("Status", "error");
          ptJson->insert("Error", strError);
        }
      }
      else
      {
        ptJson->insert("Status", "error");
        ptJson->insert("Error", "Interface does not exist.");
      }
    }
  }
  else
  {
    ptJson->insert("Status", "error");
    ptJson->insert("Error", "Please provide the Interface.");
  }
  if (ptJson->m.find("Password") != ptJson->m.end())
  {
    delete ptJson->m["Password"];
    ptJson->m.erase("Password");
  }
  ptConn->mutexShare.lock();
  ptConn->buffers.push_back(ptJson->json(strJson));
  if (ptConn->unThreads > 0)
  {
    ptConn->unThreads--;
  }
  ptConn->mutexShare.unlock();
  lws_callback_on_writable(ptConn->wsi);
  delete ptJson;
  threadDecrement();
}
// }}}
// {{{ socket()
void Websocket::socket(string strPrefix, lws_context *ptContext)
{
  int nReturn;
  stringstream ssMessage;

  strPrefix += "->Websocket::socket()";
  ssMessage.str("");
  ssMessage << strPrefix << "->lws_create_context():  Created context.";
  log(ssMessage.str());
  while (!shutdown() && (nReturn = lws_service(ptContext, 0)) >= 0)
  {
    list<list<data *>::iterator> removals;
    m_mutex.lock();
    for (auto i = m_conns.begin(); i != m_conns.end(); i++)
    {
      if ((*i)->bRemove)
      {
        removals.push_back(i);
      }
    }
    while (!removals.empty())
    {
      if ((*removals.front())->unThreads == 0)
      {
        delete (*removals.front());
        m_conns.erase(removals.front());
      }
      removals.pop_front();
    }
    m_mutex.unlock();
  }
  lws_context_destroy(ptContext);
  ssMessage.str("");
  ssMessage << strPrefix << "->lws_context_destroy():  Destroyed context.";
  log(ssMessage.str());
  while (!m_conns.empty())
  {
    delete m_conns.front();
    m_conns.pop_front();
  }
  setShutdown();
}
// }}}
// {{{ websocket()
int Websocket::websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
  bool bFound = false;
  int nResult = 0;
  list<data *>::iterator connIter;
  string *pstrBuffers[2] = {NULL, NULL}, strPrefix = "websocket->main()->Websocket::websocket()";
  stringstream ssClose;

  m_mutex.lock();
  for (auto i = m_conns.begin(); !bFound && i != m_conns.end(); i++)
  {
    if ((*i)->wsi == wsi)
    {
      bFound = true;
    }
  }
  if (!bFound)
  {
    data *ptData = new data;
    ptData->bRemove = false;
    ptData->wsi = wsi;
    ptData->unThreads = 0;
    m_conns.push_back(ptData);
  }
  connIter = m_conns.end();
  for (auto i = m_conns.begin(); connIter == m_conns.end() && i != m_conns.end(); i++)
  {
    if ((*i)->wsi == wsi)
    {
      connIter = i;
      pstrBuffers[0] = &((*i)->strBuffers[0]);
      pstrBuffers[1] = &((*i)->strBuffers[1]);
    }
  }
  m_mutex.unlock();
  switch (reason)
  {
    // {{{ LWS_CALLBACK_CLOSED
    case LWS_CALLBACK_CLOSED:
    {
      (*connIter)->wsi = NULL;
      nResult = -1;
      break;
    }
    // }}}
    // {{{ LWS_CALLBACK_RECEIVE
    case LWS_CALLBACK_RECEIVE:
    {
      pstrBuffers[0]->append((char *)in, len);
      if (lws_remaining_packet_payload(wsi) == 0 && lws_is_final_fragment(wsi))
      {
        thread threadRequest(&Websocket::request, this, strPrefix, *connIter, new Json(*pstrBuffers[0]));
        pthread_setname_np(threadRequest.native_handle(), "request");
        threadRequest.detach();
        pstrBuffers[0]->clear();
      }
      break;
    }
    // }}}
    // {{{ LWS_CALLBACK_SERVER_WRITEABLE
    case LWS_CALLBACK_SERVER_WRITEABLE:
    {
      int nWriteMode = LWS_WRITE_CONTINUATION;
      if (pstrBuffers[1]->empty())
      {
        (*connIter)->mutexShare.lock();
        if (!(*connIter)->buffers.empty())
        {
          nWriteMode = LWS_WRITE_TEXT;
          (*pstrBuffers[1]) = (*connIter)->buffers.front();
          (*connIter)->buffers.pop_front();
        }
        (*connIter)->mutexShare.unlock();
      }
      if (!pstrBuffers[1]->empty())
      {
        size_t nLength = 2048;
        unsigned char *puszBuffer;
        if (pstrBuffers[1]->size() <= 2048)
        {
          nLength = pstrBuffers[1]->size();
        }
        else
        {
          nWriteMode |= LWS_WRITE_NO_FIN;
        }
        puszBuffer = (unsigned char *)malloc(LWS_PRE + nLength);
        for (size_t i = 0; i < nLength; i++)
        {
          puszBuffer[LWS_PRE + i] = (*pstrBuffers[1])[i];
        }
        if (lws_write(wsi, &puszBuffer[LWS_PRE], nLength, (lws_write_protocol)nWriteMode) != -1)
        {
          pstrBuffers[1]->erase(0, nLength);
          lws_callback_on_writable(wsi);
        }
        else
        {
          nResult = -1;
          ssClose.str("");
          ssClose << strPrefix << "->lws_write() error [WS_CLOSED,LWS_CALLBACK_SERVER_WRITEABLE]:  Failed to write data over websocket.";
        }
        free(puszBuffer);
      }
      break;
    }
    // }}}
    // {{{ others
    default: break;
    // }}}
  }
  if (nResult < 0)
  {
    (*connIter)->bRemove = true;
    if (!ssClose.str().empty())
    {
      log(ssClose.str());
    }
  }

  return nResult;
}
// }}}
}
}
