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
// {{{ maestroInterrupt()
int maestroInterrupt(JSRuntime *rt, void *opaque)
{
  bool *pbKill = (bool *)opaque;
  int nReturn = 0;  // 0 - keep running, 1 - kill script

  if (*pbKill)
  {
    nReturn = 1;
  }

  return nReturn;
}
// }}}
// {{{ Maestro()
Maestro::Maestro(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), void (*pCallbackInotify)(string, const string, const string)) : Interface(strPrefix, "maestro", argc, argv, pCallback)
{
  map<string, list<string> > watches;
  string strError;

  m_bLoaded = false;
  m_bLoadSchedules = false;
  m_bLoadServices = false;
  // {{{ accesses
  m_a["application"] = {"emulator"};
  m_a["command"] = {"all"};
  m_a["db"] = {"dbCentralAccountTypes", "dbCentralApplicationAccountAdd", "dbCentralApplicationAccountRemove", "dbCentralApplicationAccounts", "dbCentralApplicationAccountUpdate", "dbCentralApplicationAdd", "dbCentralApplicationDependAdd", "dbCentralApplicationDependRemove", "dbCentralApplicationDepends", "dbCentralApplicationGroupAdd", "dbCentralApplicationGroupRemove", "dbCentralApplicationGroups", "dbCentralApplicationIssueAdd", "dbCentralApplicationIssueCommentAdd", "dbCentralApplicationIssueComments", "dbCentralApplicationIssueCommentUpdate", "dbCentralApplicationIssues", "dbCentralApplicationIssueUpdate", "dbCentralApplicationRemove", "dbCentralApplicationRepoAdd", "dbCentralApplicationRepoRemove", "dbCentralApplicationRepos", "dbCentralApplicationRepoUpdate", "dbCentralApplications", "dbCentralApplicationServerAdd", "dbCentralApplicationServerDetailAdd", "dbCentralApplicationServerDetailRemove", "dbCentralApplicationServerDetails", "dbCentralApplicationServerDetailUpdate", "dbCentralApplicationServerRemove", "dbCentralApplicationServers", "dbCentralApplicationUpdate", "dbCentralApplicationUserAdd", "dbCentralApplicationUserRemove", "dbCentralApplicationUsers", "dbCentralApplicationUserUpdate", "dbCentralContactTypes", "dbCentralDependents", "dbCentralGroupAdd", "dbCentralGroupRemove", "dbCentralGroups", "dbCentralGroupUpdate", "dbCentralGroupUserAdd", "dbCentralGroupUserRemove", "dbCentralGroupUsers", "dbCentralGroupUserUpdate", "dbCentralLoginTypes", "dbCentralMenuAccesses", "dbCentralNotifyPriorities", "dbCentralPackageTypes", "dbCentralPhpSession", "dbCentralPhpSessionAdd", "dbCentralPhpSessionRemove", "dbCentralRepos", "dbCentralServerAdd", "dbCentralServerDetails", "dbCentralServerGroupAdd", "dbCentralServerGroupRemove", "dbCentralServerGroups", "dbCentralServerRemove", "dbCentralServers", "dbCentralServerUpdate", "dbCentralServerUserAdd", "dbCentralServerUserRemove", "dbCentralServerUsers", "dbCentralServerUserUpdate", "dbCentralUserAdd", "dbCentralUserPasskeyAdd", "dbCentralUserPasskeyRemove", "dbCentralUserPasskeys", "dbCentralUserPasskeyUpdate", "dbCentralUserReminderAdd", "dbCentralUserReminderRemove", "dbCentralUserReminders", "dbCentralUserReminderUpdate", "dbCentralUserRemove", "dbCentralUsers", "dbCentralUserUpdate"};
  m_a["dbquery"] = {"central_r", "feedback_r"};
  m_a["dbupdate"] = {"central", "feedback"};
  m_a["log"] = {"all"};
  m_a["logger"] = {"Radial"};
  m_a["notify"] = {"all"};
  m_a["sqlite"] = {"create", "drop", "query"};
  m_a["storage"] = {"add", "remove", "update"};
  // }}}
  // {{{ functions
  m_functions["accesses"] = &Maestro::accesses;
  m_functions["accessRemove"] = &Maestro::accessRemove;
  m_functions["accessRequest"] = &Maestro::accessRequest;
  m_functions["accessRequestApprove"] = &Maestro::accessRequestApprove;
  m_functions["accessRequestCancel"] = &Maestro::accessRequestCancel;
  m_functions["accessRequestDeny"] = &Maestro::accessRequestDeny;
  m_functions["action"] = &Maestro::action;
  m_functions["cores"] = &Maestro::cores;
  m_functions["flow"] = &Maestro::flow;
  m_functions["flowAdd"] = &Maestro::flowAdd;
  m_functions["flowExec"] = &Maestro::flowExec;
  m_functions["flowRemove"] = &Maestro::flowRemove;
  m_functions["flows"] = &Maestro::flows;
  m_functions["flowUpdate"] = &Maestro::flowUpdate;
  m_functions["kill"] = &Maestro::kill;
  m_functions["plan"] = &Maestro::plan;
  m_functions["planAdd"] = &Maestro::planAdd;
  m_functions["planRemove"] = &Maestro::planRemove;
  m_functions["plans"] = &Maestro::plans;
  m_functions["running"] = &Maestro::running;
  m_functions["schedule"] = &Maestro::schedule;
  m_functions["scheduleAdd"] = &Maestro::scheduleAdd;
  m_functions["scheduleRemove"] = &Maestro::scheduleRemove;
  m_functions["schedules"] = &Maestro::schedules;
  m_functions["scheduleUpdate"] = &Maestro::scheduleUpdate;
  m_functions["scopes"] = &Maestro::scopes;
  m_functions["status"] = &Maestro::status;
  // }}}
  // {{{ javascripts
  jsreg("alert", 3, "alert(user, message, html)", "Provides the ability to send a message to a given user via the Alert framework.");
  jsreg("application", 2, "application(application, message)", "Provides the ability to send a message to a given application that is connected to Radial.");
  jsreg("centralmon", 2, "centralmon(server, process)", "Provides the ability to retrieve server and/or process information from Central Monitor.");
  jsreg("chat", 3, "chat(target, message, source)", "Provides the ability to send a message to a given channel or user in Internet Relay Chat (IRC) by default or to a user in Live using a Source of live.");
  jsreg("command", 5, "command(command, arguments, input, timeout, node)", "Provides the ability to send a command to any Radial server.");
  jsreg("db", 2, "db(function, input)", "Provides the ability to perform a given database function.");
  jsreg("dbquery", 2, "dbquery(database, query)", "Provides the ability to query (SQL: select) a given database.");
  jsreg("dbupdate", 2, "dbupdate(database, update)", "Provides the ability to update (SQL: delete, insert, update) a given database.");
  jsreg("email", 8, "email(from, to, cc, bcc, subject, text, html, file)", "Provides the ability to send an email.");
  jsreg("esc", 1, "esc(value)", "Escapes a value for use in SQL.");
  jsreg("flow", 2, "flow(plan, flow, request)");
  jsreg("getError", 0, "getError()", "Provides the ability to retrieve the error from the last Maestro function.");
  jsreg("getRequest",  0, "getRequest()", "Provides the ability to retrieve the request being provides to the given flow.");
  jsreg("getResponse", 0, "getResponse()", "Provides the ability to retrieve the response from teh last Maestro function.");
  jsreg("getStatus", 0, "getStatus()", "Provides the ability to retrieve the status from teh last Maestro function.");
  jsreg("junction", 1, "junction(request)", "Provides the ability to send a request to Service Junction.");
  jsreg("live", 4, "live(application, user, message, wait)", "Provides the ability to send a live message to an application and/or user.");
  jsreg("log", 1, "log(message)", "Provides the ability to write a log message to the local Radial instance.");
  jsreg("logger", 4, "logger(application, function, label, message)", "Provides the ability to send a message to a particular application registered in Logger.");
  jsreg("mutexCreate", 0, "mutexCreate()", "Provides the ability to create a mutex.");
  jsreg("mutexDestroy", 1, "mutexDestroy(id)", "Provides the ability to destroy a mutex.");
  jsreg("mutexLock", 1, "mutexLock(id)", "Provides the ability to lock a mutex.");
  jsreg("mutexUnlock", 1, "mutexUnlock(id)", "Provides the ability to lock a mutex.");
  jsreg("mysqlQuery", 6, "mysqlQuery(server, port, user, password, database, query)", "Provides the ability to query (SQL: select) a given MySQL server and database.");
  jsreg("mysqlUpdate", 6, "mysqlUpdate(server, port, user, password, database, query)", "Provides the ability to update (SQL: delete, insert, update) a given MySQL server and database.");
  jsreg("notify", 1, "notify(message)", "Provides the ability to send a message to the configured notification email address.");
  jsreg("semCreate", 1, "semCreate(value)", "Provides the ability to create a mutex.");
  jsreg("semDestroy", 1, "semDestroy(id)", "Provides the ability to destroy a mutex.");
  jsreg("semPost", 1, "semPost(id)", "Provides the ability to lock a mutex.");
  jsreg("semWait", 1, "semWait(id)", "Provides the ability to lock a mutex.");
  jsreg("setError", 1, "setError(error)", "Provides the ability to set the error prior to exiting the given flow.");
  jsreg("setRequest", 1, "setRequest(request)", "Provides the ability to set the request prior to entering a Maestro function.  NOTE: This is not typically required as the requested is normally set as an argument in the function.");
  jsreg("setResponse", 1, "setResponse(response)", "Provides the ability to set the response prior to exiting the given flow.");
  jsreg("setStatus", 1, "setStatus(status)", "Provides the ability to set the status prior to exiting the given flow.  The status should typically be set to a value of either \"okay\" or \"error\".");
  jsreg("sqliteCreate", 2, "sqliteCreate(database, node)", "Provides the ability to create a given SQLite database.");
  jsreg("sqliteDatabases", 0, "sqliteDatabases()", "Provides the ability to retrieve a list SQLite databases.");
  jsreg("sqliteDrop", 2, "sqliteDrop(database, node)", "Provides the ability to drop a given SQLite database.");
  jsreg("sqliteQuery", 2, "sqliteQuery(database, statement)", "Provides the ability to query (SQL: delete, insert, select, update) a given SQLite database.");
  jsreg("sshConnect", 5, "sshConnect(server, port, user, password, privatekey)", "Provides the ability to connect a SSH session.");
  jsreg("sshDisconnect", 1, "sshDisconnect(session)", "Provides the ability to disconnect a SSH session.");
  jsreg("sshSend", 3, "sshSend(session, command, wait)", "Provides the ability to send a command over a SSH session.");
  jsreg("storageAdd", 2, "storageAdd(keys, data)", "Provides the ability to add data to storage.");
  jsreg("storageRemove", 1, "storageRemove(keys)", "Provides the ability to remove data from storage.");
  jsreg("storageRetrieve", 1, "storageRetrieve(keys)", "Provides the ability to retrieve data from storage.");
  jsreg("storageRetrieveKeys", 1, "storageRetrieveKeys(keys)", "Provides the ability to retrieve key data from storage.");
  jsreg("storageUpdate", 2, "storageUpdate(keys, data)", "Provides the ability to update data into storage.");
  jsreg("threadCreate", 2, "threadCreate(plan, flow, request)", "Provides the ability to create a thread.");
  jsreg("threadDetach", 1, "threadDetach(id)", "Provides the ability to detach a thread.");
  jsreg("threadJoin", 1, "threadJoin(id)", "Provides the ability to join a thread.");
  // }}}
  m_strCipher = "AES-256 GCM";
  m_strHandle = "maestro";
  m_strPath = m_strData + (string)"/maestro";
  watches[m_strData] = {".cred"};
  m_pThreadInotify = new thread(&Maestro::inotify, this, strPrefix, watches, pCallbackInotify);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
  m_pThreadScheduler = new thread(&Maestro::scheduler, this, strPrefix);
  pthread_setname_np(m_pThreadScheduler->native_handle(), "scheduler");
}
// }}}
// {{{ ~Maestro()
Maestro::~Maestro()
{
  m_pThreadInotify->join();
  delete m_pThreadInotify;
  m_pThreadScheduler->join();
  delete m_pThreadScheduler;
  for (auto &p : m_p)
  {
    if (!p.second->r.empty())
    {
      for (auto &r : p.second->r)
      {
        for (auto &c : r.second)
        {
          (*c.second->pbKill) = true;
          for (auto &t : c.second->threads)
          {
            t.second->pThread->join();
            delete t.second->pThread;
            delete t.second;
          }
          c.second->threads.clear();
          for (auto &s : c.second->sems)
          {
            sem_destroy(s.second->pSem);
            delete s.second->pSem;
            delete s.second;
          }
          c.second->sems.clear();
          for (auto &m : c.second->mutexes)
          {
            delete m.second;
          }
          c.second->mutexes.clear();
          delete c.second;
        }
        r.second.clear();
      }
      p.second->r.clear();
    }
    for (auto &s : p.second->d)
    {
      delete s.second;
    }
    p.second->d.clear();
    for (auto &f : p.second->f)
    {
      delete f.second;
    }
    p.second->f.clear();
    for (auto &s : p.second->s)
    {
      delete s.second;
    }
    p.second->s.clear();
    delete p.second;
  }
  m_p.clear();
  for (auto &i : m_javascript)
  {
    delete i.second;
  }
  m_javascript.clear();
}
// }}}
// {{{ access
// {{{ accesses()
bool Maestro::accesses(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Type"}, i, e))
  {
    string t = i->m["Type"]->v;
    if (t == "application" || t == "user")
    {
      b = true;
      m_mutex.lock();
      for (auto &p : m_p)
      {
        if (!p.second->f.empty() && ((t == "application" && p.second->t == 'a') || (t == "user" && p.second->t == 'u')))
        {
          o->pb({{"ID", p.second->id}, {"Owner", p.second->owner}});
        }
      }
      m_mutex.unlock();
    }
    else if (t == "core")
    {
      b = true;
      for (auto &a : m_a)
      {
        o->pb({{"ID", a.first}, {"Owner", a.first}});
      }
    }
    else
    {
      e = "Please provide a valid Type:  application, core, user.";
    }
  }
  else if (dep({"Plan"}, i, e))
  {
    string p = i->m["Plan"]->v;
    e.clear();
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      b = true;
      if (m_p[p]->a != NULL)
      {
        for (auto &t : m_p[p]->a->m)
        {
          o->m[t.first] = new Json;
          for (auto &c : t.second->m)
          {
            o->m[t.first]->m[c.first] = new Json;
            if (c.first.size() > 2 && (c.first.substr(0, 2) == "a_" || c.first.substr(0, 2) == "u_") && m_p.find(c.first) != m_p.end())
            {
              o->m[t.first]->m[c.first]->i("Owner", m_p[c.first]->owner);
            }
            else
            {
              o->m[t.first]->m[c.first]->i("Owner", c.first);
            }
            o->m[t.first]->m[c.first]->i("Scopes", c.second);
          }
        }
      }
      for (auto &c : m_p)
      {
        if (c.second->a != NULL && c.second->a->exist({"r"}) && c.second->a->m["r"]->m.find(p) != c.second->a->m["r"]->m.end())
        {
          if (!o->exist({"p"}))
          {
            o->m["p"] = new Json;
          }
          o->m["p"]->m[c.first] = new Json;
          if (c.first.size() > 2 && (c.first.substr(0, 2) == "a_" || c.first.substr(0, 2) == "u_") && m_p.find(c.first) != m_p.end())
          {
            o->m["p"]->m[c.first]->i("Owner", m_p[c.first]->owner);
          }
          else
          {
            o->m["p"]->m[c.first]->i("Owner", c.first);
          }
          o->m["p"]->m[c.first]->i("Scopes", c.second->a->m["r"]->m[p]);
        }
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
  }
  else if (isLocalAdmin(d, "Maestro"))
  {
    b = true;
    e.clear();
    m_mutex.lock();
    for (auto &c : m_p)
    {
      if (c.second->a != NULL && c.second->a->exist({"r"}))
      {
        for (auto &g : c.second->a->m["r"]->m)
        {
          if (g.first.size() < 2 || (g.first.substr(0, 2) != "a_" && g.first.substr(0, 2) != "u_"))
          {
            if (!o->exist({c.first}))
            {
              o->m[c.first] = new Json;
              o->m[c.first]->i("Owner", c.second->owner);
              o->m[c.first]->m["Scopes"] = new Json;
            }
            o->m[c.first]->m["Scopes"]->i(g.first, g.second);
          }
        }
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
// {{{ accessRemove()
bool Maestro::accessRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Access", "Plan", "Scope"}, i, e))
  {
    string a = i->m["Access"]->v, p = i->m["Plan"]->v, s = i->m["Scope"]->v;
    if (isOwner(d, p))
    {
      string t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->a->exist({"a", a}))
        {
          list<Json *>::iterator scopeIter = m_p[p]->a->m["a"]->m[a]->l.end();
          for (auto c = m_p[p]->a->m["a"]->m[a]->l.begin(); scopeIter == m_p[p]->a->m["a"]->m[a]->l.end() && c != m_p[p]->a->m["a"]->m[a]->l.end(); c++)
          {
            if ((*c)->v == s)
            {
              scopeIter = c;
            }
          }
          if (scopeIter != m_p[p]->a->m["a"]->m[a]->l.end())
          {
            delete (*scopeIter);
            m_p[p]->a->m["a"]->m[a]->l.erase(scopeIter);
            if (m_p[p]->a->m["a"]->m[a]->l.empty())
            {
              delete m_p[p]->a->m["a"]->m[a];
              m_p[p]->a->m["a"]->m.erase(a);
            }
            if (m_p[p]->a->m["a"]->m.empty())
            {
              delete m_p[p]->a->m["a"];
              m_p[p]->a->m.erase("a");
            }
            if (m_p[p]->a->m.empty())
            {
              delete m_p[p]->a;
              m_p[p]->a = NULL;
            }
            if (m_p[p]->a != NULL)
            {
              string j;
              m_p[p]->a->j(j);
              if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
              {
                b = true;
              }
              else
              {
                ssChat.str("");
                ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                chat("#maestro", ssChat.str());
              }
            }
            else if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "a"}, e))
            {
              b = true;
            }
            else
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " | " << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            e = "Scope not found.";
          }
        }
        else
        {
          e = "Access not found.";
        }
      }
      else
      {
        e = "Plan not found.";
      }
      m_mutex.unlock();
      if (b && i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        Json *ptLive = new Json;
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
          ptLink->i("Function", "accessRemove");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "04Access removed by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "accessRemove");
        ptLive->i("Plan", p);
        ptLive->i("Access", a);
        ptLive->i("Scope", s);
        live("Maestro", "", ptLive);
        delete ptLive;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// {{{ accessRequest()
bool Maestro::accessRequest(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Access", "Plan", "Scope"}, i, e))
  {
    list<string> to;
    string a = i->m["Access"]->v, p = i->m["Plan"]->v, s = i->m["Scope"]->v, t = "core";
    stringstream ssHtml, ssText;
    if (a.size() > 2)
    {
      if (a.substr(0, 2) == "a_")
      {
        t = "application";
      }
      else if (a.substr(0, 2) == "u_")
      {
        t = "user";
      }
    } 
    m_mutex.lock();
    if (isOwner(d, p))
    {
      if (!haveAccess(p, a, {s}, e, false))
      {
        if (!haveAccessRequest(p, a, s))
        {
          if (t == "application" || t == "core" || t == "user")
          {
            stringstream ssLink, ssQuery;
            ssText << d.f << " " << d.l << " (" << d.u << ") has requested access to the \"" << s << "\" ";
            ssHtml << d.f << " " << d.l << " (" << d.u << ") has requested access to the \"" << s << "\" ";
            if (t == "application")
            {
              ssQuery << "select distinct c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and b.type in ('Primary Developer', 'Backup Developer') and a.application_id = '" << esc(a.substr(2, (a.size() - 2))) << "' order by c.email";
              ssText << "flow of the \"" << m_p[a]->owner << "\" plan";
              ssHtml << "flow of the \"" << m_p[a]->owner << "\" plan";
            }
            else if (t == "core")
            {
              ssQuery << "select distinct c.email from application a, application_contact b, person c where a.id = b.application_id and b.contact_id = c.id and b.admin = 1 and a.name = 'Maestro' order by c.email";
              ssText << "scope of the \"" << a << "\" core function";
              ssHtml << "scope of the \"" << a << "\" core function";
            }
            else if (t == "user")
            {
              ssQuery << "select email from person where id = '" << esc(a.substr(2, (a.size() - 2))) << "'";
              ssText << "flow of the \"" << m_p[a]->owner << "\" plan";
              ssHtml << "flow of the \"" << m_p[a]->owner << "\" plan";
            }
            ssText << " on behalf of the \"" << m_p[p]->owner << "\" plan within Maestro.";
            ssHtml << " on behalf of the \"" << m_p[p]->owner << "\" plan within Maestro.";
            ssText << endl << endl;
            ssHtml << "<br><br>";
            ssLink << "https://" << m_strServer << "/maestro/#";
            if (t == "core")
            {
              ssLink << "/Approvals";
            }
            else
            {
              ssLink << "/Plans/" << a;
            }
            ssText << ssLink.str();
            ssHtml << "<a href=\"" << ssLink.str() << "\" target=\"_blank\">" << ssLink.str() << "</a>";
            ssText << endl << endl;
            ssHtml << "<br><br>";
            ssText << "-- Maestro";
            ssHtml << "-- Maestro";
            auto getContact = dbquery("central_r", ssQuery.str(), e);
            if (getContact != NULL)
            {
              if (!getContact->empty())
              {
                string j;
                b = true;
                for (auto &getContactRow : *getContact)
                {
                  to.push_back(getContactRow["email"]);
                }
                if (m_p[p]->a == NULL)
                {
                  m_p[p]->a = new Json;
                }
                if (!m_p[p]->a->exist({"r"}))
                {
                  m_p[p]->a->m["r"] = new Json;
                }
                if (!m_p[p]->a->m["r"]->exist({a}))
                {
                  m_p[p]->a->m["r"]->m[a] = new Json;
                }
                m_p[p]->a->m["r"]->m[a]->pb(s);
                m_p[p]->a->j(j);
                if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
                {
                  b = true;
                }
                else
                {
                  ssChat.str("");
                  ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                  chat("#maestro", ssChat.str());
                }
              }
              else
              {
                e = "No approvers found.";
              }
            }
            dbfree(getContact);
          }
          else
          {
            e = "Please provide a valid Type:  application, core, user.";
          }
        }
        else
        {
          e = "You already have a pending request for access.";
        }
      }
      else
      {
        e = "You already have access.";
      }
    }
    else
    {
      e = "You are not the owner.";
    }
    m_mutex.unlock();
    if (b && i->val({"_broadcast"}) != "1")
    {
      list<string> nodes;
      Json *ptLive = new Json;
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
        ptLink->i("Function", "accessRequest");
        ptLink->m["Request"]->i("_broadcast", "1", '1');
        hub("link", ptLink);
        delete ptLink;
        nodes.pop_front();
      }
      ssChat.str("");
      ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Access requested by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
      chat("#maestro", ssChat.str());
      ptLive->i("Action", "accessRequest");
      ptLive->i("Plan", p);
      ptLive->i("Access", a);
      ptLive->i("Scope", s);
      live("Maestro", "", ptLive);
      delete ptLive;
      email("", to, "Maestro:  Access Request", ssText.str(), ssHtml.str());
    }
  }

  return b;
}
// }}}
// {{{ accessRequestApprove()
bool Maestro::accessRequestApprove(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Access", "Plan", "Scope"}, i, e))
  {
    list<string> to;
    string a = i->m["Access"]->v, p = i->m["Plan"]->v, s = i->m["Scope"]->v, t = "core";
    stringstream ssHtml, ssText;
    if (a.size() > 2)
    {
      if (a.substr(0, 2) == "a_")
      {
        t = "application";
      }
      else if (a.substr(0, 2) == "u_")
      {
        t = "user";
      }
    } 
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      if (((t == "application" || t == "user") && isOwner(d, a)) || (t == "core" && isLocalAdmin(d, "Maestro")))
      {
        if (haveAccessRequest(p, a, s))
        {
          list<Json *>::iterator scopeIter = m_p[p]->a->m["r"]->m[a]->l.end();
          for (auto c = m_p[p]->a->m["r"]->m[a]->l.begin(); scopeIter == m_p[p]->a->m["r"]->m[a]->l.end() && c != m_p[p]->a->m["r"]->m[a]->l.end(); c++)
          {
            if ((*c)->v == s)
            {
              scopeIter = c;
            }
          }
          if (scopeIter != m_p[p]->a->m["r"]->m[a]->l.end())
          {
            if (!haveAccess(p, a, {s}, e, false))
            {
              stringstream ssQuery;
              ssText << d.f << " " << d.l << " (" << d.u << ") has approved access to the \"" << s << "\" ";
              ssHtml << d.f << " " << d.l << " (" << d.u << ") has approved access to the \"" << s << "\" ";
              if (p.substr(0, 2) == "a_")
              {
                ssQuery << "select distinct c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and b.type in ('Primary Developer', 'Backup Developer') and a.application_id = '" << esc(p.substr(2, (p.size() - 2))) << "' order by c.email";
                if (t == "core")
                {
                  ssText << " core function";
                  ssHtml << " core function";
                }
                else
                {
                  ssText << "flow of the \"" << m_p[a]->owner << "\" plan";
                  ssHtml << "flow of the \"" << m_p[a]->owner << "\" plan";
                }
              }
              else if (p.substr(0, 2) == "u_")
              {
                ssQuery << "select email from person where id = '" << esc(p.substr(2, (p.size() - 2))) << "'";
                if (t == "core")
                {
                  ssText << " core function";
                  ssHtml << " core function";
                }
                else
                {
                  ssText << "flow of the \"" << m_p[a]->owner << "\" plan";
                  ssHtml << "flow of the \"" << m_p[a]->owner << "\" plan";
                }
              }
              ssText << " on behalf of the \"" << m_p[p]->owner << "\" plan within Maestro.";
              ssHtml << " on behalf of the \"" << m_p[p]->owner << "\" plan within Maestro.";
              ssText << endl << endl;
              ssHtml << "<br><br>";
              ssText << "-- Maestro";
              ssHtml << "-- Maestro";
              auto getContact = dbquery("central_r", ssQuery.str(), e);
              if (getContact != NULL)
              {
                if (!getContact->empty())
                {
                  string j;
                  for (auto &getContactRow : *getContact)
                  {
                    to.push_back(getContactRow["email"]);
                  }
                  delete (*scopeIter);
                  m_p[p]->a->m["r"]->m[a]->l.erase(scopeIter);
                  if (m_p[p]->a->m["r"]->m[a]->l.empty())
                  {
                    delete m_p[p]->a->m["r"]->m[a];
                    m_p[p]->a->m["r"]->m.erase(a);
                  }
                  if (m_p[p]->a->m["r"]->m.empty())
                  {
                    delete m_p[p]->a->m["r"];
                    m_p[p]->a->m.erase("r");
                  }
                  if (m_p[p]->a->m.find("a") == m_p[p]->a->m.end())
                  {
                    m_p[p]->a->m["a"] = new Json;
                  }
                  if (m_p[p]->a->m["a"]->m.find(a) == m_p[p]->a->m["a"]->m.end())
                  {
                    m_p[p]->a->m["a"]->m[a] = new Json;
                  }
                  m_p[p]->a->m["a"]->m[a]->pb(s);
                  m_p[p]->a->j(j);
                  if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
                  {
                    b = true;
                  }
                  else
                  {
                    ssChat.str("");
                    ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                    chat("#maestro", ssChat.str());
                  }
                }
                else
                {
                  e = "No approvers found.";
                }
              }
              dbfree(getContact);
            }
            else
            {
              delete (*scopeIter);
              m_p[p]->a->m["r"]->m[a]->l.erase(scopeIter);
              if (m_p[p]->a->m["r"]->m[a]->l.empty())
              {
                delete m_p[p]->a->m["r"]->m[a];
                m_p[p]->a->m["r"]->m.erase(a);
              }
              if (m_p[p]->a->m["r"]->m.empty())
              {
                delete m_p[p]->a->m["r"];
                m_p[p]->a->m.erase("r");
              }
              if (m_p[p]->a->m.empty())
              {
                delete m_p[p]->a;
                m_p[p]->a = NULL;
              }
              if (m_p[p]->a != NULL)
              {
                string j;
                m_p[p]->a->j(j);
                if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
                {
                  b = true;
                }
                else
                {
                  ssChat.str("");
                  ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                  chat("#maestro", ssChat.str());
                }
              }
              else if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "a"}, e))
              {
                b = true;
              }
              else
              {
                ssChat.str("");
                ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " | " << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                chat("#maestro", ssChat.str());
              }
            }
          }
          else
          {
            e = "Scope not found.";
          }
        }
        else
        {
          e = "Access request not found.";
        }
      }
      else
      {
        e = "You are not the owner.";
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
    if (b && i->val({"_broadcast"}) != "1")
    {
      list<string> nodes;
      Json *ptLive = new Json;
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
        ptLink->i("Function", "accessRequestApprove");
        ptLink->m["Request"]->i("_broadcast", "1", '1');
        hub("link", ptLink);
        delete ptLink;
        nodes.pop_front();
      }
      ssChat.str("");
      ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "03Access request approved by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
      chat("#maestro", ssChat.str());
      ptLive->i("Action", "accessRequestApprove");
      ptLive->i("Plan", p);
      ptLive->i("Access", a);
      ptLive->i("Scope", s);
      live("Maestro", "", ptLive);
      delete ptLive;
      email("", to, "Maestro:  Access Request Approved", ssText.str(), ssHtml.str());
    }
  }

  return b;
}
// }}}
// {{{ accessRequestCancel()
bool Maestro::accessRequestCancel(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Access", "Plan", "Scope"}, i, e))
  {
    list<string> to;
    string a = i->m["Access"]->v, p = i->m["Plan"]->v, s = i->m["Scope"]->v, t = "core";
    stringstream ssHtml, ssText;
    if (a.size() > 2)
    {
      if (a.substr(0, 2) == "a_")
      {
        t = "application";
      }
      else if (a.substr(0, 2) == "u_")
      {
        t = "user";
      }
    } 
    m_mutex.lock();
    if (isOwner(d, p))
    {
      if (!haveAccess(p, a, {s}, e, false))
      {
        if (haveAccessRequest(p, a, s))
        {
          stringstream ssQuery;
          ssText << d.f << " " << d.l << " (" << d.u << ") has canceled their request for access to the \"" << s << "\" ";
          ssHtml << d.f << " " << d.l << " (" << d.u << ") has canceled their request for access to the \"" << s << "\" ";
          if (t == "application")
          {
            ssQuery << "select distinct c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and b.type in ('Primary Developer', 'Backup Developer') and a.application_id = '" << esc(a.substr(2, (a.size() - 2))) << "' order by c.email";
            ssText << "flow of the \"" << m_p[a]->owner << "\" plan";
            ssHtml << "flow of the \"" << m_p[a]->owner << "\" plan";
          }
          else if (t == "core")
          {
            ssQuery << "select distinct c.email from application a, application_contact b, person c where a.id = b.application_id and b.contact_id = c.id and b.admin = 1 and a.name = 'Maestro' order by c.email";
            ssText << "scope of the \"" << a << "\" core function";
            ssHtml << "scope of the \"" << a << "\" core function";
          }
          else if (t == "user")
          {
            ssQuery << "select email from person where id = '" << esc(a.substr(2, (a.size() - 2))) << "'";
            ssText << "flow of the \"" << m_p[a]->owner << "\" plan";
            ssHtml << "flow of the \"" << m_p[a]->owner << "\" plan";
          }
          ssText << " on behalf of the \"" << m_p[p]->owner << "\" plan within Maestro.";
          ssHtml << " on behalf of the \"" << m_p[p]->owner << "\" plan within Maestro.";
          ssText << endl << endl;
          ssHtml << "<br><br>";
          ssText << "-- Maestro";
          ssHtml << "-- Maestro";
          auto getContact = dbquery("central_r", ssQuery.str(), e);
          if (getContact != NULL)
          {
            if (!getContact->empty())
            {
              list<Json *>::iterator scopeIter = m_p[p]->a->m["r"]->m[a]->l.end();
              for (auto c = m_p[p]->a->m["r"]->m[a]->l.begin(); scopeIter == m_p[p]->a->m["r"]->m[a]->l.end() && c != m_p[p]->a->m["r"]->m[a]->l.end(); c++)
              {
                if ((*c)->v == s)
                {
                  scopeIter = c;
                }
              }
              if (scopeIter != m_p[p]->a->m["r"]->m[a]->l.end())
              {
                for (auto &getContactRow : *getContact)
                {
                  to.push_back(getContactRow["email"]);
                }
                delete (*scopeIter);
                m_p[p]->a->m["r"]->m[a]->l.erase(scopeIter);
                if (m_p[p]->a->m["r"]->m[a]->l.empty())
                {
                  delete m_p[p]->a->m["r"]->m[a];
                  m_p[p]->a->m["r"]->m.erase(a);
                }
                if (m_p[p]->a->m["r"]->m.empty())
                {
                  delete m_p[p]->a->m["r"];
                  m_p[p]->a->m.erase("r");
                }
                if (m_p[p]->a->m.empty())
                {
                  delete m_p[p]->a;
                  m_p[p]->a = NULL;
                }
                if (m_p[p]->a != NULL)
                {
                  string j;
                  m_p[p]->a->j(j);
                  if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
                  {
                    b = true;
                  }
                  else
                  {
                    ssChat.str("");
                    ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                    chat("#maestro", ssChat.str());
                  }
                }
                else if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "a"}, e))
                {
                  b = true;
                }
                else
                {
                  ssChat.str("");
                  ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " | " << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                  chat("#maestro", ssChat.str());
                }
              }
              else
              {
                e = "Scope not found.";
              }
            }
            else
            {
              e = "No approvers found.";
            }
          }
          dbfree(getContact);
        }
        else
        {
          e = "You already have a pending request for access.";
        }
      }
      else
      {
        e = "You already have access.";
      }
    }
    else
    {
      e = "You are not the owner.";
    }
    m_mutex.unlock();
    if (b && i->val({"_broadcast"}) != "1")
    {
      list<string> nodes;
      Json *ptLive = new Json;
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
        ptLink->i("Function", "accessRequestCancel");
        ptLink->m["Request"]->i("_broadcast", "1", '1');
        hub("link", ptLink);
        delete ptLink;
        nodes.pop_front();
      }
      ssChat.str("");
      ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Access request canceled by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
      chat("#maestro", ssChat.str());
      ptLive->i("Action", "accessRequestCancel");
      ptLive->i("Plan", p);
      ptLive->i("Access", a);
      ptLive->i("Scope", s);
      live("Maestro", "", ptLive);
      delete ptLive;
      email("", to, "Maestro:  Access Request Canceled", ssText.str(), ssHtml.str());
    }
  }

  return b;
}
// }}}
// {{{ accessRequestDeny()
bool Maestro::accessRequestDeny(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Access", "Plan", "Scope"}, i, e))
  {
    list<string> to;
    string a = i->m["Access"]->v, p = i->m["Plan"]->v, s = i->m["Scope"]->v, t = "core";
    stringstream ssHtml, ssText;
    if (a.size() > 2)
    {
      if (a.substr(0, 2) == "a_")
      {
        t = "application";
      }
      else if (a.substr(0, 2) == "u_")
      {
        t = "user";
      }
    } 
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      if (((t == "application" || t == "user") && isOwner(d, a)) || (t == "core" && isLocalAdmin(d, "Maestro")))
      {
        if (haveAccessRequest(p, a, s))
        {
          list<Json *>::iterator scopeIter = m_p[p]->a->m["r"]->m[a]->l.end();
          for (auto c = m_p[p]->a->m["r"]->m[a]->l.begin(); scopeIter == m_p[p]->a->m["r"]->m[a]->l.end() && c != m_p[p]->a->m["r"]->m[a]->l.end(); c++)
          {
            if ((*c)->v == s)
            {
              scopeIter = c;
            }
          }
          if (scopeIter != m_p[p]->a->m["r"]->m[a]->l.end())
          {
            stringstream ssQuery;
            ssText << d.f << " " << d.l << " (" << d.u << ") has denied access to the \"" << s << "\" ";
            ssHtml << d.f << " " << d.l << " (" << d.u << ") has denied access to the \"" << s << "\" ";
            if (p.substr(0, 2) == "a_")
            {
              ssQuery << "select distinct c.email from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and b.type in ('Primary Developer', 'Backup Developer') and a.application_id = '" << esc(p.substr(2, (p.size() - 2))) << "' order by c.email";
              if (t == "core")
              {
                ssText << " core function";
                ssHtml << " core function";
              }
              else
              {
                ssText << "flow of the \"" << m_p[a]->owner << "\" plan";
                ssHtml << "flow of the \"" << m_p[a]->owner << "\" plan";
              }
            }
            else if (p.substr(0, 2) == "u_")
            {
              ssQuery << "select email from person where id = '" << esc(p.substr(2, (p.size() - 2))) << "'";
              if (t == "core")
              {
                ssText << " core function";
                ssHtml << " core function";
              }
              else
              {
                ssText << "flow of the \"" << m_p[a]->owner << "\" plan";
                ssHtml << "flow of the \"" << m_p[a]->owner << "\" plan";
              }
            }
            ssText << " on behalf of the \"" << m_p[p]->owner << "\" plan within Maestro.";
            ssHtml << " on behalf of the \"" << m_p[p]->owner << "\" plan within Maestro.";
            ssText << endl << endl;
            ssHtml << "<br><br>";
            ssText << "-- Maestro";
            ssHtml << "-- Maestro";
            auto getContact = dbquery("central_r", ssQuery.str(), e);
            if (getContact != NULL)
            {
              if (!getContact->empty())
              {
                for (auto &getContactRow : *getContact)
                {
                  to.push_back(getContactRow["email"]);
                }
                delete (*scopeIter);
                m_p[p]->a->m["r"]->m[a]->l.erase(scopeIter);
                if (m_p[p]->a->m["r"]->m[a]->l.empty())
                {
                  delete m_p[p]->a->m["r"]->m[a];
                  m_p[p]->a->m["r"]->m.erase(a);
                }
                if (m_p[p]->a->m["r"]->m.empty())
                {
                  delete m_p[p]->a->m["r"];
                  m_p[p]->a->m.erase("r");
                }
                if (m_p[p]->a->m.empty())
                {
                  delete m_p[p]->a;
                  m_p[p]->a = NULL;
                }
                if (m_p[p]->a != NULL)
                {
                  string j;
                  m_p[p]->a->j(j);
                  if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "a"}, j, e))
                  {
                    b = true;
                  }
                  else
                  {
                    ssChat.str("");
                    ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                    chat("#maestro", ssChat.str());
                  }
                }
                else if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "a"}, e))
                {
                  b = true;
                }
                else
                {
                  ssChat.str("");
                  ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " | " << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
                  chat("#maestro", ssChat.str());
                }
              }
              else
              {
                e = "No approvers found.";
              }
            }
            dbfree(getContact);
          }
          else
          {
            e = "Scope not found.";
          }
        }
        else
        {
          e = "Access request not found.";
        }
      }
      else
      {
        e = "You are not the owner.";
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
    if (b && i->val({"_broadcast"}) != "1")
    {
      list<string> nodes;
      Json *ptLive = new Json;
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
        ptLink->i("Function", "accessRequestDeny");
        ptLink->m["Request"]->i("_broadcast", "1", '1');
        hub("link", ptLink);
        delete ptLink;
        nodes.pop_front();
      }
      ssChat.str("");
      ssChat << char(3) << "00,06 " << p << " | " << a << " | " << s << " " << char(3) << " " << char(2) << char(3) << "04Access request denied by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
      chat("#maestro", ssChat.str());
      ptLive->i("Action", "accessRequestDeny");
      ptLive->i("Plan", p);
      ptLive->i("Access", a);
      ptLive->i("Scope", s);
      live("Maestro", "", ptLive);
      delete ptLive;
      email("", to, "Maestro:  Access Request Denied", ssText.str(), ssHtml.str());
    }
  }

  return b;
}
// }}}
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
    if (strNewMaster == m_strNode)
    {
      m_bLoadSchedules = true;
      m_bLoadServices = true;
    }
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
  if (strPath == m_strData && strFile == ".cred")
  {
    load(strPrefix);
  }
}
// }}}
// {{{ cores()
bool Maestro::cores(radialUser &d, string &e)
{
  bool b = true;
  Json *o = d.p->m["o"];

  for (auto &j : m_javascript)
  {
    Json *c = new Json;
    c->i("Function", j.first);
    c->i("Description", j.second->strDescription);
    c->i("Prototype", j.second->strPrototype);
    o->l.push_back(c);
  }

  return b;
}
// }}}
// {{{ flow
// {{{ flow()
bool Maestro::flow(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Flow", "Plan"}, i, e))
  {
    string f = i->m["Flow"]->v, p = i->m["Plan"]->v;
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      if (m_p[p]->f.find(f) != m_p[p]->f.end())
      {
        b = true;
        if (!isOwner(d, p) && !m_p[p]->f[f]->empty({"Script"}))
        {
          m_p[p]->f[f]->i("Script", "");
        }
        o->merge(m_p[p]->f[f], true, false);
      }
      else
      {
        e = "Flow not found.";
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
  }

  return b;
}
// }}}
// {{{ flowAdd()
bool Maestro::flowAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Flow", "Plan"}, i, e))
  {
    string f = i->m["Flow"]->v, p = i->m["Plan"]->v;
    if (isOwner(d, p))
    {
      string id, t, strOwner;
      stringstream ssP(p);
      getline(ssP, t, '_');
      getline(ssP, id);
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        strOwner = m_p[p]->owner;
        if (m_p[p]->f.find(f) == m_p[p]->f.end())
        {
          if (i->val({"_broadcast"}) == "1" || !m_p[p]->f.empty() || dataDirectoryAdd(m_strHandle, {p, "f"}, e))
          {
            string j;
            i->j(j);
            if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "f", f}, j, e))
            {
              b = true;
              m_p[p]->f[f] = new Json(i);
            }
            else
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | f | " << f << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | f " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryAdd() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Flow already exists.";
        }
      }
      else
      {
        e = "Plan not found.";
      }
      m_mutex.unlock();
      if (b && i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        Json *ptLive = new Json;
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
          ptLink->i("Function", "flowAdd");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << strOwner << " | " << f << " " << char(3) << " " << char(2) << char(3) << "03Flow added by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "flowAdd");
        ptLive->i("Plan", p);
        ptLive->i("Flow", f);
        live("Maestro", "", ptLive);
        delete ptLive;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// {{{ flowExec()
bool Maestro::flowExec(radialUser &d, string &e)
{
  bool b = false;
  long lThreadID = syscall(SYS_gettid);
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Flow", "Plan"}, i, e))
  {
    string f = i->m["Flow"]->v, p = i->m["Plan"]->v;
    Json *ptFlow = NULL;
    m_mutex.lock();
    if (p.size() < 2 || (p.substr(0, 2) != "a_" && p.substr(0, 2) != "u_"))
    {
      auto pIter = m_p.end();
      for (auto c = m_p.begin(); pIter == m_p.end() && c != m_p.end(); c++)
      {
        if (c->second->owner == p)
        {
          pIter = c;
        }
      }
      if (pIter != m_p.end())
      {
        p = pIter->first;
      }
    }
    if (m_p.find(p) != m_p.end())
    {
      if (m_p[p]->f.find(f) != m_p[p]->f.end())
      {
        ptFlow = new Json(m_p[p]->f[f]);
      }
      else
      {
        e = "Flow not found.";
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
    if (ptFlow != NULL)
    {
      if (ptFlow->val({"Public"}) == "1" || isOwner(d, p))
      {
        if (!ptFlow->empty({"Script"}))
        {
          bool *pbKill = NULL;
          JSRuntime *jsrun = JS_NewRuntime();
          JSContext *jsctx = JS_NewContext(jsrun);
          JSValue jsglobal = JS_GetGlobalObject(jsctx);
          Json *ptLive = new Json;
          for (auto &i : m_javascript)
          {
            if (i.second->function != NULL)
            {
              JS_SetPropertyStr(jsctx, jsglobal, i.first.c_str(), JS_NewCFunction(jsctx, i.second->function, i.first.c_str(), i.second->nLength));
            }
          }
          if (!m_strSecret.empty())
          {
            string strPlan;
            if (!m_manip.encodeBase64(m_manip.encryptAes(p, m_strSecret, strPlan, e, m_strCipher), strPlan).empty())
            {
              JS_SetPropertyStr(jsctx, jsglobal, "g_js_plan", JS_NewStringLen(jsctx, strPlan.c_str(), strPlan.size()));
            }
          }
          if (i->exist({"Request"}))
          {
            string strRequest;
            i->m["Request"]->j(strRequest);
            JS_SetPropertyStr(jsctx, jsglobal, "g_js_request", JS_NewStringLen(jsctx, strRequest.c_str(), strRequest.size()));
          }
          m_mutex.lock();
          if (m_p.find(p) != m_p.end())
          {
            radialMaestroRunning *ptRunning = new radialMaestroRunning;
            pbKill = new bool(false);
            if (m_p[p]->r.find(f) == m_p[p]->r.end())
            {
              m_p[p]->r[f] = {};
            }
            time(&(ptRunning->CTime));
            ptRunning->nMutex = 0;
            ptRunning->nSem = 0;
            ptRunning->nThread = 0;
            ptRunning->pbKill = pbKill;
            m_p[p]->r[f][lThreadID] = ptRunning;
            if (!m_strSecret.empty())
            {
              string strThread[2];
              Json *ptThread = new Json;
              ptThread->i("p", p);
              ptThread->i("f", f);
              ptThread->i("t", to_string(lThreadID), 'n');
              ptThread->j(strThread[0]);
              delete ptThread;
              if (!m_manip.encodeBase64(m_manip.encryptAes(strThread[0], m_strSecret, strThread[1], e, m_strCipher), strThread[1]).empty())
              {
                JS_SetPropertyStr(jsctx, jsglobal, "g_js_run", JS_NewStringLen(jsctx, strThread[1].c_str(), strThread[1].size()));
              }
            }
          }
          m_mutex.unlock();
          ptLive->i("Action", "flowExec");
          ptLive->i("Plan", p);
          ptLive->i("Flow", f);
          live("Maestro", "", ptLive);
          if (pbKill != NULL)
          {
            JS_SetInterruptHandler(jsrun, maestroInterrupt, pbKill);
          }
          JSValue jsres = JS_Eval(jsctx, ptFlow->m["Script"]->v.data(), ptFlow->m["Script"]->v.size(), "<embedded>", JS_EVAL_TYPE_GLOBAL);
          if (pbKill != NULL)
          {
            list<thread *> threads;
            JS_SetInterruptHandler(jsrun, NULL, NULL);
            (*pbKill) = true;
            m_mutex.lock();
            if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(lThreadID) != m_p[p]->r[f].end())
            {
              for (auto &t : m_p[p]->r[f][lThreadID]->threads)
              {
                threads.push_back(t.second->pThread);
                delete t.second;
              }
              m_p[p]->r[f][lThreadID]->threads.clear();
              for (auto &s : m_p[p]->r[f][lThreadID]->sems)
              {
                sem_destroy(s.second->pSem);
                delete s.second->pSem;
                delete s.second;
              }
              m_p[p]->r[f][lThreadID]->sems.clear();
              for (auto &m : m_p[p]->r[f][lThreadID]->mutexes)
              {
                delete m.second;
              }
              m_p[p]->r[f][lThreadID]->mutexes.clear();
              delete m_p[p]->r[f][lThreadID];
              m_p[p]->r[f].erase(lThreadID);
              if (m_p[p]->r[f].empty())
              {
                m_p[p]->r.erase(f);
              }
            }
            m_mutex.unlock();
            while (!threads.empty())
            {
              threads.front()->join();
              delete threads.front();
              threads.pop_front();
            }
            delete pbKill;
          }
          live("Maestro", "", ptLive);
          delete ptLive;
          if (JS_IsException(jsres))
          {
            JSValue jsexc = JS_GetException(jsctx);
            if (JS_IsError(jsctx, jsexc))
            {
              JSValue jsmsg = JS_GetPropertyStr(jsctx, jsexc, "message");
              if (JS_IsException(jsmsg))
              {
                JSValue jssubexc = JS_GetException(jsctx);
                const char *pszMessage = JS_ToCString(jsctx, jssubexc);
                if (pszMessage != NULL)
                {
                  e = pszMessage;
                  JS_FreeCString(jsctx, pszMessage);
                }
                else
                {
                  e = "Failed to retrieve the exception from the message from the exception from the script.";
                }
                JS_FreeValue(jsctx, jssubexc);
              }
              else if (JS_IsString(jsmsg))
              {
                size_t unLength = 0;
                const char *pszMessage = JS_ToCStringLen(jsctx, &unLength, jsmsg);
                if (pszMessage != NULL)
                {
                  JSValue jsstk = JS_GetPropertyStr(jsctx, jsexc, "stack");
                  e.assign(pszMessage, unLength);
                  if (JS_IsString(jsstk))
                  {
                    const char *pszStk = JS_ToCString(jsctx, jsstk);
                    if (pszStk != NULL)
                    {
                      e += (string)" " + pszStk;
                      JS_FreeCString(jsctx, pszStk);
                    }
                  }
                  JS_FreeValue(jsctx, jsstk);
                  JS_FreeCString(jsctx, pszMessage);
                }
                else
                {
                  e = "Failed to retrieve the message within the exception from the script.";
                }
              }
              else
              {
                e = "Encountered a non-string message within the exception from the script.";
              }
              JS_FreeValue(jsctx, jsmsg);
            }
            else
            {
              e = "Encountered a non-error exception from the script.";
            }
            JS_FreeValue(jsctx, jsexc);
          }
          else
          {
            JSValue jserror = JS_GetPropertyStr(jsctx, jsglobal, "g_js_error"), jsresponse = JS_GetPropertyStr(jsctx, jsglobal, "g_js_response"), jsstatus = JS_GetPropertyStr(jsctx, jsglobal, "g_js_status");
            const char *pszError = JS_ToCString(jsctx, jserror), *pszResponse = JS_ToCString(jsctx, jsresponse), *pszStatus = JS_ToCString(jsctx, jsstatus);
            b = true;
            if (pszStatus != NULL)
            {
              o->i("Status", pszStatus);
              JS_FreeCString(jsctx, pszStatus);
            }
            JS_FreeValue(jsctx, jsstatus);
            if (pszError != NULL)
            {
              o->i("Error", pszError);
              JS_FreeCString(jsctx, pszError);
            }
            JS_FreeValue(jsctx, jserror);
            if (pszResponse != NULL)
            {
              Json *ptResponse = new Json(pszResponse);
              o->i("Response", ptResponse);
              delete ptResponse;
              JS_FreeCString(jsctx, pszResponse);
            }
            JS_FreeValue(jsctx, jsresponse);
          }
          JS_FreeValue(jsctx, jsglobal);
          JS_FreeValue(jsctx, jsres);
          JS_FreeContext(jsctx);
          JS_FreeRuntime(jsrun);
        }
        else
        {
          e = "Script not found.";
        }
      }
      else
      {
        e = "You are not the owner.";
      }
      delete ptFlow;
    }
  }

  return b;
}
// }}}
// {{{ flowLaunch()
void Maestro::flowLaunch(const string p, const string f, const string r, Json *ptFlow)
{
  long lThreadID = syscall(SYS_gettid);

  if (!ptFlow->empty({"Script"}))
  {
    bool *pbKill = NULL;
    JSRuntime *jsrun = JS_NewRuntime();
    JSContext *jsctx = JS_NewContext(jsrun);
    JSValue jsglobal = JS_GetGlobalObject(jsctx);
    Json *ptLive = new Json;
    for (auto &c : m_javascript)
    {
      if (c.second->function != NULL)
      {
        JS_SetPropertyStr(jsctx, jsglobal, c.first.c_str(), JS_NewCFunction(jsctx, c.second->function, c.first.c_str(), c.second->nLength));
      }
    }
    if (!m_strSecret.empty())
    {
      string e, strPlan;
      if (!m_manip.encodeBase64(m_manip.encryptAes(p, m_strSecret, strPlan, e, m_strCipher), strPlan).empty())
      {
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_plan", JS_NewStringLen(jsctx, strPlan.c_str(), strPlan.size()));
      }
    }
    if (!r.empty())
    {
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_request", JS_NewStringLen(jsctx, r.c_str(), r.size()));
    }
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      radialMaestroRunning *ptRunning = new radialMaestroRunning;
      pbKill = new bool(false);
      if (m_p[p]->r.find(f) == m_p[p]->r.end())
      {
        m_p[p]->r[f] = {};
      }
      time(&(ptRunning->CTime));
      ptRunning->nMutex = 0;
      ptRunning->nSem = 0;
      ptRunning->nThread = 0;
      ptRunning->pbKill = pbKill;
      m_p[p]->r[f][lThreadID] = ptRunning;
      if (!m_strSecret.empty())
      {
        string e, strThread[2];
        Json *ptThread = new Json;
        ptThread->i("p", p);
        ptThread->i("f", f);
        ptThread->i("t", to_string(lThreadID), 'n');
        ptThread->j(strThread[0]);
        delete ptThread;
        if (!m_manip.encodeBase64(m_manip.encryptAes(strThread[0], m_strSecret, strThread[1], e, m_strCipher), strThread[1]).empty())
        {
          JS_SetPropertyStr(jsctx, jsglobal, "g_js_run", JS_NewStringLen(jsctx, strThread[1].c_str(), strThread[1].size()));
        }
      }
    }
    m_mutex.unlock();
    ptLive->i("Action", "flowExec");
    ptLive->i("Plan", p);
    ptLive->i("Flow", f);
    live("Maestro", "", ptLive);
    if (pbKill != NULL)
    {
      JS_SetInterruptHandler(jsrun, maestroInterrupt, pbKill);
    }
    JSValue jsres = JS_Eval(jsctx, ptFlow->m["Script"]->v.data(), ptFlow->m["Script"]->v.size(), "<embedded>", JS_EVAL_TYPE_GLOBAL);
    if (pbKill != NULL)
    {
      list<thread *> threads;
      JS_SetInterruptHandler(jsrun, NULL, NULL);
      (*pbKill) = true;
      m_mutex.lock();
      if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(lThreadID) != m_p[p]->r[f].end())
      {
        for (auto &t : m_p[p]->r[f][lThreadID]->threads)
        {
          threads.push_back(t.second->pThread);
          delete t.second;
        }
        m_p[p]->r[f][lThreadID]->threads.clear();
        for (auto &s : m_p[p]->r[f][lThreadID]->sems)
        {
          sem_destroy(s.second->pSem);
          delete s.second->pSem;
          delete s.second;
        }
        m_p[p]->r[f][lThreadID]->sems.clear();
        for (auto &m : m_p[p]->r[f][lThreadID]->mutexes)
        {
          delete m.second;
        }
        m_p[p]->r[f][lThreadID]->mutexes.clear();
        delete m_p[p]->r[f][lThreadID];
        m_p[p]->r[f].erase(lThreadID);
        if (m_p[p]->r[f].empty())
        {
          m_p[p]->r.erase(f);
        }
      }
      m_mutex.unlock();
      while (!threads.empty())
      {
        threads.front()->join();
        delete threads.front();
        threads.pop_front();
      }
      delete pbKill;
    }
    live("Maestro", "", ptLive);
    delete ptLive;
    JS_FreeValue(jsctx, jsglobal);
    JS_FreeValue(jsctx, jsres);
    JS_FreeContext(jsctx);
    JS_FreeRuntime(jsrun);
  }
  delete ptFlow;
}
// }}}
// {{{ flowRemove()
bool Maestro::flowRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Flow", "Plan"}, i, e))
  {
    string f = i->m["Flow"]->v, p = i->m["Plan"]->v;
    if (isOwner(d, p))
    {
      string strOwner, t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        strOwner = m_p[p]->owner;
        if (m_p[p]->f.find(f) != m_p[p]->f.end())
        {
          if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "f", f}, e))
          {
            b = true;
            if (m_p[p]->r.find(f) != m_p[p]->r.end())
            {
              for (auto &r : m_p[p]->r[f])
              {
                (*r.second->pbKill) = true;
                delete r.second;
              }
            }
            delete m_p[p]->f[f];
            m_p[p]->f.erase(f);
            if (i->val({"_broadcast"}) != "1" && m_p[p]->f.empty() && !dataDirectoryRemove(m_strHandle, {p, "f"}, e))
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | f | " << f << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | f " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Flow not found.";
        }
      }
      else
      {
        e = "Plan not found.";
      }
      m_mutex.unlock();
      if (b && i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        Json *ptLive = new Json;
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
          ptLink->i("Function", "flowRemove");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << strOwner << " | " << f << " " << char(3) << " " << char(2) << char(3) << "04Flow removed by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "flowRemove");
        ptLive->i("Plan", p);
        ptLive->i("Flow", f);
        live("Maestro", "", ptLive);
        delete ptLive;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// {{{ flows()
bool Maestro::flows(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Plan"}, i, e))
  {
    string p = i->m["Plan"]->v;
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      b = true;
      for (auto &f : m_p[p]->f)
      {
        o->pb(f.first);
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
  }

  return b;
}
// }}}
// {{{ flowThread()
void Maestro::flowThread(const string sp, string p, const string f, const string r, const int32_t nID, const string run)
{
  string e;
  Json *ptFlow = NULL;

  m_mutex.lock();
  if (p.size() < 2 || (p.substr(0, 2) != "a_" && p.substr(0, 2) != "u_"))
  {
    auto pIter = m_p.end();
    for (auto c = m_p.begin(); pIter == m_p.end() && c != m_p.end(); c++)
    {
      if (c->second->owner == p)
      {
        pIter = c;
      }
    }
    if (pIter != m_p.end())
    {
      p = pIter->first;
    }
  }
  if (m_p.find(p) != m_p.end() && m_p[p]->f.find(f) != m_p[p]->f.end())
  {
    ptFlow = new Json(m_p[p]->f[f]);
  }
  m_mutex.unlock();
  if (ptFlow != NULL)
  {
    if (sp == p || haveAccess(sp, p, {f}, e))
    {
      if (!ptFlow->empty({"Script"}))
      {
        bool *pbKill = NULL;
        JSRuntime *jsrun = JS_NewRuntime();
        JSContext *jsctx = JS_NewContext(jsrun);
        JSValue jsglobal = JS_GetGlobalObject(jsctx);
        radialMaestroThread *ptThread = NULL;
        for (auto &i : m_javascript)
        {
          if (i.second->function != NULL)
          {
            JS_SetPropertyStr(jsctx, jsglobal, i.first.c_str(), JS_NewCFunction(jsctx, i.second->function, i.first.c_str(), i.second->nLength));
          }
        }
        if (!m_strSecret.empty())
        {
          string strPlan;
          if (!m_manip.encodeBase64(m_manip.encryptAes(p, m_strSecret, strPlan, e, m_strCipher), strPlan).empty())
          {
            JS_SetPropertyStr(jsctx, jsglobal, "g_js_plan", JS_NewStringLen(jsctx, strPlan.c_str(), strPlan.size()));
          }
        }
        if (!r.empty())
        {
          JS_SetPropertyStr(jsctx, jsglobal, "g_js_request", JS_NewStringLen(jsctx, r.c_str(), r.size()));
        }
        if (!run.empty())
        {
          string strRun;
          if (!m_manip.decryptAes(m_manip.decodeBase64(run, strRun), m_strSecret, strRun, e, m_strCipher).empty())
          {
            Json *ptRun = new Json(strRun);
            if (dep({"f", "p", "t"}, ptRun, e))
            {
              long lThread = atoi(ptRun->m["t"]->v.c_str());
              m_mutex.lock();
              if (m_p.find(ptRun->m["p"]->v) != m_p.end() && m_p[ptRun->m["p"]->v]->r.find(ptRun->m["f"]->v) != m_p[ptRun->m["p"]->v]->r.end() && m_p[ptRun->m["p"]->v]->r[ptRun->m["f"]->v].find(lThread) != m_p[ptRun->m["p"]->v]->r[ptRun->m["f"]->v].end())
              {
                pbKill = m_p[ptRun->m["p"]->v]->r[ptRun->m["f"]->v][lThread]->pbKill;
                if (m_p[ptRun->m["p"]->v]->r[ptRun->m["f"]->v][lThread]->threads.find(nID) != m_p[ptRun->m["p"]->v]->r[ptRun->m["f"]->v][lThread]->threads.end())
                {
                  ptThread = m_p[ptRun->m["p"]->v]->r[ptRun->m["f"]->v][lThread]->threads[nID];
                }
              }
              m_mutex.unlock();
              JS_SetPropertyStr(jsctx, jsglobal, "g_js_run", JS_NewStringLen(jsctx, run.c_str(), run.size()));
            }
            delete ptRun;
          }
        }
        if (pbKill != NULL)
        {
          JS_SetInterruptHandler(jsrun, maestroInterrupt, pbKill);
        }
        JSValue jsres = JS_Eval(jsctx, ptFlow->m["Script"]->v.data(), ptFlow->m["Script"]->v.size(), "<embedded>", JS_EVAL_TYPE_GLOBAL);
        if (ptThread != NULL)
        {
          ptThread->bDone = true;
        }
        if (pbKill != NULL)
        {
          JS_SetInterruptHandler(jsrun, NULL, NULL);
        }
        if (!JS_IsException(jsres))
        {
          JSValue jserror = JS_GetPropertyStr(jsctx, jsglobal, "g_js_error"), jsresponse = JS_GetPropertyStr(jsctx, jsglobal, "g_js_response"), jsstatus = JS_GetPropertyStr(jsctx, jsglobal, "g_js_status");
          const char *pszError = JS_ToCString(jsctx, jserror), *pszResponse = JS_ToCString(jsctx, jsresponse), *pszStatus = JS_ToCString(jsctx, jsstatus);
          Json *ptResponse = new Json;
          if (pszStatus != NULL)
          {
            ptResponse->i("Status", pszStatus);
            JS_FreeCString(jsctx, pszStatus);
          }
          JS_FreeValue(jsctx, jsstatus);
          if (pszError != NULL)
          {
            ptResponse->i("Error", pszError);
            JS_FreeCString(jsctx, pszError);
          }
          JS_FreeValue(jsctx, jserror);
          if (pszResponse != NULL)
          {
            ptResponse->m["Response"] = new Json(pszResponse);
            JS_FreeCString(jsctx, pszResponse);
          }
          JS_FreeValue(jsctx, jsresponse);
          if (ptThread != NULL)
          {
            ptResponse->j(ptThread->strResponse);
          }
          delete ptResponse;
        }
        if (ptThread != NULL)
        {
          ptThread->bDone = true;
        }
        JS_FreeValue(jsctx, jsglobal);
        JS_FreeValue(jsctx, jsres);
        JS_FreeContext(jsctx);
        JS_FreeRuntime(jsrun);
      }
    }
    delete ptFlow;
  }
}
// }}}
// {{{ flowUpdate()
bool Maestro::flowUpdate(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Flow", "Plan"}, i, e))
  {
    string f = i->m["Flow"]->v, p = i->m["Plan"]->v;
    if (isOwner(d, p))
    {
      string id, strOwner, t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      getline(ssP, id);
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        strOwner = m_p[p]->owner;
        if (m_p[p]->f.find(f) != m_p[p]->f.end())
        {
          string j;
          i->j(j);
          if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "f", f}, j, e))
          {
            b = true;
            if (m_p[p]->f.find(f) != m_p[p]->f.end())
            {
              delete m_p[p]->f[f];
            }
            m_p[p]->f[f] = new Json(i);
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | f | " << f << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Flow not found.";
        }
      }
      else
      {
        e = "Plan not found.";
      }
      m_mutex.unlock();
      if (b && i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        Json *ptLive = new Json;
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
          ptLink->i("Function", "flowUpdate");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << strOwner << " | " << f << " " << char(3) << " " << char(2) << char(3) << "07Flow updated by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "flowUpdate");
        ptLive->i("Plan", p);
        ptLive->i("Flow", f);
        live("Maestro", "", ptLive);
        delete ptLive;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// }}}
// {{{ getPlan()
bool Maestro::getPlan(JSContext *jsctx, string &strPlan, string &e)
{
  bool bResult = false;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  JSValue jsplan = JS_GetPropertyStr(jsctx, jsglobal, "g_js_plan");

  if (JS_IsString(jsplan))
  {
    const char *pszPlan = JS_ToCString(jsctx, jsplan);
    if (pszPlan != NULL)
    {
      if (!m_strSecret.empty())
      {
        if (!m_manip.decryptAes(m_manip.decodeBase64(pszPlan, strPlan), m_strSecret, strPlan, e, m_strCipher).empty())
        {
          bResult = true;
        }
        else if (e.empty())
        {
          e = "Failed to decode/decrypt plan.";
        }
      }
      else
      {
        e = "Secret is not configured.";
      }
      JS_FreeCString(jsctx, pszPlan);
    }
    else
    {
      e = "Unabled to convert plan to string.";
    }
  }
  else
  {
    e = "Plan not found in context.";
  }
  JS_FreeValue(jsctx, jsplan);
  JS_FreeValue(jsctx, jsglobal);

  return bResult;
}
// }}}
// {{{ haveAccess
// {{{ haveAccess()
bool Maestro::haveAccess(const string strPlan, const string strAccess, list<string> scopes, string &strError, const bool bMutex)
{
  bool bResult = false;

  if (!strPlan.empty() && !strAccess.empty() && !scopes.empty())
  {
    if (bMutex)
    {
      m_mutex.lock();
    }
    if (m_p.find(strPlan) != m_p.end() && m_p[strPlan]->a != NULL && m_p[strPlan]->a->exist({"a", strAccess}))
    {
      for (auto i = scopes.begin(); !bResult && i != scopes.end(); i++)
      {
        for (auto j = m_p[strPlan]->a->m["a"]->m[strAccess]->l.begin(); !bResult && j != m_p[strPlan]->a->m["a"]->m[strAccess]->l.end(); j++)
        {
          if ((*i) == (*j)->v)
          {
            bResult = true;
          }
        }
      }
    }
    if (bMutex)
    {
      m_mutex.unlock();
    }
  }
  if (!bResult)
  {
    strError = "Plan is not authorized to perform this action.";
  }

  return bResult;
}
bool Maestro::haveAccess(JSContext *jsctx, const string strAccess, list<string> scopes, string &strError)
{
  bool bResult = false;
  string strPlan;

  if (getPlan(jsctx, strPlan, strError) && haveAccess(strPlan, strAccess, scopes, strError))
  {
    bResult = true;
  }

  return bResult;
}
// }}}
// {{{ haveAccessRequest()
bool Maestro::haveAccessRequest(const string strPlan, const string strAccess, const string strScope)
{
  bool bResult = false;

  if (!strPlan.empty() && !strAccess.empty() && !strScope.empty())
  {
    if (m_p.find(strPlan) != m_p.end() && m_p[strPlan]->a != NULL && m_p[strPlan]->a->exist({"r", strAccess}))
    {
      for (auto i = m_p[strPlan]->a->m["r"]->m[strAccess]->l.begin(); !bResult && i != m_p[strPlan]->a->m["r"]->m[strAccess]->l.end(); i++)
      {
        if ((*i)->v == strScope)
        {
          bResult = true;
        }
      }
    }
  }

  return bResult;
}
// }}}
// }}}
// {{{ isOwner()
bool Maestro::isOwner(radialUser &d, const string p)
{
  bool b = false;
  string e;

  if (isValid(d))
  {
    radialUser u;
    userInit(d, u);
    u.p->m["i"]->i("userid", d.u);
    if (user(u, e) && !u.p->empty({"o", "id"}) && isOwner(u.p->m["o"]->m["id"]->v, p))
    {
      b = true;
    }
    userDeinit(u);
  }
  else if (auth(d.r, e) && !d.r->empty({"User"}) && m_applications.find(d.r->m["User"]->v) != m_applications.end())
  {
    map<string, string> getApplicationRow;
    Json *ptApplication = new Json;
    ptApplication->i("name", m_applications[d.r->m["User"]->v]);
    if (db("dbCentralApplications", ptApplication, getApplicationRow, e) && !getApplicationRow["id"].empty() && isOwner(getApplicationRow["id"], p, true))
    {
      b = true;
      d.l = m_applications[d.r->m["User"]->v];
    }
    delete ptApplication;
  }

  return b;
}
bool Maestro::isOwner(const string id, const string p, const bool a)
{
  bool b = false;
  string e;

  if (p.size() > 2 && (p.substr(0, 2) == "a_" || p.substr(0, 2) == "u_"))
  {
    string i, t;
    stringstream ssP(p);
    getline(ssP, t, '_');
    getline(ssP, i);
    if (!t.empty() && !i.empty())
    {
      if (t == "a")
      {
        if (!a)
        {
          stringstream q;
          q << "select a.id from application a, application_contact b, contact_type c, person d where a.id = b.application_id and b.type_id = c.id and b.contact_id = d.id and c.type in ('Primary Developer', 'Backup Developer') and a.id = '" << esc(i) << "' and d.id = '" << esc(id) << "' limit 1";
          auto g = dbquery("central_r", q.str(), e);
          if (g != NULL && !g->empty())
          {
            b = true;
          }
          dbfree(g);
        }
        else if (id == i)
        {
          b = true;
        }
      }
      else if (!a && id == i)
      {
        b = true;
      }
    }
  }
  else
  {
    e = "Please provide a valid plan.";
  }

  return b;
}
// }}}
// {{{ js_
// {{{ js_alert()
JSValue Maestro::js_alert(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc >= 2 && argc <= 3)
  {
    const char *pszUser = JS_ToCString(jsctx, argv[0]), *pszMessage = JS_ToCString(jsctx, argv[1]), *pszHtml = ((argc >= 3)?JS_ToCString(jsctx, argv[2]):NULL);
    if (pszUser != NULL && pszMessage != NULL)
    {
      if (alert(pszUser, pszMessage, ((pszHtml != NULL)?pszHtml:""), e))
      {
        b = true;
      }
    }
    else
    {
      e = "Please provide the user and message.";
    }
    if (pszUser != NULL)
    {
      JS_FreeCString(jsctx, pszUser);
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
    if (pszHtml != NULL)
    {
      JS_FreeCString(jsctx, pszHtml);
    }
  }
  else
  {
    e = "Please provide the user and message.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_application()
JSValue Maestro::js_application(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszApplication = JS_ToCString(jsctx, argv[0]), *pszMessage = JS_ToCString(jsctx, argv[1]);
    if (pszApplication != NULL && pszMessage != NULL)
    {
      if (haveAccess(jsctx, "application", {pszApplication}, e))
      {
        Json *ptMessage = new Json(pszMessage);
        if (application(pszApplication, ptMessage, e))
        {
          b = true;
        }
        delete ptMessage;
      }
    }
    else
    {
      e = "Please provide the application and message.";
    }
    if (pszApplication != NULL)
    {
      JS_FreeCString(jsctx, pszApplication);
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
  }
  else
  {
    e = "Please provide the appllication and message.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_centralmon()
JSValue Maestro::js_centralmon(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszServer = JS_ToCString(jsctx, argv[0]), *pszProcess = JS_ToCString(jsctx, argv[1]);
    if (pszServer != NULL && pszProcess != NULL)
    {
      string strResponse;
      Json *ptResponse = new Json;
      ptResponse->m["Data"] = new Json;
      if (centralmon(pszServer, pszProcess, ptResponse->m["Data"], e))
      {
        b = true;
      }
      ptResponse->j(strResponse);
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
      delete ptResponse;
    }
    else
    {
      e = "Please provide the server and process.";
    }
    if (pszServer != NULL)
    {
      JS_FreeCString(jsctx, pszServer);
    }
    if (pszProcess != NULL)
    {
      JS_FreeCString(jsctx, pszProcess);
    }
  }
  else
  {
    e = "Please provide the server and process.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_chat()
JSValue Maestro::js_chat(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;

  if (argc >= 2 && argc <= 3)
  {
    const char *pszTarget = JS_ToCString(jsctx, argv[0]), *pszMessage = JS_ToCString(jsctx, argv[1]), *pszSource = ((argc >= 3)?JS_ToCString(jsctx, argv[2]):NULL);
    if (pszTarget != NULL && pszMessage != NULL && (argc < 3 || pszSource != NULL))
    {
      b = true;
      chat(pszTarget, pszMessage, ((argc >= 3)?pszSource:""));
    }
    if (pszTarget != NULL)
    {
      JS_FreeCString(jsctx, pszTarget);
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
    if (pszSource != NULL)
    {
      JS_FreeCString(jsctx, pszSource);
    }
  }

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_command()
JSValue Maestro::js_command(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e, p;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 5)
  {
    const char *pszCommand = JS_ToCString(jsctx, argv[0]), *pszArguments = JS_ToCString(jsctx, argv[1]), *pszInput = JS_ToCString(jsctx, argv[2]), *pszTimeout = JS_ToCString(jsctx, argv[3]), *pszNode = JS_ToCString(jsctx, argv[4]);
    if (pszCommand != NULL && pszArguments != NULL && pszInput != NULL && pszTimeout != NULL && pszNode != NULL)
    {
      if (haveAccess(jsctx, "command", {"all"}, e))
      {
        list<string> arguments;
        string strOutput, strResponse;
        stringstream ssDuration, ssTimeout(pszTimeout);
        size_t unDuration = 0;
        time_t CTimeout;
        Json *ptArguments = new Json(pszArguments), *ptResponse = new Json;
        for (auto &i : ptArguments->l)
        {
          arguments.push_back(i->v);
        }
        delete ptArguments;
        ssTimeout >> CTimeout;
        ptResponse->m["Output"] = new Json;
        if (command(pszCommand, arguments, pszInput, strOutput, unDuration, e, CTimeout, pszNode))
        {
          b = true;
        }
        ptResponse->i("Output", strOutput);
        ssDuration << unDuration;
        ptResponse->i("Duration", ssDuration.str(), 'n');
        ptResponse->j(strResponse);
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
        delete ptResponse;
      }
    }
    else
    {
      e = "Please provide the command, arguments, input, timeout, and node.";
    }
    if (pszCommand != NULL)
    {
      JS_FreeCString(jsctx, pszCommand);
    }
    if (pszArguments != NULL)
    {
      JS_FreeCString(jsctx, pszArguments);
    }
    if (pszInput != NULL)
    {
      JS_FreeCString(jsctx, pszInput);
    }
    if (pszTimeout != NULL)
    {
      JS_FreeCString(jsctx, pszTimeout);
    }
    if (pszNode != NULL)
    {
      JS_FreeCString(jsctx, pszNode);
    }
  }
  else
  {
    e = "Please provide the command, arguments, input, timeout, and node.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_db()
JSValue Maestro::js_db(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszFunction = JS_ToCString(jsctx, argv[0]), *pszInput = JS_ToCString(jsctx, argv[1]);
    if (pszFunction != NULL && pszInput != NULL)
    {
      if (haveAccess(jsctx, "db", {pszFunction}, e))
      {
        string strID, strQuery, strResponse;
        Json *ptInput = new Json(pszInput), *ptResponse = new Json;
        ptResponse->m["Response"] = new Json;
        if (db(pszFunction, ptInput, ptResponse->m["Response"], strID, strQuery, e))
        {
          b = true;
        }
        delete ptInput;
        ptResponse->i("ID", strID);
        ptResponse->i("Query", strQuery);
        ptResponse->j(strResponse);
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
        delete ptResponse;
      }
    }
    else
    {
      e = "Please provide the function and input.";
    }
    if (pszFunction != NULL)
    {
      JS_FreeCString(jsctx, pszFunction);
    }
    if (pszInput != NULL)
    {
      JS_FreeCString(jsctx, pszInput);
    }
  }
  else
  {
    e = "Please provide the function and input.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_dbquery()
JSValue Maestro::js_dbquery(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszDatabase = JS_ToCString(jsctx, argv[0]), *pszQuery = JS_ToCString(jsctx, argv[1]);
    if (pszDatabase != NULL && pszQuery != NULL)
    {
      if (haveAccess(jsctx, "dbquery", {pszDatabase}, e))
      {
        string strResponse;
        stringstream ssRows;
        unsigned long long ullRows = 0;
        Json *ptResponse = new Json;
        ptResponse->m["Response"] = new Json;
        auto get = dbquery(pszDatabase, pszQuery, ullRows, e);
        if (get != NULL)
        {
          b = true;
          for (auto &row : *get)
          {
            ptResponse->m["Response"]->pb(row);
          }
        }
        dbfree(get);
        ssRows << ullRows;
        ptResponse->i("Rows", ssRows.str(), 'n');
        ptResponse->j(strResponse);
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
        delete ptResponse;
      }
    }
    else
    {
      e = "Please provide the database and query.";
    }
    if (pszDatabase != NULL)
    {
      JS_FreeCString(jsctx, pszDatabase);
    }
    if (pszQuery != NULL)
    {
      JS_FreeCString(jsctx, pszQuery);
    }
  }
  else
  {
    e = "Please provide the database and query.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_dbupdate()
JSValue Maestro::js_dbupdate(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszDatabase = JS_ToCString(jsctx, argv[0]), *pszUpdate = JS_ToCString(jsctx, argv[1]);
    if (pszDatabase != NULL && pszUpdate != NULL)
    {
      if (haveAccess(jsctx, "dbupdate", {pszDatabase}, e))
      {
        string strResponse;
        stringstream ssID, ssRows;
        unsigned long long ullID = 0, ullRows = 0;
        Json *ptResponse = new Json;
        ptResponse->m["Response"] = new Json;
        if (dbupdate(pszDatabase, pszUpdate, ullID, ullRows, e))
        {
          b = true;
        }
        ssID << ullID;
        ptResponse->i("ID", ssID.str(), 'n');
        ssRows << ullRows;
        ptResponse->i("Rows", ssRows.str(), 'n');
        ptResponse->j(strResponse);
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
        delete ptResponse;
      }
    }
    else
    {
      e = "Please provide the database and update.";
    }
    if (pszDatabase != NULL)
    {
      JS_FreeCString(jsctx, pszDatabase);
    }
    if (pszUpdate != NULL)
    {
      JS_FreeCString(jsctx, pszUpdate);
    }
  }
  else
  {
    e = "Please provide the database and update.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_email()
JSValue Maestro::js_email(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 8)
  {
    const char *pszFrom = JS_ToCString(jsctx, argv[0]), *pszTo = JS_ToCString(jsctx, argv[1]), *pszCc = JS_ToCString(jsctx, argv[2]), *pszBcc = JS_ToCString(jsctx, argv[3]), *pszSubject = JS_ToCString(jsctx, argv[4]), *pszText = JS_ToCString(jsctx, argv[5]), *pszHtml = JS_ToCString(jsctx, argv[6]), *pszFile = JS_ToCString(jsctx, argv[7]);
    if (pszFrom != NULL && pszTo != NULL && pszCc != NULL && pszBcc != NULL && pszSubject != NULL && pszText != NULL && pszHtml != NULL && pszFile != NULL)
    {
      list<string> cc, bcc, to;
      map<string, string> file;
      Json *ptTo = new Json(pszTo), *ptCc = new Json(pszCc), *ptBcc = new Json(pszBcc), *ptFile = new Json(pszFile);
      b = true;
      for (auto &i : ptTo->l)
      {
        to.push_back(i->v);
      }
      delete ptTo;
      for (auto &i : ptCc->l)
      {
        cc.push_back(i->v);
      }
      delete ptCc;
      for (auto &i : ptBcc->l)
      {
        bcc.push_back(i->v);
      }
      delete ptBcc;
      ptFile->flatten(file, true, false);
      delete ptFile;
      email(pszFrom, to, cc, bcc, pszSubject, pszText, pszHtml, file);
    }
    else
    {
      e = "Please provide the from, to, cc, bcc, subject, text, html, file.";
    }
    if (pszFrom != NULL)
    {
      JS_FreeCString(jsctx, pszFrom);
    }
    if (pszTo != NULL)
    {
      JS_FreeCString(jsctx, pszTo);
    }
    if (pszCc != NULL)
    {
      JS_FreeCString(jsctx, pszCc);
    }
    if (pszBcc != NULL)
    {
      JS_FreeCString(jsctx, pszBcc);
    }
    if (pszSubject != NULL)
    {
      JS_FreeCString(jsctx, pszSubject);
    }
    if (pszText != NULL)
    {
      JS_FreeCString(jsctx, pszText);
    }
    if (pszHtml != NULL)
    {
      JS_FreeCString(jsctx, pszHtml);
    }
    if (pszFile != NULL)
    {
      JS_FreeCString(jsctx, pszFile);
    }
  }
  else
  {
    e = "Please provide the from, to, cc, bcc, subject, text, html, file.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_esc()
JSValue Maestro::js_esc(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  string v;

  if (argc == 1)
  {
    const char *pszValue = JS_ToCString(jsctx, argv[0]);
    if (pszValue != NULL)
    {
      v = esc(pszValue);
      JS_FreeCString(jsctx, pszValue);
    }
  }

  return JS_NewStringLen(jsctx, v.c_str(), v.size());
}
// }}}
// {{{ js_flow()
JSValue Maestro::js_flow(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e, sp;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (getPlan(jsctx, sp, e))
  {
  	if (argc >= 2 && argc <= 3)
    {
      const char *pszPlan = JS_ToCString(jsctx, argv[0]), *pszFlow = JS_ToCString(jsctx, argv[1]), *pszRequest = ((argc >= 3)?JS_ToCString(jsctx, argv[2]):NULL);
      if (pszPlan != NULL && pszFlow != NULL)
      {
        string f = pszFlow, p = pszPlan;
        Json *ptFlow = NULL;
        m_mutex.lock();
        if (p.size() < 2 || (p.substr(0, 2) != "a_" && p.substr(0, 2) != "u_"))
        {
          auto pIter = m_p.end();
          for (auto c = m_p.begin(); pIter == m_p.end() && c != m_p.end(); c++)
          {
            if (c->second->owner == p)
            {
              pIter = c;
            }
          }
          if (pIter != m_p.end())
          {
            p = pIter->first;
          }
        }
        if (m_p.find(p) != m_p.end())
        {
          if (m_p[p]->f.find(f) != m_p[p]->f.end())
          {
            ptFlow = new Json(m_p[p]->f[f]);
          }
          else
          {
            e = "Flow not found.";
          }
        }
        else
        {
          e = "Plan not found.";
        }
        m_mutex.unlock();
        if (ptFlow != NULL)
        {
          if (sp == p || haveAccess(jsctx, p, {f}, e))
          {
            if (!ptFlow->empty({"Script"}))
            {
              bool *pbKill = NULL;
              JSRuntime *jssubrun = JS_NewRuntime();
              JSContext *jssubctx = JS_NewContext(jssubrun);
              JSValue jssubglobal = JS_GetGlobalObject(jssubctx), jsthread = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
              const char *pszThread = JS_ToCString(jsctx, jsthread);
              for (auto &i : m_javascript)
              {
                if (i.second->function != NULL)
                {
                  JS_SetPropertyStr(jssubctx, jssubglobal, i.first.c_str(), JS_NewCFunction(jssubctx, i.second->function, i.first.c_str(), i.second->nLength));
                }
              }
              if (!m_strSecret.empty())
              {
                string strPlan;
                if (!m_manip.encodeBase64(m_manip.encryptAes(p, m_strSecret, strPlan, e, m_strCipher), strPlan).empty())
                {
                  JS_SetPropertyStr(jssubctx, jssubglobal, "g_js_plan", JS_NewStringLen(jssubctx, strPlan.c_str(), strPlan.size()));
                }
              }
              if (pszRequest != NULL)
              {
                JS_SetPropertyStr(jssubctx, jssubglobal, "g_js_request", JS_NewStringLen(jssubctx, pszRequest, strlen(pszRequest)));
              }
              if (pszThread != NULL)
              {
                string strThread;
                if (!m_manip.decryptAes(m_manip.decodeBase64(pszThread, strThread), m_strSecret, strThread, e, m_strCipher).empty())
                {
                  Json *ptThread = new Json(strThread);;
                  if (dep({"f", "p", "t"}, ptThread, e))
                  {
                    long lThread = atoi(ptThread->m["t"]->v.c_str());
                    m_mutex.lock();
                    if (m_p.find(ptThread->m["p"]->v) != m_p.end() && m_p[ptThread->m["p"]->v]->r.find(ptThread->m["f"]->v) != m_p[ptThread->m["p"]->v]->r.end() && m_p[ptThread->m["p"]->v]->r[ptThread->m["f"]->v].find(lThread) != m_p[ptThread->m["p"]->v]->r[ptThread->m["f"]->v].end())
                    {
                      pbKill = m_p[ptThread->m["p"]->v]->r[ptThread->m["f"]->v][lThread]->pbKill;
                    }
                    m_mutex.unlock();
                    JS_SetPropertyStr(jssubctx, jssubglobal, "g_js_run", JS_NewStringLen(jssubctx, pszThread, strlen(pszThread)));
                  }
                  delete ptThread;
                }
                JS_FreeCString(jsctx, pszThread);
              }
              if (pbKill != NULL)
              {
                JS_SetInterruptHandler(jssubrun, maestroInterrupt, pbKill);
              }
              JSValue jssubres = JS_Eval(jssubctx, ptFlow->m["Script"]->v.data(), ptFlow->m["Script"]->v.size(), "<embedded>", JS_EVAL_TYPE_GLOBAL);
              if (pbKill != NULL)
              {
                JS_SetInterruptHandler(jssubrun, NULL, NULL);
              }
              if (JS_IsException(jssubres))
              {
                JSValue jssubexc = JS_GetException(jssubctx);
                if (JS_IsError(jssubctx, jssubexc))
                {
                  JSValue jssubmsg = JS_GetPropertyStr(jssubctx, jssubexc, "message");
                  if (JS_IsException(jssubmsg))
                  {
                    JSValue jsdeepexc = JS_GetException(jssubctx);
                    const char *pszMessage = JS_ToCString(jssubctx, jsdeepexc);
                    if (pszMessage != NULL)
                    {
                      e = pszMessage;
                      JS_FreeCString(jssubctx, pszMessage);
                    }
                    else
                    {
                      e = "Failed to retrieve the exception from the message from the exception from the script.";
                    }
                    JS_FreeValue(jssubctx, jsdeepexc);
                  }
                  else if (JS_IsString(jssubmsg))
                  {
                    size_t unLength = 0;
                    const char *pszMessage = JS_ToCStringLen(jssubctx, &unLength, jssubmsg);
                    if (pszMessage != NULL)
                    {
                      JSValue jssubstk = JS_GetPropertyStr(jssubctx, jssubexc, "stack");
                      e.assign(pszMessage, unLength);
                      if (JS_IsString(jssubstk))
                      {
                        const char *pszStk = JS_ToCString(jssubctx, jssubstk);
                        if (pszStk != NULL)
                        {
                          e += (string)" " + pszStk;
                          JS_FreeCString(jssubctx, pszStk);
                        }
                      }
                      JS_FreeValue(jssubctx, jssubstk);
                      JS_FreeCString(jssubctx, pszMessage);
                    }
                    else
                    {
                      e = "Failed to retrieve the message within the exception from the script.";
                    }
                  }
                  else
                  {
                    e = "Encountered a non-string message within the exception from the script.";
                  }
                }
                else
                {
                  e = "Encountered a non-error exception from the script.";
                }
                JS_FreeValue(jssubctx, jssubexc);
              }
              else
              {
                string strResponse;
                JSValue jssuberror = JS_GetPropertyStr(jssubctx, jssubglobal, "g_js_error"), jssubresponse = JS_GetPropertyStr(jssubctx, jssubglobal, "g_js_response"), jssubstatus = JS_GetPropertyStr(jssubctx, jssubglobal, "g_js_status");
                const char *pszError = JS_ToCString(jssubctx, jssuberror), *pszResponse = JS_ToCString(jssubctx, jssubresponse), *pszStatus = JS_ToCString(jssubctx, jssubstatus);
                Json *ptResponse = new Json;
                b = true;
                if (pszStatus != NULL)
                {
                  ptResponse->i("Status", pszStatus);
                  JS_FreeCString(jssubctx, pszStatus);
                }
                JS_FreeValue(jsctx, jssubstatus);
                if (pszError != NULL)
                {
                  ptResponse->i("Error", pszError);
                  JS_FreeCString(jssubctx, pszError);
                }
                JS_FreeValue(jsctx, jssuberror);
                if (pszResponse != NULL)
                {
                  ptResponse->m["Response"] = new Json(pszResponse);
                  JS_FreeCString(jssubctx, pszResponse);
                }
                JS_FreeValue(jsctx, jssubresponse);
                ptResponse->j(strResponse);
                delete ptResponse;
                JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, strResponse.c_str(), strResponse.size()));
              }
              JS_FreeValue(jssubctx, jssubglobal);
              JS_FreeValue(jssubctx, jssubres);
              JS_FreeContext(jssubctx);
              JS_FreeRuntime(jssubrun);
            }
            else
            {
              e = "Script not found.";
            }
          }
          delete ptFlow;
        }
      }
      else
      {
        e = "Please provide the plan and flow.";
      }
      if (pszRequest != NULL)
      {
        JS_FreeCString(jsctx, pszRequest);
      }
      if (pszFlow != NULL)
      {
        JS_FreeCString(jsctx, pszFlow);
      }
      if (pszPlan != NULL)
      {
        JS_FreeCString(jsctx, pszPlan);
      }
    }
    else
    {
      e = "Please provide the plan and flow.";
    }
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_getError()
JSValue Maestro::js_getError(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  JSValue jserror = JS_GetPropertyStr(jsctx, jsglobal, "g_js_error");

  JS_FreeValue(jsctx, jsglobal);

  return jserror;
}
// }}}
// {{{ js_getRequest()
JSValue Maestro::js_getRequest(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  JSValue jsrequest = JS_GetPropertyStr(jsctx, jsglobal, "g_js_request");

  JS_FreeValue(jsctx, jsglobal);

  return jsrequest;
}
// }}}
// {{{ js_getResponse()
JSValue Maestro::js_getResponse(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  JSValue jsresponse = JS_GetPropertyStr(jsctx, jsglobal, "g_js_internal_response");

  JS_FreeValue(jsctx, jsglobal);

  return jsresponse;
}
// }}}
// {{{ js_getStatus()
JSValue Maestro::js_getStatus(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  JSValue jsstatus = JS_GetPropertyStr(jsctx, jsglobal, "g_js_internal_status");

  JS_FreeValue(jsctx, jsglobal);

  return jsstatus;
}
// }}}
// {{{ js_junction()
JSValue Maestro::js_junction(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszRequest = JS_ToCString(jsctx, argv[0]);
    if (pszRequest != NULL)
    {
      string j;
      list<Json *> in, out;
      Json *ptRequest = new Json(pszRequest), *ptResponse = new Json;
      for (auto &i : ptRequest->l)
      {
        in.push_back(i);
      }
      if (junction(in, out, e))
      {
        b = true;
      }
      delete ptRequest;
      for (auto &i : out)
      {
        ptResponse->l.push_back(i);
      }
      ptResponse->j(j);
      delete ptResponse;
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, j.c_str(), j.size()));
    }
    else
    {
      e = "Please provide the request.";
    }
    if (pszRequest != NULL)
    {
      JS_FreeCString(jsctx, pszRequest);
    }
  }
  else
  {
    e = "Please provide the request.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_live()
JSValue Maestro::js_live(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc >= 3 && argc <= 4)
  {
    const char *pszApplication = JS_ToCString(jsctx, argv[0]), *pszUser = JS_ToCString(jsctx, argv[1]), *pszMessage = JS_ToCString(jsctx, argv[2]), *pszWait = ((argc >= 4)?JS_ToCString(jsctx, argv[3]):NULL);
    if (pszApplication != NULL && pszUser != NULL && pszMessage != NULL && (argc < 4 || pszWait != NULL))
    {
      bool bWait = (pszWait != NULL && (string)pszWait == "true");
      Json *ptMessage = new Json(pszMessage);
      b = true;
      live(pszApplication, pszUser, ptMessage, bWait);
      delete ptMessage;
    }
    else
    {
      e = "Please provide the application, user, and message.";
    }
    if (pszApplication != NULL)
    {
      JS_FreeCString(jsctx, pszApplication);
    }
    if (pszUser != NULL)
    {
      JS_FreeCString(jsctx, pszUser);
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
    if (pszWait != NULL)
    {
      JS_FreeCString(jsctx, pszWait);
    }
  }
  else
  {
    e = "Please provide the application, user, and message.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_log()
JSValue Maestro::js_log(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszMessage = JS_ToCString(jsctx, argv[0]);
    if (pszMessage != NULL)
    {
      if (haveAccess(jsctx, "log", {"all"}, e))
      {
        string strPlan;
        stringstream ssMessage;
        b = true;
        getPlan(jsctx, strPlan, e);
        ssMessage << "maestro->main()->Maestro::js_log() [" << strPlan << "]:  " << pszMessage;
        log(ssMessage.str());
      }
    }
    else
    {
      e = "Please provide the message.";
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
  }
  else
  {
    e = "Please provide the message.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_logger()
JSValue Maestro::js_logger(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 4)
  {
    const char *pszApplication = JS_ToCString(jsctx, argv[0]), *pszFunction = JS_ToCString(jsctx, argv[1]), *pszLabel = JS_ToCString(jsctx, argv[2]), *pszMessage = JS_ToCString(jsctx, argv[3]);
    if (pszApplication != NULL && pszFunction != NULL && pszLabel != NULL && pszMessage != NULL)
    {
      if (haveAccess(jsctx, "logger", {pszApplication}, e))
      {
        map<string, string> label;
        Json *ptLabel = new Json(pszLabel);
        b = true;
        ptLabel->flatten(label, true, false);
        delete ptLabel;
        logger(pszApplication, pszFunction, label, pszMessage);
      }
    }
    else
    {
      e = "Please provide the application, function, label, and message.";
    }
    if (pszApplication != NULL)
    {
      JS_FreeCString(jsctx, pszApplication);
    }
    if (pszFunction != NULL)
    {
      JS_FreeCString(jsctx, pszFunction);
    }
    if (pszLabel != NULL)
    {
      JS_FreeCString(jsctx, pszLabel);
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
  }
  else
  {
    e = "Please provide the application, function, label, and message.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_mutexCreate()
JSValue Maestro::js_mutexCreate(JSContext *jsctx, JSValueConst na, int argc, JSValueConst *argv)
{
  int32_t nID = -1;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx), jsrun = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
  const char *pszRun = JS_ToCString(jsctx, jsrun);

  if (pszRun != NULL)
  {
    string strRun;
    if (!m_manip.decryptAes(m_manip.decodeBase64(pszRun, strRun), m_strSecret, strRun, e, m_strCipher).empty())
    {
      Json *r = new Json(strRun);
      if (dep({"f", "p", "t"}, r, e))
      {
        long t = atol(r->m["t"]->v.c_str());
        string p = r->m["p"]->v, f = r->m["f"]->v;
        m_mutex.lock();
        if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end())
        {
          while (m_p[p]->r[f][t]->nMutex != -1 && m_p[p]->r[f][t]->mutexes.find(m_p[p]->r[f][t]->nMutex) != m_p[p]->r[f][t]->mutexes.end())
          {
            m_p[p]->r[f][t]->nMutex++;
          }
          nID = m_p[p]->r[f][t]->nMutex;
          m_p[p]->r[f][t]->mutexes[nID] = new mutex;
        }
        else
        {
          e = "Failed to find plan, flow, and thread.";
        }
        m_mutex.unlock();
      }
      delete r;
    }
    JS_FreeCString(jsctx, pszRun);
  }
  else
  {
    e = "Failed to retrieve g_js_run.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewInt32(jsctx, nID);
}
// }}}
// {{{ js_mutexDestroy()
JSValue Maestro::js_mutexDestroy(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx), jsrun = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
  const char *pszRun = JS_ToCString(jsctx, jsrun);

  if (pszRun != NULL)
  {
    string strRun;
    if (!m_manip.decryptAes(m_manip.decodeBase64(pszRun, strRun), m_strSecret, strRun, e, m_strCipher).empty())
    {
      Json *r = new Json(strRun);
      if (dep({"f", "p", "t"}, r, e))
      {
        long t = atol(r->m["t"]->v.c_str());
        string p = r->m["p"]->v, f = r->m["f"]->v;
        if (argc == 1)
        {
          const char *pszID = JS_ToCString(jsctx, argv[0]);
          if (pszID != NULL)
          {
            int32_t nID;
            stringstream ssID(pszID);
            mutex *pMutex = NULL;
            ssID >> nID;
            m_mutex.lock();
            if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end() && m_p[p]->r[f][t]->mutexes.find(nID) != m_p[p]->r[f][t]->mutexes.end())
            {
              pMutex = m_p[p]->r[f][t]->mutexes[nID];
              m_p[p]->r[f][t]->mutexes.erase(nID);
            }
            m_mutex.unlock();
            if (pMutex != NULL)
            {
              b = true;
              delete pMutex;
            }
            else
            {
              e = "Please provide a valid id.";
            }
          }
          else
          {
            e = "Please provide the id.";
          }
          if (pszID != NULL)
          {
            JS_FreeCString(jsctx, pszID);
          }
        }
        else
        {
          e = "Please provide the id.";
        }
      }
      delete r;
    }
    JS_FreeCString(jsctx, pszRun);
  }
  else
  {
    e = "Failed to retrieve g_js_run.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_mutexLock()
JSValue Maestro::js_mutexLock(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx), jsrun = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
  const char *pszRun = JS_ToCString(jsctx, jsrun);

  if (pszRun != NULL)
  {
    string strRun;
    if (!m_manip.decryptAes(m_manip.decodeBase64(pszRun, strRun), m_strSecret, strRun, e, m_strCipher).empty())
    {
      Json *r = new Json(strRun);
      if (dep({"f", "p", "t"}, r, e))
      {
        long t = atol(r->m["t"]->v.c_str());
        string p = r->m["p"]->v, f = r->m["f"]->v;
        if (argc == 1)
        {
          const char *pszID = JS_ToCString(jsctx, argv[0]);
          if (pszID != NULL)
          {
            int32_t nID;
            stringstream ssID(pszID);
            mutex *pMutex = NULL;
            ssID >> nID;
            m_mutex.lock();
            if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end() && m_p[p]->r[f][t]->mutexes.find(nID) != m_p[p]->r[f][t]->mutexes.end())
            {
              pMutex = m_p[p]->r[f][t]->mutexes[nID];
            }
            m_mutex.unlock();
            if (pMutex != NULL)
            {
              b = true;
              pMutex->lock();
            }
            else
            {
              e = "Please provide a valid id.";
            }
          }
          else
          {
            e = "Please provide the id.";
          }
          if (pszID != NULL)
          {
            JS_FreeCString(jsctx, pszID);
          }
        }
        else
        {
          e = "Please provide the id.";
        }
      }
      delete r;
    }
    JS_FreeCString(jsctx, pszRun);
  }
  else
  {
    e = "Failed to retrieve g_js_run.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_mutexUnlock()
JSValue Maestro::js_mutexUnlock(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx), jsrun = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
  const char *pszRun = JS_ToCString(jsctx, jsrun);

  if (pszRun != NULL)
  {
    string strRun;
    if (!m_manip.decryptAes(m_manip.decodeBase64(pszRun, strRun), m_strSecret, strRun, e, m_strCipher).empty())
    {
      Json *r = new Json(strRun);
      if (dep({"f", "p", "t"}, r, e))
      {
        long t = atol(r->m["t"]->v.c_str());
        string p = r->m["p"]->v, f = r->m["f"]->v;
        if (argc == 1)
        {
          const char *pszID = JS_ToCString(jsctx, argv[0]);
          if (pszID != NULL)
          {
            int32_t nID;
            stringstream ssID(pszID);
            mutex *pMutex = NULL;
            ssID >> nID;
            m_mutex.lock();
            if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end() && m_p[p]->r[f][t]->mutexes.find(nID) != m_p[p]->r[f][t]->mutexes.end())
            {
              pMutex = m_p[p]->r[f][t]->mutexes[nID];
            }
            m_mutex.unlock();
            if (pMutex != NULL)
            {
              b = true;
              pMutex->unlock();
            }
            else
            {
              e = "Please provide a valid id.";
            }
          }
          else
          {
            e = "Please provide the id.";
          }
          if (pszID != NULL)
          {
            JS_FreeCString(jsctx, pszID);
          }
        }
        else
        {
          e = "Please provide the id.";
        }
      }
      delete r;
    }
    JS_FreeCString(jsctx, pszRun);
  }
  else
  {
    e = "Failed to retrieve g_js_run.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_mysqlQuery()
JSValue Maestro::js_mysqlQuery(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 6)
  {
    const char *pszServer = JS_ToCString(jsctx, argv[0]), *pszPort = JS_ToCString(jsctx, argv[1]), *pszUser = JS_ToCString(jsctx, argv[2]), *pszPassword = JS_ToCString(jsctx, argv[3]), *pszDatabase = JS_ToCString(jsctx, argv[4]), *pszQuery = JS_ToCString(jsctx, argv[5]);
    if (pszServer != NULL && pszPort != NULL && pszUser != NULL && pszPassword != NULL && pszDatabase != NULL && pszQuery != NULL)
    {
      list<map<string, string> > rows;
      unsigned int unPort = atoi(pszPort);
      unsigned long long ullRows = 0;
      if (mysqlQuery(pszServer, unPort, pszUser, pszPassword, pszDatabase, pszQuery, ullRows, rows, e))
      {
        string j;
        stringstream ssRows;
        Json *ptResponse = new Json;
        b = true;
        ssRows << ullRows;
        ptResponse->i("Rows", ssRows.str(), 'n');
        ptResponse->m["Response"] = new Json;
        for (auto &row : rows)
        {
          ptResponse->m["Response"]->pb(row);
        }
        ptResponse->j(j);
        delete ptResponse;
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, j.c_str(), j.size()));
      }
    }
    else
    {
      e = "Please provide the server, port, user, password, database, query.";
    }
    if (pszServer != NULL)
    {
      JS_FreeCString(jsctx, pszServer);
    }
    if (pszPort != NULL)
    {
      JS_FreeCString(jsctx, pszPort);
    }
    if (pszUser != NULL)
    {
      JS_FreeCString(jsctx, pszUser);
    }
    if (pszPassword != NULL)
    {
      JS_FreeCString(jsctx, pszPassword);
    }
    if (pszDatabase != NULL)
    {
      JS_FreeCString(jsctx, pszDatabase);
    }
    if (pszQuery != NULL)
    {
      JS_FreeCString(jsctx, pszQuery);
    }
  }
  else
  {
    e = "Please provide the server, port, user, password, database, query.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_mysqlUpdate()
JSValue Maestro::js_mysqlUpdate(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 6)
  {
    const char *pszServer = JS_ToCString(jsctx, argv[0]), *pszPort = JS_ToCString(jsctx, argv[1]), *pszUser = JS_ToCString(jsctx, argv[2]), *pszPassword = JS_ToCString(jsctx, argv[3]), *pszDatabase = JS_ToCString(jsctx, argv[4]), *pszQuery = JS_ToCString(jsctx, argv[5]);
    if (pszServer != NULL && pszPort != NULL && pszUser != NULL && pszPassword != NULL && pszDatabase != NULL && pszQuery != NULL)
    {
      list<map<string, string> > rows;
      unsigned int unPort = atoi(pszPort);
      unsigned long long ullID = 0, ullRows = 0;
      if (mysqlUpdate(pszServer, unPort, pszUser, pszPassword, pszDatabase, pszQuery, ullID, ullRows, e))
      {
        string j;
        stringstream ssID, ssRows;
        Json *ptResponse = new Json;
        b = true;
        ssID << ullID;
        ptResponse->i("ID", ssID.str(), 'n');
        ssRows << ullRows;
        ptResponse->i("Rows", ssRows.str(), 'n');
        ptResponse->j(j);
        delete ptResponse;
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, j.c_str(), j.size()));
      }
    }
    else
    {
      e = "Please provide the server, port, user, password, database, query.";
    }
    if (pszServer != NULL)
    {
      JS_FreeCString(jsctx, pszServer);
    }
    if (pszPort != NULL)
    {
      JS_FreeCString(jsctx, pszPort);
    }
    if (pszUser != NULL)
    {
      JS_FreeCString(jsctx, pszUser);
    }
    if (pszPassword != NULL)
    {
      JS_FreeCString(jsctx, pszPassword);
    }
    if (pszDatabase != NULL)
    {
      JS_FreeCString(jsctx, pszDatabase);
    }
    if (pszQuery != NULL)
    {
      JS_FreeCString(jsctx, pszQuery);
    }
  }
  else
  {
    e = "Please provide the server, port, user, password, database, query.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_notify()
JSValue Maestro::js_notify(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszMessage = JS_ToCString(jsctx, argv[0]);
    if (pszMessage != NULL)
    {
      if (haveAccess(jsctx, "notify", {"all"}, e))
      {
        string strPlan;
        stringstream ssMessage;
        b = true;
        getPlan(jsctx, strPlan, e);
        ssMessage << "maestro->main()->Maestro::js_notify() [" << strPlan << "]:  " << pszMessage;
        notify(ssMessage.str());
      }
    }
    else
    {
      e = "Please provide the message.";
    }
    if (pszMessage != NULL)
    {
      JS_FreeCString(jsctx, pszMessage);
    }
  }
  else
  {
    e = "Please provide the message.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_semCreate()
JSValue Maestro::js_semCreate(JSContext *jsctx, JSValueConst na, int argc, JSValueConst *argv)
{
  int32_t nID = -1;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx), jsrun = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
  const char *pszRun = JS_ToCString(jsctx, jsrun);

  if (pszRun != NULL)
  {
    string strRun;
    if (!m_manip.decryptAes(m_manip.decodeBase64(pszRun, strRun), m_strSecret, strRun, e, m_strCipher).empty())
    {
      Json *r = new Json(strRun);
      if (dep({"f", "p", "t"}, r, e))
      {
        long t = atol(r->m["t"]->v.c_str());
        string p = r->m["p"]->v, f = r->m["f"]->v;
        if (argc == 1)
        {
          const char *pszValue = JS_ToCString(jsctx, argv[0]);
          if (pszValue != NULL)
          {
            sem_t *pSem = new sem_t;
            stringstream ssValue(pszValue);
            unsigned int unValue;
            ssValue >> unValue;
            if (sem_init(pSem, 0, unValue) == 0)
            {
              radialMaestroSem *ptSem = new radialMaestroSem;
              ptSem->pSem = pSem;
              ptSem->unInUse = 0;
              m_mutex.lock();
              if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end())
              {
                while (m_p[p]->r[f][t]->nSem != -1 && m_p[p]->r[f][t]->sems.find(m_p[p]->r[f][t]->nSem) != m_p[p]->r[f][t]->sems.end())
                {
                  m_p[p]->r[f][t]->nSem++;
                }
                nID = m_p[p]->r[f][t]->nSem;
                m_p[p]->r[f][t]->sems[nID] = ptSem;
              }
              else
              {
                e = "Failed to find plan, flow, and thread.";
                sem_destroy(pSem);
                delete pSem;
                delete ptSem;
              }
              m_mutex.unlock();
            }
            else
            {
              delete pSem;
            }
          }
          else
          {
            e = "Please provide the id.";
          }
          if (pszValue != NULL)
          {
            JS_FreeCString(jsctx, pszValue);
          }
        }
        else
        {
          e = "Please provide the id.";
        }
      }
      delete r;
    }
    JS_FreeCString(jsctx, pszRun);
  }
  else
  {
    e = "Failed to retrieve g_js_run.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewInt32(jsctx, nID);
}
// }}}
// {{{ js_semDestroy()
JSValue Maestro::js_semDestroy(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx), jsrun = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
  const char *pszRun = JS_ToCString(jsctx, jsrun);

  if (pszRun != NULL)
  {
    string strRun;
    if (!m_manip.decryptAes(m_manip.decodeBase64(pszRun, strRun), m_strSecret, strRun, e, m_strCipher).empty())
    {
      Json *r = new Json(strRun);
      if (dep({"f", "p", "t"}, r, e))
      {
        long t = atol(r->m["t"]->v.c_str());
        string p = r->m["p"]->v, f = r->m["f"]->v;
        if (argc == 1)
        {
          const char *pszID = JS_ToCString(jsctx, argv[0]);
          if (pszID != NULL)
          {
            int32_t nID;
            stringstream ssID(pszID);
            radialMaestroSem *ptSem = NULL;
            ssID >> nID;
            m_mutex.lock();
            if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end() && m_p[p]->r[f][t]->sems.find(nID) != m_p[p]->r[f][t]->sems.end())
            {
              ptSem = m_p[p]->r[f][t]->sems[nID];
            }
            m_mutex.unlock();
            if (ptSem != NULL)
            {
              b = true;
              while (!shutdown() && ptSem->unInUse > 0)
              {
                msleep(100);
              }
              sem_destroy(ptSem->pSem);
              delete ptSem->pSem;
              delete ptSem;
            }
            else
            {
              e = "Please provide a valid id.";
            }
            m_mutex.lock();
            if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end() && m_p[p]->r[f][t]->sems.find(nID) != m_p[p]->r[f][t]->sems.end())
            {
              m_p[p]->r[f][t]->sems.erase(nID);
            }
            m_mutex.unlock();
          }
          else
          {
            e = "Please provide the id.";
          }
          if (pszID != NULL)
          {
            JS_FreeCString(jsctx, pszID);
          }
        }
        else
        {
          e = "Please provide the id.";
        }
      }
      delete r;
    }
    JS_FreeCString(jsctx, pszRun);
  }
  else
  {
    e = "Failed to retrieve g_js_run.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_semPost()
JSValue Maestro::js_semPost(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx), jsrun = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
  const char *pszRun = JS_ToCString(jsctx, jsrun);

  if (pszRun != NULL)
  {
    string strRun;
    if (!m_manip.decryptAes(m_manip.decodeBase64(pszRun, strRun), m_strSecret, strRun, e, m_strCipher).empty())
    {
      Json *r = new Json(strRun);
      if (dep({"f", "p", "t"}, r, e))
      {
        long t = atol(r->m["t"]->v.c_str());
        string p = r->m["p"]->v, f = r->m["f"]->v;
        if (argc == 1)
        {
          const char *pszID = JS_ToCString(jsctx, argv[0]);
          if (pszID != NULL)
          {
            int32_t nID;
            stringstream ssID(pszID);
            radialMaestroSem *ptSem = NULL;
            ssID >> nID;
            m_mutex.lock();
            if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end() && m_p[p]->r[f][t]->sems.find(nID) != m_p[p]->r[f][t]->sems.end())
            {
              ptSem = m_p[p]->r[f][t]->sems[nID];
            }
            m_mutex.unlock();
            if (ptSem != NULL)
            {
              b = true;
              sem_post(ptSem->pSem);
              if (ptSem->unInUse > 0)
              {
                ptSem->unInUse--;
              }
            }
            else
            {
              e = "Please provide a valid id.";
            }
          }
          else
          {
            e = "Please provide the id.";
          }
          if (pszID != NULL)
          {
            JS_FreeCString(jsctx, pszID);
          }
        }
        else
        {
          e = "Please provide the id.";
        }
      }
      delete r;
    }
    JS_FreeCString(jsctx, pszRun);
  }
  else
  {
    e = "Failed to retrieve g_js_run.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_semWait()
JSValue Maestro::js_semWait(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx), jsrun = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
  const char *pszRun = JS_ToCString(jsctx, jsrun);

  if (pszRun != NULL)
  {
    string strRun;
    if (!m_manip.decryptAes(m_manip.decodeBase64(pszRun, strRun), m_strSecret, strRun, e, m_strCipher).empty())
    {
      Json *r = new Json(strRun);
      if (dep({"f", "p", "t"}, r, e))
      {
        long t = atol(r->m["t"]->v.c_str());
        string p = r->m["p"]->v, f = r->m["f"]->v;
        if (argc == 1)
        {
          const char *pszID = JS_ToCString(jsctx, argv[0]);
          if (pszID != NULL)
          {
            int32_t nID;
            stringstream ssID(pszID);
            radialMaestroSem *ptSem = NULL;
            ssID >> nID;
            m_mutex.lock();
            if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end() && m_p[p]->r[f][t]->sems.find(nID) != m_p[p]->r[f][t]->sems.end())
            {
              ptSem = m_p[p]->r[f][t]->sems[nID];
            }
            m_mutex.unlock();
            if (ptSem != NULL)
            {
              b = true;
              sem_wait(ptSem->pSem);
              ptSem->unInUse++;
            }
            else
            {
              e = "Please provide a valid id.";
            }
          }
          else
          {
            e = "Please provide the id.";
          }
          if (pszID != NULL)
          {
            JS_FreeCString(jsctx, pszID);
          }
        }
        else
        {
          e = "Please provide the id.";
        }
      }
      delete r;
    }
    JS_FreeCString(jsctx, pszRun);
  }
  else
  {
    e = "Failed to retrieve g_js_run.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_setError()
JSValue Maestro::js_setError(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszError = JS_ToCString(jsctx, argv[0]);
    if (pszError != NULL)
    {
      b = true;
      JSValue jsglobal = JS_GetGlobalObject(jsctx);
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, pszError, strlen(pszError)));
      JS_FreeValue(jsctx, jsglobal);
    }
    else
    {
      e = "Please provide the error.";
    }
    if (pszError != NULL)
    {
      JS_FreeCString(jsctx, pszError);
    }
  }
  else
  {
    e = "Please provide the error.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_setRequest()
JSValue Maestro::js_setRequest(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszRequest = JS_ToCString(jsctx, argv[0]);
    if (pszRequest != NULL)
    {
      b = true;
      JSValue jsglobal = JS_GetGlobalObject(jsctx);
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_request", JS_NewStringLen(jsctx, pszRequest, strlen(pszRequest)));
      JS_FreeValue(jsctx, jsglobal);
    }
    else
    {
      e = "Please provide the nequest.";
    }
    if (pszRequest != NULL)
    {
      JS_FreeCString(jsctx, pszRequest);
    }
  }
  else
  {
    e = "Please provide the request.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_setResponse()
JSValue Maestro::js_setResponse(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszResponse = JS_ToCString(jsctx, argv[0]);
    if (pszResponse != NULL)
    {
      b = true;
      JSValue jsglobal = JS_GetGlobalObject(jsctx);
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_response", JS_NewStringLen(jsctx, pszResponse, strlen(pszResponse)));
      JS_FreeValue(jsctx, jsglobal);
    }
    else
    {
      e = "Please provide the response.";
    }
    if (pszResponse != NULL)
    {
      JS_FreeCString(jsctx, pszResponse);
    }
  }
  else
  {
    e = "Please provide the response.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_setStatus()
JSValue Maestro::js_setStatus(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszStatus = JS_ToCString(jsctx, argv[0]);
    if (pszStatus != NULL)
    {
      b = true;
      JSValue jsglobal = JS_GetGlobalObject(jsctx);
      JS_SetPropertyStr(jsctx, jsglobal, "g_js_status", JS_NewStringLen(jsctx, pszStatus, strlen(pszStatus)));
      JS_FreeValue(jsctx, jsglobal);
    }
    else
    {
      e = "Please provide the status.";
    }
    if (pszStatus != NULL)
    {
      JS_FreeCString(jsctx, pszStatus);
    }
  }
  else
  {
    e = "Please provide the status.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_sqliteCreate()
JSValue Maestro::js_sqliteCreate(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszDatabase = JS_ToCString(jsctx, argv[0]), *pszNode = JS_ToCString(jsctx, argv[1]);
    if (pszDatabase != NULL && pszNode != NULL)
    {
      if (haveAccess(jsctx, "sqlite", {"create"}, e))
      {
        if (sqliteCreate(pszDatabase, pszNode, e))
        {
          b = true;
        }
      }
    }
    else
    {
      e = "Please provide the database and node.";
    }
    if (pszDatabase != NULL)
    {
      JS_FreeCString(jsctx, pszDatabase);
    }
    if (pszNode != NULL)
    {
      JS_FreeCString(jsctx, pszNode);
    }
  }
  else
  {
    e = "Please provide the database and node.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_sqliteDatabases()
JSValue Maestro::js_sqliteDatabases(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  map<string, map<string, string> > databases;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (sqliteDatabases(databases, e))
  {
    string j;
    Json *ptResponse = new Json;
    b = true;
    for (auto &node : databases)
    {
      ptResponse->i(node.first, node.second);
    }
    ptResponse->j(j);
    delete ptResponse;
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, j.c_str(), j.size()));
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_sqliteDrop()
JSValue Maestro::js_sqliteDrop(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszDatabase = JS_ToCString(jsctx, argv[0]), *pszNode = JS_ToCString(jsctx, argv[1]);
    if (pszDatabase != NULL && pszNode != NULL)
    {
      if (haveAccess(jsctx, "sqlite", {"drop"}, e))
      {
        if (sqliteDrop(pszDatabase, pszNode, e))
        {
          b = true;
        }
      }
    }
    else
    {
      e = "Please provide the database and node.";
    }
    if (pszDatabase != NULL)
    {
      JS_FreeCString(jsctx, pszDatabase);
    }
    if (pszNode != NULL)
    {
      JS_FreeCString(jsctx, pszNode);
    }
  }
  else
  {
    e = "Please provide the database and node.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_sqliteQuery()
JSValue Maestro::js_sqliteQuery(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszDatabase = JS_ToCString(jsctx, argv[0]), *pszStatement = JS_ToCString(jsctx, argv[1]);
    if (pszDatabase != NULL && pszStatement != NULL)
    {
      if (haveAccess(jsctx, "sqlite", {"query"}, e))
      {
        size_t unID = 0, unRows = 0;
        Json *ptResult = new Json;
        if (sqliteQuery(pszDatabase, pszStatement, ptResult, unID, unRows, e))
        {
          string j;
          stringstream ssID, ssRows;
          Json *ptResponse = new Json;
          b = true;
          ssID << unID;
          ptResponse->i("ID", ssID.str(), 'n');
          ssRows << unRows;
          ptResponse->i("Rows", ssRows.str(), 'n');
          ptResponse->m["Response"] = ptResult;
          ptResponse->j(j);
          delete ptResponse;
          JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, j.c_str(), j.size()));
        }
        else
        {
          delete ptResult;
        }
      }
    }
    else
    {
      e = "Please provide the database and statement.";
    }
    if (pszDatabase != NULL)
    {
      JS_FreeCString(jsctx, pszDatabase);
    }
    if (pszStatement != NULL)
    {
      JS_FreeCString(jsctx, pszStatement);
    }
  }
  else
  {
    e = "Please provide the database and statement.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_sshConnect()
JSValue Maestro::js_sshConnect(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 5)
  {
    const char *pszServer = JS_ToCString(jsctx, argv[0]), *pszPort = JS_ToCString(jsctx, argv[1]), *pszUser = JS_ToCString(jsctx, argv[2]), *pszPassword = JS_ToCString(jsctx, argv[3]), *pszPrivateKey = JS_ToCString(jsctx, argv[4]);
    if (pszServer != NULL && pszPort != NULL && pszUser != NULL && pszPassword != NULL && pszPrivateKey != NULL)
    {
      string strData, strSession;
      if (sshConnect(pszServer, pszPort, pszUser, pszPassword, pszPrivateKey, strSession, strData, e))
      {
        string j;
        Json *ptResponse = new Json;
        b = true;
        ptResponse->i("Data", strData);
        ptResponse->i("Session", strSession);
        ptResponse->j(j);
        delete ptResponse;
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, j.c_str(), j.size()));
      }
    }
    else
    {
      e = "Please provide the server, port, user, password, and privatekey.";
    }
    if (pszServer != NULL)
    {
      JS_FreeCString(jsctx, pszServer);
    }
    if (pszPort != NULL)
    {
      JS_FreeCString(jsctx, pszPort);
    }
    if (pszUser != NULL)
    {
      JS_FreeCString(jsctx, pszUser);
    }
    if (pszPassword != NULL)
    {
      JS_FreeCString(jsctx, pszPassword);
    }
    if (pszPrivateKey != NULL)
    {
      JS_FreeCString(jsctx, pszPrivateKey);
    }
  }
  else
  {
    e = "Please provide the server, port, user, password, and privatekey.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_sshDisconnect()
JSValue Maestro::js_sshDisconnect(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszSession = JS_ToCString(jsctx, argv[0]);
    if (pszSession != NULL)
    {
      if (sshDisconnect(pszSession, e))
      {
        b = true;
      }
    }
    else
    {
      e = "Please provide the session.";
    }
    if (pszSession != NULL)
    {
      JS_FreeCString(jsctx, pszSession);
    }
  }
  else
  {
    e = "Please provide the session.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_sshSend()
JSValue Maestro::js_sshSend(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc >= 2 && argc <= 3)
  {
    const char *pszSession = JS_ToCString(jsctx, argv[0]), *pszCommand = JS_ToCString(jsctx, argv[1]), *pszWait = ((argc >= 3)?JS_ToCString(jsctx, argv[2]):NULL);
    if (pszSession != NULL && pszCommand != NULL && (argc < 3 || pszWait != NULL))
    {
      string strData, strSession = pszSession;
      time_t CWait = 5;
      if (pszWait != NULL)
      {
        CWait = atoi(pszWait);
      }
      if (sshSend(strSession, pszCommand, strData, e, CWait))
      {
        string j;
        Json *ptResponse = new Json;
        b = true;
        ptResponse->i("Data", strData);
        ptResponse->i("Session", strSession);
        ptResponse->j(j);
        delete ptResponse;
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, j.c_str(), j.size()));
      }
    }
    else
    {
      e = "Please provide the session and command.";
    }
    if (pszSession != NULL)
    {
      JS_FreeCString(jsctx, pszSession);
    }
    if (pszCommand != NULL)
    {
      JS_FreeCString(jsctx, pszCommand);
    }
    if (pszWait != NULL)
    {
      JS_FreeCString(jsctx, pszWait);
    }
  }
  else
  {
    e = "Please provide the session and command.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_storageAdd()
JSValue Maestro::js_storageAdd(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszKeys = JS_ToCString(jsctx, argv[0]), *pszData = JS_ToCString(jsctx, argv[1]);
    if (pszKeys != NULL && pszData != NULL)
    {
      if (haveAccess(jsctx, "storage", {"add"}, e))
      {
        list<string> keys;
        Json *ptKeys = new Json(pszKeys);
        for (auto &key : ptKeys->l)
        {
          keys.push_back(key->v);
        }
        delete ptKeys;
        if (!keys.empty())
        {
          Json *ptData = new Json(pszData);
          if (storageAdd(keys, ptData, e))
          {
            b = true;
          }
          delete ptData;
        }
        else
        {
          e = "Please provide the keys.";
        }
      }
    }
    else
    {
      e = "Please provide the keys and data.";
    }
    if (pszKeys != NULL)
    {
      JS_FreeCString(jsctx, pszKeys);
    }
    if (pszData != NULL)
    {
      JS_FreeCString(jsctx, pszData);
    }
  }
  else
  {
    e = "Please provide the keys and data.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_storageRemove()
JSValue Maestro::js_storageRemove(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszKeys = JS_ToCString(jsctx, argv[0]);
    if (pszKeys != NULL)
    {
      if (haveAccess(jsctx, "storage", {"remove"}, e))
      {
        list<string> keys;
        Json *ptKeys = new Json(pszKeys);
        for (auto &key : ptKeys->l)
        {
          keys.push_back(key->v);
        }
        delete ptKeys;
        if (!keys.empty())
        {
          if (storageRemove(keys, e))
          {
            b = true;
          }
        }
        else
        {
          e = "Please provide the keys.";
        }
      }
    }
    else
    {
      e = "Please provide the keys.";
    }
    if (pszKeys != NULL)
    {
      JS_FreeCString(jsctx, pszKeys);
    }
  }
  else
  {
    e = "Please provide the keys.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_storageRetrieve()
JSValue Maestro::js_storageRetrieve(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszKeys = JS_ToCString(jsctx, argv[0]);
    if (pszKeys != NULL)
    {
      list<string> keys;
      Json *ptData = new Json, *ptKeys = new Json(pszKeys);
      for (auto &key : ptKeys->l)
      {
        keys.push_back(key->v);
      }
      delete ptKeys;
      if (storageRetrieve(keys, ptData, e))
      {
        string j;
        Json *ptResponse = new Json;
        b = true;
        ptResponse->m["Data"] = ptData;
        ptResponse->j(j);
        delete ptResponse;
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, j.c_str(), j.size()));
      }
      else
      {
        delete ptData;
      }
    }
    else
    {
      e = "Please provide the keys.";
    }
    if (pszKeys != NULL)
    {
      JS_FreeCString(jsctx, pszKeys);
    }
  }
  else
  {
    e = "Please provide the keys.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_storageRetrieveKeys()
JSValue Maestro::js_storageRetrieveKeys(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszKeys = JS_ToCString(jsctx, argv[0]);
    if (pszKeys != NULL)
    {
      list<string> keys, subkeys;
      Json *ptKeys = new Json(pszKeys);
      for (auto &key : ptKeys->l)
      {
        keys.push_back(key->v);
      }
      delete ptKeys;
      if (storageRetrieveKeys(keys, subkeys, e))
      {
        string j;
        Json *ptResponse = new Json;
        b = true;
        ptResponse->i("Data", subkeys);
        ptResponse->j(j);
        delete ptResponse;
        JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, j.c_str(), j.size()));
      }
    }
    else
    {
      e = "Please provide the keys.";
    }
    if (pszKeys != NULL)
    {
      JS_FreeCString(jsctx, pszKeys);
    }
  }
  else
  {
    e = "Please provide the keys.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_storageUpdate()
JSValue Maestro::js_storageUpdate(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 2)
  {
    const char *pszKeys = JS_ToCString(jsctx, argv[0]), *pszData = JS_ToCString(jsctx, argv[1]);
    if (pszKeys != NULL && pszData != NULL)
    {
      if (haveAccess(jsctx, "storage", {"update"}, e))
      {
        list<string> keys;
        Json *ptKeys = new Json(pszKeys);
        for (auto &key : ptKeys->l)
        {
          keys.push_back(key->v);
        }
        delete ptKeys;
        if (!keys.empty())
        {
          Json *ptData = new Json(pszData);
          if (storageUpdate(keys, ptData, e))
          {
            b = true;
          }
          delete ptData;
        }
        else
        {
          e = "Please provide the keys.";
        }
      }
    }
    else
    {
      e = "Please provide the keys and data.";
    }
    if (pszKeys != NULL)
    {
      JS_FreeCString(jsctx, pszKeys);
    }
    if (pszData != NULL)
    {
      JS_FreeCString(jsctx, pszData);
    }
  }
  else
  {
    e = "Please provide the keys and data.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_threadCreate()
JSValue Maestro::js_threadCreate(JSContext *jsctx, JSValueConst na, int argc, JSValueConst *argv)
{
  int32_t nID = -1;
  string e, sp;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (getPlan(jsctx, sp, e))
  {
  	if (argc >= 2 && argc <= 3)
    {
      const char *pszPlan = JS_ToCString(jsctx, argv[0]), *pszFlow = JS_ToCString(jsctx, argv[1]), *pszRequest = ((argc >= 3)?JS_ToCString(jsctx, argv[2]):NULL);
      if (pszPlan != NULL && pszFlow != NULL)
      {
        string strPlan = pszPlan, strFlow = pszFlow, strRequest = ((argc >= 3)?pszRequest:"");
        JSValue jsrun = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
        const char *pszRun = JS_ToCString(jsctx, jsrun);
        if (pszRun != NULL)
        {
          string strRun;
          if (!m_manip.decryptAes(m_manip.decodeBase64(pszRun, strRun), m_strSecret, strRun, e, m_strCipher).empty())
          {
            Json *r = new Json(strRun);
            if (dep({"f", "p", "t"}, r, e))
            {
              long t = atol(r->m["t"]->v.c_str());
              string p = r->m["p"]->v, f = r->m["f"]->v;
              radialMaestroThread *ptThread = new radialMaestroThread;
              ptThread->bDetached = false;
              ptThread->bDone = false;
              m_mutex.lock();
              if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end())
              {
                while (m_p[p]->r[f][t]->nThread != -1 && m_p[p]->r[f][t]->threads.find(m_p[p]->r[f][t]->nThread) != m_p[p]->r[f][t]->threads.end())
                {
                  m_p[p]->r[f][t]->nThread++;
                }
                nID = m_p[p]->r[f][t]->nThread;
                ptThread->pThread = new thread(&Maestro::flowThread, this, sp, strPlan, strFlow, strRequest, nID, pszRun);
                m_p[p]->r[f][t]->threads[m_p[p]->r[f][t]->nThread] = ptThread;
              }
              else
              {
                e = "Failed to find plan, flow, and thread.";
                delete ptThread;
              }
              m_mutex.unlock();
            }
            delete r;
          }
          JS_FreeCString(jsctx, pszRun);
        }
        else
        {
          e = "Failed to retrieve g_js_run.";
        }
      }
      else
      {
        e = "Please provide the plan and flow.";
      }
      if (pszRequest != NULL)
      {
        JS_FreeCString(jsctx, pszRequest);
      }
      if (pszFlow != NULL)
      {
        JS_FreeCString(jsctx, pszFlow);
      }
      if (pszPlan != NULL)
      {
        JS_FreeCString(jsctx, pszPlan);
      }
    }
    else
    {
      e = "Please provide the plan and flow.";
    }
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewInt32(jsctx, nID);
}
// }}}
// {{{ js_threadDetach()
JSValue Maestro::js_threadDetach(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);
  string e;

  if (argc == 1)
  {
    const char *pszID = JS_ToCString(jsctx, argv[0]);
    if (pszID != NULL)
    {
      int32_t nID;
      JSValue jsrun = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
      const char *pszRun = JS_ToCString(jsctx, jsrun);
      stringstream ssID(pszID);
      ssID >> nID;
      if (pszRun != NULL)
      {
        string e, strRun;
        if (!m_manip.decryptAes(m_manip.decodeBase64(pszRun, strRun), m_strSecret, strRun, e, m_strCipher).empty())
        {
          Json *r = new Json(strRun);
          if (dep({"f", "p", "t"}, r, e))
          {
            long t = atol(r->m["t"]->v.c_str());
            string p = r->m["p"]->v, f = r->m["f"]->v;
            radialMaestroThread *ptThread = NULL;
            m_mutex.lock();
            if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end() && m_p[p]->r[f][t]->threads.find(nID) != m_p[p]->r[f][t]->threads.end())
            {
              ptThread = m_p[p]->r[f][t]->threads[nID];
            }
            m_mutex.unlock();
            if (ptThread != NULL)
            {
              b = true;
              ptThread->bDetached = true;
            }
            else
            {
              e = "Please provide a valid id.";
            }
          }
          delete r;
        }
        JS_FreeCString(jsctx, pszRun);
      }
      else
      {
        e = "Failed to retrieve g_js_run.";
      }
    }
    else
    {
      e = "Please provide the id.";
    }
    if (pszID != NULL)
    {
      JS_FreeCString(jsctx, pszID);
    }
  }
  else
  {
    e = "Please provide the id.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// {{{ js_threadJoin()
JSValue Maestro::js_threadJoin(JSContext *jsctx, JSValueConst, int argc, JSValueConst *argv)
{
  bool b = false;
  string e;
  JSValue jsglobal = JS_GetGlobalObject(jsctx);

  if (argc == 1)
  {
    const char *pszID = JS_ToCString(jsctx, argv[0]);
    if (pszID != NULL)
    {
      int32_t nID;
      JSValue jsrun = JS_GetPropertyStr(jsctx, jsglobal, "g_js_run");
      const char *pszRun = JS_ToCString(jsctx, jsrun);
      stringstream ssID(pszID);
      ssID >> nID;
      if (pszRun != NULL)
      {
        string e, strRun;
        if (!m_manip.decryptAes(m_manip.decodeBase64(pszRun, strRun), m_strSecret, strRun, e, m_strCipher).empty())
        {
          Json *r = new Json(strRun);
          if (dep({"f", "p", "t"}, r, e))
          {
            long t = atol(r->m["t"]->v.c_str());
            string p = r->m["p"]->v, f = r->m["f"]->v;
            radialMaestroThread *ptThread = NULL;
            m_mutex.lock();
            if (m_p.find(p) != m_p.end() && m_p[p]->r.find(f) != m_p[p]->r.end() && m_p[p]->r[f].find(t) != m_p[p]->r[f].end() && m_p[p]->r[f][t]->threads.find(nID) != m_p[p]->r[f][t]->threads.end())
            {
              ptThread = m_p[p]->r[f][t]->threads[nID];
              m_p[p]->r[f][t]->threads.erase(nID);
            }
            m_mutex.unlock();
            if (ptThread != NULL)
            {
              b = true;
              ptThread->pThread->join();
              delete ptThread->pThread;
              if (!ptThread->strResponse.empty())
              {
                JS_SetPropertyStr(jsctx, jsglobal, "g_js_internal_response", JS_NewStringLen(jsctx, ptThread->strResponse.c_str(), ptThread->strResponse.size()));
              }
              delete ptThread;
            }
            else
            {
              e = "Please provide a valid id.";
            }
          }
          delete r;
        }
        JS_FreeCString(jsctx, pszRun);
      }
      else
      {
        e = "Failed to retrieve g_js_run.";
      }
    }
    else
    {
      e = "Please provide the id.";
    }
    if (pszID != NULL)
    {
      JS_FreeCString(jsctx, pszID);
    }
  }
  else
  {
    e = "Please provide the id.";
  }
  if (!e.empty())
  {
    JS_SetPropertyStr(jsctx, jsglobal, "g_js_error", JS_NewStringLen(jsctx, e.c_str(), e.size()));
  }
  JS_FreeValue(jsctx, jsglobal);

  return JS_NewBool(jsctx, ((b)?1:0));
}
// }}}
// }}}
// {{{ jsreg()
void Maestro::jsreg(map<string, JSCFunction *> javascript)
{
  for (auto &j : javascript)
  {
    if (m_javascript.find(j.first) != m_javascript.end())
    {
      m_javascript[j.first]->function = j.second;
    }
  }
}
void Maestro::jsreg(const string strFunction, const int nLength, const string strPrototype, const string strDescription)
{
  if (m_javascript.find(strFunction) == m_javascript.end())
  {
    radialMaestroJavascript *ptJavascript = new radialMaestroJavascript;
    ptJavascript->function = NULL;
    ptJavascript->nLength = nLength;
    ptJavascript->strDescription = strDescription;
    ptJavascript->strPrototype = strPrototype;
    m_javascript[strFunction] = ptJavascript;
  }
}
// }}}
// {{{ load()
void Maestro::load(string strPrefix)
{
  string e;
  Json *ptCred = new Json;

  strPrefix += "->Maestro::load()";
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"maestro", "Secret"}, m_strSecret, e) && m_pWarden->vaultRetrieve({"radial"}, ptCred, e))
  {
    m_mutex.lock();
    m_applications.clear();
    for (auto &i : ptCred->m)
    {
      if (!i.second->empty({"Application"}))
      {
        m_applications[i.first] = i.second->m["Application"]->v;
      }
    }
    m_mutex.unlock();
  }
  delete ptCred;
  if (!m_bLoaded)
  {
    bool bLoaded = false;
    map<string, radialMaestroPlan *> p;
    stringstream ssChat;
    Json *l = NULL;
    if (dataDirectoryList(m_strHandle, {}, &l, e))
    {
      bLoaded = true;
      for (auto &i : l->m)
      {
        if (i.second->val({"Type"}) == "directory" && (i.first.substr(0, 2) == "a_" || i.first.substr(0, 2) == "u_"))
        {
          map<string, string> row;
          string id, t;
          stringstream ssP(i.first);
          Json *sl = NULL, *ptData = new Json;
          radialMaestroPlan *ptPlan = new radialMaestroPlan;
          getline(ssP, t, '_');
          getline(ssP, id);
          ptPlan->a = NULL;
          ptPlan->id = id;
          ptPlan->t = t[0];
          ptData->i("id", id);
          if (db(((ptPlan->t == 'a')?"dbCentralApplications":"dbCentralUsers"), ptData, row, e))
          {
            stringstream ssOwner;
            if (t == "a")
            {
              ssOwner << row["name"];
            }
            else
            {
              ssOwner << row["first_name"] << " " << row["last_name"];
            }
            ptPlan->owner = ssOwner.str();
            if (dataDirectoryList(m_strHandle, {i.first}, &sl, e))
            {
              for (auto &j : sl->m)
              {
                if (j.second->val({"Type"}) == "regular file" && j.first == "a")
                {
                  string b;
                  if (dataRead(m_strHandle, {i.first, j.first}, b, e))
                  {
                    ptPlan->a = new Json(b);
                  }
                  else
                  {
                    bLoaded = false;
                    ssChat.str("");
                    ssChat << char(2) << char(3) << "07Interface::dataRead() [" << m_strHandle << "," << i.first << "," << j.first << "] " << e << char(3) << char(2);
                    chat("#maestro", ssChat.str());
                  }
                }
                else if (j.second->val({"Type"}) == "directory" && j.first == "f")
                {
                  Json *dl = NULL;
                  if (dataDirectoryList(m_strHandle, {i.first, j.first}, &dl, e))
                  {
                    for (auto &k : dl->m)
                    {
                      if (k.second->val({"Type"}) == "regular file")
                      {
                        string b;
                        if (dataRead(m_strHandle, {i.first, j.first, k.first}, b, e))
                        {
                          ptPlan->f[k.first] = new Json(b);
                        }
                        else
                        {
                          bLoaded = false;
                          ssChat.str("");
                          ssChat << char(2) << char(3) << "07Interface::dataRead() [" << m_strHandle << "," << i.first << "," << j.first << "," << k.first << "] " << e << char(3) << char(2);
                          chat("#maestro", ssChat.str());
                        }
                      }
                    }
                  }
                  else
                  {
                    bLoaded = false;
                    ssChat.str("");
                    ssChat << char(2) << char(3) << "07Interface::dataDirectoryList() [" << m_strHandle << "," << i.first << "," << j.first << "] " << e << char(3) << char(2);
                    chat("#maestro", ssChat.str());
                  }
                }
                else if (j.second->val({"Type"}) == "directory" && j.first == "s")
                {
                  Json *dl = NULL;
                  if (dataDirectoryList(m_strHandle, {i.first, j.first}, &dl, e))
                  {
                    for (auto &k : dl->m)
                    {
                      if (k.second->val({"Type"}) == "regular file")
                      {
                        string b;
                        if (dataRead(m_strHandle, {i.first, j.first, k.first}, b, e))
                        {
                          ptPlan->s[k.first] = new Json(b);
                        }
                        else
                        {
                          bLoaded = false;
                          ssChat.str("");
                          ssChat << char(2) << char(3) << "07Interface::dataRead() [" << m_strHandle << "," << i.first << "," << j.first << "," << k.first << "] " << e << char(3) << char(2);
                          chat("#maestro", ssChat.str());
                        }
                      }
                    }
                  }
                  else
                  {
                    bLoaded = false;
                    ssChat.str("");
                    ssChat << char(2) << char(3) << "07Interface::dataDirectoryList() [" << m_strHandle << "," << i.first << "," << j.first << "] " << e << char(3) << char(2);
                    chat("#maestro", ssChat.str());
                  }
                }
              }
              delete sl;
            }
            else
            {
              bLoaded = false;
              ssChat.str("");
              ssChat << char(2) << char(3) << "07Interface::dataDirectoryList() [" << m_strHandle << "," << i.first << "] " << e << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            bLoaded = false;
            ssChat.str("");
            ssChat << char(2) << char(3) << "07Interface::db(dbCentral" << ((ptPlan->t == 'a')?"application":"user") << "s) [" << m_strHandle << "," << i.first << "] " << e << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
          p[i.first] = ptPlan;
        }
      }
      delete l;
    }
    else
    {
      ssChat.str("");
      ssChat << char(2) << char(3) << "07Interface::dataDirectoryList() [" << m_strHandle << "] " << e << char(3) << char(2);
      chat("#maestro", ssChat.str());
    }
    if (bLoaded)
    {
      size_t unFlows = 0, unPlans = p.size();
      string v;
      m_mutex.lock();
      for (auto &i : p)
      {
        unFlows += i.second->f.size();
        m_p[i.first] = i.second;
      }
      m_mutex.unlock();
      m_bLoaded = true;
      ssChat.str("");
      ssChat << "Loaded " << m_manip.toShort(unPlans, v) << " plan" << ((unPlans != 1)?"s":"") << " containing " << m_manip.toShort(unFlows, v) << " flow" << ((unFlows != 1)?"s":"") << " from disk into memory.";
      chat("#maestro", ssChat.str());
    }
    else
    {
      bLoaded = false;
      ssChat.str("");
      ssChat << char(2) << char(3) << "04Failed to load plans." << char(3) << char(2);
      chat("#maestro", ssChat.str());
    }
  }
}
// }}}
// {{{ kill()
bool Maestro::kill(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (dep({"Flow", "Node", "Plan", "Thread"}, i, e))
  {
    long lThread = atol(i->m["Thread"]->v.c_str());
    string f = i->m["Flow"]->v, n = i->m["Node"]->v, p = i->m["Plan"]->v;
    if (isLocalAdmin(d, "Maestro") || isOwner(d, p))
    {
      if (n == m_strNode)
      {
        m_mutex.lock();
        if (m_p.find(p) != m_p.end())
        {
          if (m_p[p]->r.find(f) != m_p[p]->r.end())
          {
            if (m_p[p]->r[f].find(lThread) != m_p[p]->r[f].end())
            {
              Json *ptLive = new Json;
              b = true;
              (*m_p[p]->r[f][lThread]->pbKill) = true;
              ptLive->i("Action", "flowExec");
              ptLive->i("Plan", p);
              ptLive->i("Flow", f);
              live("Maestro", "", ptLive);
              delete ptLive;
            }
            else
            {
              e = "Failed to find the Thread.";
            }
          }
          else
          {
            e = "Failed to find the Flow.";
          }
        }
        else
        {
          e = "Failed to find the Plan.";
        }
        m_mutex.unlock();
      }
      else
      {
        Json *ptLink = new Json(d.r);
        ptLink->i("Interface", "maestro");
        ptLink->i("Node", n);
        ptLink->i("Function", "kill");
        if (hub("link", ptLink, e))
        {
          b = true;
        }
        delete ptLink;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// {{{ plan
// {{{ plan()
bool Maestro::plan(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Plan"}, i, e))
  {
    string p = i->m["Plan"]->v;
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      bool bEdit = isOwner(d, p);
      size_t unApprovals = 0, unRunning = 0;
      stringstream ssPlan, ssType;
      b = true;
      o->i("edit", ((bEdit)?"1":"0"), ((bEdit)?'1':'0'));
      o->i("ID", m_p[p]->id);
      o->i("NumAccesses", to_string((m_p[p]->a != NULL && m_p[p]->a->exist({"a"}))?m_p[p]->a->m["a"]->m.size():0), 'n');
      for (auto &c : m_p)
      {
        if (c.second->a != NULL && c.second->a->exist({"r"}) && c.second->a->m["r"]->m.find(p) != c.second->a->m["r"]->m.end())
        {
          unApprovals++;
        }
      }
      o->i("NumApprovals", to_string(unApprovals), 'n');
      o->i("NumRequests", to_string((m_p[p]->a != NULL && m_p[p]->a->exist({"r"}))?m_p[p]->a->m["r"]->m.size():0), 'n');
      o->i("NumFlows", to_string(m_p[p]->f.size()), 'n');
      o->i("NumSchedules", to_string(m_p[p]->s.size()), 'n');
      o->i("NumServices", to_string(m_p[p]->d.size()), 'n');
      for (auto &c : m_p[p]->r)
      {
        unRunning += c.second.size();
      }
      o->i("NumRunning", to_string(unRunning), 'n');
      o->i("Owner", m_p[p]->owner);
      ssPlan << m_p[p]->t << "_" << m_p[p]->id;
      o->i("Plan", ssPlan.str());
      ssType << m_p[p]->t;
      o->i("Type", ssType.str());
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
  }

  return b;
}
// }}}
// {{{ planAdd()
bool Maestro::planAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Plan"}, i, e))
  {
    string p = i->m["Plan"]->v;
    if (isOwner(d, p))
    {
      map<string, string> row;
      string id, t;
      stringstream ssP(p);
      Json *ptData = new Json;
      getline(ssP, t, '_');
      getline(ssP, id);
      ptData->i("id", id);
      if (db(((t == "a")?"dbCentralApplications":"dbCentralUsers"), ptData, row, e))
      {
        string strOwner;
        m_mutex.lock();
        if (m_p.find(p) == m_p.end())
        {
          if (i->val({"_broadcast"}) == "1" || dataDirectoryAdd(m_strHandle, {p}, e))
          {
            stringstream ssOwner;
            b = true;
            m_p[p] = new radialMaestroPlan;
            m_p[p]->a = NULL;
            m_p[p]->id = id;
            m_p[p]->t = t[0];
            if (m_p[p]->t == 'a')
            {
              ssOwner << row["name"];
            }
            else
            {
              ssOwner << row["first_name"] << " " << row["last_name"];
            }
            strOwner = ssOwner.str();
            m_p[p]->owner = strOwner;
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryAdd() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Plan already exists.";
        }
        m_mutex.unlock();
        if (b && i->val({"_broadcast"}) != "1")
        {
          list<string> nodes;
          Json *ptLive = new Json;
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
            ptLink->i("Function", "planAdd");
            ptLink->m["Request"]->i("_broadcast", "1", '1');
            hub("link", ptLink);
            delete ptLink;
            nodes.pop_front();
          }
          ssChat.str("");
          ssChat << char(3) << "00,06 " << strOwner << " " << char(3) << " " << char(2) << char(3) << "03Plan added by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
          chat("#maestro", ssChat.str());
          ptLive->i("Action", "planAdd");
          ptLive->i("Plan", p);
          live("Maestro", "", ptLive);
          delete ptLive;
        }
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// {{{ planRemove()
bool Maestro::planRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Plan"}, i, e))
  {
    string p = i->m["Plan"]->v;
    if (isOwner(d, p))
    {
      string strOwner, t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        b = true;
        strOwner = m_p[p]->owner;
        if (!m_p[p]->r.empty())
        {
          for (auto &c : m_p[p]->r)
          {
            for (auto &r : c.second)
            {
              (*r.second->pbKill) = true;
              delete r.second;
            }
          }
        }
        if (m_p[p]->a != NULL)
        {
          delete m_p[p]->a;
          if (i->val({"_broadcast"}) == "1")
          {
            dataRemove(m_strHandle, {p, "a"}, e);
          }
        }
        for (auto &s : m_p[p]->d)
        {
          delete s.second;
          if (i->val({"_broadcast"}) == "1")
          {
            if (!dataRemove(m_strHandle, {p, "d", s.first}, e))
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | d | " << s.first << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
        }
        if (i->val({"_broadcast"}) == "1")
        {
          if (!dataDirectoryRemove(m_strHandle, {p, "d"}, e))
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | d " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        for (auto &f : m_p[p]->f)
        {
          delete f.second;
          if (i->val({"_broadcast"}) == "1")
          {
            if (!dataRemove(m_strHandle, {p, "f", f.first}, e))
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | f | " << f.first << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
        }
        if (i->val({"_broadcast"}) == "1")
        {
          if (!dataDirectoryRemove(m_strHandle, {p, "f"}, e))
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | f " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        for (auto &s : m_p[p]->s)
        {
          delete s.second;
          if (i->val({"_broadcast"}) == "1")
          {
            if (!dataRemove(m_strHandle, {p, "s", s.first}, e))
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | s | " << s.first << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
        }
        if (i->val({"_broadcast"}) == "1")
        {
          if (!dataDirectoryRemove(m_strHandle, {p, "s"}, e))
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | s " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        delete m_p[p];
        m_p.erase(p);
        if (i->val({"_broadcast"}) == "1")
        {
          if (!dataDirectoryRemove(m_strHandle, {p}, e))
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
      }
      else
      {
        e = "Plan not found.";
      }
      m_mutex.unlock();
      if (b && i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        Json *ptLive = new Json;
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
          ptLink->i("Function", "planRemove");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << strOwner << " " << char(3) << " " << char(2) << char(3) << "04Plan removed by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "planRemove");
        ptLive->i("Plan", p);
        live("Maestro", "", ptLive);
        delete ptLive;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// {{{ plans()
bool Maestro::plans(radialUser &d, string &e)
{
  bool b = true;
  Json *o = d.p->m["o"];

  m_mutex.lock();
  for (auto &p : m_p)
  {
    bool bEdit = isOwner(d, p.first);
    size_t unApprovals = 0, unRunning = 0;
    stringstream ssType;
    Json *ptPlan = new Json;
    ptPlan->i("edit", ((bEdit)?"1":"0"), ((bEdit)?'1':'0'));
    ptPlan->i("ID", p.second->id);
    ptPlan->i("NumAccesses", to_string((p.second->a != NULL && p.second->a->exist({"a"}))?p.second->a->m["a"]->m.size():0), 'n');
    for (auto &c : m_p)
    {
      if (c.second->a != NULL && c.second->a->exist({"r"}) && c.second->a->m["r"]->m.find(p.first) != c.second->a->m["r"]->m.end())
      {
        unApprovals++;
      }
    }
    ptPlan->i("NumApprovals", to_string(unApprovals), 'n');
    ptPlan->i("NumRequests", to_string((p.second->a != NULL && p.second->a->exist({"r"}))?p.second->a->m["r"]->m.size():0), 'n');
    ptPlan->i("NumFlows", to_string(p.second->f.size()), 'n');
    ptPlan->i("NumSchedules", to_string(p.second->s.size()), 'n');
    ptPlan->i("NumServices", to_string(p.second->d.size()), 'n');
    for (auto &c : p.second->r)
    {
      unRunning += c.second.size();
    }
    ptPlan->i("NumRunning", to_string(unRunning), 'n');
    ptPlan->i("Owner", p.second->owner);
    ptPlan->i("Plan", p.first);
    ssType << p.second->t;
    ptPlan->i("Type", ssType.str());
    o->l.push_back(ptPlan);
  }
  m_mutex.unlock();

  return b;
}
// }}}
// }}}
// {{{ running()
bool Maestro::running(radialUser &d, string &e)
{
  bool b = true;
  string strPlan = d.p->m["i"]->val({"Plan"});
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  o->m[m_strNode] = new Json;
  m_mutex.lock();
  for (auto &p : m_p)
  {
    if ((strPlan.empty() || strPlan == p.first) && !p.second->r.empty())
    {
      o->m[m_strNode]->m[p.first] = new Json;
      o->m[m_strNode]->m[p.first]->i("Owner", p.second->owner);
      o->m[m_strNode]->m[p.first]->m["Flows"] = new Json;
      for (auto &f : p.second->r)
      {
        o->m[m_strNode]->m[p.first]->m["Flows"]->m[f.first] = new Json;
        for (auto &r : f.second)
        {
          o->m[m_strNode]->m[p.first]->m["Flows"]->m[f.first]->m[to_string(r.first)] = new Json;
          o->m[m_strNode]->m[p.first]->m["Flows"]->m[f.first]->m[to_string(r.first)]->i("Status", ((*r.second->pbKill)?"terminating":"running"));
          o->m[m_strNode]->m[p.first]->m["Flows"]->m[f.first]->m[to_string(r.first)]->i("Time", to_string(r.second->CTime), 'n');
        }
      }
    }
  }
  m_mutex.unlock();
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
      ptLink->i("Function", "running");
      if (!ptLink->exist({"Request"}))
      {
        ptLink->m["Request"] = new Json;
      }
      ptLink->m["Request"]->i("_broadcast", "1", '1');
      if (hub("link", ptLink, e) && ptLink->exist({"Response"}))
      {
        o->merge(ptLink->m["Response"], true, false);
      }
      delete ptLink;
      nodes.pop_front();
    }
  }

  return b;
}
// }}}
// {{{ schedule
// {{{ schedule()
bool Maestro::schedule(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Plan", "Schedule"}, i, e))
  {
    string p = i->m["Plan"]->v, s = i->m["Schedule"]->v;
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      if (m_p[p]->s.find(s) != m_p[p]->s.end())
      {
        b = true;
        if (!isOwner(d, p) && !m_p[p]->s[s]->empty({"Request"}))
        {
          m_p[p]->s[s]->i("Request", "");
        }
        o->merge(m_p[p]->s[s], true, false);
      }
      else
      {
        e = "Schedule not found.";
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
  }

  return b;
}
// }}}
// {{{ scheduleAdd()
bool Maestro::scheduleAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Plan", "Schedule"}, i, e))
  {
    string p = i->m["Plan"]->v, s = i->m["Schedule"]->v;
    if (isOwner(d, p))
    {
      string id, t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      getline(ssP, id);
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->s.find(s) == m_p[p]->s.end())
        {
          if (i->val({"_broadcast"}) == "1" || !m_p[p]->s.empty() || dataDirectoryAdd(m_strHandle, {p, "s"}, e))
          {
            string j;
            i->j(j);
            if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "s", s}, j, e))
            {
              b = true;
              m_p[p]->s[s] = new Json(i);
              if (isMasterSettled() && isMaster())
              {
                m_bLoadSchedules = true;
              }
            }
            else
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | s | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | s " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryAdd() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Schedule already exists.";
        }
      }
      else
      {
        e = "Plan not found.";
      }
      m_mutex.unlock();
      if (b && i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        Json *ptLive = new Json;
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
          ptLink->i("Function", "scheduleAdd");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << p << " | " << s << " " << char(3) << " " << char(2) << char(3) << "03Schedule added by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "scheduleAdd");
        ptLive->i("Plan", p);
        ptLive->i("Schedule", s);
        live("Maestro", "", ptLive);
        delete ptLive;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// {{{ scheduler()
void Maestro::scheduler(string strPrefix)
{
  map<string, map<string, time_t> > schedules;
  map<string, map<string, long> > services;
  string e;
  time_t CNow, CThread;

  threadIncrement();
  strPrefix += "->Maestro::scheduler()";
  time(&CThread);
  while (!shutdown())
  {
    if (!m_bLoaded)
    {
      load(strPrefix);
    }
    time(&CNow);
    if (isMasterSettled() && isMaster())
    {
      // {{{ schedules
      for (auto &p : schedules)
      {
        for (auto &s : p.second)
        {
          if (CNow >= s.second)
          {
            bool bLaunch = false;
            string strFlow, strPlan, strRequest;
            Json *ptFlow = NULL;
            m_bLoadSchedules = true;
            m_mutex.lock();
            if (m_p.find(p.first) != m_p.end() && m_p[p.first]->s.find(s.first) != m_p[p.first]->s.end() && m_p[p.first]->s[s.first]->val({"Enabled"}) == "1" && !m_p[p.first]->s[s.first]->empty({"Flow"}) && m_p[p.first]->f.find(m_p[p.first]->s[s.first]->m["Flow"]->v) != m_p[p.first]->f.end())
            {
              bLaunch = true;
              strPlan = p.first;
              strFlow = m_p[p.first]->s[s.first]->m["Flow"]->v;
              strRequest = m_p[p.first]->s[s.first]->val({"Request"});
              ptFlow = new Json(m_p[p.first]->f[m_p[p.first]->s[s.first]->m["Flow"]->v]);
            }
            m_mutex.unlock();
            if (bLaunch)
            {
              thread tThread(&Maestro::flowLaunch, this, strPlan, strFlow, strRequest, ptFlow);
              pthread_setname_np(tThread.native_handle(), "flowLaunch");
              tThread.detach();
            }
          }
        }
      }
      if (m_bLoadSchedules)
      {
        m_bLoadSchedules = false;
        schedules.clear();
        m_mutex.lock();
        for (auto &p : m_p)
        {
          map<string, time_t> scheds;
          for (auto &s : p.second->s)
          {
            if (s.second->val({"Enabled"}) == "1" && !s.second->empty({"Cron"}) && !s.second->empty({"Flow"}) && !s.second->empty({"Sched"}) && p.second->f.find(s.second->m["Flow"]->v) != p.second->f.end())
            {
              if (s.second->val({"Cron"}) == "0")
              {
                stringstream ssSched(s.second->m["Sched"]->v);
                time_t CSched;
                ssSched >> CSched;
                if (CSched > CNow)
                {
                  scheds[s.first] = CSched;
                }
              }
              else if (!cron(scheds[s.first], s.second->m["Sched"]->v, e))
              {
                scheds.erase(s.first);
              }
            }
          }
          if (!scheds.empty())
          {
            schedules[p.first] = scheds;
          }
        }
        m_mutex.unlock();
      }
      // }}}
      // {{{ services
      for (auto &p : services)
      {
        for (auto &s : p.second)
        {
          // check that each service have a running thread
          // -1 means the service has never been launched
          // need to check that launched services are still running
          // relaunch them if not
        }
      }
      if (m_bLoadServices)
      {
        m_bLoadServices = false;
        services.clear();
        m_mutex.lock();
        for (auto &p : m_p)
        {
          map<string, long> servs;
          for (auto &s : p.second->d)
          {
            if (s.second->val({"Enabled"}) == "1" && !s.second->empty({"Flow"}) && p.second->f.find(s.second->m["Flow"]->v) != p.second->f.end())
            {
              servs[s.first] = -1;
            }
          }
          if (!servs.empty())
          {
            services[p.first] = servs;
          }
        }
        m_mutex.unlock();
      }
      // }}}
    }
    // {{{ threads
    if ((CNow - CThread) > 60)
    {
      CThread = CNow;
      m_mutex.lock();
      for (auto &p : m_p)
      {
        for (auto &f : p.second->r)
        {
          for (auto &r : f.second)
          {
            list<int32_t> rs;
            for (auto &t : r.second->threads)
            {
              if (t.second->bDetached && t.second->bDone)
              {
                t.second->pThread->join();
                delete t.second->pThread;
                delete t.second;
                rs.push_back(t.first);
              }
            }
            while (!rs.empty())
            {
              r.second->threads.erase(rs.front());
              rs.pop_front();
            }
          }
        }
      }
      m_mutex.unlock();
    }
    // }}}
    msleep(1000);
  }
  threadDecrement();
}
// }}}
// {{{ scheduleRemove()
bool Maestro::scheduleRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Plan", "Schedule"}, i, e))
  {
    string p = i->m["Plan"]->v, s = i->m["Schedule"]->v;
    if (isOwner(d, p))
    {
      string t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->s.find(s) != m_p[p]->s.end())
        {
          if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "s", s}, e))
          {
            b = true;
            delete m_p[p]->s[s];
            m_p[p]->s.erase(s);
            if (isMasterSettled() && isMaster())
            {
              m_bLoadSchedules = true;
            }
            if (i->val({"_broadcast"}) != "1" && m_p[p]->s.empty() && !dataDirectoryRemove(m_strHandle, {p, "s"}, e))
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | s | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | s " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Schedule not found.";
        }
      }
      else
      {
        e = "Plan not found.";
      }
      m_mutex.unlock();
      if (b && i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        Json *ptLive = new Json;
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
          ptLink->i("Function", "scheduleRemove");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << p << " | " << s << " " << char(3) << " " << char(2) << char(3) << "04Schedule removed by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "scheduleRemove");
        ptLive->i("Plan", p);
        ptLive->i("Schedule", s);
        live("Maestro", "", ptLive);
        delete ptLive;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// {{{ schedules()
bool Maestro::schedules(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Plan"}, i, e))
  {
    string p = i->m["Plan"]->v;
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      b = true;
      for (auto &s : m_p[p]->s)
      {
        o->pb(s.first);
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
  }

  return b;
}
// }}}
// {{{ scheduleUpdate()
bool Maestro::scheduleUpdate(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Plan", "Schedule"}, i, e))
  {
    string p = i->m["Plan"]->v, s = i->m["Schedule"]->v;
    if (isOwner(d, p))
    {
      string id, t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      getline(ssP, id);
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->s.find(s) != m_p[p]->s.end())
        {
          string j;
          i->j(j);
          if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "s", s}, j, e))
          {
            b = true;
            if (m_p[p]->s.find(s) != m_p[p]->s.end())
            {
              delete m_p[p]->s[s];
            }
            m_p[p]->s[s] = new Json(i);
            if (isMasterSettled() && isMaster())
            {
              m_bLoadSchedules = true;
            }
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | s | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Schedule not found.";
        }
      }
      else
      {
        e = "Plan not found.";
      }
      m_mutex.unlock();
      if (b && i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        Json *ptLive = new Json;
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
          ptLink->i("Function", "scheduleUpdate");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << p << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Schedule updated by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "scheduleUpdate");
        ptLive->i("Plan", p);
        ptLive->i("Schedule", s);
        live("Maestro", "", ptLive);
        delete ptLive;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// }}}
// {{{ scopes()
bool Maestro::scopes(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Access", "Type"}, i, e))
  {
    string a = i->m["Access"]->v, t = i->m["Type"]->v;
    if (t == "application" || t == "user")
    {
      string p = ((t == "application")?"a_":"u_") + a;
      m_mutex.lock();
      if (m_p.find(p) != m_p.end() && ((t == "application" && m_p[p]->t == 'a') || (t == "user" && m_p[p]->t == 'u')))
      {
        b = true;
        for (auto &f : m_p[p]->f)
        {
          o->pb(f.first);
        }
      }
      else
      {
        e = "Access not found.";
      }
      m_mutex.unlock();
    }
    else if (t == "core")
    {
      if (m_a.find(a) != m_a.end())
      {
        b = true;
        for (auto &s : m_a[a])
        {
          o->pb(s);
        }
      }
      else
      {
        e = "Access not found.";
      }
    }
    else
    {
      e = "Please provide a valid Type:  application, core, user.";
    }
  }

  return b;
}
// }}}
// {{{ service
// {{{ service()
bool Maestro::service(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Plan", "Service"}, i, e))
  {
    string p = i->m["Plan"]->v, s = i->m["Service"]->v;
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      if (m_p[p]->d.find(s) != m_p[p]->d.end())
      {
        b = true;
        if (!isOwner(d, p) && !m_p[p]->d[s]->empty({"Request"}))
        {
          m_p[p]->d[s]->i("Request", "");
        }
        o->merge(m_p[p]->d[s], true, false);
      }
      else
      {
        e = "Service not found.";
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
  }

  return b;
}
// }}}
// {{{ serviceAdd()
bool Maestro::serviceAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Plan", "Service"}, i, e))
  {
    string p = i->m["Plan"]->v, s = i->m["Service"]->v;
    if (isOwner(d, p))
    {
      string id, t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      getline(ssP, id);
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->d.find(s) == m_p[p]->d.end())
        {
          if (i->val({"_broadcast"}) == "1" || !m_p[p]->d.empty() || dataDirectoryAdd(m_strHandle, {p, "d"}, e))
          {
            string j;
            i->j(j);
            if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "d", s}, j, e))
            {
              b = true;
              m_p[p]->d[s] = new Json(i);
              if (isMasterSettled() && isMaster())
              {
                m_bLoadServices = true;
              }
            }
            else
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | d | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | d " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryAdd() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Service already exists.";
        }
      }
      else
      {
        e = "Plan not found.";
      }
      m_mutex.unlock();
      if (b && i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        Json *ptLive = new Json;
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
          ptLink->i("Function", "serviceAdd");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << p << " | " << s << " " << char(3) << " " << char(2) << char(3) << "03Service added by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "serviceAdd");
        ptLive->i("Plan", p);
        ptLive->i("Service", s);
        live("Maestro", "", ptLive);
        delete ptLive;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// {{{ serviceRemove()
bool Maestro::serviceRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Plan", "Service"}, i, e))
  {
    string p = i->m["Plan"]->v, s = i->m["Service"]->v;
    if (isOwner(d, p))
    {
      string t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->d.find(s) != m_p[p]->d.end())
        {
          if (i->val({"_broadcast"}) == "1" || dataRemove(m_strHandle, {p, "d", s}, e))
          {
            b = true;
            delete m_p[p]->d[s];
            m_p[p]->d.erase(s);
            if (isMasterSettled() && isMaster())
            {
              m_bLoadServices = true;
            }
            if (i->val({"_broadcast"}) != "1" && m_p[p]->d.empty() && !dataDirectoryRemove(m_strHandle, {p, "d"}, e))
            {
              ssChat.str("");
              ssChat << char(3) << "00,06 " << p << " | d | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataDirectoryRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
              chat("#maestro", ssChat.str());
            }
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | d " << char(3) << " " << char(2) << char(3) << "07Interface::dataRemove() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Service not found.";
        }
      }
      else
      {
        e = "Plan not found.";
      }
      m_mutex.unlock();
      if (b && i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        Json *ptLive = new Json;
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
          ptLink->i("Function", "serviceRemove");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << p << " | " << s << " " << char(3) << " " << char(2) << char(3) << "04Service removed by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "serviceRemove");
        ptLive->i("Plan", p);
        ptLive->i("Service", s);
        live("Maestro", "", ptLive);
        delete ptLive;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// {{{ services()
bool Maestro::services(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (dep({"Plan"}, i, e))
  {
    string p = i->m["Plan"]->v;
    m_mutex.lock();
    if (m_p.find(p) != m_p.end())
    {
      b = true;
      for (auto &s : m_p[p]->d)
      {
        o->pb(s.first);
      }
    }
    else
    {
      e = "Plan not found.";
    }
    m_mutex.unlock();
  }

  return b;
}
// }}}
// {{{ serviceUpdate()
bool Maestro::serviceUpdate(radialUser &d, string &e)
{
  bool b = false;
  stringstream ssChat;
  Json *i = d.p->m["i"];

  if (dep({"Plan", "Service"}, i, e))
  {
    string p = i->m["Plan"]->v, s = i->m["Service"]->v;
    if (isOwner(d, p))
    {
      string id, t;
      stringstream ssP(p);
      getline(ssP, t, '_');
      getline(ssP, id);
      m_mutex.lock();
      if (m_p.find(p) != m_p.end())
      {
        if (m_p[p]->d.find(s) != m_p[p]->d.end())
        {
          string j;
          i->j(j);
          if (i->val({"_broadcast"}) == "1" || dataWrite(m_strHandle, {p, "d", s}, j, e))
          {
            b = true;
            if (m_p[p]->d.find(s) != m_p[p]->d.end())
            {
              delete m_p[p]->d[s];
            }
            m_p[p]->d[s] = new Json(i);
            if (isMasterSettled() && isMaster())
            {
              m_bLoadServices = true;
            }
          }
          else
          {
            ssChat.str("");
            ssChat << char(3) << "00,06 " << p << " | d | " << s << " " << char(3) << " " << char(2) << char(3) << "07Interface::dataWrite() " << e << " [" << d.f << " " << d.l << " (" << d.u << ")]" << char(3) << char(2);
            chat("#maestro", ssChat.str());
          }
        }
        else
        {
          e = "Service not found.";
        }
      }
      else
      {
        e = "Plan not found.";
      }
      m_mutex.unlock();
      if (b && i->val({"_broadcast"}) != "1")
      {
        list<string> nodes;
        Json *ptLive = new Json;
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
          ptLink->i("Function", "serviceUpdate");
          ptLink->m["Request"]->i("_broadcast", "1", '1');
          hub("link", ptLink);
          delete ptLink;
          nodes.pop_front();
        }
        ssChat.str("");
        ssChat << char(3) << "00,06 " << p << " | " << s << " " << char(3) << " " << char(2) << char(3) << "07Service updated by " << d.f << " " << d.l << " (" << d.u << ")." << char(3) << char(2);
        chat("#maestro", ssChat.str());
        ptLive->i("Action", "serviceUpdate");
        ptLive->i("Plan", p);
        ptLive->i("Schedule", s);
        live("Maestro", "", ptLive);
        delete ptLive;
      }
    }
    else
    {
      e = "You are not the owner.";
    }
  }

  return b;
}
// }}}
// }}}
}
}
