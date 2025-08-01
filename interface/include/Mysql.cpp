// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Mysql.cpp
// author     : Ben Kietzman
// begin      : 2022-04-21
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Mysql"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Mysql()
Mysql::Mysql(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "mysql", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Mysql()
Mysql::~Mysql()
{
  for (auto &i : m_conn)
  {
    for (auto &j : i.second)
    {
      mysql_close(j->conn);
      delete j;
    }
    i.second.clear();
  }
  m_conn.clear();
}
// }}}
// {{{ callback()
void Mysql::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError, strValue;
  stringstream ssMessage;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Mysql::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Server"))
  {
    if (!empty(ptJson, "User"))
    {
      if (!empty(ptJson, "Password"))
      {
        if (!empty(ptJson, "Database"))
        {
          if (!empty(ptJson, "Query") || !empty(ptJson, "Update"))
          {
            bool bRetry = true;
            size_t unAttempt = 0;
            while (bRetry && unAttempt++ < 10)
            {
              bRetry = false;
              list<radial_mysql *>::iterator mysqlIter;
              string strPort, strServer;
              stringstream ssError, ssServer(ptJson->m["Server"]->v);
              unsigned int unPort = 0;
              unsigned long long ullRows = 0;
              getline(ssServer, strServer, ':');
              getline(ssServer, strPort, ':');
              if (!empty(ptJson, "Port"))
              {
                stringstream ssPort(ptJson->m["Port"]->v);
                ssPort >> unPort;
              }
              if (connect(strServer, unPort, ptJson->m["User"]->v, ptJson->m["Password"]->v, ptJson->m["Database"]->v, mysqlIter, strError))
              {
                bool bForce = false;
                (*mysqlIter)->secure.lock();
                if (!empty(ptJson, "Query"))
                {
                  MYSQL_RES *result = query(mysqlIter, ptJson->m["Query"]->v, ullRows, strError);
                  if (result != NULL)
                  {
                    vector<string> subFields;
                    if (fields(result, subFields))
                    {
                      size_t unSize = 16;
                      map<string, string> *row;
                      string strJson;
                      stringstream ssRows;
                      ssRows << ullRows;
                      ptJson->i("Rows", ssRows.str(), 'n');
                      ptJson->m["Response"] = new Json;
                      unSize += ptJson->j(strJson).size();
                      while (unSize < m_unMaxPayload && (row = fetch(result, subFields)) != NULL)
                      {
                        for (auto &i : *row)
                        {
                          unSize += i.first.size() + i.second.size() + 6;
                        }
                        if (unSize < m_unMaxPayload)
                        {
                          ptJson->m["Response"]->pb(*row);
                        }
                        row->clear();
                        delete row;
                      }
                      if (unSize < m_unMaxPayload)
                      {
                        bResult = true;
                      }
                      else
                      {
                        delete ptJson->m["Response"];
                        ptJson->m.erase("Response");
                        ssMessage.str("");
                        ssMessage << "Payload of " << m_manip.toShortByte(unSize, strValue) << " exceeded " << m_manip.toShortByte(m_unMaxPayload, strValue) << " maximum.  Response has been removed.";
                        strError = ssMessage.str();
                      }
                    }
                    else
                    {
                      strError = "Failed to fetch field names.";
                    }
                    subFields.clear();
                    free(result);
                  }
                  else if (strError.find("Lost connection") != string::npos)
                  {
                    bForce = bRetry = true;
                  }
                }
                else
                {
                  unsigned long long ullID = 0;
                  if ((bResult = update(mysqlIter, ptJson->m["Update"]->v, ullID, ullRows, strError)))
                  {
                    stringstream ssID, ssRows;
                    ssID << ullID;
                    ptJson->i("ID", ssID.str(), 'n');
                    ssRows << ullRows;
                    ptJson->i("Rows", ssRows.str(), 'n');
                  }
                  else if (strError.find("Lost connection") != string::npos)
                  {
                    bForce = bRetry = true;
                  }
                }
                (*mysqlIter)->secure.unlock();
                disconnect(mysqlIter, bForce);
              }
              else
              {
                bRetry = true;
              }
            }
          }
          else
          {
            strError = "Please provide the Query or Update.";
          }
        }
        else
        {
          strError = "Please provide the Database.";
        }
      }
      else
      {
        strError = "Please provide the Password.";
      }
    }
    else
    {
      strError = "Please provide the User.";
    }
  }
  else
  {
    strError = "Please provide the Server.";
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
// {{{ conn()
map<string, list<radial_mysql *> > *Mysql::conn()
{
  return &m_conn;
}
// }}}
// {{{ connect()
bool Mysql::connect(const string strServer, const unsigned int unPort, const string strUser, const string strPassword, const string strDatabase, list<radial_mysql *>::iterator &iter, string &strError)
{
  bool bResult = false;
  string strName;
  stringstream ssError, ssName;

  ssName << strServer << "_" << strUser << "_" << strDatabase;
  strName = ssName.str();
  lock();
  if (m_conn.find(strName) == m_conn.end())
  {
    list<radial_mysql *> mysqlList;
    m_conn[strName] = mysqlList;
  }
  if (m_conn.find(strName) != m_conn.end())
  {
    iter = m_conn[strName].end();
    for (auto i = m_conn[strName].begin(); iter == m_conn[strName].end() && i != m_conn[strName].end(); i++)
    {
      if (!(*i)->bClose && (*i)->unThreads < 5)
      {
        iter = i;
      }
    }
    if (iter == m_conn[strName].end() && m_conn[strName].size() >= 20)
    {
      iter = m_conn[strName].end();
      for (auto i = m_conn[strName].begin(); iter == m_conn[strName].end() && i != m_conn[strName].end(); i++)
      {
        if (!(*i)->bClose)
        {
          iter = i;
        }
      }
      if (iter != m_conn[strName].end())
      {
        for (auto i = iter; i != m_conn[strName].end(); i++)
        {
          if (!(*i)->bClose && (*i)->unThreads < (*iter)->unThreads)
          {
            iter = i;
          }
        }
      }
    }
    if (iter == m_conn[strName].end())
    {
      if (!strName.empty())
      {
        radial_mysql *ptMysql = new radial_mysql;
        ptMysql->bClose = false;
        if ((ptMysql->conn = mysql_init(NULL)) != NULL)
        {
          unsigned int unTimeoutConnect = 5, unTimeoutRead = 30, unTimeoutWrite = 30;
          mysql_options(ptMysql->conn, MYSQL_OPT_CONNECT_TIMEOUT, &unTimeoutConnect);
          mysql_options(ptMysql->conn, MYSQL_OPT_READ_TIMEOUT, &unTimeoutRead);
          mysql_options(ptMysql->conn, MYSQL_OPT_WRITE_TIMEOUT, &unTimeoutWrite);
          if (mysql_real_connect(ptMysql->conn, strServer.c_str(), strUser.c_str(), strPassword.c_str(), strDatabase.c_str(), unPort, NULL, 0) != NULL)
          {
            ptMysql->unThreads = 0;
            m_conn[strName].push_back(ptMysql);
            iter = m_conn[strName].end();
            iter--;
          }
          else
          {
            ssError.str("");
            ssError << "mysql_real_connect(" << mysql_errno(ptMysql->conn) << ") [" << strServer << "," << strUser << "," << strDatabase << "]:  " << mysql_error(ptMysql->conn);
            strError = ssError.str();
            mysql_close(ptMysql->conn);
            delete ptMysql;
          }
        }
        else
        {
          strError = "mysql_init():  Failed to initialize MySQL library.";
          delete ptMysql;
        }
      }
      else
      {
        strError = "Please provide the Name.";
      }
    }
    if (iter != m_conn[strName].end())
    {
      bResult = true;
      (*iter)->unThreads++;
      time(&((*iter)->CTime));
    }
  }
  else
  {
    strError = "Failed to insert database name into connection pool.";
  }
  unlock();

  return bResult;
}
// }}}
// {{{ disconnect()
void Mysql::disconnect(list<radial_mysql *>::iterator &iter, const bool bForce)
{
  list<map<string, list<radial_mysql *> >::iterator> removeName;
  time_t CTime;

  time(&CTime);
  lock();
  if (bForce)
  {
    (*iter)->bClose = true;
  }
  (*iter)->unThreads--;
  for (auto i = m_conn.begin(); i != m_conn.end(); i++)
  {
    list<list<radial_mysql *>::iterator> removeConn;
    for (auto j = i->second.begin(); j != i->second.end(); j++)
    {
      if ((*j)->bClose || ((*j)->unThreads == 0 && (CTime - (*j)->CTime) > 60))
      {
        removeConn.push_back(j);
      }
    }
    for (auto &j : removeConn)
    {
      mysql_close((*j)->conn);
      delete (*j);
      i->second.erase(j);
      if (i->second.empty())
      {
        removeName.push_back(i);
      }
    }
    removeConn.clear();
  }
  for (auto &i : removeName)
  {
    m_conn.erase(i);
  }
  removeName.clear();
  unlock();
}
// }}}
// {{{ fetch()
map<string, string> *Mysql::fetch(MYSQL_RES *result, vector<string> subFields)
{
  map<string, string> *pRow = NULL;
  MYSQL_ROW row;

  if ((row = mysql_fetch_row(result)))
  {
    map<string, string> rowMap;
    for (unsigned int i = 0; i < subFields.size(); i++)
    {
      rowMap[subFields[i]] = (row[i] != NULL)?row[i]:"";
    }
    pRow = new map<string, string>(rowMap);
    rowMap.clear();
  }

  return pRow;
}
// }}}
// {{{ fields()
bool Mysql::fields(MYSQL_RES *result, vector<string> &subFields)
{
  bool bResult = false;
  MYSQL_FIELD *field;

  while ((field = mysql_fetch_field(result)) != NULL)
  {
    string strValue;
    bResult = true;
    strValue.assign(field->name, field->name_length);
    subFields.push_back(strValue);
  }

  return bResult;
}
// }}}
// {{{ free()
void Mysql::free(MYSQL_RES *result)
{
  mysql_free_result(result);
}
// }}}
// {{{ lock()
void Mysql::lock()
{
  m_mutex.lock();
}
// }}}
// {{{ query()
MYSQL_RES *Mysql::query(list<radial_mysql *>::iterator &iter, const string strQuery, unsigned long long &ullRows, string &strError)
{
  bool bRetry = true;
  size_t unAttempt = 0;
  stringstream ssError;
  MYSQL_RES *result = NULL;

  while (bRetry && unAttempt++ < 10)
  {
    bRetry = false;
    if (mysql_query((*iter)->conn, strQuery.c_str()) == 0)
    {
      if ((result = mysql_store_result((*iter)->conn)) != NULL)
      {
        ullRows = mysql_num_rows(result);
      }
      else
      {
        ssError.str("");
        ssError << "mysql_store_result(" << mysql_errno((*iter)->conn) << "):  " << mysql_error((*iter)->conn);
        strError = ssError.str();
      }
    }
    else
    {
      ssError.str("");
      ssError << "mysql_query(" << mysql_errno((*iter)->conn) << "):  " << mysql_error((*iter)->conn);
      strError = ssError.str();
      if (strError.find("try restarting transaction") != string::npos)
      {
        bRetry = true;
      }
    }
    if (bRetry)
    {
      msleep(250);
    }
  }

  return result;
}
// }}}
// {{{ unlock()
void Mysql::unlock()
{
  m_mutex.unlock();
}
// }}}
// {{{ update()
bool Mysql::update(list<radial_mysql *>::iterator &iter, const string strQuery, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  bool bResult = false, bRetry = true;
  size_t unAttempt = 0;
  stringstream ssError;

  while (bRetry && unAttempt++ < 10)
  {
    bRetry = false;
    if (mysql_real_query((*iter)->conn, strQuery.c_str(), strQuery.size()) == 0)
    {
      bResult = true;
      ullID = mysql_insert_id((*iter)->conn);
      ullRows = mysql_affected_rows((*iter)->conn);
    }
    else
    {
      ssError.str("");
      ssError << "mysql_real_query(" << mysql_errno((*iter)->conn) << "):  " << mysql_error((*iter)->conn);
      strError = ssError.str();
      if (strError.find("try restarting transaction") != string::npos)
      {
        bRetry = true;
      }
    }
    if (bRetry)
    {
      msleep(250);
    }
  }

  return bResult;
}
// }}}
}
}
