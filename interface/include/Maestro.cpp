// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Maestro.cpp
// author     : Ben Kietzman
// begin      : 2026-10-07
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Maestro"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Maestro()
Maestro::Maestro(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), void (*pCallbackInotify)(string, const string, const string)) : Interface(strPrefix, "maestro", argc, argv, pCallback)
{
  map<string, list<string> > watches;

  // {{{ functions
  m_functions["action"] = &Maestro::action;
  m_functions["compositionAdd"] = &Maestro::compositionAdd;
  m_functions["compositionRemove"] = &Maestro::compositionRemove;
  m_functions["compositions"] = &Maestro::compositions;
  m_functions["status"] = &Maestro::status;
  // }}}
  m_c = NULL;
  load(strPrefix, true);
  watches[m_strData + "/maestro"] = {"compositions.json"};
  m_pThreadInotify = new thread(&Maestro::inotify, this, strPrefix, watches, pCallbackInotify);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
}
// }}}
// {{{ ~Maestro()
Maestro::~Maestro()
{
  m_pThreadInotify->join();
  delete m_ptThreadInotify;
  delete m_c;
}
// }}}
// {{{ autoMode()
void Maestro::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->Maestro::autoMode()";
  if (strOldMaster != strNewMaster)
  {
    stringstream ssMessage;
    ssMessage << strPrefix << " [" << strNewMaster << "]:  Updated master.";
    log(ssMessage.str());
  }
  threadDecrement();
}
// }}}
// {{{ callback()
void Maestro::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Maestro::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (dep({"Function"}, ptJson, strError))
  {
    string strFunction = ptJson->m["Function"]->v;
    radialUser d;
    userInit(ptJson, d);
    if (m_functions.find(strFunction) != m_functions.end())
    {
      if ((this->*m_functions[strFunction])(d, strError))
      {
        bResult = true;
      }
    }
    else
    {
      strError = "Please provide a valid Function.";
    }
    if (bResult)
    {
      if (ptJson->exist({"Response"}))
      {
        delete ptJson->m["Response"];
      }
      ptJson->m["Response"] = d.p->m["o"];
      d.p->m.erase("o");
    }
    userDeinit(d);
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
// {{{ callbackInotify()
void Maestro::callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Maestro::callbackInotify()";
  if (strPath == (m_strData + "/maestro") &&  strFile == "config.json")
  {
    load(strPrefix);
  }
}
// }}}
// {{{ compositionAdd()
bool Maestro::compositionAdd(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (isValid(d))
  {
    if (dep({"Name"}, i, e))
    {
      m_mutex.lock();
      if (!compositionExist(i->m["Name"]->v))
      {
        b = true;
        m_c->m[i->m["Name"]->v] = new Json;
        m_c->m[i->m["Name"]->v]->m["Owners"];
        m_c->m[i->m["Name"]->v]->m["Owners"]->pb(d.u);
      }
      else
      {
        e = "Composition already exists.";
      }
      m_mutex.unlock();
      if (b)
      {
        compositionsWrite(e);
      }
      if (i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        m_mutexShare.lock();
        for (auto &link : m_l)
        {
          if (link->interfaces.find("maestro") != link->interfaces.end())
          {
            nodes.push_back(link->strNode);
          }
        }
        m_mutexShare.unlock();
        while (!nodes.empty())
        {
          Json *ptLink = new Json(d.r);
          ptLink->i("Interface", "maestro");
          ptLink->i("Node", nodes.front());
          ptLink->i("Function", "compositionAdd");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          if (hub("link", ptLink, e))
          {
            b = true;
          }
          delete ptLink;
          nodes.pop_front();
        }
      }
    }
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ compositionExist()
bool Maestro::compositionExist(const string strName)
{
  return compositions->exist({strName});
}
// }}}
// {{{ compositionOwner()
bool Maestro::compositionOwner(const string strName, const string strOwner)
{
  bool b = false;

  if (compositionExist(strName))
  {
    if (m_compositions->m[strName]->exist({"Owners"}))
    {
      for (auto i = m_compositions->m[strName]->m["Owners"]->l.begin(); !b && i != m_compositions->m[strName]->m["Owners"]->l.end(); i++)
      {
        if ((*i)->v == d.u)
        {
          b = true;
        }
      }
    }
  }

  return b;
}
// }}}
// {{{ compositionRemove()
bool Maestro::compositionRemove(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Name"}, i, e))
  {
    m_mutex.lock();
    if (compositionExist(i->m["Name"]->v))
    {
      if (compositionOwner(i->m["Name"]->v, d.u))
      {
        b = true;
        delete m_compositions->m[i->m["Name"]->v];
        m_compositions->m.erase(i->m["Name"]->v);
      }
      else
      {
        e = "You are not authorized to perform this action.";
      }
    }
    else
    {
      e = "Composition not found.";
    }
    m_mutex.unlock();
    if (b)
    {
      compositionsWrite(e);
    }
    if (i->val({"_broadcast"}) != "1")
    {
      list<string> nodes;
      m_mutexShare.lock();
      for (auto &link : m_l)
      {
        if (link->interfaces.find("maestro") != link->interfaces.end())
        {
          nodes.push_back(link->strNode);
        }
      }
      m_mutexShare.unlock();
      while (!nodes.empty())
      {
        Json *ptLink = new Json(d.r);
        ptLink->i("Interface", "maestro");
        ptLink->i("Node", nodes.front());
        ptLink->i("Function", "compositionRemove");
        ptLink->m["Request"]->i("_broadcast", "1", '1');
        if (hub("link", ptLink, e))
        {
          b = true;
        }
        delete ptLink;
        nodes.pop_front();
      }
    }
  }

  return b;
}
// }}}
// {{{ compositions()
bool Maestro::compositions(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (isValid(d))
  {
    b = true;
    m_mutex.lock();
    for (auto &composition : m_compositions)
    {
      if (composition->exist({"Owners"}))
      {
        bool bOwner = false;
        for (auto c = composition->m["Owners"]->l
        o->pb(composition);
      }
    }
    m_mutex.unlock();
  }
  else
  {
    e = "You are not authorized to perform this action.";
  }

  return b;
}
// }}}
// {{{ compositionsWrite()
bool Maestro::compositionsWrite(string &e)
{
  bool b = false;
  ofstream outCompositions;
  stringstream ssMessage, ssNew, ssOld:

  ssNew << m_strData << "/maestro/compositions_new.json";
  ssOld << m_strData << "/maestro/compositions.json";
  m_mutex.lock();
  outCompositions.open(ssNew.str());
  if (outCompositions)
  {
    b = true;
    outCompositions << m_compositions << endl;
  }
  else
  {
    ssMessage.str("");
    ssMesssage << "ofstream::open(" << errno << ") " << strerror(errno);
    e = ssMessage.str();
  }
  outCompositions.close();
  if (b)
  {
    if (!m_file.rename(ssNew.str(), ssOld.str()))
    {
      b = false;
      ssMessage.str("");
      ssMesssage << "File::rename(" << errno << ") " << strerror(errno);
      e = ssMessage.str();
    }
  }
  m_mutex.unlock();

  return b;
}
// }}}
// {{{ load()
void Maestro::load(string strPrefix, const bool bSilent)
{
  ifstream inCompositions;
  stringstream ssCompositions, ssMessage;

  strPrefix += "->Maestro::load()";
  ssCompositions << m_strData << "/maestro/compositions.json";
  inCompositions.open(ssCompositions.str());
  ssCompositions.str("");
  if (inCompositions)
  {
    string strLine;
    while (getline(inCompositions, strLine))
    {
      ssCompositions << strLine;
    }
  }
  else if (!bSilent)
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->ifstream::open(" << errno << ") error [" << m_strData << "/maestro/compositions.json]:  " << strerror(errno);
    log(ssMessage.str());
  }
  inCompositions.close();
  m_mutex.lock();
  if (m_compositions != NULL)
  {
    delete m_compositions;
  }
  m_compositions = new Json(ssCompositions.str());
  m_mutex.unlock();
}
// }}}
}
}
