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
  m_functions["publickKey"] = &Builder::publickKey;
  m_functions["status"] = &Builder::status;
	m_functions["uninstall"] = &Builder::uninstall;
  // }}}
  // {{{ packages
  m_packages["apt"] = &Builder::pkgApt;
  m_packages["certificates"] = &Builder::pkgCertificates;
  m_packages["common"] = &Builder::pkgCommon;
  m_packages["dir"] = &Builder::pkgDir;
  m_packages["logger"] = &Builder::pkgLogger;
  m_packages["mjson"] = &Builder::pkgMjson;
  m_packages["portconcentrator"] = &Builder::pkgPortConcentrator;
  m_packages["radial"] = &Builder::pkgRadial;
  m_packages["servicejunction"] = &Builder::pkgServiceJunction;
  m_packages["warden"] = &Builder::pkgWarden;
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
bool Builder::cmdApt(const string ws, string &s, const string p, list<string> &q, string &e, const bool a, const string x)
{
  bool b = false;
  stringstream c;

  c << "apt " << ((a)?"install":"remove") << " -y " << p;
  if (a)
  {
    if ((!x.empty() && cmdExist(ws, s, x, q, e)) || (send(ws, s, "apt update -y", q, e) && send(ws, s, c.str(), q, e)))
    {
      b = true;
    }
  }
  else if ((!x.empty() && !cmdExist(ws, s, x, q, e)) || (send(ws, s, c.str(), q, e) && send(ws, s, "apt autoremove -y", q, e)))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ cmdCd()
bool Builder::cmdCd(const string ws, string &s, const string p, list<string> &q, string &e, const bool r)
{
  stringstream c;

  c << "cd '" << p << "'";

  return send(ws, s, c.str(), q, e);
}
// }}}
// {{{ cmdChmod()
bool Builder::cmdChmod(const string ws, string &s, const string p, const string m, list<string> &q, string &e, const bool r)
{
  stringstream c;

  c << "chmod" << ((r)?" -R":"") << " " << m << " '" << p << "'";

  return send(ws, s, c.str(), q, e);
}
// }}}
// {{{ cmdChown()
bool Builder::cmdChown(const string ws, string &s, const string p, const string u, const string g, list<string> &q, string &e, const bool r)
{
  stringstream c;

  c << "chown" << ((r)?" -R":"") << " " << u << ":" << g << " '" << p << "'";

  return (cmdUser(ws, s, u, g, q, e, true) && send(ws, s, c.str(), q, e));
}
// }}}
// {{{ cmdChsh()
bool Builder::cmdChsh(const string ws, string &s, const string u, const string i, list<string> &q, string &e, const bool r)
{
  stringstream c;

  c << "chsh -s " << i << " " << u;

  return send(ws, s, c.str(), q, e);
}
// }}}
// {{{ cmdExist()
bool Builder::cmdExist(const string ws, string &s, const string p, list<string> &q, string &e)
{
  stringstream c;

  c << "ls -d '" << p << "'";

  return send(ws, s, c.str(), q, e);
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
// {{{ cmdGit()
bool Builder::cmdGit(const string ws, string &s, const string strRepository, const string strDirectory, list<string> &q, string &e, const string strProxy)
{
  stringstream c;

  c << "git clone " << strRepository << " " << strDirectory;

  return ((strProxy.empty() || send(ws, s, (string)"git config --global http.proxy " + strProxy, q, e)) && send(ws, s, c.str(), q, e));
}
// }}}
// {{{ cmdGroups()
bool Builder::cmdGroups(const string ws, string &s, const string u, Json *ptGroups, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string g;
  stringstream c;

  for (auto &i : ptGroups->l)
  {
    if (send(ws, s, (string)"getent group " + i->v, q, e))
    {
      if (!g.empty() && send(ws, s, (string)"getent group " + i->v, q, e))
      {
        g += ",";
      }
      g += i->v;
    }
  }
  c << "usermod -" << ((a)?"a":"r") << "G " << g << " " << u;
  if (send(ws, s, c.str(), q, e))
  {
    b = true;
  }

  return b;
}
// }}}
// {{{ cmdMkdir()
bool Builder::cmdMkdir(const string ws, string &s, const string p, list<string> &q, string &e, const bool r)
{
  stringstream c;

  c << "mkdir" << ((r)?" -p":"") << " '" << p << "'";

  return send(ws, s, c.str(), q, e);
}
// }}}
// {{{ cmdRm()
bool Builder::cmdRm(const string ws, string &s, const string p, list<string> &q, string &e, const bool r)
{
  stringstream c;

  c << "rm" << ((r)?" -r":"") << " '" << p << "'";

  return send(ws, s, c.str(), q, e);
}
// }}}
// {{{ cmdScp()
bool Builder::cmdScp(const string ws, string &s, const string strSource, const string strTarget, list<string> &q, string &e)
{
  stringstream c;

  c << "scp -o StrictHostKeyChecking=no -r " << strSource << " " << strTarget;

  return send(ws, s, c.str(), q, e);
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
    live(ws, {{"Action", "terminal"}, {"Data", v}});
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
// {{{ cmdUser()
bool Builder::cmdUser(const string ws, string &s, const string u, const string g, list<string> &q, string &e, const bool a)
{
  bool b = false;

  if (a)
  {
    string strGroup = u;
    if (!g.empty())
    {
      strGroup = g;
    }
    if ((send(ws, s, (string)"getent group " + strGroup, q, e) || send(ws, s, (string)"groupadd " + strGroup, q, e)) && (send(ws, s, (string)"id --user " + u, q, e) || (e.find("no such user") != string::npos && send(ws, s, (string)"useradd --create-home --gid " + strGroup + (string)" " + u, q, e))))
    {
      b = true;
    }
  }
  else if (send(ws, s, (string)"id --user " + u, q, e))
  {
    if (send(ws, s, (string)"userdel " + u, q, e))
    {
      b = true;
    }
  }
  else if (e.find("no such user") != string::npos)
  {
    b = true;
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
    live(ws, {{"Action", "terminal"}, {"Data", v}});
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
    string p = i->m["Package"]->v, s, strPassword, strPort = get(i, "Port", "22"), strPrivateKey, strServer = i->m["Server"]->v, strSudo, strUser;
    init(u, strUser, strPassword, strPrivateKey, strSudo);
    chat("#builder", "Establishing connection...");
    live(u, {{"Action", "section"}, {"Section", (string)"Establishing connection..."}});
    if (connect(ws, strServer, strPort, strUser, strPassword, strPrivateKey, s, q, e))
    {
      string se;
      chat("#builder", "Switching to authorized user...");
      live(u, {{"Action", "section"}, {"Section", "Switching to authorized user..."}});
      if (cmdSudo(ws, s, strSudo, q, e))
      {
        if (pkg(u, p, s, q, e, true))
        {
          b = true;
        }
        chat("#builder", "Exiting from  authorized user...");
        live(u, {{"Action", "section"}, {"Section", "Exiting from authorized user..."}});
        cmdExit(ws, s, q, se);
      }
      chat("#builder", "Terminating connection...");
      live(u, {{"Action", "section"}, {"Section", "Terminating connection..."}});
      cmdExit(ws, s, q, se);
      disconnect(ws, s, se);
    }
    if (!q.empty())
    {
      o->i("Terminal", q);
    }
    if (!ws.empty())
    {
      chat("#builder", ((b)?"Processing completed.":e));
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
// {{{ live()
void Builder::live(const string ws, map<string, string> message)
{
  Interface::live(ws, message);
}
void Builder::live(radialUser &u, map<string, string> message)
{
  if (!empty(u.r, "wsRequestID"))
  {
    live(u.r->m["wsRequestID"]->v, message);
  }
}
// }}}
// {{{ pkg
// {{{ pkg()
bool Builder::pkg(radialUser &u, string p, string &s, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string sp = p;
  Json *c = new Json;

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
            if (!pkg(u, d.front(), s, q, e, a))
            {
              b = false;
            }
            d.pop();
          }
        }
        if (b)
        {
          chat("#builder", (string)"Installing " + p + " package...");
          live(u, {{"Action", "section"}, {"Section", (string)"Installing " + p + " package..."}});
          if (!(this->*m_packages[sp])(u, s, c, q, e, a))
          {
            b = false;
          }
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
              if (f && !pkg(u, i->first, s, q, e, a))
              {
                b = false;
              }
            }
          }
          if (b)
          {
            chat("#builder", (string)"Uninstalling " + p + " package...");
            live(u, {{"Action", "section"}, {"Section", (string)"Uninstalling " + p + " package..."}});
            if (!(this->*m_packages[sp])(u, s, c, q, e, a))
            {
              b = false;
            }
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
bool Builder::pkgApt(radialUser &u, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string ws = get(u.r, "wsRequestID");

  if (dep({"aptpkg"}, c, e))
  {
    string p = c->m["aptpkg"]->v;
    if (cmdApt(ws, s, p, q, e, a, get(c, "exist")))
    {
      b = true;
    }
  }

  return b;
}
// }}}
// {{{ pkgCertificates()
bool Builder::pkgCertificates(radialUser &u, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string ws = get(u.r, "wsRequestID");
  Json *i = u.p->m["i"];

  if (dep({"Server"}, i, e) && dep({"directory", "path"}, c, e))
  {
    string strPort = "22", strServer;
    if (exist(c, "master"))
    {
      if (!empty(c, "master"))
      {
        strServer = c->m["master"]->v;
      }
      else if (!c->m["master"]->m.empty() && !empty(c->m["master"], "server"))
      {
        strPort = get(c->m["master"], "port", strPort);
        strServer = c->m["master"]->m["server"]->v;
      }
    }
    if (!strServer.empty())
    {
      if (i->m["Server"]->v != strServer)
      {
        list<string> sq;
        string strPassword, strPrivateKey, strSession, strSudo, strUser;
        init(u, strUser, strPassword, strPrivateKey, strSudo);
        if (connect(ws, strServer, strPort, strUser, strPassword, strPrivateKey, strSession, sq, e))
        {
          string se;
          if (cmdSudo(ws, strSession, strSudo, sq, e))
          {
            if (a)
            {
              if (cmdExist(ws, strSession, c->m["path"]->v + (string)"/" + c->m["directory"]->v, sq, e))
              {
                if (cmdExist(ws, strSession, (string)"/etc/cron.daily/certificates_" + i->m["Server"]->v, sq, e) || (e.find("No such file or directory") != string::npos && send(ws, strSession, (string)"echo '#!/bin/sh\nset -e\nrsync -amvze \"ssh -o StrictHostKeyChecking=no\" --delete " + c->m["path"]->v + (string)"/" + c->m["directory"]->v + (string)" " + i->m["Server"]->v + (string)":" + c->m["path"]->v + (string)" >/root/certificates_" + i->m["Server"]->v + (string)".log 2>&1' > /etc/cron.daily/certificates_" + i->m["Server"]->v, sq, e) && cmdChmod(ws, strSession, (string)"/etc/cron.daily/certificates_" + i->m["Server"]->v, "755", sq, e)))
                {
                  if (cmdExist(ws, s, c->m["path"]->v + (string)"/" + c->m["directory"]->v, q, e) || (e.find("No such file or directory") != string::npos && send(ws, strSession, "/etc/cron.daily/certificates_" + i->m["Server"]->v, sq, e)))
                  {
                    b = true;
                  }
                }
              }
            }
            else if (!cmdExist(ws, strSession, (string)"/etc/cron.daily/certificates_" + i->m["Server"]->v, sq, e) || cmdRm(ws, strSession, (string)"/etc/cron.daily/certificates_" + i->m["Server"]->v, sq, e))
            {
              if (!cmdExist(ws, s, c->m["path"]->v + (string)"/" + c->m["directory"]->v, q, e) || cmdRm(ws, s, c->m["path"]->v + (string)"/" + c->m["directory"]->v, q, e, true))
              {
                b = true;
              }
            }
            cmdExit(ws, strSession, sq, se);
          }
          cmdExit(ws, strSession, sq, se);
          disconnect(ws, strSession, se);
        }
      }
      else
      {
        b = true;
      }
    }
    else
    {
      e = "Please provide the master.";
    }
  }

  return b;
}
// }}}
// {{{ pkgCommon()
bool Builder::pkgCommon(radialUser &u, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string ws = get(u.r, "wsRequestID");

  if (dep({"git", "source"}, c, e))
  {
    if (a)
    {
      if (cmdExist(ws, s, c->m["source"]->v, q, e) || (e.find("No such file or directory") != string::npos && cmdGit(ws, s, c->m["git"]->v, c->m["source"]->v, q, e, get(c, "proxy")) && cmdCd(ws, s, c->m["source"]->v, q, e) && send(ws, s, "./configure", q, e) && send(ws, s, "make", q, e) && (empty(c, "user") || empty(c, "group") || cmdChown(ws, s, c->m["source"]->v, c->m["user"]->v, c->m["group"]->v, q, e, true))))
      {
        b = true;
      }
    }
    else if (cmdExist(ws, s, c->m["source"]->v, q, e))
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
bool Builder::pkgDir(radialUser &u, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string ws = get(u.r, "wsRequestID");

  if (dep({"path"}, c, e))
  {
    string p = c->m["path"]->v;
    if (cmdExist(ws, s, p, q, e))
    {
      if (a || cmdRm(ws, s, p, q, e, true))
      {
        b = true;
      }
    }
    else if (e.find("No such file or directory") != string::npos && (!a || (cmdMkdir(ws, s, p, q, e) && (empty(c, "user") || empty(c, "group") || cmdChown(ws, s, p, c->m["user"]->v, c->m["group"]->v, q, e)) && (empty(c, "mode") || cmdChmod(ws, s, p, c->m["mode"]->v, q, e)))))
    {
      b = true;
    }
  }

  return b;
}
// }}}
// {{{ pkgLogger()
bool Builder::pkgLogger(radialUser &u, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string ws = get(u.r, "wsRequestID"), v;

  if (dep({"cert", "data", "email", "git", "key", "source", "user"}, c, e))
  {
    if (a)
    {
      if (exist(c, "warden") && !empty(c->m["warden"], "socket") && exist(c->m["warden"], "vault"))
      {
        if (cmdExist(ws, s, c->m["source"]->v, q, e) || (e.find("No such file or directory") != string::npos && cmdGit(ws, s, c->m["git"]->v, c->m["source"]->v, q, e, get(c, "proxy")) && cmdCd(ws, s, c->m["source"]->v, q, e) && send(ws, s, "make install", q, e) && send(ws, s, "make clean", q, e) && (empty(c, "group") || cmdChown(ws, s, c->m["source"]->v, c->m["user"]->v, c->m["group"]->v, q, e, true)) && (!exist(c, "groups") || cmdGroups(ws, s, c->m["user"]->v, c->m["groups"], q, e, true))))
        {
          if (cmdExist(ws, s, c->m["data"]->v, q, e) || (e.find("No such file or directory") != string::npos && cmdMkdir(ws, s, c->m["data"]->v, q, e) && cmdCd(ws, s, c->m["data"]->v, q, e) && cmdMkdir(ws, s, "storage", q, e) && send(ws, s, (string)"ln -s '" + c->m["cert"]->v + "' server.crt", q, e) && send(ws, s, (string)"ln -s '" + c->m["key"]->v + "' server.key", q, e) && (empty(c, "group") || cmdChown(ws, s, c->m["data"]->v, c->m["user"]->v, c->m["group"]->v, q, e, true))))
          {
            if (send(ws, s, (string)"/usr/local/warden/vault export " + c->m["warden"]->m["socket"]->v + " Logger", q, e) || (send(ws, s, (string)"echo '" + c->m["warden"]->m["vault"]->j(v) + "' > warden.json", q, e) && send(ws, s, "/usr/local/warden/vault import " + c->m["warden"]->m["socket"]->v + " Logger warden.json", q, e) && cmdRm(ws, s, "warden.json", q, e)))
            {
              if (send(ws, s, (string)"sed -i 's/logger\\/logger/logger\\/logger --email=" + c->m["email"]->v + (string)"/g' /lib/systemd/system/logger.service", q, e) && send(ws, s, (string)"sed -i 's/^User=logger/User=" + c->m["user"]->v + "/g' /lib/systemd/system/logger.service", q, e) && send(ws, s, "systemctl enable logger", q, e) && send(ws, s, "systemctl start logger", q, e))
              {
                b = true;
              }
            }
          }
        }
      }
      else
      {
        e = "Please provide the warden.";
      }
    }
    else if (cmdExist(ws, s, c->m["source"]->v, q, e))
    {
      if (send(ws, s, "systemctl stop logger", q, e) && send(ws, s, "systemctl disable logger", q, e) && cmdRm(ws, s, "/lib/systemd/system/logger.service", q, e))
      {
        if (!cmdExist(ws, s, "/usr/local/logger", q, e) || cmdRm(ws, s, "/usr/local/logger", q, e, true))
        {
          if (!cmdExist(ws, s, c->m["data"]->v, q, e) || cmdRm(ws, s, c->m["data"]->v, q, e, true))
          {
            if (cmdRm(ws, s, c->m["source"]->v, q, e, true))
            {
              if (!exist(c, "groups") || cmdGroups(ws, s, c->m["user"]->v, c->m["groups"], q, e, false))
              {
                if (empty(c, "group") || cmdUser(ws, s, c->m["user"]->v, c->m["group"]->v, q, e, false))
                {
                  b = true;
                }
              }
            }
          }
        }
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
bool Builder::pkgMjson(radialUser &u, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string ws = get(u.r, "wsRequestID");

  if (a)
  {
    if (cmdExist(ws, s, "/usr/local/include/mjson-1.7", q, e) || (e.find("No such file or directory") != string::npos && (empty(c, "proxy") || send(ws, s, (string)"export http_proxy=" + c->m["proxy"]->v + (string)" https_proxy=" + c->m["proxy"]->v, q, e)) && send(ws, s, "wget https://downloads.sourceforge.net/project/mjson/mjson/mjson-1.7.0.tar.gz", q, e) && send(ws, s, "tar -xvf mjson-1.7.0.tar.gz", q, e) && cmdRm(ws, s, "mjson-1.7.0.tar.gz", q, e) && cmdCd(ws, s, "json-1.7.0", q, e) && send(ws, s, "./configure", q, e) && send(ws, s, "make install", q, e) && send(ws, s, (string)"cd ..", q, e) && cmdRm(ws, s, "json-1.7.0", q, e, true)))
    {
      b = true;
    }
  }
  else if (cmdExist(ws, s, "/usr/local/include/mjson-1.7", q, e))
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
bool Builder::pkgPortConcentrator(radialUser &u, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string ws = get(u.r, "wsRequestID");

  if (dep({"data", "email", "git", "source", "user"}, c, e))
  {
    if (a)
    {
      if (cmdExist(ws, s, c->m["source"]->v, q, e) || (e.find("No such file or directory") != string::npos && cmdGit(ws, s, c->m["git"]->v, c->m["source"]->v, q, e, get(c, "proxy")) && cmdCd(ws, s, c->m["source"]->v, q, e) && send(ws, s, "make install", q, e) && send(ws, s, "make clean", q, e) && (empty(c, "group") || cmdChown(ws, s, c->m["source"]->v, c->m["user"]->v, c->m["group"]->v, q, e, true)) && (!exist(c, "groups") || cmdGroups(ws, s, c->m["user"]->v, c->m["groups"], q, e, true))))
      {
        if (cmdExist(ws, s, c->m["data"]->v, q, e) || (e.find("No such file or directory") != string::npos && cmdMkdir(ws, s, c->m["data"]->v, q, e) && (empty(c, "group") || cmdChown(ws, s, c->m["data"]->v, c->m["user"]->v, c->m["group"]->v, q, e, true))))
        {
          if (send(ws, s, (string)"sed -i 's/portconcentrator\\/concentrator/portconcentrator\\/concentrator --email=" + c->m["email"]->v + (string)"/g' /lib/systemd/system/concentrator.service", q, e) && send(ws, s, (string)"sed -i 's/^User=concentrator/User=" + c->m["user"]->v + "/g' /lib/systemd/system/concentrator.service", q, e) && send(ws, s, "systemctl enable concentrator", q, e) && send(ws, s, "systemctl start concentrator", q, e))
          {
            b = true;
          }
        }
      }
    }
    else if (cmdExist(ws, s, c->m["source"]->v, q, e))
    {
      if (send(ws, s, "systemctl stop concentrator", q, e) && send(ws, s, "systemctl disable concentrator", q, e) && cmdRm(ws, s, "/lib/systemd/system/concentrator.service", q, e))
      {
        if (!cmdExist(ws, s, "/usr/local/portconcentrator", q, e) || cmdRm(ws, s, "/usr/local/portconcentrator", q, e, true))
        {
          if (!cmdExist(ws, s, c->m["data"]->v, q, e) || cmdRm(ws, s, c->m["data"]->v, q, e, true))
          {
            if (cmdRm(ws, s, c->m["source"]->v, q, e, true))
            {
              if (!exist(c, "groups") || cmdGroups(ws, s, c->m["user"]->v, c->m["groups"], q, e, false))
              {
                if (empty(c, "group") || cmdUser(ws, s, c->m["user"]->v, c->m["group"]->v, q, e, false))
                {
                  b = true;
                }
              }
            }
          }
        }
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
// {{{ pkgRadial()
bool Builder::pkgRadial(radialUser &u, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string ws = get(u.r, "wsRequestID");

  b = true;

  return b;
}
// }}}
// {{{ pkgServiceJunction()
bool Builder::pkgServiceJunction(radialUser &u, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string ws = get(u.r, "wsRequestID");

  b = true;

  return b;
}
// }}}
// {{{ pkgWarden()
bool Builder::pkgWarden(radialUser &u, string &s, Json *c, list<string> &q, string &e, const bool a)
{
  bool b = false;
  string ws = get(u.r, "wsRequestID");

  if (dep({"data", "email", "git", "secret", "source", "user"}, c, e))
  {
    if (a)
    {
      if (cmdExist(ws, s, c->m["source"]->v, q, e) || (e.find("No such file or directory") != string::npos && cmdGit(ws, s, c->m["git"]->v, c->m["source"]->v, q, e, get(c, "proxy")) && cmdCd(ws, s, c->m["source"]->v, q, e) && send(ws, s, "make install", q, e) && send(ws, s, "make clean", q, e) && (empty(c, "group") || cmdChown(ws, s, c->m["source"]->v, c->m["user"]->v, c->m["group"]->v, q, e, true)) && (!exist(c, "groups") || cmdGroups(ws, s, c->m["user"]->v, c->m["groups"], q, e, true))))
      {
        if (cmdExist(ws, s, c->m["data"]->v, q, e) || (e.find("No such file or directory") != string::npos && cmdMkdir(ws, s, c->m["data"]->v, q, e) && cmdCd(ws, s, c->m["data"]->v, q, e) && (empty(c, "secret") || (cmdMkdir(ws, s, "vault", q, e) && send(ws, s, (string)"echo '" + c->m["secret"]->v + (string)"' > vault/.secret", q, e) && cmdChmod(ws, s, "vault/.secret", "600", q, e) && send(ws, s, "touch vault/storage", q, e) && cmdChmod(ws, s, "vault/storage", "600", q, e))) && (empty(c, "group") || cmdChown(ws, s, c->m["data"]->v, c->m["user"]->v, c->m["group"]->v, q, e, true))))
        {
          if (send(ws, s, (string)"sed -i 's/warden\\/warden/warden\\/warden --email=" + c->m["email"]->v + (string)"/g' /lib/systemd/system/warden.service", q, e) && send(ws, s, (string)"sed -i 's/^User=warden/User=" + c->m["user"]->v + "/g' /lib/systemd/system/warden.service", q, e) && send(ws, s, "systemctl enable warden", q, e) && send(ws, s, "systemctl start warden", q, e))
          {
            b = true;
          }
        }
      }
    }
    else if (cmdExist(ws, s, c->m["source"]->v, q, e))
    {
      if (send(ws, s, "systemctl stop warden", q, e) && send(ws, s, "systemctl disable warden", q, e) && cmdRm(ws, s, "/lib/systemd/system/warden.service", q, e))
      {
        if (!cmdExist(ws, s, "/usr/local/warden", q, e) || cmdRm(ws, s, "/usr/local/warden", q, e, true))
        {
          if (!cmdExist(ws, s, c->m["data"]->v, q, e) || cmdRm(ws, s, c->m["data"]->v, q, e, true))
          {
            if (cmdRm(ws, s, c->m["source"]->v, q, e, true))
            {
              if (!exist(c, "groups") || cmdGroups(ws, s, c->m["user"]->v, c->m["groups"], q, e, false))
              {
                if (empty(c, "group") || cmdUser(ws, s, c->m["user"]->v, c->m["group"]->v, q, e, false))
                {
                  b = true;
                }
              }
            }
          }
        }
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
// {{{ publickKey()
bool Builder::publickKey(radialUser &u, string &e)
{
  u.p->m["o"]->i("PublickKey", m_strPublicKey);

  return true;
}
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
        live(ws, {{"Action", "terminal"}, {"Data", v}});
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
    live(ws, {{"Action", "terminal"}, {"Data", v}});
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
  string ws = get(u.r, "wsRequestID");
  Json *i = u.p->m["i"], *o = u.p->m["o"];

  if (dep({"Package", "Server"}, i, e))
  {
    list<string> q;
    string p = i->m["Package"]->v, s, strPassword, strPort = get(i, "Port", "22"), strPrivateKey, strServer = i->m["Server"]->v, strSudo, strUser;
    init(u, strUser, strPassword, strPrivateKey, strSudo);
    chat("#builder", "Establishing connection...");
    live(u, {{"Action", "section"}, {"Section", "Establishing connection..."}});
    if (connect(ws, strServer, strPort, strUser, strPassword, strPrivateKey, s, q, e))
    {
      string se;
      chat("#builder", "Switching to authorized user...");
      live(u, {{"Action", "section"}, {"Section", "Switching to authorized user..."}});
      if (cmdSudo(ws, s, strSudo, q, e))
      {
        if (pkg(u, p, s, q, e, false))
        {
          b = true;
        }
        chat("#builder", "Exiting from authorized user...");
        live(u, {{"Action", "section"}, {"Section", "Exiting from authorized user..."}});
        cmdExit(ws, s, q, se);
      }
      chat("#builder", "Terminating connection...");
      live(u, {{"Action", "section"}, {"Section", "Terminating connection..."}});
      cmdExit(ws, s, q, se);
      disconnect(ws, s, se);
    }
    if (!q.empty())
    {
      o->i("Terminal", q);
    }
    if (!ws.empty())
    {
      chat("#builder", ((b)?"Processing completed.":e));
    }
  }

  return b;
}
// }}}
}
}
