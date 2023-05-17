// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Logger.cpp
// author     : Ben Kietzman
// begin      : 2022-05-17
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Logger"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Logger()
Logger::Logger(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "logger", argc, argv, pCallback)
{
  string strError;

  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-p" || (strArg.size() > 7 && strArg.substr(0, 7) == "--port="))
    {
      if (strArg == "-p" && i + 1 < argc && argv[i+1][0] != '-')
      {
        m_strPort = argv[++i];
      }
      else
      {
        m_strPort = strArg.substr(7, strArg.size() - 7);
      }
      m_manip.purgeChar(m_strPort, m_strPort, "'");
      m_manip.purgeChar(m_strPort, m_strPort, "\"");
    }
    else if (strArg == "-s" || (strArg.size() > 9 && strArg.substr(0, 9) == "--server="))
    {
      if (strArg == "-s" && i + 1 < argc && argv[i+1][0] != '-')
      {
        m_strServer = argv[++i];
      }
      else
      {
        m_strServer = strArg.substr(9, strArg.size() - 9);
      }
      m_manip.purgeChar(m_strServer, m_strServer, "'");
      m_manip.purgeChar(m_strServer, m_strServer, "\"");
    }
  }
  // }}}
  if (m_pWarden != NULL)
  {
    Json *ptCred = new Json;
    if (m_pWarden->vaultRetrieve({"logger"}, ptCred, strError))
    {
      if (!empty(ptCred, "Password"))
      {
        m_strPassword = ptCred->m["Password"]->v;
      }
      if (!empty(ptCred, "User"))
      {
        m_strUser = ptCred->m["User"]->v;
      }
    }
    delete ptCred;
  }
}
// }}}
// {{{ ~Logger()
Logger::~Logger()
{
}
// }}}
// {{{ callback()
void Logger::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Logger::callback()";
  if (exist(ptJson, "Label"))
  {
    map<string, string> label;
    bResult = true;
    ptJson->m["Label"]->flatten(label, true, false);
    m_mutex.lock();
    m_messages.push_back(label);
    m_mutex.unlock();
  }
  else
  {
    strError = "Please provide the Label.";
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
// {{{ push()
void Logger::push(string strPrefix)
{
  addrinfo hints, *result, *rp;
  bool bConnected[2], bExit, bGood = true;
  deque<map<string, string> > entries;
  int fdSocket, nReturn;
  list<radialConn *> conns;
  list<radialConn *>::iterator connIter;
  map<string, string> label;
  size_t unPosition;
  string strError, strFunction, strJson, strMessage;
  stringstream ssMessage;
  time_t CTime[2];
  vector<string> buffer;
  Json *ptJson;

  threadIncrement();
  strPrefix += "->Logger::logger()";
  time(&CTime[0]);
  while (bGood)
  {
    bExit = false;
    // {{{ m_messages --> entry
    if (!m_messages.empty() && entries.empty())
    {
      m_mutex.lock();
      entries = m_messages;
      m_messages.clear();
      m_mutex.unlock();
    }
    // }}}
    // {{{ entry --> item
    while (!bExit && !entries.empty())
    {
      connIter = conns.end();
      for (auto i = conns.begin(); connIter == conns.end() && i != conns.end(); i++)
      {
        if ((*i)->message.size() < 10)
        {
          connIter = i;
        }
      }
      if (connIter == conns.end() && conns.size() >= 10)
      {
        connIter = conns.begin();
        for (auto i = conns.begin(); i != conns.end(); i++)
        {
          if ((*i)->message.size() < (*connIter)->message.size())
          {
            connIter = i;
          }
        }
      }
      if (connIter == conns.end())
      {
        memset(&hints, 0, sizeof(addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if ((nReturn = getaddrinfo(m_strServer.c_str(), m_strPort.c_str(), &hints, &result)) == 0)
        {
          bConnected[0] = bConnected[1] = false;
          for (rp = result; !bConnected[1] && rp != NULL; rp = rp->ai_next)
          {
            bConnected[0] = false;
            if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
            {
              bConnected[0] = true;
              if (connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
              {
                long lArg;
                bConnected[1] = true;
                if ((lArg = fcntl(fdSocket, F_GETFL, NULL)) >= 0)
                {
                  lArg |= O_NONBLOCK;
                  fcntl(fdSocket, F_SETFL, lArg);
                }
              }
              else
              {
                close(fdSocket);
              }
            }
          }
          freeaddrinfo(result);
          if (bConnected[1])
          {
            radialConn *ptConn = new radialConn;
            ptConn->fdSocket = fdSocket;
            ptConn->buffer.push_back("");
            ptConn->buffer.push_back("");
            conns.push_back(ptConn);
            connIter = conns.end();
            connIter--;
          }
          else
          {
            ssMessage.str("");
            ssMessage << strPrefix << "->" << ((!bConnected[0])?"socket":"connect") << "(" << errno << ") error:  " << strerror(errno);
            log(ssMessage.str());
          }
        }
        else
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->getaddrinfo(" << nReturn << ") error:  " << gai_strerror(nReturn);
          log(ssMessage.str());
        }
      }
      if (connIter != conns.end())
      {
        label = entries.front();
        strFunction = "log";
        strMessage = ".";
        ptJson = new Json;
        if (label.find("loggerFunction") != label.end())
        {
          if (label["loggerFunction"] == "message")
          {
            strFunction = label["loggerFunction"];
          }
          label.erase("loggerFunction");
        }
        if (label.find("loggerMessage") != label.end())
        {
          if (!label["loggerMessage"].empty())
          {
            strMessage = label["loggerMessage"];
          }
          label.erase("loggerMessage");
        }
        label["radialServer"] = m_strNode;
        ptJson->insert("Application", "Radial");
        ptJson->insert("User", m_strUser);
        ptJson->insert("Password", m_strPassword);
        ptJson->insert("Function", strFunction);
        ptJson->insert("Message", strMessage);
        ptJson->m["Label"] = new Json(label);
        (*connIter)->message.push_back(ptJson->j(strJson));
        delete ptJson;
        label.clear();
        entries.front().clear();
        entries.pop_front();
      }
      else
      {
        bExit = true;
      }
    }
    // }}}
    // {{{ item --> logger
    if (!conns.empty())
    {
      map<int, int> socketList;
      pollfd *fds = new pollfd[conns.size()];
      size_t unIndex = 0;
      for (auto &i : conns)
      {
        fds[unIndex].fd = i->fdSocket;
        fds[unIndex].events = POLLIN;
        if (i->buffer[1].empty() && !i->message.empty())
        {
          i->buffer[1] += i->message.front() + "\n";
          i->message.pop_front();
        }
        if (!i->buffer[1].empty())
        {
          fds[unIndex].events |= POLLOUT;
        }
        socketList[i->fdSocket] = unIndex;
        unIndex++;
      }
      if ((nReturn = poll(fds, conns.size(), 250)) > 0)
      {
        list<list<radialConn *>::iterator> removeList;
        for (auto i = conns.begin(); i != conns.end(); i++)
        {
          if (fds[socketList[(*i)->fdSocket]].revents & POLLIN)
          {
            if (m_pUtility->fdRead((*i)->fdSocket, (*i)->buffer[0], nReturn))
            {
              time(&((*i)->CTime));
              while ((unPosition = (*i)->buffer[0].find("\n")) != string::npos)
              {
                ptJson = new Json((*i)->buffer[0].substr(0, unPosition));
                (*i)->buffer[0].erase(0, (unPosition + 1));
                if (ptJson->m.find("Status") != ptJson->m.end() && ptJson->m["Status"]->v == "error" && ptJson->m.find("Error") != ptJson->m.end() && !ptJson->m["Error"]->v.empty())
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << " error:  " << ptJson->m["Error"]->v;
                  log(ssMessage.str());
                }
                delete ptJson;
              }
            }
            else
            {
              removeList.push_back(i);
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::fdRead(" << errno << ") error:  " << strerror(errno);
                log(ssMessage.str());
              }
            }
          }
          if (fds[socketList[(*i)->fdSocket]].revents & POLLOUT)
          {
            if (m_pUtility->fdWrite((*i)->fdSocket, (*i)->buffer[1], nReturn))
            {
              time(&((*i)->CTime));
            }
            else
            {
              bool bFound = false;
              for (auto j = removeList.begin(); !bFound && j != removeList.end(); j++)
              {
                if (i == (*j))
                {
                  bFound = true;
                }
              }
              if (!bFound)
              {
                removeList.push_back(i);
              }
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::fdWrite(" << errno << ") error:  " << strerror(errno);
                log(ssMessage.str());
              }
            }
          }
          time(&CTime[1]);
          if (((*i)->buffer[1].empty() && (*i)->message.empty() && (CTime[1] - (*i)->CTime) > 2) || (CTime[1] - (*i)->CTime) > 30)
          {
            bool bFound = false;
            for (auto j = removeList.begin(); !bFound && j != removeList.end(); j++)
            {
              if (i == (*j))
              {
                bFound = true;
              }
            }
            if (!bFound)
            {
              removeList.push_back(i);
            }
          }
        }
        for (auto &i : removeList)
        {
          close((*i)->fdSocket);
          delete (*i);
          conns.erase(i);
        }
      }
      else if (nReturn < 0)
      {
        bGood = false;
        ssMessage.str("");
        ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
        log(ssMessage.str());
      }
      delete[] fds;
    }
    else
    {
      for (size_t i = 0; !shutdown() && i < 5; i++)
      {
        msleep(1000);
      }
    }
    // }}}
    time(&CTime[1]);
    if ((CTime[1] - CTime[0]) > 900)
    {
      size_t unProcessing = 0;
      CTime[0] = CTime[1];
      for (auto &i : conns)
      {
        unProcessing += i->message.size();
      }
      ssMessage.str("");
      ssMessage << strPrefix << ":  Currently processing " << unProcessing << " requests with " << entries.size() << " backlogged locally and " << m_messages.size() << " backlogged externally.";
      log(ssMessage.str());
    }
    if (shutdown())
    {
      bGood = false;
    }
  }
  for (auto &i : conns)
  {
    if (i->fdSocket != -1)
    {
      close(i->fdSocket);
    }
  }
  threadDecrement();
  setShutdown();
}
// }}}
}
}
