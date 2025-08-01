// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Storage.cpp
// author     : Ben Kietzman
// begin      : 2022-04-27
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Storage"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Storage()
Storage::Storage(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "storage", argc, argv, pCallback)
{
  m_bInitialized = false;
}
// }}}
// {{{ ~Storage()
Storage::~Storage()
{
}
// }}}
// {{{ autoMode()
void Storage::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->Storage::autoMode()";
  if (strOldMaster != strNewMaster)
  {
    stringstream ssMessage;
    ssMessage << strPrefix << " [" << strNewMaster << "]:  Updated master.";
    log(ssMessage.str());
  }
  if (!m_bInitialized)
  {
    mutexInitialize.lock();
    if (!m_bInitialized && !strNewMaster.empty() && m_strNode != strNewMaster)
    {
      string strError;
      stringstream ssMessage;
      Json *ptJson = new Json;
      ptJson->i("Interface", "storage");
      ptJson->i("Function", "retrieve");
      ptJson->i("Node", strNewMaster);
      ptJson->m["Keys"] = new Json;
      if (hub("link", ptJson, strError))
      {
        Json *ptData;
        ssMessage.str("");
        ssMessage << strPrefix << "->hub(link,storage,retrieve) [" << strNewMaster << "]:  Retrieved initial storage.";
        log(ssMessage.str());
        if (exist(ptJson, "Response"))
        {
          ptData = new Json(ptJson->m["Response"]);
        }
        else
        {
          ptData = new Json;
        }
        if (m_storage.add({}, ptData, strError))
        {
          m_bInitialized = true;
          ssMessage.str("");
          ssMessage << strPrefix << "->Storage::add():  Initialized storage.";
          log(ssMessage.str());
        }
        else
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->Storage::add():  " << strError;
          log(ssMessage.str());
        }
        delete ptData;
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->hub(link,storage,retrieve) error [" << strNewMaster << "]:  " << strError;
        log(ssMessage.str());
      }
      delete ptJson;
    }
    mutexInitialize.unlock();
  }
  threadDecrement();
}
// }}}
// {{{ callback()
void Storage::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Storage::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    list<string> keys;
    Json *ptData = NULL;
    if (exist(ptJson, "Keys"))
    {
      for (auto &ptKey : ptJson->m["Keys"]->l)
      {
        keys.push_back(ptKey->v);
      }
    }
    if (ptJson->m["Function"]->v == "add" || ptJson->m["Function"]->v == "update")
    {
      if (exist(ptJson, "Request"))
      {
        ptData = new Json(ptJson->m["Request"]);
      }
      else
      {
        ptData = new Json;
      }
    }
    else if (ptJson->m["Function"]->v == "retrieve" || ptJson->m["Function"]->v == "retrieveKeys")
    {
      ptData = new Json;
    }
    if (m_storage.request(ptJson->m["Function"]->v, keys, ptData, strError))
    {
      bResult = true;
    }
    if (ptData != NULL)
    {
      if (ptJson->m["Function"]->v == "add" || ptJson->m["Function"]->v == "update")
      {
        delete ptData;
      }
      else
      {
        if (exist(ptJson, "Response"))
        {
          delete ptJson->m["Response"];
        }
        ptJson->m["Response"] = ptData;
      }
    }
    if (bResult && (ptJson->m["Function"]->v == "add" || ptJson->m["Function"]->v == "remove" || ptJson->m["Function"]->v == "update") && (!exist(ptJson, "Broadcast") || ptJson->m["Broadcast"]->v == "1"))
    {
      Json *ptLink = new Json;
      ptLink->i("Interface", "storage");
      ptLink->i("Function", ptJson->m["Function"]->v);
      ptLink->i("Broadcast", "0", '0');
      ptLink->i("Keys", keys);
      if (ptJson->m["Function"]->v != "remove" && exist(ptJson, "Request"))
      {
        ptLink->m["Request"] = new Json(ptJson->m["Request"]);
      }
      hub("link", ptLink, false);
      delete ptLink;
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
}
// }}}
}
}
