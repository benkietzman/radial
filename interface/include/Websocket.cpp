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
Websocket::Websocket(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), int (*pWebsocket)(lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)) : Interface(strPrefix, "websocket", argc, argv, pCallback)
{
  m_pWebsocket = pWebsocket;
}
// }}}
// {{{ ~Websocket()
Websocket::~Websocket()
{
}
// }}}
// {{{ callback()
void Websocket::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError, strJson;
  stringstream ssMessage;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Websocket::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "wsRequestID"))
  {
    string strIdentity, strName, strNode;
    stringstream ssRequestID(ptJson->m["wsRequestID"]->v);
    ssRequestID >> strNode >> strName >> strIdentity;
    if (!strNode.empty() && strName == m_strName && !strIdentity.empty())
    {
      if (strNode == m_strNode)
      {
        list<data *>::iterator connIter = m_conns.end();
        for (auto i = m_conns.begin(); connIter == m_conns.end() && i != m_conns.end(); i++)
        {
          stringstream ssIdentity;
          ssIdentity << (*i)->wsi;
          if (strIdentity == ssIdentity.str())
          {
            connIter = i;
          }
        }
        if (connIter != m_conns.end())
        {
          Json *ptSubJson = new Json(ptJson);
          bResult = true;
          keyRemovals(ptSubJson);
          if (exist(ptSubJson, "Interface"))
          {
            delete ptSubJson->m["Interface"];
            ptSubJson->m.erase("Interface");
          }
          (*connIter)->buffers.push_back(ptSubJson->j(strJson));
          lws_callback_on_writable((*connIter)->wsi);
          delete ptSubJson;
        }
        else
        {
          strError = "Failed to find the upstream connection.";
        }
      }
      else
      {
        Json *ptSubJson = new Json(ptJson);
        ptSubJson->i("Node", strNode);
        if (hub("link", ptSubJson, strError))
        {
          bResult = true;
        }
        delete ptSubJson;
      }
    }
    else
    {
      strError = "Please provide a valid wsRequestID.";
    }
  }
  else if (!empty(ptJson, "Function"))
  {
    if (ptJson->m["Function"]->v == "list")
    {
      bResult = true;
      ptJson->m["Response"] = new Json;
      for (auto &conn : m_conns)
      {
        stringstream ssIdentity;
        Json *ptConn = new Json;
        ssIdentity << m_strNode << " " << m_strName << " " << conn->wsi;
        ptConn->i("Application", conn->strApplication);
        ptConn->i("User", conn->strUserID);
        ptJson->m["Response"]->m[ssIdentity.str()] = ptConn;
      }
    }
    else if (ptJson->m["Function"]->v == "ping")
    {
      bResult = true;
    }
    else
    {
      strError = "Please provide a valid Function:  list, ping.";
    }
  }
  else
  {
    strError = "Please provide the Function or wsRequestID.";
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
// {{{ request()
void Websocket::request(string strPrefix, data *ptConn, Json *ptJson)
{
  string strApplication, strError, strJson, strPassword, strUser, strUserID;
  stringstream ssMessage, ssRequestID;

  threadIncrement();
  strPrefix += "->Websocket::request()";
  throughput("request");
  ptConn->mutexShare.lock();
  ptConn->unThreads++;
  ptConn->mutexShare.unlock();
  ssRequestID << m_strNode << " " << m_strName << " " << ptConn->wsi;
  ptJson->i("wsRequestID", ssRequestID.str());
  if (!ptConn->strApplication.empty())
  {
    strApplication = ptConn->strApplication;
  }
  else if (!empty(ptJson, "reqApp"))
  {
    strApplication = ptConn->strApplication = ptJson->m["reqApp"]->v;
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
  if ((strUser.empty() || strPassword.empty()) && !empty(ptJson, "wsJwt"))
  {
    string strBase64 = ptJson->m["wsJwt"]->v;
    if (!m_strJwtSecret.empty())
    {
      if (!m_strJwtSigner.empty())
      {
        string strPayload, strValue;
        Json *ptJwt = new Json;
        m_manip.decryptAes(m_manip.decodeBase64(strBase64, strValue), m_strJwtSecret, strPayload, strError);
        if (strPayload.empty())
        {
          strPayload = strBase64;
        }
        if (jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
        {
          if (exist(ptJwt, "RadialCredentials"))
          {
            if (exist(ptJwt->m["RadialCredentials"], strApplication))
            {
              if (!empty(ptJwt->m["RadialCredentials"]->m[strApplication], "User"))
              {
                strUser = ptConn->strUser = ptJwt->m["RadialCredentials"]->m[strApplication]->m["User"]->v;
                if (!empty(ptJwt->m["RadialCredentials"]->m[strApplication], "Password"))
                {
                  strPassword = ptConn->strPassword = ptJwt->m["RadialCredentials"]->m[strApplication]->m["Password"]->v;
                }
                else
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << " error [" << strApplication << "," << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to find Password in RadialCredentials in jwt.";
                  log(ssMessage.str());
                }
              }
              else
              {
                ssMessage.str("");
                ssMessage << strPrefix << " error [" << strApplication << "," << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to find User in RadialCredentials in jwt.";
                log(ssMessage.str());
              }
            }
            else
            {
              ssMessage.str("");
              ssMessage << strPrefix << " error [" << strApplication << "," << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to find Application in RadialCredentials in jwt.";
              log(ssMessage.str());
            }
          }
          else
          {
            ssMessage.str("");
            ssMessage << strPrefix << " error [" << strApplication << "," << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to find RadialCredentials in jwt.";
            log(ssMessage.str());
          }
          if (!empty(ptJwt, "sl_login"))
          {
            strUserID = ptConn->strUserID = ptJwt->m["sl_login"]->v;
          }
        }
        else if (strError != "Failed: exp")
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->Interface::jwt(decode) error [" << strApplication << "," << ptConn->strUser << "," << ptConn->strUserID << "]:  " << strError;
          log(ssMessage.str());
        }
        delete ptJwt;
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << " error [" << strApplication << "," << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to locate the JWT Signer.";
        log(ssMessage.str());
      }
    }
    else
    {
      ssMessage.str("");
      ssMessage << strPrefix << " error [" << strApplication << "," << ptConn->strUser << "," << ptConn->strUserID << "]:  Failed to locate the JWT Secret.";
      log(ssMessage.str());
    }
  }
  // }}}
  // {{{ wsSessionID
  else if ((strUser.empty() || strPassword.empty()) && !empty(ptJson, "wsSessionID"))
  {
    map<string, string> getSessionRow;
    Json *ptSession = new Json;
    ptSession->i("Function", "read");
    ptSession->m["Request"] = new Json;
    ptSession->m["Request"]->i("ID", ptJson->m["wsSessionID"]->v);
    if (hub("session", ptSession, strError))
    {
      if (exist(ptSession, "Response"))
      {
        if (exist(ptSession->m["Response"], "RadialCredentials"))
        {
          if (exist(ptSession->m["Response"]->m["RadialCredentials"], strApplication))
          {
            if (!empty(ptSession->m["Response"]->m["RadialCredentials"]->m[strApplication], "User"))
            {
              strUser = ptConn->strUser = ptSession->m["Response"]->m["RadialCredentials"]->m[strApplication]->m["User"]->v;
              if (!empty(ptSession->m["Response"]->m["RadialCredentials"]->m[strApplication], "Password"))
              {
                strPassword = ptConn->strPassword = ptSession->m["Response"]->m["RadialCredentials"]->m[strApplication]->m["Password"]->v;
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
        if (!empty(ptSession->m["Response"], "sl_login"))
        {
          strUserID = ptConn->strUserID = ptSession->m["Response"]->m["sl_login"]->v;
        }
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->Interface::hub(session,read) error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  No results returned.";
        log(ssMessage.str());
      }
    }
    else
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->Interface::hub(session,read) error [" << ptConn->strUser << "," << ptConn->strUserID << "]:  " << strError;
      log(ssMessage.str());
    }
    delete ptSession;
  }
  // }}}
  if (!strUser.empty() && !strPassword.empty())
  {
    ptJson->i("User", strUser);
    ptJson->i("Password", strPassword);
  }
  if (!strUserID.empty())
  {
    ptJson->i("UserID", strUserID);
  }
  else if (exist(ptJson, "UserID"))
  {
    delete ptJson->m["UserID"];
    ptJson->m.erase("UserID");
  }
  if (!empty(ptJson, "Interface"))
  {
    if (ptJson->m["Interface"]->v == "hub")
    {
      if (!empty(ptJson, "Function"))
      {
        if (ptJson->m["Function"]->v == "list" || ptJson->m["Function"]->v == "ping")
        {
          hub(ptJson);
        }
        else
        {
          ptJson->i("Status", "error");
          ptJson->i("Error", "Please provide a valid Function:  list, ping.");
        }
      }
      else
      {
        ptJson->i("Status", "error");
        ptJson->i("Error", "Please provide the Function.");
      }
    }
    else
    {
      bool bRestricted = false;
      string strTarget = ptJson->m["Interface"]->v;
      m_mutexShare.lock();
      if (m_i.find(ptJson->m["Interface"]->v) != m_i.end())
      {
        bRestricted = m_i[ptJson->m["Interface"]->v]->bRestricted;
      }
      else
      {
        list<radialLink *>::iterator linkIter = m_l.end();
        for (auto i = m_l.begin(); linkIter == m_l.end() && i != m_l.end(); i++)
        {
          if ((*i)->interfaces.find(ptJson->m["Interface"]->v) != (*i)->interfaces.end())
          { 
            linkIter = i;
          }
        }
        if (linkIter != m_l.end() && m_i.find("link") != m_i.end() && (bRestricted = (*linkIter)->interfaces[ptJson->m["Interface"]->v]->bRestricted))
        {
          ptJson->i("Node", (*linkIter)->strNode);
          strTarget = "link";
        }
      }
      m_mutexShare.unlock();
      if (!bRestricted || auth(ptJson, strError))
      {
        hub(strTarget, ptJson);
      }
      else
      {
        ptJson->i("Status", "error");
        ptJson->i("Error", strError);
      }
    }
  }
  else
  {
    ptJson->i("Status", "error");
    ptJson->i("Error", "Please provide the Interface.");
  }
  if (exist(ptJson, "Password"))
  {
    delete ptJson->m["Password"];
    ptJson->m.erase("Password");
  }
  ptConn->mutexShare.lock();
  ptConn->buffers.push_back(ptJson->j(strJson));
  if (ptConn->wsi != NULL)
  {
    lws_callback_on_writable(ptConn->wsi);
  }
  if (ptConn->unThreads > 0)
  {
    ptConn->unThreads--;
  }
  ptConn->mutexShare.unlock();
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
      m_mutex.lock();
      (*connIter)->wsi = NULL;
      m_mutex.unlock();
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
