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
  m_pWebsocket = pWebsocket;
}
// }}}
// {{{ ~Websocket()
Websocket::~Websocket()
{
}
// }}}
// {{{ callback()
void Websocket::callback(string strPrefix, Json *ptJson, const bool bResponse = true)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

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
    response(ptJson);
  }
  delete ptJson;
}
// }}}
// {{{ request()
void Websocket::request(data *ptConn, Json *ptJson)
{
  string strError, strJson;

  ptConn->mutexShare.lock();
  ptConn->unThreads++;
  ptConn->mutexShare.unlock();
  if (ptJson->m.find("Interface") != ptJson->m.end() && !ptJson->m["Interface"]->v.empty())
  {
    list<string> removals;
    if (ptJson->m["Interface"]->v == "hub")
    {
      if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
      {
        if (ptJson->m["Function"]->v == "list" || ptJson->m["Function"]->v == "ping")
        {
          target(ptJson);
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
      Json *ptInterfaces = new Json;
      ptInterfaces->insert("Function", "list");
      target(ptInterfaces);
      if (ptInterfaces->m.find("Response") != ptInterfaces->m.end())
      {
        if (ptInterfaces->m["Response"]->m.find(ptJson->m["Interface"]->v) != ptInterfaces->m["Response"]->m.end())
        {
          if (ptInterfaces->m["Response"]->m[ptJson->m["Interface"]->v]->m.find("Restricted") == ptInterfaces->m["Response"]->m[ptJson->m["Interface"]->v]->m.end() || ptInterfaces->m["Response"]->m[ptJson->m["Interface"]->v]->m["Restricted"]->v == "0" || auth(ptJson, strError))
          {
            target(ptJson->m["Interface"]->v, ptJson);
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
      else
      {
        ptJson->insert("Status", "error");
        ptJson->insert("Error", "Failed to retrieve interfaces.");
      }
      delete ptInterfaces;
    }
    for (auto &i : ptJson->m)
    {
      if (!i.first.empty() && i.first[0] == '_')
      {
        removals.push_back(i.first);
      }
    }
    for (auto &removal : removals)
    {
      if (ptJson->m.find(removal) != ptJson->m.end())
      {
        delete ptJson->m[removal];
        ptJson->m.erase(removal);
      }
    }
  }
  else
  {
    ptJson->insert("Status", "error");
    ptJson->insert("Error", "Please provide the Interface.");
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
    for (auto i = m_conns.begin(); i != m_conns.end(); i++)
    {
      if ((*i)->bRemove)
      {
        removals.push_back(i);
      }
    }
    while (!removals.empty())
    {
      while ((*removals.front())->unThreads > 0)
      {
        msleep(10);
      }
      delete (*removals.front());
      m_conns.erase(removals.front());
    }
  }
  while (!m_conns.empty())
  {
    while (m_conns.front()->unThreads > 0)
    {
      msleep(10);
    }
    delete m_conns.front();
    m_conns.pop_front();
  }
  lws_context_destroy(ptContext);
  ssMessage.str("");
  ssMessage << strPrefix << "->lws_context_destroy():  Destroyed context.";
  log(ssMessage.str());
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
  switch (reason)
  {
    // {{{ LWS_CALLBACK_CLOSED
    case LWS_CALLBACK_CLOSED:
    {
      if (connIter != m_conns.end())
      {
        (*connIter)->wsi = NULL;
      }
      nResult = -1;
      ssClose.str("");
      ssClose << strPrefix << " [WS_CLOSED,LWS_CALLBACK_CLOSED]:  Websocket closed.";
      break;
    }
    // }}}
    // {{{ LWS_CALLBACK_RECEIVE
    case LWS_CALLBACK_RECEIVE:
    {
      pstrBuffers[0]->append((char *)in, len);
      if (lws_remaining_packet_payload(wsi) == 0 && lws_is_final_fragment(wsi))
      {
        thread threadRequest(&Websocket::request, this, *connIter, new Json(*pstrBuffers[0]));
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
    if (ssClose.str().empty())
    {
      ssClose.str("");
      ssClose << strPrefix << " [WS_CLOSED];  Closed for an unknown reason.";
    }
    log(ssClose.str());
  }

  return nResult;
}
// }}}
}
}
