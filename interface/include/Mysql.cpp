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
  m_fdPipe[0] = -1;
  m_fdPipe[1] = -1;
  m_pThreadRequests = new thread(&Mysql::requests, this, strPrefix);
  pthread_setname_np(m_pThreadRequests->native_handle(), "requests");
}
// }}}
// {{{ ~Mysql()
Mysql::~Mysql()
{
  m_pThreadRequests->join();
  delete m_pThreadRequests;
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
            int fdPipe[2] = {-1, -1}, nReturn;
            if ((nReturn = pipe(fdPipe)) == 0)
            {
              bool bExit = true;
              char cChar = '\n';
              int nReturn;
              string strJson, strPort;
              stringstream ssHandle, ssServer(strServer);
              unsigned int unPort = 0;
              radial_mysql_request *ptRequest = new radial_mysql_request;
              ptRequest->bQuery = ((!empty(ptJson, "Query"))?true:false);
              ptRequest->bResult = false;
              ptRequest->fdPipe = fdPipe[1];
              ptRequest->strDatabase = strDatabase;
              ptRequest->strPassword = strPassword;
              ptRequest->strQuery = ((ptRequest->bQuery)?ptJson->m["Query"]->v:ptJson->m["Update"]->v);
              getline(ssServer, strServer, ':');
              ptRequest->strServer = strServer;
              ptRequest->strUser = strUser;
              ptRequest->ullID = 0;
              ptRequest->ullRows = 0;
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
              ptRequest->unPort = unPort; 
              ssHandle << strServer << "_" << unPort << "_" << strDatabase << "_" << strUser << "_" << strPassword;
              ptRequest->strHandle = ssHandle.str();
              if (exist(ptJson, "Response"))
              {
                delete ptJson->m["Response"];
              }
              ptJson->m["Response"] = new Json;
              ptRequest->ptResults = ptJson->m["Response"];
              ptRequest->unSize = 16 + ptRequest->ptResults->j(strJson).size();
              m_mutexRequests.lock();
              m_requests.push(ptRequest);
              m_mutexRequests.unlock();
              m_mutexPipe.lock();
              if (m_fdPipe[1] != -1)
              {
                if (write(m_fdPipe[1], &cChar, 1) > 0)
                {
                  bExit = false;
                }
                else
                {
                  ssMessage.str("");
                  ssMessage << "write(" << errno << ") " << strerror(errno);
                  strError = ssMessage.str();
                }
              }
              else
              {
                strError = "Not ready to receive requests.";
              }
              m_mutexPipe.unlock();
              while (!bExit)
              {
                pollfd fds[1];
                fds[0].fd = fdPipe[0];
                fds[0].events = POLLIN;
                if ((nReturn = poll(fds, 1, 2000)) > 0)
                {
                  if (fds[0].revents & (POLLHUP | POLLIN))
                  {
                    bExit = true;
                    if (read(fds[0].fd, &cChar, 1) > 0)
                    {
                      if (ptRequest->bResult)
                      {
                        if (!ptRequest->bQuery)
                        {
                          ptJson->i("ID", to_string(ptRequest->ullID), 'n');
                        }
                        ptJson->i("Rows", to_string(ptRequest->ullRows), 'n');
                      }
                      else if (!ptRequest->strError.empty())
                      {
                        strError = ptRequest->strError;
                      }
                      else
                      {
                        strError = "Encountered an unknown error.";
                      }
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << "read(" << errno << ") " << strerror(errno);
                      strError = ssMessage.str();
                    }
                  }
                  if (fds[0].revents & POLLERR)
                  {
                    bExit = true;
                    strError = "poll() Encountered a POLLERR.";
                  }
                  if (fds[0].revents & POLLNVAL)
                  {
                    bExit = true;
                    strError = "poll() Encountered a POLLNVAL.";
                  }
                }
                else if (nReturn < 0 && errno != EINTR)
                {
                  bExit = true;
                  ssMessage.str("");
                  ssMessage << "poll(" << errno << ") " << strerror(errno);
                  strError = ssMessage.str();
                }
              }
              delete ptRequest;
              close(fdPipe[0]);
              close(fdPipe[1]);
            }
            else
            {
              ssMessage.str("");
              ssMessage << "pipe(" << errno << ") " << strerror(errno);
              strError = ssMessage.str();
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
// {{{ connection()
void Mysql::connection(string strPrefix, radial_mysql_connection *ptConnection)
{
  // {{{ prep work
  bool bClose = false, bExit = false;
  char cChar;
  int nReturn;
  MYSQL *conn = NULL;
  queue<radial_mysql_request *> requests;
  string strError, strValue;
  stringstream ssMessage;
  unsigned int unTimeoutConnect = 5, unTimeoutRead = 30, unTimeoutWrite = 30;

  mysql_thread_init();
  threadIncrement();
  strPrefix += "->Mysql::connection()";
  // }}}
  while (!bExit)
  {
    if (!bClose)
    {
      pollfd fds[1];
      fds[0].fd = ptConnection->fdPipe[0];
      fds[0].events = POLLIN;
      if ((nReturn = poll(fds, 1, 2000)) > 0)
      {
        if (fds[0].revents & (POLLHUP | POLLIN))
        {
          if (read(fds[0].fd, &cChar, 1) <= 0)
          {
            bClose = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->read(" << errno << ") error:  " << strerror(errno);
            log(ssMessage.str());
          }
        }
        if (fds[0].revents & POLLERR)
        {
          bClose = true;
          ssMessage.str("");
          ssMessage << strPrefix << "->poll() error:  Encountered a POLLERR.";
          log(ssMessage.str());
        }
        if (fds[0].revents & POLLNVAL)
        {
          bClose = true;
          ssMessage.str("");
          ssMessage << strPrefix << "->poll() error:  Encountered a POLLNVAL.";
          log(ssMessage.str());
        }
      }
      else if (nReturn < 0 && errno != EINTR)
      {
        bClose = true;
        ssMessage.str("");
        ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
        log(ssMessage.str());
      }
    }
    ptConnection->mutexConnection.lock();
    while (!ptConnection->requests.empty())
    {
      requests.push(ptConnection->requests.front());
      ptConnection->requests.pop();
    }
    ptConnection->bIdle = requests.empty();
    ptConnection->mutexConnection.unlock();
    if (!requests.empty())
    {
      while (!requests.empty())
      {
        if (conn == NULL)
        {
          if ((conn = mysql_init(NULL)) != NULL)
          {
            mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &unTimeoutConnect);
            mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &unTimeoutRead);
            mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &unTimeoutWrite);
            if (mysql_real_connect(conn, requests.front()->strServer.c_str(), requests.front()->strUser.c_str(), requests.front()->strPassword.c_str(), requests.front()->strDatabase.c_str(), requests.front()->unPort, NULL, 0) == NULL)
            {
              ssMessage.str("");
              ssMessage << "mysql_real_connect(" << mysql_errno(conn) << ") " << mysql_error(conn);
              strError = ssMessage.str();
              mysql_close(conn);
            }
          }
          else
          {
            strError = "mysql_init() Failed to initialize MySQL library.";
          }
        }
        if (conn != NULL)
        {
          if (requests.front()->bQuery)
          {
            MYSQL_RES *result = query(conn, requests.front()->strQuery, requests.front()->ullRows, requests.front()->strError);
            if (result != NULL)
            {
              vector<string> subFields;
              if (fields(result, subFields))
              {
                map<string, string> *row;
                while (requests.front()->unSize < m_unMaxPayload && (row = fetch(result, subFields)) != NULL)
                {
                  for (auto &i : *row)
                  {
                    requests.front()->unSize += i.first.size() + i.second.size() + 6;
                  }
                  if (requests.front()->unSize < m_unMaxPayload)
                  {
                    requests.front()->ptResults->pb(*row);
                  }
                  row->clear();
                  delete row;
                }
                if (requests.front()->unSize < m_unMaxPayload)
                {
                  requests.front()->bResult = true;
                }
                else
                {
                  requests.front()->ptResults->clear();
                  ssMessage.str("");
                  ssMessage << "Payload of " << m_manip.toShortByte(requests.front()->unSize, strValue) << " exceeded " << m_manip.toShortByte(m_unMaxPayload, strValue) << " maximum.  Response has been removed.";
                  requests.front()->strError = ssMessage.str();
                }
              }
              else
              {
                requests.front()->strError = "Failed to fetch field names.";
              }
              subFields.clear();
              free(result);
            }
          }
          else if (update(conn, requests.front()->strQuery, requests.front()->ullID, requests.front()->ullRows, requests.front()->strError))
          {
            requests.front()->bResult = true;
          }
        }
        else
        {
          requests.front()->strError = strError;
        }
        cChar = '\n';
        write(requests.front()->fdPipe, &cChar, 1);
        requests.pop();
        time(&(ptConnection->CTime));
      }
      ptConnection->mutexConnection.lock();
      ptConnection->bIdle = ptConnection->requests.empty();
      ptConnection->mutexConnection.unlock();
    }
    if (bClose || ptConnection->bClose)
    {
      ptConnection->mutexConnection.lock();
      if (ptConnection->requests.empty())
      {
        bExit = true;
      }
      ptConnection->mutexConnection.unlock();
    }
  }
  // {{{ post work
  if (conn != NULL)
  {
    mysql_close(conn);
  }
  threadDecrement();
  mysql_thread_end();
  // }}}
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
// {{{ requests()
void Mysql::requests(string strPrefix)
{
  // {{{ prep work
  int nReturn;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Mysql::requests()";
  // }}}
  if ((nReturn = pipe(m_fdPipe)) == 0)
  {
    bool bExit = false;
    char cChar;
    map<string, list<radial_mysql_connection *> > handles;
    queue<string> removals;
    time_t CTime;
    while (!bExit)
    {
      pollfd fds[1];
      fds[0].fd = m_fdPipe[0];
      fds[0].events = POLLIN;
      if ((nReturn = poll(fds, 1, 2000)) > 0)
      {
        if (fds[0].revents & (POLLHUP | POLLIN))
        {
          if (read(fds[0].fd, &cChar, 1) > 0)
          {
            queue<radial_mysql_request *> requests;
            cChar = '\n';
            m_mutexRequests.lock();
            while (!m_requests.empty())
            {
              requests.push(m_requests.front());
              m_requests.pop();
            }
            m_mutexRequests.unlock();
            while (!requests.empty())
            {
              string strHandle = requests.front()->strHandle;
              list<radial_mysql_connection *>::iterator connectionIter = handles[strHandle].end();
              if (handles.find(strHandle) == handles.end())
              {
                handles[strHandle] = {};
              }
              for (auto i = handles[strHandle].begin(); i != handles[strHandle].end(); i++)
              {
                if (connectionIter == handles[strHandle].end() || (*i)->requests.size() < (*connectionIter)->requests.size())
                {
                  connectionIter = i;
                }
              }
              if (connectionIter != handles[strHandle].end() && ((*connectionIter)->requests.size() < 5 || handles[strHandle].size() >= 20))
              {
                (*connectionIter)->mutexConnection.lock();
                time(&((*connectionIter)->CTime));
                (*connectionIter)->requests.push(requests.front());
                (*connectionIter)->mutexConnection.unlock();
                write((*connectionIter)->fdPipe[1], &cChar, 1);
              }
              else
              {
                radial_mysql_connection *ptConnection = new radial_mysql_connection;
                if ((nReturn = pipe(ptConnection->fdPipe)) == 0)
                {
                  ptConnection->bClose = false;
                  ptConnection->bIdle = false;
                  time(&(ptConnection->CTime));
                  ptConnection->requests.push(requests.front());
                  ptConnection->pThread = new thread(&Mysql::connection, this, strPrefix, ptConnection);
                  pthread_setname_np(ptConnection->pThread->native_handle(), "connection");
                  write(ptConnection->fdPipe[1], &cChar, 1);
                  handles[strHandle].push_back(ptConnection);
                }
                else
                {
                  delete ptConnection;
                  bExit = true;
                  ssMessage.str("");
                  ssMessage << strPrefix << "->pipe(" << errno << ") error:  " << strerror(errno);
                  log(ssMessage.str());
                  ssMessage.str("");
                  ssMessage << "pipe(" << errno << ") " << strerror(errno);
                  requests.front()->strError = ssMessage.str();
                  write(requests.front()->fdPipe, &cChar, 1);
                }
              }
              requests.pop();
            }
          }
          else
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->read(" << errno << ") error:  " << strerror(errno);
            log(ssMessage.str());
          }
        }
        if (fds[0].revents & POLLERR)
        {
          bExit = true;
          ssMessage.str("");
          ssMessage << strPrefix << "->poll() error:  Encountered a POLLERR.";
          log(ssMessage.str());
        }
        if (fds[0].revents & POLLNVAL)
        {
          bExit = true;
          ssMessage.str("");
          ssMessage << strPrefix << "->poll() error:  Encountered a POLLNVAL.";
          log(ssMessage.str());
        }
      }
      else if (nReturn < 0 && errno != EINTR)
      {
        bExit = true;
        ssMessage.str("");
        ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
        log(ssMessage.str());
      }
      cChar = '\n';
      time(&CTime);
      for (auto &handle : handles)
      {
        list<radial_mysql_connection *>::iterator connectionIter;
        do
        {
          connectionIter = handle.second.end();
          for (auto i = handle.second.begin(); connectionIter == handle.second.end() && i != handle.second.end(); i++)
          {
            if ((*i)->bIdle && CTime > (*i)->CTime && (CTime - (*i)->CTime) > 60)
            {
              connectionIter = i;
            }
          }
          if (connectionIter != handle.second.end())
          {
            (*connectionIter)->bClose = true;
            write((*connectionIter)->fdPipe[1], &cChar, 1);
            (*connectionIter)->pThread->join();
            delete (*connectionIter)->pThread;
            close((*connectionIter)->fdPipe[0]);
            close((*connectionIter)->fdPipe[1]);
            delete (*connectionIter);
            handle.second.erase(connectionIter);
          }
        } while (connectionIter != handle.second.end());
        if (handle.second.empty())
        {
          removals.push(handle.first);
        }
      }
      while (!removals.empty())
      {
        handles.erase(removals.front());
        removals.pop();
      }
      if (shutdown())
      {
        bExit = true;
      }
    }
    close(m_fdPipe[0]);
    m_fdPipe[0] = -1;
    m_mutexPipe.lock();
    close(m_fdPipe[1]);
    m_fdPipe[1] = -1;
    m_mutexPipe.unlock();
    cChar = '\n';
    for (auto &handle : handles)
    {
      while (!handle.second.empty())
      {
        handle.second.front()->bClose = true;
        write(handle.second.front()->fdPipe[1], &cChar, 1);
        handle.second.front()->pThread->join();
        delete handle.second.front()->pThread;
        close(handle.second.front()->fdPipe[0]);
        close(handle.second.front()->fdPipe[1]);
        delete handle.second.front();
        handle.second.pop_front();
      }
    }
    m_mutexRequests.lock();
    while (!m_requests.empty())
    {
      m_requests.front()->strError = "Interface is shutting down.";
      write(m_requests.front()->fdPipe, &cChar, 1);
      m_requests.pop();
    }
    m_mutexRequests.unlock();
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->pipe(" << errno << ") error:  " << strerror(errno);
    log(ssMessage.str());
  }
  // {{{ post work
  setShutdown();
  threadDecrement();
  // }}}
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
