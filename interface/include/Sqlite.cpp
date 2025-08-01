// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Sqlite.cpp
// author     : Ben Kietzman
// begin      : 2024-10-21
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Sqlite"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Sqlite()
Sqlite::Sqlite(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), int (*pCallbackFetch)(void *, int, char **, char **)) : Interface(strPrefix, "sqlite", argc, argv, pCallback)
{
  m_bMasterUpdated = false;
  m_pCallbackFetch = pCallbackFetch;
  m_pThreadInotify = new thread(&Sqlite::inotify, this, strPrefix);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
}
// }}}
// {{{ ~Sqlite()
Sqlite::~Sqlite()
{
  m_pThreadInotify->join();
  delete m_pThreadInotify;
}
// }}}
// {{{ autoMode()
void Sqlite::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->Sqlite::autoMode()";
  if (strOldMaster != strNewMaster)
  {
    stringstream ssMessage;
    ssMessage << strPrefix << " [" << strNewMaster << "]:  Updated master.";
    log(ssMessage.str());
    m_bMasterUpdated = true;
  }
  threadDecrement();
}
// }}}
// {{{ callback()
void Sqlite::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError, strValue;
  stringstream ssMessage;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Sqlite::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    string strFunction = ptJson->m["Function"]->v;
    // {{{ action
    if (strFunction == "action")
    {
      radialUser d;
      userInit(ptJson, d);
      if (Sqlite::action(d, strError))
      {
        bResult = true;
        if (exist(ptJson, "Response"))
        {
          delete ptJson->m["Response"];
        }
        ptJson->m["Response"] = d.p->m["o"];
        d.p->m.erase("o");
      }
      userDeinit(d);
    }
    // }}}
    // {{{ databases
    else if (strFunction == "databases")
    {
      bResult = true;
      if (exist(ptJson, "Response"))
      {
        delete ptJson->m["Response"];
        ptJson->m.erase("Response");
      }
      ptJson->m["Response"] = new Json;
      m_mutex.lock();
      for (auto &i : m_databases)
      {
        ptJson->m["Response"]->m[i.first] = new Json;
        for (auto &j : i.second)
        {
          ptJson->m["Response"]->m[i.first]->i(j.first, ((j.second)?"master":"slave"));
        }
      }
      m_mutex.unlock();
    }
    // }}}
    // {{{ status
    else if (strFunction == "status")
    {
      radialUser d;
      userInit(ptJson, d);
      if (Sqlite::status(d, strError))
      {
        bResult = true;
        if (exist(ptJson, "Response"))
        {
          delete ptJson->m["Response"];
        }
        ptJson->m["Response"] = d.p->m["o"];
        d.p->m.erase("o");
      }
      userDeinit(d);
    }
    // }}}
    // {{{ request
    else if (exist(ptJson, "Request"))
    {
      if (!empty(ptJson->m["Request"], "Database"))
      {
        string strDatabase = ptJson->m["Request"]->m["Database"]->v, strNode;
        if (!empty(ptJson->m["Request"], "Node"))
        {
          strNode = ptJson->m["Request"]->m["Node"]->v;
        }
        // {{{ create
        if (strFunction == "create")
        {
          if (strNode.empty() || strNode == m_strNode)
          {
            bool bExists = false;
            m_mutex.lock();
            if (m_databases.find(strDatabase) != m_databases.end() && (strNode.empty() || m_databases[strDatabase].find(strNode) != m_databases[strDatabase].end()))
            {
              bExists = true;
            }
            m_mutex.unlock();
            if (!bExists)
            {
              int nReturn;
              sqlite3 *db;
              string strFile;
              stringstream ssFile;
              ssFile << "file:" << m_strData << "/sqlite/" << strDatabase << ".db";
              strFile = ssFile.str();
              if ((nReturn = sqlite3_open_v2(strFile.c_str(), &db, (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE), NULL)) == SQLITE_OK)
              {
                bResult = true;
              }
              else
              {
                ssMessage.str("");
                ssMessage << "sqlit3_open(" << nReturn << ") " << sqlite3_errmsg(db);
              }
              sqlite3_close(db);
            }
            else
            {
              strError = "Database already exists.";
            }
          }
          else
          {
            Json *ptLink = new Json(ptJson);
            ptLink->i("Node", strNode);
            ptLink->m["Request"]->i("Node", strNode);
            if (hub("link", ptLink, strError))
            {
              bResult = true;
            }
            delete ptLink;
          }
        }
        // }}}
        // {{{ drop
        else if (strFunction == "drop")
        {
          if (strNode.empty() || strNode == m_strNode)
          {
            bool bExists = false;
            m_mutex.lock();
            if (m_databases.find(strDatabase) != m_databases.end() && (strNode.empty() || m_databases[strDatabase].find(strNode) != m_databases[strDatabase].end()))
            {
              bExists = true;
            }
            m_mutex.unlock();
            if (bExists)
            {
              list<string> nodes;
              if (!strNode.empty())
              {
                nodes.push_back(strNode);
              }
              else
              {
                m_mutex.lock();
                if (m_databases.find(strDatabase) != m_databases.end())
                {
                  for (auto &i : m_databases[strDatabase])
                  {
                    nodes.push_back(i.first);
                  }
                }
                m_mutex.unlock();
              }
              while (!nodes.empty())
              {
                if (nodes.front() == m_strNode)
                {
                  string strFile;
                  stringstream ssFile;
                  ssFile << m_strData << "/sqlite/" << strDatabase << ".db";
                  strFile = ssFile.str();
                  if (remove(strFile.c_str()) == 0)
                  {
                    bResult = true;
                  }
                  else
                  {
                    ssMessage.str("");
                    ssMessage << "remove(" << errno << ") " << strerror(errno);
                    strError = ssMessage.str();
                  }
                }
                else
                {
                  Json *ptLink = new Json(ptJson);
                  ptLink->i("Node", nodes.front());
                  ptLink->m["Request"]->i("Node", nodes.front());
                  hub("link", ptLink, strError);
                  delete ptLink;
                }
                nodes.pop_front();
              }
            }
            else
            {
              strError = "Database does not exist.";
            }
          }
          else
          {
            Json *ptLink = new Json(ptJson);
            ptLink->i("Node", strNode);
            if (hub("link", ptLink, strError))
            {
              bResult = true;
            }
            delete ptLink;
          }
        }
        // }}}
        // {{{ query
        else if (strFunction == "query")
        {
          if (!empty(ptJson->m["Request"], "Statement"))
          {
            string strAction, strStatement = ptJson->m["Request"]->m["Statement"]->v, strValue;
            stringstream ssStatement(strStatement);
            ssStatement >> strValue;
            m_manip.toLower(strAction, strValue);
            if (strAction == "select" || isMasterSettled())
            {
              bool bLocal = false, bReadOnly = false;
              m_mutex.lock();
              if ((!empty(ptJson->m["Request"], "_local") && ptJson->m["Request"]->m["_local"]->v == "1") || (m_databases.find(strDatabase) != m_databases.end() && m_databases[strDatabase].find(m_strNode) != m_databases[strDatabase].end() && (strAction == "select" || m_databases[strDatabase][m_strNode])))
              {
                bLocal = true;
              }
              if (!empty(ptJson->m["Request"], "Access") && ptJson->m["Request"]->m["Access"]->v == "r")
              {
                bReadOnly = true;
              }
              m_mutex.unlock();
              if (bLocal)
              {
                int nReturn;
                sqlite3 *db;
                string strFile;
                stringstream ssFile;
                ssFile << "file:" << m_strData << "/sqlite/" << strDatabase << ".db";
                strFile = ssFile.str();
                if ((nReturn = sqlite3_open_v2(strFile.c_str(), &db, ((bReadOnly)?SQLITE_OPEN_READONLY:(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)), NULL)) == SQLITE_OK)
                {
                  char *pszError = NULL;
                  Json *ptRows = new Json;
                  if ((nReturn = sqlite3_exec(db, "PRAGMA foreign_keys = ON", NULL, NULL, &pszError)) == SQLITE_OK)
                  {
                    if ((nReturn = sqlite3_exec(db, strStatement.c_str(), ((strAction == "select")?m_pCallbackFetch:NULL), ((strAction == "select")?ptRows:NULL), &pszError)) == SQLITE_OK)
                    {
                      size_t unSize = 16;
                      string strJson;
                      stringstream ssRows;
                      if (exist(ptJson, "Response"))
                      {
                        delete ptJson->m["Response"];
                        ptJson->m.erase("Response");
                      }
                      ptJson->m["Response"] = new Json;
                      if (strAction == "select")
                      {
                        ssRows << ptRows->l.size();
                      }
                      else
                      {
                        ssRows << sqlite3_changes(db);
                      }
                      ptJson->m["Response"]->i("Rows", ssRows.str(), 'n');
                      if (strAction != "select")
                      {
                        list<string> nodes;
                        if (strAction == "insert")
                        {
                          char *pszSubError = NULL;
                          Json *ptSubRows = new Json;
                          if ((nReturn = sqlite3_exec(db, "select last_insert_rowid()", m_pCallbackFetch, ptSubRows, &pszSubError)) == SQLITE_OK)
                          {
                            if (!ptSubRows->l.empty() && !empty(ptSubRows->l.front(), "last_insert_rowid()"))
                            {
                              ptJson->m["Response"]->i("ID", ptSubRows->l.front()->m["last_insert_rowid()"]->v, 'n');
                            }
                          }
                          else
                          {
                            sqlite3_free(pszSubError);
                          }
                          delete ptSubRows;
                        }
                        m_mutex.lock();
                        if (m_databases.find(strDatabase) != m_databases.end() && m_databases[strDatabase].find(m_strNode) != m_databases[strDatabase].end() && m_databases[strDatabase][m_strNode])
                        {
                          for (auto &i : m_databases[strDatabase])
                          {
                            if (i.first != m_strNode)
                            {
                              nodes.push_back(i.first);
                            }
                          }
                        }
                        m_mutex.unlock();
                        while (!nodes.empty())
                        {
                          Json *ptLink = new Json(ptJson);
                          ptLink->i("Node", nodes.front());
                          ptLink->m["Request"]->i("_local", "1", '1');
                          hub("link", ptLink, strError);
                          delete ptLink;
                          nodes.pop_front();
                        }
                      }
                      unSize += ptJson->j(strJson).size() + 14;
                      for (auto i = ptRows->l.begin(); unSize < m_unMaxPayload && i != ptRows->l.end(); i++)
                      {
                        for (auto &j : (*i)->m)
                        {
                          unSize += j.first.size() + j.second->v.size() + 6;
                        }
                      }
                      if (unSize < m_unMaxPayload)
                      {
                        bResult = true;
                        ptJson->m["Response"]->m["ResultSet"] = ptRows;
                      }
                      else
                      {
                        delete ptRows;
                        ssMessage.str("");
                        ssMessage << "Payload of " << m_manip.toShortByte(unSize, strValue) << " exceeded " << m_manip.toShortByte(m_unMaxPayload, strValue) << " maximum.  ResultSet has been removed.";
                        strError = ssMessage.str();
                      }
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << "sqlite3_exec(" << nReturn << ") " << pszError;
                      sqlite3_free(pszError);
                      strError = ssMessage.str();
                    }
                  }
                  else
                  {
                    delete ptRows;
                    ssMessage.str("");
                    ssMessage << "sqlite3_exec(" << nReturn << ") " << pszError;
                    sqlite3_free(pszError);
                    strError = ssMessage.str();
                  }
                }
                else
                {
                  ssMessage.str("");
                  ssMessage << "sqlit3_open(" << nReturn << ") " << sqlite3_errmsg(db);
                }
                sqlite3_close(db);
              }
              else if (strAction == "select")
              {
                vector<string> nodes;
                m_mutex.lock();
                if (m_databases.find(strDatabase) != m_databases.end())
                {
                  for (auto &i : m_databases[strDatabase])
                  {
                    nodes.push_back(i.first);
                  }
                }
                m_mutex.unlock();
                if (!nodes.empty())
                {
                  unsigned int unPick = 0, unSeed = time(NULL);
                  Json *ptLink = new Json(ptJson);
                  srand(unSeed);
                  unPick = rand_r(&unSeed) % nodes.size();
                  ptLink->i("Node", nodes[unPick]);
                  ptLink->m["Request"]->i("_local", "1", '1');
                  if (hub("link", ptLink, strError))
                  {
                    bResult = true;
                    if (!empty(ptLink, "ID"))
                    {
                      ptJson->i("ID", ptLink->m["ID"]->v);
                    }
                    if (exist(ptLink, "Response"))
                    {
                      ptJson->i("Response", ptLink->m["Response"]);
                    }
                    if (!empty(ptLink, "Rows"))
                    {
                      ptJson->i("Rows", ptLink->m["Rows"]->v);
                    }
                  }
                  delete ptLink;
                }
                else
                {
                  strError = "Please provide a valid Database.";
                }
              }
              else
              {
                bool bExists = false;
                string strSubNode;
                m_mutex.lock();
                if (m_databases.find(strDatabase) != m_databases.end())
                {
                  for (auto i = m_databases[strDatabase].begin(); strSubNode.empty() && i != m_databases[strDatabase].end(); i++)
                  {
                    bExists = true;
                    if (i->second)
                    {
                      strSubNode = i->first;
                    }
                  }
                }
                m_mutex.unlock();
                if (!strSubNode.empty())
                {
                  Json *ptLink = new Json(ptJson);
                  ptLink->i("Node", strSubNode);
                  ptLink->m["Request"]->i("_local", "1", '1');
                  if (hub("link", ptLink, strError))
                  {
                    bResult = true;
                    if (!empty(ptLink, "ID"))
                    {
                      ptJson->i("ID", ptLink->m["ID"]->v);
                    }
                    if (exist(ptLink, "Response"))
                    {
                      ptJson->i("Response", ptLink->m["Response"]);
                    }
                    if (!empty(ptLink, "Rows"))
                    {
                      ptJson->i("Rows", ptLink->m["Rows"]->v);
                    }
                  }
                  delete ptLink;
                }
                else if (bExists)
                {
                  strError = "Missing master for this Database.  Try again later.";
                }
                else
                {
                  strError = "Please provide a valid Database.";
                }
              }
            }
            else
            {
              strError = "Master is not settled.";
            }
          }
          else
          {
            strError = "Please provide the Statement within the Request.";
          }
        }
        // }}}
        // {{{ node
        else if (!strNode.empty())
        {
          // {{{ add
          if (strFunction == "add")
          {
            bool bMaster = false;
            bResult = true;
            databaseAdd(strPrefix, strDatabase, strNode, bMaster);
            if (bMaster && isMaster())
            {
              databaseMaster(strPrefix, strDatabase, strNode);
            }
          }
          // }}}
          // {{{ master
          else if (strFunction == "master")
          {
            bResult = true;
            databaseMaster(strPrefix, strDatabase, strNode);
          }
          // }}}
          // {{{ remove
          else if (strFunction == "remove")
          {
            string strMaster;
            bResult = true;
            databaseRemove(strPrefix, strDatabase, strNode, strMaster);
            if (!strMaster.empty() && isMaster())
            {
              databaseMaster(strPrefix, strDatabase, strMaster);
            }
          }
          // }}}
          // {{{ invalid
          else
          {
            strError = "Please provide a valid Function:  action, add, create, drop, list, master, query. remove, status.";
          }
          // }}}
        }
        // }}}
        // {{{ invalid
        else
        {
          strError = "Please provide the Node within the Request.";
        }
        // }}}
      }
      else
      {
        strError = "Please provide the Database within the Request.";
      }
    }
    // }}}
    // {{{ invalid
    else
    {
      strError = "Please provide the Request.";
    }
    // }}}
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
// {{{ callbackFetch()
int Sqlite::callbackFetch(void *vptRows, int nCols, char *szCols[], char *szNames[])
{
  int nResult = SQLITE_OK;
  Json *ptRow = new Json, *ptRows = (Json *)vptRows;

  for (int i = 0; i < nCols; i++)
  {
    char cType = 's';
    string strValue;
    if (szCols[i] != NULL)
    {
      strValue = szCols[i];
    }
    else
    {
      cType = '\0';
    }
    ptRow->i(szNames[i], strValue, cType);
  }
  ptRows->l.push_back(ptRow);

  return nResult;
}
// }}}
// {{{ databaseAdd()
void Sqlite::databaseAdd(string strPrefix, const string strDatabase, const string strNode, bool &bMaster)
{
  bool bUpdated = false;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Sqlite::databaseAdd()";
  bMaster = false;
  m_mutex.lock();
  if (m_databases.find(strDatabase) == m_databases.end())
  {
    bMaster = true;
    m_databases[strDatabase] = {};
  }
  if (m_databases[strDatabase].find(strNode) == m_databases[strDatabase].end())
  {
    bUpdated = true;
    m_databases[strDatabase][strNode] = false;
    ssMessage.str("");
    ssMessage << strPrefix << " [" << strDatabase << "," << strNode << "]:  Added database.";
    log(ssMessage.str());
    ssMessage.str("");
    ssMessage << char(3) << "13,06 " << strDatabase << " | " << strNode << " " << char(3) << " Added database.";
    chat("#sqlite", ssMessage.str());
  }
  m_mutex.unlock();
  if (bUpdated && isMaster())
  {
    list<string> nodes;
    m_mutexShare.lock();
    for (auto &link : m_l)
    {
      if (link->interfaces.find("sqlite") != link->interfaces.end())
      {
        nodes.push_back(link->strNode);
      }
    }
    m_mutexShare.unlock();
    while (!nodes.empty())
    {
      Json *ptLink = new Json;
      ptLink->i("Interface", "sqlite");
      ptLink->i("Node", nodes.front());
      ptLink->i("Function", "add");
      ptLink->m["Request"] = new Json;
      ptLink->m["Request"]->i("Database", strDatabase);
      ptLink->m["Request"]->i("Node", strNode);
      hub("link", ptLink, strError);
      delete ptLink;
      nodes.pop_front();
    }
  }
}
// }}}
// {{{ databaseMaster()
void Sqlite::databaseMaster(string strPrefix, const string strDatabase, const string strNode)
{
  bool bUpdated = false;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Sqlite::databaseMaster()";
  m_mutex.lock();
  if (m_databases.find(strDatabase) != m_databases.end())
  {
    for (auto &i : m_databases[strDatabase])
    {
      if (i.first == strNode)
      {
        if (!i.second)
        {
          bUpdated = i.second = true;
          ssMessage.str("");
          ssMessage << strPrefix << " [" << strDatabase << "," << strNode << "]:  Set master database.";
          log(ssMessage.str());
          ssMessage.str("");
          ssMessage << char(3) << "13,06 " << strDatabase << " | " << strNode << " " << char(3) << " Set master database.";
          chat("#sqlite", ssMessage.str());
        }
      }
      else if (i.second)
      {
        i.second = false;
      }
    }
  }
  m_mutex.unlock();
  if (bUpdated && isMaster())
  {
    list<string> nodes;
    m_mutexShare.lock();
    for (auto &link : m_l)
    {
      if (link->interfaces.find("sqlite") != link->interfaces.end())
      {
        nodes.push_back(link->strNode);
      }
    }
    m_mutexShare.unlock();
    while (!nodes.empty())
    {
      Json *ptLink = new Json;
      ptLink->i("Interface", "sqlite");
      ptLink->i("Node", nodes.front());
      ptLink->i("Function", "master");
      ptLink->m["Request"] = new Json;
      ptLink->m["Request"]->i("Database", strDatabase);
      ptLink->m["Request"]->i("Node", strNode);
      hub("link", ptLink, strError);
      delete ptLink;
      nodes.pop_front();
    }
  }
}
// }}}
// {{{ databaseRemove()
void Sqlite::databaseRemove(string strPrefix, const string strDatabase, const string strNode, string &strMaster)
{
  bool bUpdated = false;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Sqlite::databaseRemove()";
  strMaster.clear();
  m_mutex.lock();
  if (m_databases.find(strDatabase) != m_databases.end() && m_databases[strDatabase].find(strNode) != m_databases[strDatabase].end())
  {
    bool bMaster = m_databases[strDatabase][strNode];
    bUpdated = true;
    m_databases[strDatabase].erase(strNode);
    if (m_databases[strDatabase].empty())
    {
      m_databases.erase(strDatabase);
    }
    else if (bMaster && isMaster())
    {
      unsigned int unPick = 0, unSeed = time(NULL);
      vector<string> subNodes;
      for (auto &i : m_databases[strDatabase])
      {
        subNodes.push_back(i.first);
      }
      srand(unSeed);
      unPick = rand_r(&unSeed) % subNodes.size();
      strMaster = subNodes[unPick];
    }
    ssMessage.str("");
    ssMessage << strPrefix << " [" << strDatabase << "," << strNode << "]:  Removed database.";
    log(ssMessage.str());
    ssMessage.str("");
    ssMessage << char(3) << "13,06 " << strDatabase << " | " << strNode << " " << char(3) << " Removed database.";
    chat("#sqlite", ssMessage.str());
  }
  m_mutex.unlock();
  if (bUpdated && isMaster())
  {
    list<string> nodes;
    m_mutexShare.lock();
    for (auto &link : m_l)
    {
      if (link->interfaces.find("sqlite") != link->interfaces.end())
      {
        nodes.push_back(link->strNode);
      }
    }
    m_mutexShare.unlock();
    while (!nodes.empty())
    {
      Json *ptLink = new Json;
      ptLink->i("Interface", "sqlite");
      ptLink->i("Node", nodes.front());
      ptLink->i("Function", "remove");
      ptLink->m["Request"] = new Json;
      ptLink->m["Request"]->i("Database", strDatabase);
      ptLink->m["Request"]->i("Node", strNode);
      hub("link", ptLink, strError);
      delete ptLink;
      nodes.pop_front();
    }
  }
}
// }}}
// {{{ inotify()
void Sqlite::inotify(string strPrefix)
{
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Sqlite::inotify()";
  while (!shutdown())
  {
    if (isMasterSettled())
    {
      int fdNotify;
      if ((fdNotify = inotify_init1(IN_NONBLOCK)) != -1)
      {
        int wdNotify;
        if ((wdNotify = inotify_add_watch(fdNotify, (m_strData + "/sqlite").c_str(), (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO))) != -1)
        {
          bool bExit = false;
          inotify_event *pEvent;
          int nReturn;
          string strNotify;
          time_t CSync, CTime;
          Json *ptLink;
          if (isMaster())
          {
            chat("#sqlite", "Switched to master mode.");
          }
          sync(strPrefix);
          load(strPrefix);
          time(&CTime);
          CSync = CTime;
          while (!bExit)
          {
            pollfd fds[1];
            fds[0].fd = fdNotify;
            fds[0].events = POLLIN;
            if ((nReturn = poll(fds, 1, 2000)) > 0)
            {
              if (fds[0].revents & POLLIN)
              {
                if (m_pUtility->fdRead(fds[0].fd, strNotify, nReturn))
                {
                  while (strNotify.size() >= sizeof(inotify_event))
                  {
                    pEvent = (inotify_event *)strNotify.c_str();
                    if (pEvent->wd == wdNotify && pEvent->len > 0 && (pEvent->mask & (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO)))
                    {
                      string strName = pEvent->name;
                      if (strName.size() > 3 && strName.substr((strName.size() - 3), 3) == ".db")
                      {
                        string strDatabase = strName.substr(0, (strName.size() - 3));
                        if (pEvent->mask & (IN_CREATE | IN_MOVED_TO))
                        {
                          if (isMaster())
                          {
                            bool bMaster = false;
                            databaseAdd(strPrefix, strDatabase, m_strNode, bMaster);
                            if (bMaster)
                            {
                              databaseMaster(strPrefix, strDatabase, m_strNode);
                            }
                          }
                          else
                          {
                            ptLink = new Json;
                            ptLink->i("Interface", "sqlite");
                            ptLink->i("Node", master());
                            ptLink->i("Function", "add");
                            ptLink->m["Request"] = new Json;
                            ptLink->m["Request"]->i("Database", strDatabase);
                            ptLink->m["Request"]->i("Node", m_strNode);
                            hub("link", ptLink, strError);
                            delete ptLink;
                          }
                        }
                        else if (pEvent->mask & (IN_DELETE | IN_MOVED_FROM))
                        {
                          if (isMaster())
                          {
                            string strMaster;
                            databaseRemove(strPrefix, strDatabase, m_strNode, strMaster);
                            if (!strMaster.empty())
                            {
                              databaseMaster(strPrefix, strDatabase, strMaster);
                            }
                          }
                          else
                          {
                            ptLink = new Json;
                            ptLink->i("Interface", "sqlite");
                            ptLink->i("Node", master());
                            ptLink->i("Function", "remove");
                            ptLink->m["Request"] = new Json;
                            ptLink->m["Request"]->i("Database", strDatabase);
                            ptLink->m["Request"]->i("Node", m_strNode);
                            hub("link", ptLink, strError);
                            delete ptLink;
                          }
                        }
                      }
                    }
                    strNotify.erase(0, sizeof(inotify_event));
                  }
                }
                else
                {
                  bExit = true;
                  if (nReturn < 0)
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << "->Utility::fdRead(" << errno << ") error [" << m_strData << "/sqlite]:  " << strerror(errno);
                    log(ssMessage.str());
                  }
                }
              }
            }
            else if (nReturn < 0 && errno != EINTR)
            {
              bExit = true;
              ssMessage.str("");
              ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
              log(ssMessage.str());
            }
            time(&CTime);
            if ((CTime - CSync) > 60)
            {
              CSync = CTime;
              sync(strPrefix);
            }
            if (m_bMasterUpdated || shutdown())
            {
              bExit = true;
              m_bMasterUpdated = false;
            }
          }
          inotify_rm_watch(fdNotify, wdNotify);
        }
        else
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->inotify_add_watch(" << errno << ") error [" << m_strData << "/sqlite]:  " << strerror(errno);
          log(ssMessage.str());
        }
        close(fdNotify);
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->inotify_init1(" << errno << ") error:  " << strerror(errno);
        log(ssMessage.str());
      }
    }
    else
    {
      msleep(2000);
    }
  }
  setShutdown();
  threadDecrement();
}
// }}}
// {{{ load()
void Sqlite::load(string strPrefix)
{
  list<string> entries;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Sqlite::load()";
  m_file.directoryList(m_strData + "/sqlite", entries);
  while (!entries.empty())
  {
    if (entries.front().size() > 3 && entries.front().substr((entries.front().size() - 3), 3) == ".db")
    {
      bool bAdd = false;
      string strDatabase = entries.front().substr(0, (entries.front().size() - 3));
      m_mutex.lock();
      if (m_databases.find(strDatabase) == m_databases.end() || m_databases[strDatabase].find(m_strNode) == m_databases[strDatabase].end())
      {
        bAdd = true;
      }
      m_mutex.unlock();
      if (bAdd)
      {
        if (isMaster())
        {
          bool bMaster = false;
          databaseAdd(strPrefix, strDatabase, m_strNode, bMaster);
          if (bMaster)
          {
            databaseMaster(strPrefix, strDatabase, m_strNode);
          }
        }
        else
        {
          Json *ptLink = new Json;
          ptLink->i("Interface", "sqlite");
          ptLink->i("Node", master());
          ptLink->i("Function", "add");
          ptLink->m["Request"] = new Json;
          ptLink->m["Request"]->i("Database", strDatabase);
          ptLink->m["Request"]->i("Node", m_strNode);
          hub("link", ptLink, strError);
          delete ptLink;
        }
      }
    }
    entries.pop_front();
  }
}
// }}}
// {{{ sync()
void Sqlite::sync(string strPrefix)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Sqlite::sync()";
  if (isMaster())
  {
    list<string> nodes;
    m_mutexShare.lock();
    for (auto &link : m_l)
    {
      if (link->interfaces.find("sqlite") != link->interfaces.end())
      {
        nodes.push_back(link->strNode);
      }
    }
    m_mutexShare.unlock();
    while (!nodes.empty())
    {
      map<string, map<string, bool> > databases;
      Json *ptJson = new Json;
      ptJson->i("Interface", "sqlite");
      ptJson->i("Node", nodes.front());
      ptJson->i("Function", "databases");
      if (hub("link", ptJson, strError) && exist(ptJson, "Response"))
      {
        for (auto &i : ptJson->m["Response"]->m)
        {
          for (auto &j : i.second->m)
          {
            bool bMaster = false;
            databaseAdd(strPrefix, i.first, j.first, bMaster);
            if (bMaster)
            {
              databaseMaster(strPrefix, i.first, j.first);
            }
          }
        }
      }
      delete ptJson;
      nodes.pop_front();
    }
  }
  else
  {
    Json *ptJson = new Json;
    ptJson->i("Interface", "sqlite");
    ptJson->i("Node", master());
    ptJson->i("Function", "databases");
    if (hub("link", ptJson, strError) && exist(ptJson, "Response"))
    {
      for (auto &i : ptJson->m["Response"]->m)
      {
        for (auto &j : i.second->m)
        {
          bool bMaster = false;
          databaseAdd(strPrefix, i.first, j.first, bMaster);
          if (j.second->v == "master")
          {
            databaseMaster(strPrefix, i.first, j.first);
          }
        }
      }
    }
    delete ptJson;
  }
}
// }}}
}
}
