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
  mysql_library_init(0, NULL, NULL);
}
// }}}
// {{{ ~Mysql()
Mysql::~Mysql()
{
  mysql_library_end();
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
    string strServer = ptJson->m["Server"]->v;
    if (!empty(ptJson, "User"))
    {
      string strUser = ptJson->m["User"]->v;
      if (!empty(ptJson, "Password"))
      {
        string strPassword = ptJson->m["Password"]->v;
        if (!empty(ptJson, "Database"))
        {
          string strDatabase = ptJson->m["Database"]->v;
          if (!empty(ptJson, "Query") || !empty(ptJson, "Update"))
          {
            MYSQL *conn = NULL;
            if ((conn = mysql_init(NULL)) != NULL)
            {
              string strPort;
              stringstream ssError, ssServer(ptJson->m["Server"]->v);
              unsigned int unPort = 0, unTimeoutConnect = 5, unTimeoutRead = 30, unTimeoutWrite = 30;
              mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &unTimeoutConnect);
              mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &unTimeoutRead);
              mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &unTimeoutWrite);
              getline(ssServer, strServer, ':');
              getline(ssServer, strPort, ':');
              if (!empty(ptJson, "Port"))
              {
                strPort = ptJson->m["Port"]->v;
              }
              if (!strPort.empty())
              {
                stringstream ssPort(strPort);
                ssPort >> unPort;
              }
              if (mysql_real_connect(conn, strServer.c_str(), strUser.c_str(), strPassword.c_str(), strDatabase.c_str(), unPort, NULL, 0) != NULL)
              {
                unsigned long long ullRows = 0;
                if (!empty(ptJson, "Query"))
                {
                  MYSQL_RES *result = query(conn, ptJson->m["Query"]->v, ullRows, strError);
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
                }
                else
                {
                  unsigned long long ullID = 0;
                  if ((bResult = update(conn, ptJson->m["Update"]->v, ullID, ullRows, strError)))
                  {
                    stringstream ssID, ssRows;
                    ssID << ullID;
                    ptJson->i("ID", ssID.str(), 'n');
                    ssRows << ullRows;
                    ptJson->i("Rows", ssRows.str(), 'n');
                  }
                }
              }
              else
              {
                ssError.str("");
                ssError << "mysql_real_connect(" << mysql_errno(conn) << ") [" << strServer << "," << strUser << "," << strDatabase << "]:  " << mysql_error(conn);
                strError = ssError.str();
              }
              mysql_close(conn);
            }
            else
            {
              strError = "mysql_init():  Failed to initialize MySQL library.";
            }
            mysql_thread_end();
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
// {{{ query()
MYSQL_RES *Mysql::query(MYSQL *conn, const string strQuery, unsigned long long &ullRows, string &strError)
{
  bool bRetry = true;
  size_t unAttempt = 0;
  stringstream ssError;
  MYSQL_RES *result = NULL;

  while (bRetry && unAttempt++ < 10)
  {
    bRetry = false;
    if (mysql_query(conn, strQuery.c_str()) == 0)
    {
      if ((result = mysql_store_result(conn)) != NULL)
      {
        ullRows = mysql_num_rows(result);
      }
      else
      {
        ssError.str("");
        ssError << "mysql_store_result(" << mysql_errno(conn) << "):  " << mysql_error(conn);
        strError = ssError.str();
      }
    }
    else
    {
      ssError.str("");
      ssError << "mysql_query(" << mysql_errno(conn) << "):  " << mysql_error(conn);
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
// {{{ update()
bool Mysql::update(MYSQL *conn, const string strQuery, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  bool bResult = false, bRetry = true;
  size_t unAttempt = 0;
  stringstream ssError;

  while (bRetry && unAttempt++ < 10)
  {
    bRetry = false;
    if (mysql_real_query(conn, strQuery.c_str(), strQuery.size()) == 0)
    {
      bResult = true;
      ullID = mysql_insert_id(conn);
      ullRows = mysql_affected_rows(conn);
    }
    else
    {
      ssError.str("");
      ssError << "mysql_real_query(" << mysql_errno(conn) << "):  " << mysql_error(conn);
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
