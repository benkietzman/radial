// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Build.cpp
// author     : Ben Kietzman
// begin      : 2025-10-13
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Build"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Build()
Build::Build(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), void (*pCallbackInotify)(string, const string, const string)) : Interface(strPrefix, "build", argc, argv, pCallback)
{
  map<string, list<string> > watches;

  // {{{ functions
  m_functions["action"] = &Build::action;
  m_functions["construct"] = &Build::construct;
  m_functions["destruct"] = &Build::destruct;
  m_functions["install"] = &Build::install;
  m_functions["remove"] = &Build::remove;
  m_functions["status"] = &Build::status;
  // }}}
  // {{{ packages
  m_packages["src"] = &Build::pkgSrc;
  // }}}
  m_c = NULL;
  cred(strPrefix, true);
  load(strPrefix, true);
  watches[m_strData] = {".cred"};
  watches[m_strData + "/build"] = {"config.json"};
  m_pThreadInotify = new thread(&Build::inotify, this, strPrefix, watches, pCallbackInotify);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
}
// }}}
// {{{ ~Build()
Build::~Build()
{
}
// }}}
// {{{ callback()
void Build::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Build::callback()";
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
      if (exist(ptJson, "Response"))
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
void Build::callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Build::callbackInotify()";
  if (strPath == m_strData)
  {
    if (strFile == ".cred")
    {
      cred(strPrefix);
    }
  }
  else if (strPath == (m_strData + "/build"))
  {
    if (strFile == "config.json")
    {
      load(strPrefix);
    }
  }
}
// }}}
// {{{ chgrp()
bool Build::chgrp(string &s, const string p, const string g, string &d, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "chown" << ((r)?" -R":"") << " " << g << " \"" << p << "\"" << endl;
  if (sshSend(s, c.str(), d, e))
  {
    string strLast = last(d);
    if (strLast.size() <= 7 || strLast.substr(0, 7) != "chgrp: ")
    {
      b = true;
    }
    else
    {
      e = strLast;
    }
  }

  return b;
}
// }}}
// {{{ chmod()
bool Build::chmod(string &s, const string p, const string m, string &d, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "chmod" << ((r)?" -R":"") << " " << m << " \"" << p << "\"" << endl;
  if (sshSend(s, c.str(), d, e))
  {
    string strLast = last(d);
    if (strLast.size() <= 7 || strLast.substr(0, 7) != "chmod: ")
    {
      b = true;
    }
    else
    {
      e = strLast;
    }
  }

  return b;
}
// }}}
// {{{ chown()
bool Build::chown(string &s, const string p, const string u, string &d, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "chown" << ((r)?" -R":"") << " " << u << " \"" << p << "\"" << endl;
  if (sshSend(s, c.str(), d, e))
  {
    string strLast = last(d);
    if (strLast.size() <= 7 || strLast.substr(0, 7) != "chown: ")
    {
      b = true;
    }
    else
    {
      e = strLast;
    }
  }

  return b;
}
// }}}
// {{{ chsh()
bool Build::chsh(string &s, const string u, const string i, string &d, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "chsh -s " << i << " " << u << endl;
  if (sshSend(s, c.str(), d, e))
  {
    string strLast = last(d);
    if (strLast.size() <= 6 || strLast.substr(0, 6) != "chsh: ")
    {
      b = true;
    }
    else
    {
      e = strLast;
    }
  }

  return b;
}
// }}}
// {{{ confPkg()
bool Build::confPkg(const string p, Json *c, string &e)
{
  bool b = false;

  m_mutex.lock();
  if (exist(m_c, "packages"))
  {
    if (exist(m_c->m["packages"], p))
    {
      b = true;
      c->merge(m_c->m["packages"]->m[p], true, false);
    }
    else
    {
      e = "The src package configuration does not exist.";
    }
  }
  else
  {
    e = "The packages configuration does not exist.";
  }
  m_mutex.unlock();

  return b;
}
// }}}
// {{{ construct()
bool Build::construct(radialUser &u, string &e)
{
  bool b = false;
  Json *i = u.p->m["i"];

  if (dep({"Server"}, i, e))
  {
  }

  return b;
}
// }}}
// {{{ cred()
void Build::cred(string strPrefix, const bool bSilent)
{
  string strError;
  stringstream ssMessage;
  Json *ptCred = new Json;

  strPrefix += "->Build::cred()";
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"build"}, ptCred, strError))
  {
    if (exist(ptCred, "key"))
    {
      if (!empty(ptCred->m["key"], "private"))
      {
        m_strPrivateKey = ptCred->m["key"]->m["private"]->v;
      }
      if (!empty(ptCred->m["key"], "public"))
      {
        m_strPublicKey = ptCred->m["key"]->m["public"]->v;
      }
    }
    if (!empty(ptCred, "password"))
    {
      m_strPassword = ptCred->m["password"]->v;
    }
    if (!empty(ptCred, "sudo"))
    {
      m_strSudo = ptCred->m["sudo"]->v;
    }
    if (!empty(ptCred, "user"))
    {
      m_strUser = ptCred->m["user"]->v;
    }
  }
  else if (!bSilent)
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->Warden::vaultRetrieve() error [build]:  " << strError;
    log(ssMessage.str());
  }
  delete ptCred;
}
// }}}
// {{{ destruct()
bool Build::destruct(radialUser &u, string &e)
{
  bool b = false;
  Json *i = u.p->m["i"];

  if (dep({"Server"}, i, e))
  {
  }

  return b;
}
// }}}
// {{{ dir()
bool Build::dir(string &s, const string p, string &d, string &e)
{
  bool b = false;
  stringstream c;

  c << "ls -d \"" << p << "\"" << endl;
  if (sshSend(s, c.str(), d, e))
  {
    string strLast = last(d);
    if (strLast.size() <= 4 || strLast.substr(0, 4) != "ls: ")
    {
      b = true;
    }
    else
    {
      e = strLast;
    }
  }

  return b;
}
// }}}
// {{{ init()
void Build::init(radialUser &u, string &strUser, string &strPassword, string &strPrivateKey, string &strSudo)
{
  Json *i = u.p->m["i"];

  strPassword = m_strPassword;
  strPrivateKey = m_strPrivateKey;
  strSudo = m_strSudo;
  strUser = m_strUser;
  if (exist(i, "default"))
  {
    if (exist(i->m["default"], "key"))
    {
      if (!empty(i->m["default"]->m["key"], "private"))
      {
        strPrivateKey = i->m["default"]->m["key"]->m["private"]->v;
      }
    }
    if (!empty(i->m["default"], "password"))
    {
      strPassword = i->m["default"]->m["password"]->v;
    }
    if (!empty(i->m["default"], "sudo"))
    {
      strSudo = i->m["default"]->m["sudo"]->v;
    }
    if (!empty(i->m["default"], "user"))
    {
      strUser = i->m["default"]->m["user"]->v;
    }
  }
}
// }}}
// {{{ install()
bool Build::install(radialUser &u, string &e)
{
  bool b = false;
  Json *i = u.p->m["i"];

  if (dep({"Package", "Server"}, i, e))
  {
    string strPackage = i->m["Package"]->v, strPort, strServer = i->m["Server"]->v;
    if (!empty(i, "Port"))
    {
      strPort = i->m["Port"]->v;
    }
    if (m_packages.find(strPackage) != m_packages.end())
    {
      string d, s, strPassword, strPrivateKey, strSudo, strUser;
      init(u, strUser, strPassword, strPrivateKey, strSudo);
      if (sshConnect(strServer, strPort, strUser, strPassword, strPrivateKey, s, d, e))
      {
        //if (sudo(s, strSudo, d, e) && (this->*m_packages[strPackage])(s, u, d, e, true))
        if ((this->*m_packages[strPackage])(s, u, d, e, true))
        {
          b = true;
        }
        sshDisconnect(s, e);
      }
    }
    else
    {
      e = "Please provide a valid Package.";
    }
  }

  return b;
}
// }}}
// {{{ last()
string Build::last(const string d)
{
  queue<string> a;
  string i, l;
  stringstream s(d);

  while (getline(s, i))
  {
    a.push(i);
  }
  if (a.size() >= 2)
  {
    a.pop();
    l = a.back();
  }

  return l;
}
// }}}
// {{{ load()
void Build::load(string strPrefix, const bool bSilent)
{
  ifstream inConf;
  stringstream ssConf, ssMessage;

  strPrefix += "->Build::load()";
  ssConf << m_strData << "/build.json";
  inConf.open(ssConf.str());
  if (inConf)
  {
    string strLine;
    stringstream ssJson;
    while (getline(inConf, strLine))
    {
      ssJson << strLine;
    }
    m_mutex.lock();
    if (m_c != NULL)
    {
      delete m_c;
    }
    m_c = new Json(ssJson.str());
    m_mutex.unlock();
  }
  else if (!bSilent)
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->ifstream::open(" << errno << ") error [" << m_strData << "/build.json]:  " << strerror(errno);
    log(ssMessage.str());
  }
  inConf.close();
}
// }}}
// {{{ mkdir()
bool Build::mkdir(string &s, const string p, string &d, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "mkdir" << ((r)?" -p":"") << " \"" << p << "\"" << endl;
  if (sshSend(s, c.str(), d, e))
  {
    string strLast = last(d);
    if (strLast.size() <= 7 || strLast.substr(0, 7) != "mkdir: ")
    {
      b = true;
    }
    else
    {
      e = strLast;
    }
  }

  return b;
}
// }}}
// {{{ pkgSrc()
bool Build::pkgSrc(string &s, radialUser &u, string &d, string &e, const bool a)
{
  bool b = false;
  Json *c = new Json;

  if (confPkg("src", c, e))
  {
    if (dir(s, "/src", d, e))
    {
      if (a || rmdir(s, "/src", d, e))
      {
        b = true;
      }
    }
    else if (e.find("No such file or directory") != string::npos && (!a || (mkdir(s, "/src", d, e) && (empty(c, "user") || chown(s, "/src", c->m["user"]->v, d, e)) && (empty(c, "group") || chgrp(s, "/src", c->m["group"]->v, d, e)) && chmod(s, "/src", "770", d, e) && chmod(s, "/src", "g+s", d, e))))
    {
      b = true;
    }
  }
  delete c;

  return b;
}
// }}}
// {{{ remove()
bool Build::remove(radialUser &u, string &e)
{
  bool b = false;
  Json *i = u.p->m["i"];

  if (dep({"Package", "Port", "Server"}, i, e))
  {
    string strPackage = i->m["Package"]->v, strPort = "22", strServer = i->m["Server"]->v;
    if (!empty(i, "Port"))
    {
      strPort = i->m["Port"]->v;
    }
    if (m_packages.find(strPackage) != m_packages.end())
    {
      string d, s, strPassword, strPrivateKey, strSudo, strUser;
      init(u, strUser, strPassword, strPrivateKey, strSudo);
      if (sshConnect(strServer, strPort, strUser, strPassword, strPrivateKey, s, d, e))
      {
        if (sudo(s, strSudo, d, e) && (this->*m_packages[strPackage])(s, u, d, e, false))
        {
          b = true;
        }
        sshDisconnect(s, e);
      }
    }
    else
    {
      e = "Please provide a valid Package.";
    }
  }

  return b;
}
// }}}
// {{{ rmdir()
bool Build::rmdir(string &s, const string p, string &d, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "rmdir" << ((r)?" -r":"") << " \"" << p << "\"" << endl;
  if (sshSend(s, c.str(), d, e))
  {
    string strLast = last(d);
    if (strLast.size() <= 7 || strLast.substr(0, 7) != "rmdir: ")
    {
      b = true;
    }
    else
    {
      e = strLast;
    }
  }

  return b;
}
// }}}
// {{{ sudo()
bool Build::sudo(string &s, const string c, string &d, string &e)
{
  bool b = false;

  if (sshSend(s, c + "\n", d, e))
  {
    queue<string> a;
    string i;
    stringstream ss(d);
    while (getline(ss, i))
    {
      a.push(i);
    }
    if (!a.empty())
    {
      if (a.back().find("root@") != string::npos || a.back().find("# ") != string::npos)
      {
        b = true;
      }
      else
      {
        e = "Failed to become root.";
      }
    }
  }

  return b;
}
// }}}
}
}
