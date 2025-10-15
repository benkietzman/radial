// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Builder.cpp
// author     : Ben Kietzman
// begin      : 2025-10-13
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Builder"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Builder()
Builder::Builder(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), void (*pCallbackInotify)(string, const string, const string)) : Interface(strPrefix, "builder", argc, argv, pCallback)
{
  map<string, list<string> > watches;

  // {{{ functions
  m_functions["action"] = &Builder::action;
  m_functions["construct"] = &Builder::construct;
  m_functions["destruct"] = &Builder::destruct;
  m_functions["install"] = &Builder::install;
  m_functions["remove"] = &Builder::remove;
  m_functions["status"] = &Builder::status;
  // }}}
  // {{{ packages
  m_packages["src"] = &Builder::pkgSrc;
  // }}}
  m_c = NULL;
  cred(strPrefix, true);
  load(strPrefix, true);
  watches[m_strData] = {".cred"};
  watches[m_strData + "/builder"] = {"config.json"};
  m_pThreadInotify = new thread(&Builder::inotify, this, strPrefix, watches, pCallbackInotify);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
}
// }}}
// {{{ ~Builder()
Builder::~Builder()
{
}
// }}}
// {{{ callback()
void Builder::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Builder::callback()";
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
void Builder::callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Builder::callbackInotify()";
  if (strPath == m_strData)
  {
    if (strFile == ".cred")
    {
      cred(strPrefix);
    }
  }
  else if (strPath == (m_strData + "/builder"))
  {
    if (strFile == "config.json")
    {
      load(strPrefix);
    }
  }
}
// }}}
// {{{ cmd
// {{{ cmdChgrp()
bool Builder::cmdChgrp(string &s, const string p, const string g, string &d, string &e, const bool r)
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
// {{{ cmdChmod()
bool Builder::cmdChmod(string &s, const string p, const string m, string &d, string &e, const bool r)
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
// {{{ cmdChown()
bool Builder::cmdChown(string &s, const string p, const string u, string &d, string &e, const bool r)
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
// {{{ cmdChsh()
bool Builder::cmdChsh(string &s, const string u, const string i, string &d, string &e, const bool r)
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
// {{{ cmdDir()
bool Builder::cmdDir(string &s, const string p, string &d, string &e)
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
// {{{ cmdMkdir()
bool Builder::cmdMkdir(string &s, const string p, string &d, string &e, const bool r)
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
// {{{ cmdRmdir()
bool Builder::cmdRmdir(string &s, const string p, string &d, string &e, const bool r)
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
// {{{ cmdSudo()
bool Builder::cmdSudo(string &s, const string c, string &d, string &e)
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
// }}}
// {{{ confPkg()
bool Builder::confPkg(const string p, Json *c, string &e)
{
  bool b = false;

chat("#radial", "Builder::confPkg() 0");
  m_mutex.lock();
chat("#radial", "Builder::confPkg() 1");
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
chat("#radial", "Builder::confPkg() 2");
  m_mutex.unlock();
chat("#radial", "Builder::confPkg() 3");

  return b;
}
// }}}
// {{{ construct()
bool Builder::construct(radialUser &u, string &e)
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
void Builder::cred(string strPrefix, const bool bSilent)
{
  string strError;
  stringstream ssMessage;
  Json *ptCred = new Json;

  strPrefix += "->Builder::cred()";
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"builder"}, ptCred, strError))
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
    ssMessage << strPrefix << "->Warden::vaultRetrieve() error [builder]:  " << strError;
    log(ssMessage.str());
  }
  delete ptCred;
}
// }}}
// {{{ destruct()
bool Builder::destruct(radialUser &u, string &e)
{
  bool b = false;
  Json *i = u.p->m["i"];

  if (dep({"Server"}, i, e))
  {
  }

  return b;
}
// }}}
// {{{ init()
void Builder::init(radialUser &u, string &strUser, string &strPassword, string &strPrivateKey, string &strSudo)
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
bool Builder::install(radialUser &u, string &e)
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
chat("#radial", "Builder::install() 0");
      if (sshConnect(strServer, strPort, strUser, strPassword, strPrivateKey, s, d, e))
      {
chat("#radial", "Builder::install() 0-0");
        //if (sudo(s, strSudo, d, e) && (this->*m_packages[strPackage])(s, u, d, e, true))
        if ((this->*m_packages[strPackage])(s, u, d, e, true))
        {
chat("#radial", "Builder::install() 0-0-0");
          b = true;
        }
chat("#radial", "Builder::install() 0-1");
        sshDisconnect(s, e);
chat("#radial", "Builder::install() 0-2");
      }
chat("#radial", "Builder::install() 1-0");
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
string Builder::last(const string d)
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
void Builder::load(string strPrefix, const bool bSilent)
{
  ifstream inConf;
  stringstream ssConf, ssMessage;

  strPrefix += "->Builder::load()";
  ssConf << m_strData << "/builder.json";
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
    ssMessage << strPrefix << "->ifstream::open(" << errno << ") error [" << m_strData << "/builder.json]:  " << strerror(errno);
    log(ssMessage.str());
  }
  inConf.close();
}
// }}}
// {{{ pkg
// {{{ pkgSrc()
bool Builder::pkgSrc(string &s, radialUser &u, string &d, string &e, const bool a)
{
  bool b = false;
  Json *c = new Json;

chat("#radial", "Builder::pkgSrc() 0");
  if (confPkg("src", c, e))
  {
chat("#radial", "Builder::pkgSrc() 0-0");
    if (cmdDir(s, "/src", d, e))
    {
chat("#radial", "Builder::pkgSrc() 0-0-0");
      if (a || cmdRmdir(s, "/src", d, e))
      {
        b = true;
      }
chat("#radial", "Builder::pkgSrc() 0-0-1");
    }
    else if (e.find("No such file or directory") != string::npos && (!a || (cmdMkdir(s, "/src", d, e) && (empty(c, "user") || cmdChown(s, "/src", c->m["user"]->v, d, e)) && (empty(c, "group") || cmdChgrp(s, "/src", c->m["group"]->v, d, e)) && cmdChmod(s, "/src", "770", d, e) && cmdChmod(s, "/src", "g+s", d, e))))
    {
chat("#radial", "Builder::pkgSrc() 0-0-2");
      b = true;
    }
chat("#radial", "Builder::pkgSrc() 0-1");
  }
chat("#radial", "Builder::pkgSrc() 1");
  delete c;

  return b;
}
// }}}
// }}}
// {{{ remove()
bool Builder::remove(radialUser &u, string &e)
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
        if (cmdSudo(s, strSudo, d, e) && (this->*m_packages[strPackage])(s, u, d, e, false))
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
}
}
