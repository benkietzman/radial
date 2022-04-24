// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Request.cpp
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
#include "Request"
// }}}
extern "C++"
{ 
namespace radial
{
// {{{ Request()
Request::Request(int argc, char **argv) : Interface("request", argc, argv)
{
  m_bCallback = false;
}
Request::Request(int argc, char **argv, function<bool(string, Json *, string &)> callback) : Interface("request", argc, argv)
{
  m_bCallback = true;
  m_callback = callback;
}
// }}}
// {{{ ~Request()
Request::~Request()
{
}
// }}}
// {{{ accept()
void Request::accept()
{
  SSL_CTX *ctx = NULL;
  string strError, strPrefix = "Request::accept()";
  stringstream ssMessage;

  setlocale(LC_ALL, "");
  if ((ctx = m_pUtility->sslInitServer(m_strData + "/server.crt", m_strData + "/server.key", strError)) != NULL)
  {
    addrinfo hints, *result;
    bool bBound[3] = {false, false, false};
    int fdSocket, nReturn;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((nReturn = getaddrinfo(NULL, "7234", &hints, &result)) == 0)
    {
      addrinfo *rp;
      bBound[0] = true;
      for (rp = result; !bBound[2] && rp != NULL; rp = rp->ai_next)
      {
        bBound[1] = false;
        if ((fdSocket = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
        {
          int nOn = 1;
          bBound[1] = true;
          setsockopt(fdSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&nOn, sizeof(nOn));
          if (bind(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
          {
            bBound[2] = true;
          }
          else
          {
            close(fdSocket);
          }
        }
      }
      freeaddrinfo(result);
    }
    if (bBound[2])
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->bind() [" << fdSocket << "]:  Bound incoming socket.";
      log(ssMessage.str());
      if (listen(fdSocket, 5) == 0)
      {
        bool bExit = false;
        ssMessage.str("");
        ssMessage << strPrefix << "->listen() [" << fdSocket << "]:  Listening to incoming socket.";
        log(ssMessage.str());
        while (!bExit)
        {
          pollfd fds[1];
          fds[0].fd = fdSocket;
          fds[0].events = POLLIN;
          if ((nReturn = poll(fds, 1, 250)) > 0)
          {
            if (fds[0].revents & POLLIN)
            {
              int fdClient;
              sockaddr_in cli_addr;
              socklen_t clilen = sizeof(cli_addr);
              if ((fdClient = ::accept(fdSocket, (sockaddr *)&cli_addr, &clilen)) >= 0)
              {
                thread threadRequestSocket(&Request::socket, this, strPrefix, ctx, fdClient);
                threadRequestSocket.detach();
              }
              else
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->accept(" << errno << ") error [" << fdSocket << "]:  " << strerror(errno);
                notify(ssMessage.str());
              }
            }
          }
          else if (nReturn < 0)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->poll(" << errno << ") error [" << fdSocket << "]:  " << strerror(errno);
            notify(ssMessage.str());
          }
        }
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->listen(" << errno << ") error [" << fdSocket << "]:  " << strerror(errno);
        notify(ssMessage.str());
      }
      close(fdSocket);
    }
    else
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->";
      if (!bBound[0])
      {
        ssMessage << "getaddrinfo(" << nReturn << ") error:  " << gai_strerror(nReturn);
      }
      else
      {
        ssMessage << ((!bBound[1])?"socket":"bind") << "(" << errno << ") error:  " << strerror(errno);
      }
      notify(ssMessage.str());
    }
    SSL_CTX_free(ctx);
    EVP_cleanup();
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->Utility::sslInitServer() error:  " << strError;
    notify(ssMessage.str());
  }
  m_pUtility->sslDeinit();
}
// }}}
// {{{ callback()
void Request::callback(string strPrefix, Json *ptJson)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Request::callback()";
  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
  {
    if (ptJson->m["Function"]->v == "ping")
    {
      bResult = true;
    }
    else
    {
      strError = "Please provide a valid Function:  ping.";
    }
  }
  else
  {
    strError = "Please provide the Function.";
  }
  ptJson->insert("Status", ((bResult)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->insert("Error", strError);
  }
  response(ptJson);
}
// }}}
// {{{ request()
void Request::request(string strPrefix, Json *ptJson)
{
  // {{{ prep work
  bool bResult = false;
  string strError;
  stringstream ssMessage;
  // }}}
  if (ptJson->m.find("Section") != ptJson->m.end() && !ptJson->m["Section"]->v.empty())
  {
    // {{{ mysql
    if (ptJson->m["Section"]->v == "mysql")
    {
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
                unsigned int unPort = 0;
                unsigned long long ullRows = 0;
                if (ptJson->m.find("Port") != ptJson->m.end() && !ptJson->m["Port"]->v.empty())
                {
                  stringstream ssPort(ptJson->m["Port"]->v);
                  ssPort >> unPort;
                }
                if (ptJson->m.find("Query") != ptJson->m.end() && !ptJson->m["Query"]->v.empty())
                {
                  list<map<string, string> > rows;
                  if (mysqlQuery(ptJson->m["Server"]->v, unPort, ptJson->m["User"]->v, ptJson->m["Password"]->v, ptJson->m["Database"]->v, ptJson->m["Query"]->v, ullRows, rows, strError))
                  {
                    bResult = true;
                    ptJson->m["Data"] = new Json;
                    for (auto &row: rows)
                    {
                      ptJson->m["Data"]->push_back(row);
                    }
                  }
                }
                else
                {
                  unsigned long long ullID = 0;
                  if (mysqlUpdate(ptJson->m["Server"]->v, unPort, ptJson->m["User"]->v, ptJson->m["Password"]->v, ptJson->m["Database"]->v, ptJson->m["Update"]->v, ullID, ullRows, strError))
                  {
                    stringstream ssID;
                    bResult = true;
                    ssID << ullID;
                    ptJson->insert("ID", ssID.str(), 'n');
                  }
                }
                if (bResult)
                {
                  stringstream ssRows;
                  ssRows << ullRows;
                  ptJson->insert("Rows", ssRows.str(), 'n');
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
    }
    // }}}
    // {{{ ping
    else if (ptJson->m["Section"]->v == "ping")
    {
      bResult = true;
    }
    // }}}
    // {{{ callback
    else if (m_bCallback)
    {
      bResult = m_callback(strPrefix, ptJson, strError);
    }
    // }}}
    // {{{ invalid
    else
    {
      strError = "Please provide a valid Section.";
    }
    // }}}
  }
  else
  {
    strError = "Please provide the Section.";
  }
  // {{{ post work
  ptJson->insert("Status", ((bResult)?"okay":"error"));
  if (!strError.empty())
  {
    ptJson->insert("Error", strError);
  }
  // }}}
}
// }}}
// {{{ socket()
void Request::socket(string strPrefix, SSL_CTX *ctx, int fdSocket)
{
  bool bExit = false;
  int nReturn;
  size_t unPosition;
  string strBuffer[2], strError;
  stringstream ssMessage;
  SSL *ssl;
  common_socket_type eSocketType = COMMON_SOCKET_UNKNOWN;
  Json *ptJson;

  strPrefix += "->Request::socket()";
  while (!bExit)
  {
    pollfd fds[1];
    fds[0].fd = fdSocket;
    fds[0].events = POLLIN;
    if (!strBuffer[1].empty())
    {
      fds[0].events |= POLLOUT;
    }
    if ((nReturn = poll(fds, 1, 250)) > 0)
    {
      if (fds[0].revents & POLLIN)
      {
        if (eSocketType == COMMON_SOCKET_UNKNOWN)
        {
          if (m_pUtility->socketType(fdSocket, eSocketType, strError))
          {
            if (eSocketType == COMMON_SOCKET_ENCRYPTED)
            {
              ERR_clear_error();
              if ((ssl = m_pUtility->sslAccept(ctx, fdSocket, strError)) == NULL)
              {
                bExit = true;
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslAccept() error [" << fdSocket << "]:  " << strError;
                log(ssMessage.str());
              }
            }
          }
          else
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->Utility::socketType() error [" << fdSocket << "]:  " << strError;
            log(ssMessage.str());
          }
        }
        if (!bExit && ((eSocketType == COMMON_SOCKET_ENCRYPTED && m_pUtility->sslRead(ssl, strBuffer[0], nReturn)) || (eSocketType == COMMON_SOCKET_UNENCRYPTED && m_pUtility->fdRead(fdSocket, strBuffer[0], nReturn))))
        {
          if ((unPosition = strBuffer[0].find("\n")) != string::npos)
          {
            ptJson = new Json(strBuffer[0].substr(0, unPosition));
            strBuffer[0].erase(0, (unPosition + 1));
            request(strPrefix, ptJson);
            ptJson->json(strBuffer[1]);
            strBuffer[1] += "\n";
            delete ptJson;
          }
        }
        else
        {
          bExit = true;
          if (nReturn < 0)
          {
            ssMessage.str("");
            ssMessage << strPrefix << "->Utility::";
            if (eSocketType == COMMON_SOCKET_ENCRYPTED)
            {
              ssMessage << "sslRead(" << SSL_get_error(ssl, nReturn) << ") error [" << fdSocket << "]:  " << m_pUtility->sslstrerror();
            }
            else
            {
              ssMessage << "fdRead(" << errno << ") error [" << fdSocket << "]:  " << strerror(errno);
            }
            log(ssMessage.str());
          }
        }
      }
      if (fds[0].revents & POLLOUT)
      {
        if ((eSocketType == COMMON_SOCKET_ENCRYPTED && m_pUtility->sslWrite(ssl, strBuffer[1], nReturn)) || (eSocketType == COMMON_SOCKET_UNENCRYPTED && m_pUtility->fdWrite(fdSocket, strBuffer[1], nReturn)))
        {
          if (strBuffer[1].empty())
          {
            bExit = true;
          }
        }
        else
        {
          bExit = true;
          if (nReturn < 0)
          {
            ssMessage.str("");
            ssMessage << strPrefix << "->Utility::";
            if (eSocketType == COMMON_SOCKET_ENCRYPTED)
            {
              ssMessage << "sslWrite(" << SSL_get_error(ssl, nReturn) << ") error [" << fdSocket << "]:  " << m_pUtility->sslstrerror();
            }
            else
            {
              ssMessage << "fdWrite(" << errno << ") error [" << fdSocket << "]:  " << strerror(errno);
            }
            log(ssMessage.str());
          }
        }
      }
    }
    else if (nReturn < 0)
    {
      bExit = true;
      ssMessage.str("");
      ssMessage << strPrefix << "->poll(" << errno << ") error [" << fdSocket << "]:  " << strerror(errno);
      log(ssMessage.str());
    }
  }
  if (eSocketType == COMMON_SOCKET_ENCRYPTED)
  {
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }
  close(fdSocket);
}
// }}}
}
}
