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
  m_functions["config"] = &Builder::config;
  m_functions["install"] = &Builder::install;
  m_functions["status"] = &Builder::status;
	m_functions["uninstall"] = &Builder::uninstall;
  // }}}
  // {{{ packages
  m_packages["apt"] = &Builder::pkgApt;
  m_packages["common"] = &Builder::pkgCommon;
  m_packages["dir"] = &Builder::pkgDir;
  m_packages["logger"] = &Builder::pkgLogger;
  m_packages["mjson"] = &Builder::pkgMjson;
  m_packages["portconcentrator"] = &Builder::pkgPortConcentrator;
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
// {{{ cmdApt()
bool Builder::cmdApt(const string ws, string &s, const string p, list<string> &q, string &e, const bool a)
{
  bool b = false;
  stringstream c;

  c << "apt " << ((a)?"install":"remove") << " -y " << p;
  if (a)
  {
    if (send(ws, s, "apt update -y", q, e) && send(ws, s, c.str(), q, e))
    {
      b = true;
    }
  }
  else if (send(ws, s, c.str(), q, e) && send(ws, s, "apt autoremove -y", q, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ cmdCd()
bool Builder::cmdCd(const string ws, string &s, const string p, list<string> &q, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "cd \"" << p << "\"";
  if (send(ws, s, c.str(), q, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ cmdChgrp()
bool Builder::cmdChgrp(const string ws, string &s, const string p, const string g, list<string> &q, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "chgrp" << ((r)?" -R":"") << " " << g << " \"" << p << "\"";
  if (send(ws, s, c.str(), q, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ cmdChmod()
bool Builder::cmdChmod(const string ws, string &s, const string p, const string m, list<string> &q, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "chmod" << ((r)?" -R":"") << " " << m << " \"" << p << "\"";
  if (send(ws, s, c.str(), q, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ cmdChown()
bool Builder::cmdChown(const string ws, string &s, const string p, const string u, list<string> &q, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "chown" << ((r)?" -R":"") << " " << u << " \"" << p << "\"";
  if (send(ws, s, c.str(), q, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ cmdChsh()
bool Builder::cmdChsh(const string ws, string &s, const string u, const string i, list<string> &q, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "chsh -s " << i << " " << u;
  if (send(ws, s, c.str(), q, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ cmdDir()
bool Builder::cmdDir(const string ws, string &s, const string p, list<string> &q, string &e)
{
  bool b = false;
  stringstream c;

  c << "ls -d \"" << p << "\"";
  if (send(ws, s, c.str(), q, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ cmdExit()
bool Builder::cmdExit(const string ws, string &s, list<string> &q, string &e)
{
  bool b = false;

  if (send(ws, s, "exit", q, e))
  {
    string strLast = last(q.back());
    if (strLast == "logout")
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
bool Builder::cmdMkdir(const string ws, string &s, const string p, list<string> &q, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "mkdir" << ((r)?" -p":"") << " \"" << p << "\"";
  if (send(ws, s, c.str(), q, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ cmdRm()
bool Builder::cmdRm(const string ws, string &s, const string p, list<string> &q, string &e, const bool r)
{
  bool b = false;
  stringstream c;

  c << "rm" << ((r)?" -r":"") << " \"" << p << "\"";
  if (send(ws, s, c.str(), q, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ cmdSudo()
bool Builder::cmdSudo(const string ws, string &s, const string c, list<string> &q, string &e)
{
  bool b = false;
  size_t p;
  string d, v;

  if (sshSend(s, c+"\n", d, e))
  {
    v = strip(d);
    if (!ws.empty())
    {
      live(ws, {{"Action", "terminal"}, {"Data", v}});
    }
    if (!q.empty() && (p = q.back().rfind("\n")) != string::npos && (p+1) < q.back().size())
    {
      v.insert(0, q.back().substr(p+1));
      q.back().erase(p);
    }
    q.push_back(v);
    if (send(ws, s, "PS1='RADIAL_BUILDER> '", q, e))
    {
      b = true;
    }
  }

  return b;
}
// }}}
// }}}
// {{{ config()
bool Builder::config(radialUser &u, string &e)
{
  Json *o;

  m_mutex.lock();
  u.p->i("o", m_c);
  m_mutex.unlock();
  o = u.p->m["o"];
  o->i("publickey", m_strPublicKey);
  o->i("sudo", m_strSudo);
  o->i("user", m_strUser);

  return true;
}
// }}}
// {{{ confPkg()
bool Builder::confPkg(const string p, Json *c, string &e)
{
  bool b = false;

  m_mutex.lock();
  if (m_c != NULL)
  {
    if (exist(m_c, "packages"))
    {
      if (exist(m_c->m["packages"], p))
      {
        b = true;
        c->merge(m_c->m["packages"]->m[p], true, false);
      }
      else
      {
        e = (string)"The " + p + " package configuration does not exist.";
      }
    }
    else
    {
      e = "The packages configuration does not exist.";
    }
  }
  else
  {
    e = "The configuration does not exist.";
  }
  m_mutex.unlock();

  return b;
}
// }}}
// {{{ connect()
bool Builder::connect(const string ws, const string strServer, const string strPort, const string strUser, const string strPassword, const string strPrivateKey, string &s, list<string> &q, string &e)
{
  bool b = false;
  string d, v;

  if (sshConnect(strServer, strPort, strUser, strPassword, strPrivateKey, s, d, e))
  {
    v = strip(d);
    if (!ws.empty())
    {
      live(ws, {{"Action", "terminal"}, {"Data", v}});
    }
    q.push_back(v);
    if (send(ws, s, "PS1='RADIAL_BUILDER> '", q, e))
    {
      b = true;
    }
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
// {{{ disconnect()
bool Builder::disconnect(const string ws, string &s, string &e)
{
  bool b = false;

  if (s.empty() || sshDisconnect(s, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ init()
void Builder::init(radialUser &u, string &strUser, string &strPassword, string &strPrivateKey, string &strSudo)
{
  Json *i = u.p->m["i"];

  strPassword = m_strPassword;
  if (!empty(i, "Password"))
  {
    strPassword = i->m["Password"]->v;
  }
  strPrivateKey = m_strPrivateKey;
  if (exist(i, "PrivateKey"))
  {
    strPrivateKey = i->m["PrivateKey"]->v;
  }
  strSudo = m_strSudo;
  if (!empty(i, "Sudo"))
  {
    strSudo = i->m["Sudo"]->v;
  }
  strUser = m_strUser;
  if (!empty(i, "User"))
  {
    strUser = i->m["User"]->v;
  }
}
// }}}
// {{{ install()
bool Builder::install(radialUser &u, string &e)
{
  bool b = false;
  string ws;
  Json *i = u.p->m["i"], *o = u.p->m["o"];

  if (!empty(u.r, "wsRequestID"))
  {
    ws = u.r->m["wsRequestID"]->v;
  }
  if (dep({"Package", "Server"}, i, e))
  {
    list<string> q;
    string p = i->m["Package"]->v, s, strPassword, strPort, strPrivateKey, strServer = i->m["Server"]->v, strSudo, strUser;
    init(u, strUser, strPassword, strPrivateKey, strSudo);
    if (!empty(i, "Port"))
    {
      strPort = i->m["Port"]->v;
    }
    if (!ws.empty())
    {
      live(ws, {{"Action", "section"}, {"Section", "Establish connection."}});
    }
    if (connect(ws, strServer, strPort, strUser, strPassword, strPrivateKey, s, q, e))
    {
      string se;
      if (!ws.empty())
      {
        live(ws, {{"Action", "section"}, {"Section", "Switch to authorized user."}});
      }
      if (cmdSudo(ws, s, strSudo, q, e))
      {
        if (pkg(ws, p, s, q, e, true))
        {
          b = true;
        }
        if (!ws.empty())
        {
          live(ws, {{"Action", "section"}, {"Section", "Exit from authorized user."}});
        }
        cmdExit(ws, s, q, se);
      }
      if (!ws.empty())
      {
        live(ws, {{"Action", "section"}, {"Section", "Terminate connection."}});
      }
      cmdExit(ws, s, q, se);
      disconnect(ws, s, se);
    }
    if (!q.empty())
    {
      o->i("Terminal", q);
    }
  }

  return b;
}
// }}}
// {{{ last()
string Builder::last(const string d)
{
  stack<string> a;
  string i, l;
  stringstream s(d);

  while (getline(s, i))
  {
    a.push(i);
  }
  if (a.size() >= 2)
  {
    a.pop();
    l = a.top();
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
  ssConf << m_strData << "/builder/config.json";
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
    ssMessage << strPrefix << "->ifstream::open(" << errno << ") error [" << m_strData << "/builder/config.json]:  " << strerror(errno);
    log(ssMessage.str());
  }
  inConf.close();
}
// }}}
// {{{ pkg
// {{{ pkg()
bool Builder::pkg(const string ws, string p, string &s, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string sp;
  Json *c = new Json;

  if (!ws.empty())
  {
    stringstream ss;
    ss << ((a)?"I":"Uni") << "nstall package:  " << p << ".";
    live(ws, {{"Action", "section"}, {"Section", ss.str()}});
  }
  if (confPkg(p, c, e))
  {
    if (!empty(c, "pkg"))
    {
      sp = c->m["pkg"]->v;
    }
    if (m_packages.find(sp) != m_packages.end())
    {
      if (a)
      {
        b = true;
        if (exist(c, "dependencies"))
        {
          queue<string> d;
          for (auto &i : c->m["dependencies"]->l)
          {
            d.push(i->v);
          }
          while (b && !d.empty())
          {
            if (!pkg(ws, d.front(), s, q, e, a))
            {
              b = false;
            }
            d.pop();
          }
        }
        if (b && !(this->*m_packages[sp])(ws, s, c, q, e, a))
        {
          b = false;
        }
      }
      else
      {
        Json *r = NULL;
        m_mutex.lock();
        if (m_c != NULL)
        {
          if (exist(m_c, "packages"))
          {
            r = new Json(m_c->m["packages"]);
          }
          else
          {
            e = "The packages configuration does not exist.";
          }
        }
        else
        {
          e = "The configuration does not exist.";
        }
        m_mutex.unlock();
        if (r != NULL)
        {
          b = true;
          for (auto i = r->m.begin(); b && i != r->m.end(); i++)
          {
            if (exist(i->second, "dependencies"))
            {
              bool f = false;
              for (auto j = i->second->m["dependencies"]->l.begin(); !f && j != i->second->m["dependencies"]->l.end(); j++)
              {
                if ((*j)->v == p)
                {
                  f = true;
                }
              }
              if (f && !pkg(ws, i->first, s, q, e, a))
              {
                b = false;
              }
            }
          }
          if (b && !(this->*m_packages[sp])(ws, s, c, q, e, a))
          {
            b = false;
          }
          delete r;
        }
      }
    }
    else
    {
      e = (string)"[" + sp + "] Please provide a valid Package.";
    }
  }
  delete c;

  return b;
}
// }}}
// {{{ pkgApt()
bool Builder::pkgApt(const string ws, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;

  if (dep({"aptpkg"}, c, e))
  {
    string p = c->m["aptpkg"]->v;
    if (cmdApt(ws, s, p, q, e, a))
    {
      b = true;
    }
  }

  return b;
}
// }}}
// {{{ pkgCommon()
bool Builder::pkgCommon(const string ws, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;

  if (dep({"source"}, c, e))
  {
    if (a)
    {
      if (cmdDir(ws, s, c->m["source"]->v, q, e) || ((empty(c, "proxy") || send(ws, s, (string)"git config --global http.proxy " + c->m["proxy"]->v, q, e)) && send(ws, s, "git clone https://github.com/benkietzman/common.git", q, e) && cmdCd(ws, s, c->m["source"]->v, q, e) && send(ws, s, "./configure", q, e) && send(ws, s, "make", q, e) && (empty(c, "user") || cmdChown(ws, s, c->m["source"]->v, c->m["user"]->v, q, e, true)) && (empty(c, "group") || cmdChgrp(ws, s, c->m["source"]->v, c->m["group"]->v, q, e, true))))
      {
        b = true;
      }
    }
    else if (cmdDir(ws, s, c->m["source"]->v, q, e))
    {
      if (cmdRm(ws, s, c->m["source"]->v, q, e, true))
      {
        b = true;
      }
    }
    else if (e.find("No such file or directory") != string::npos)
    {
      b = true;
    }
  }

  return b;
}
// }}}
// {{{ pkgDir()
bool Builder::pkgDir(const string ws, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;

  if (dep({"path"}, c, e))
  {
    string p = c->m["path"]->v;
    if (cmdDir(ws, s, p, q, e))
    {
      if (a || cmdRm(ws, s, p, q, e, true))
      {
        b = true;
      }
    }
    else if (e.find("No such file or directory") != string::npos && (!a || (cmdMkdir(ws, s, p, q, e) && (empty(c, "user") || cmdChown(ws, s, p, c->m["user"]->v, q, e)) && (empty(c, "group") || cmdChgrp(ws, s, p, c->m["group"]->v, q, e)) && (empty(c, "mode") || cmdChmod(ws, s, p, c->m["mode"]->v, q, e)))))
    {
      b = true;
    }
  }

  return b;
}
// }}}
// {{{ pkgLogger()
bool Builder::pkgLogger(const string ws, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;

  if (dep({"cert", "data", "email", "key", "source"}, c, e))
  {
    if (a)
    {
      if ((cmdDir(ws, s, c->m["source"]->v, q, e) || ((empty(c, "proxy") || send(ws, s, (string)"git config --global http.proxy " + c->m["proxy"]->v, q, e)) && send(ws, s, "git clone https://github.com/benkietzman/logger.git", q, e) && cmdCd(ws, s, c->m["source"]->v, q, e) && send(ws, s, "make install", q, e) && (empty(c, "user") || cmdChown(ws, s, c->m["source"]->v, c->m["user"]->v, q, e, true)) && (empty(c, "group") || cmdChgrp(ws, s, c->m["source"]->v, c->m["group"]->v, q, e, true)))) && (cmdDir(ws, s, c->m["data"]->v, q, e) || (cmdMkdir(ws, s, c->m["data"]->v, q, e) && cmdCd(ws, s, c->m["data"]->v, q, e) && cmdMkdir(ws, s, "storage", q, e) && send(ws, s, (string)"ln -s '" + c->m["cert"]->v + "' server.crt", q, e) && send(ws, s, (string)"ln -s '" + c->m["key"]->v + "' server.key", q, e) && (empty(c, "user") || cmdChown(ws, s, c->m["data"]->v, c->m["user"]->v, q, e, true)) && (empty(c, "group") || cmdChgrp(ws, s, c->m["data"]->v, c->m["group"]->v, q, e, true)))) && (send(ws, s, (string)"sed -i 's/logger\\/logger/logger\\/logger --email=" + c->m["email"]->v + (string)"/g' /lib/systemd/system/logger.service", q, e) && send(ws, s, "systemctl enable logger", q, e) && send(ws, s, "systemctl start logger", q, e)))
      {
        b = true;
      }
    }
    else if (cmdDir(ws, s, c->m["source"]->v, q, e))
    {
      if (send(ws, s, "systemctl stop logger", q, e) && send(ws, s, "systemctl disable logger", q, e) && cmdRm(ws, s, "/lib/systemd/system/logger.service", q, e), (!cmdDir(ws, s, "/usr/local/logger", q, e) || cmdRm(ws, s, "/usr/local/logger", q, e, true)) && (!cmdDir(ws, s, c->m["data"]->v, q, e) || cmdRm(ws, s, c->m["data"]->v, q, e, true)) && cmdRm(ws, s, c->m["source"]->v, q, e, true))
      {
        b = true;
      }
    }
    else if (e.find("No such file or directory") != string::npos)
    {
      b = true;
    }
  }

  return b;
}
// }}}
// {{{ pkgMjson()
bool Builder::pkgMjson(const string ws, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;

  if (a)
  {
    if (cmdDir(ws, s, "/usr/local/include/mjson-1.7", q, e) || ((empty(c, "proxy") || send(ws, s, (string)"export http_proxy=" + c->m["proxy"]->v + (string)" https_proxy=" + c->m["proxy"]->v, q, e)) && send(ws, s, "wget https://downloads.sourceforge.net/project/mjson/mjson/mjson-1.7.0.tar.gz", q, e) && send(ws, s, "tar -xvf mjson-1.7.0.tar.gz", q, e) && cmdRm(ws, s, "mjson-1.7.0.tar.gz", q, e) && cmdCd(ws, s, "json-1.7.0", q, e) && send(ws, s, "./configure", q, e) && send(ws, s, "make install", q, e) && send(ws, s, (string)"cd ..", q, e) && cmdRm(ws, s, "json-1.7.0", q, e, true)))
    {
      b = true;
    }
  }
  else if (cmdDir(ws, s, "/usr/local/include/mjson-1.7", q, e))
  {
    if (cmdRm(ws, s, "/usr/local/include/mjson-1.7", q, e, true) && send(ws, s, "rm /usr/local/lib/libmjson*", q, e))
    {
      b = true;
    }
  }
  else if (e.find("No such file or directory") != string::npos)
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ pkgPortConcentrator()
bool Builder::pkgPortConcentrator(const string ws, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;

  if (dep({"data", "email", "source"}, c, e))
  {
    if (a)
    {
      if ((cmdDir(ws, s, c->m["source"]->v, q, e) || ((empty(c, "proxy") || send(ws, s, (string)"git config --global http.proxy " + c->m["proxy"]->v, q, e)) && send(ws, s, "git clone https://github.com/benkietzman/portconcentrator.git", q, e) && cmdCd(ws, s, c->m["source"]->v, q, e) && send(ws, s, "make install", q, e) && (empty(c, "user") || cmdChown(ws, s, c->m["source"]->v, c->m["user"]->v, q, e, true)) && (empty(c, "group") || cmdChgrp(ws, s, c->m["source"]->v, c->m["group"]->v, q, e, true)))) && (cmdDir(ws, s, c->m["data"]->v, q, e) || (cmdMkdir(ws, s, c->m["data"]->v, q, e) && (empty(c, "user") || cmdChown(ws, s, c->m["data"]->v, c->m["user"]->v, q, e, true)) && (empty(c, "group") || cmdChgrp(ws, s, c->m["data"]->v, c->m["group"]->v, q, e, true)))) && (send(ws, s, (string)"sed -i 's/portconcentrator\\/concentrator/portconcentrator\\/concentrator --email=" + c->m["email"]->v + (string)"/g' /lib/systemd/system/concentrator.service", q, e) && send(ws, s, "systemctl enable concentrator", q, e) && send(ws, s, "systemctl start concentrator", q, e)))
      {
        b = true;
      }
    }
    else if (cmdDir(ws, s, c->m["source"]->v, q, e))
    {
      if (send(ws, s, "systemctl stop concentrator", q, e) && send(ws, s, "systemctl disable concentrator", q, e) && cmdRm(ws, s, "/lib/systemd/system/concentrator.service", q, e), (!cmdDir(ws, s, "/usr/local/portconcentrator", q, e) || cmdRm(ws, s, "/usr/local/portconcentrator", q, e, true)) && (!cmdDir(ws, s, c->m["data"]->v, q, e) || cmdRm(ws, s, c->m["data"]->v, q, e, true)) && cmdRm(ws, s, c->m["source"]->v, q, e, true))
      {
        b = true;
      }
    }
    else if (e.find("No such file or directory") != string::npos)
    {
      b = true;
    }
  }

  return b;
}
// }}}
// }}}
// {{{ send()
bool Builder::send(const string ws, string &s, const string c, list<string> &q, string &e, const size_t r)
{
  bool b = false, f = false;
  size_t p;
  string d, sd, v;

  if (sshSend(s, (c+"\n"), d, e))
  {
    b = true;
    if ((p = d.rfind("RADIAL_BUILDER> ")) != string::npos)
    {
      f = true;
      d[p+6] = '-';
    }
    while (!f && sshSend(s, "", sd, e))
    {
      if (!sd.empty())
      {
        v = strip(sd);
        if (!ws.empty())
        {
          live(ws, {{"Action", "terminal"}, {"Data", v}});
        }
        d.append(v);
        sd.clear();
        if ((p = d.rfind("RADIAL_BUILDER> ")) != string::npos)
        {
          f = true;
          d[p+6] = '-';
        }
      }
    }
    v = strip(d);
    if (!ws.empty())
    {
      live(ws, {{"Action", "terminal"}, {"Data", v}});
    }
    if (!q.empty() && (p = q.back().rfind("\n")) != string::npos && (p+1) < q.back().size())
    {
      v.insert(0, q.back().substr(p+1));
      q.back().erase(p);
    }
    q.push_back(v);
    if (!s.empty())
    {
      string j;
      b = false;
      if (sshSend(s, "echo $?\n", j, e))
      {
        size_t n;
        stringstream k;
        k.str(last(strip(j)));
        k >> n;
        if (n == r)
        {
          b = true;
        }
        else
        {
          e = last(v);
        }
      }
    }
  }

  return b;
}
// }}}
// {{{ strip()
string Builder::strip(const string v)
{
  list<string> q;
  size_t p;
  string l, r, t;
  stringstream s;
  regex ansi_escape_regex(R"(\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~]))");

  t = regex_replace(v, ansi_escape_regex, "");
  while ((p = t.find('\007')) != string::npos)
  {
    t.erase(p, 1);
  }
  while ((p = t.find('\033')) != string::npos)
  {
    t.erase(p, 1);
  }
  while ((p = t.find("\r")) != string::npos)
  {
    t.erase(p, 1);
  }
  s.str(t);
  while (getline(s, l))
  {
    q.push_back(l);
  }
  if (!q.empty() && q.back().size() >= 2 && q.back().substr(0, 2) == "0;")
  {
    q.back().erase(0, 2);
  }
  while (!q.empty())
  {
    r.append(q.front() + ((q.size() > 1)?"\n":""));
    q.pop_front();
  }

  return r;
}
// }}}
// {{{ uninstall()
bool Builder::uninstall(radialUser &u, string &e)
{
  bool b = false;
  string ws;
  Json *i = u.p->m["i"], *o = u.p->m["o"];

  if (!empty(u.r, "wsRequestID"))
  {
    ws = u.r->m["wsRequestID"]->v;
  }
  if (dep({"Package", "Server"}, i, e))
  {
    list<string> q;
    string p = i->m["Package"]->v, s, strPassword, strPort = "22", strPrivateKey, strServer = i->m["Server"]->v, strSudo, strUser;
    if (!empty(i, "Port"))
    {
      strPort = i->m["Port"]->v;
    }
    init(u, strUser, strPassword, strPrivateKey, strSudo);
    if (!ws.empty())
    {
      live(ws, {{"Action", "section"}, {"Section", "Establish connection."}});
    }
    if (connect(ws, strServer, strPort, strUser, strPassword, strPrivateKey, s, q, e))
    {
      string se;
      if (!ws.empty())
      {
        live(ws, {{"Action", "section"}, {"Section", "Switch to authorized user."}});
      }
      if (cmdSudo(ws, s, strSudo, q, e))
      {
        if (pkg(ws, p, s, q, e, false))
        {
          b = true;
        }
        if (!ws.empty())
        {
          live(ws, {{"Action", "section"}, {"Section", "Exit from authorized user."}});
        }
        cmdExit(ws, s, q, se);
      }
      if (!ws.empty())
      {
        live(ws, {{"Action", "section"}, {"Section", "Terminate connection."}});
      }
      cmdExit(ws, s, q, se);
      disconnect(ws, s, se);
    }
    if (!q.empty())
    {
      o->i("Terminal", q);
    }
  }

  return b;
}
// }}}
}
}
