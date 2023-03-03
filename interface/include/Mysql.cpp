// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Mysql.cpp
// author     : Ben Kietzman
// begin      : 2022-04-21
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Mysql"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Mysql()
Mysql::Mysql(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "mysql", argc, argv, pCallback)
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
void Mysql::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;

  threadIncrement();
  strPrefix += "->Mysql::callback()";
  if (ptJson->m.find("Server") != ptJson->m.end() && !ptJson->m["Server"]->v.empty())
  {
    if (ptJson->m.find("User") != ptJson->m.end() && !ptJson->m["User"]->v.empty())
    {
      if (ptJson->m.find("Password") != ptJson->m.end() && !ptJson->m["Password"]->v.empty())
      {
        if (ptJson->m.find("Database") != ptJson->m.end() && !ptJson->m["Database"]->v.empty())
        {
          if ((ptJson->m.find("Query") != ptJson->m.end() && !ptJson->m["Query"]->v.empty()) || (ptJson->m.find("Update") != ptJson->m.end() && !ptJson->m["Update"]->v.empty()))
          {
            list<radial_mysql *>::iterator mysqlIter;
            string strPort, strServer;
            stringstream ssError, ssServer(ptJson->m["Server"]->v);;
            unsigned int unPort = 0;
            unsigned long long ullRows = 0;
            getline(ssServer, strServer, ':');
            getline(ssServer, strPort, ':');
            if (ptJson->m.find("Port") != ptJson->m.end() && !ptJson->m["Port"]->v.empty())
            {
              stringstream ssPort(ptJson->m["Port"]->v);
              ssPort >> unPort;
            }
            if (connect(strServer, unPort, ptJson->m["User"]->v, ptJson->m["Password"]->v, ptJson->m["Database"]->v, mysqlIter, strError))
            {
              (*mysqlIter)->secure.lock();
              if (ptJson->m.find("Query") != ptJson->m.end() && !ptJson->m["Query"]->v.empty())
              {
                MYSQL_RES *result = query(mysqlIter, ptJson->m["Query"]->v, ullRows, strError);
                if (result != NULL)
                {
                  vector<string> subFields;
                  if (fields(result, subFields))
                  {
                    map<string, string> *row;
                    stringstream ssRows;
                    bResult = true;
                    ssRows << ullRows;
                    ptJson->i("Rows", ssRows.str(), 'n');
                    ptJson->m["Response"] = new Json;
                    while ((row = fetch(result, subFields)) != NULL)
                    {
                      ptJson->m["Response"]->pb(*row);
                      row->clear();
                      delete row;
                    }
                  }
                  else
                  {
                    strError = "Failed to fetch field names.";
                  }
                  subFields.clear();
                  free(result);
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
              }
              (*mysqlIter)->secure.unlock();
              disconnect(mysqlIter);
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
    hub(ptJson, false);
  }
  delete ptJson;
  threadDecrement();
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
      if ((*i)->unThreads < 5)
      {
        iter = i;
      }
    }
    if (iter == m_conn[strName].end() && m_conn[strName].size() >= 20)
    {
      iter = m_conn[strName].begin();
      for (auto i = m_conn[strName].begin(); i != m_conn[strName].end(); i++)
      {
        if ((*i)->unThreads < (*iter)->unThreads)
        {
          iter = i;
        }
      }
    }
    if (iter == m_conn[strName].end())
    {
      if (!strName.empty())
      {
        bool bConnected = false;
        radial_mysql *ptMysql = new radial_mysql;
        if ((ptMysql->conn = mysql_init(NULL)) != NULL)
        {
          unsigned int unTimeoutConnect = 5, unTimeoutRead = 30, unTimeoutWrite = 30;
          mysql_options(ptMysql->conn, MYSQL_OPT_CONNECT_TIMEOUT, &unTimeoutConnect);
          mysql_options(ptMysql->conn, MYSQL_OPT_READ_TIMEOUT, &unTimeoutRead);
          mysql_options(ptMysql->conn, MYSQL_OPT_WRITE_TIMEOUT, &unTimeoutWrite);
          if (mysql_real_connect(ptMysql->conn, strServer.c_str(), strUser.c_str(), strPassword.c_str(), strDatabase.c_str(), unPort, NULL, 0) != NULL)
          {
            bConnected = true;
          }
          else
          {
            ssError.str("");
            ssError << "mysql_real_connect(" << mysql_errno(ptMysql->conn) << ") [" << strServer << "," << strUser << "," << strDatabase << "]:  " << mysql_error(ptMysql->conn);
            strError = ssError.str();
            mysql_close(ptMysql->conn);
          }
        }
        else
        {
          strError = "mysql_init():  Failed to initialize MySQL library.";
        }
        if (bConnected)
        {
          ptMysql->unThreads = 0;
          m_conn[strName].push_back(ptMysql);
          iter = m_conn[strName].end();
          iter--;
        }
        else
        {
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
void Mysql::disconnect(list<radial_mysql *>::iterator &iter)
{
  list<map<string, list<radial_mysql *> >::iterator> removeName;
  time_t CTime;

  time(&CTime);
  lock();
  (*iter)->unThreads--;
  for (auto i = m_conn.begin(); i != m_conn.end(); i++)
  {
    list<list<radial_mysql *>::iterator> removeConn;
    for (auto j = i->second.begin(); j != i->second.end(); j++)
    {
      if ((*j)->unThreads == 0 && (CTime - (*j)->CTime) > 60)
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
