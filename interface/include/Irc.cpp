// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Irc.cpp
// author     : Ben Kietzman
// begin      : 2022-12-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Irc"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Irc()
Irc::Irc(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "irc", argc, argv, pCallback)
{
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
  m_bEnabled = false;
  m_CMonitorChannelsModify = 0;
}
// }}}
// {{{ ~Irc()
Irc::~Irc()
{
}
// }}}
// {{{ analyze()
void Irc::analyze(const string strNick, const string strTarget, const string strMessage)
{
  if (m_ptMonitor != NULL && strNick != m_strNick)
  {
    stringstream ssText;
    ssText << char(3) << "08,03 " << strNick << " @ " << strTarget << " " << char(3) << " " << strMessage;
    for (auto &i : m_ptMonitor->m)
    {
      if (i.first != strTarget && i.second->m.find("Alerts") != i.second->m.end())
      {
        for (auto &j : i.second->m["Alerts"]->l)
        {
          if (strMessage.size() >= (j->v.size() + 1) && strMessage.substr(0, (j->v.size() + 1)) == (j->v + (string)" "))
          {
            chat(i.first, ssText.str());
          }
        }
      }
    }
  }
}
void Irc::analyze(string strPrefix, const string strTarget, const string strUserID, const string strIdent, stringstream &ssData)
{
  bool bAdmin = false;
  map<string, bool> auth;
  string strError, strFirstName, strLastName;
  stringstream ssQuery;

  strPrefix += "->analyze()";
  ssQuery.str("");
  ssQuery << "select id, admin, first_name, last_name from person where userid = '" << strIdent << "' and active = 1 and locked = 0";
  auto getPerson = dbquery("central_r", ssQuery.str(), strError);
  if (getPerson != NULL)
  {
    if (!getPerson->empty())
    {
      map<string, string> getPersonRow = getPerson->front();
      strFirstName = getPersonRow["first_name"];
      strLastName = getPersonRow["last_name"];
      if (getPersonRow["admin"] == "1")
      {
        bAdmin = true;
      }
      else
      {
        ssQuery.str("");
        ssQuery << "select a.name, b.admin from application a, application_contact b where a.id = b.application_id and b.contact_id = " << getPersonRow["id"] << " and b.locked = 0";
        auto getApplicationContact = dbquery("central_r", ssQuery.str(), strError);
        if (getApplicationContact != NULL)
        {
          if (!getApplicationContact->empty())
          {
            for (auto &getApplicationContactRow : *getApplicationContact)
            {
              auth[getApplicationContactRow["name"]] = ((getApplicationContactRow["admin"] == "1")?true:false);
            }
          }
        }
        m_pCentral->free(getApplicationContact);
      }
    }
  }
  m_pCentral->free(getPerson);
  analyze(strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, ssData);
}
void Irc::analyze(string strPrefix, const string strTarget, const string strUserID, const string strIdent, const string strFirstName, const string strLastName, const bool bAdmin, map<string, bool> &auth, stringstream &ssData)
{
  string strAction;
  Json *ptRequest = new Json;

  strPrefix += "->analyze()";
  ssData >> strAction;
  if (!strAction.empty())
  {
    ptRequest->i("Action", strAction);
    // {{{ storage
    if (strAction == "storage")
    {
      string strJson, strRequest;
      getline(ssData, strRequest);
      m_manip.trim(strJson, strRequest);
      if (!strJson.empty())
      {
        ptRequest->i("Json", strJson);
      }
    }
    // }}}
  }
  analyze(strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, ptRequest);
  delete ptRequest;
}
void Irc::analyze(string strPrefix, const string strTarget, const string strUserID, const string strIdent, const string strFirstName, const string strLastName, const bool bAdmin, map<string, bool> &auth, Json *ptData)
{
  // {{{ prep work
  string strAction = var("Action", ptData), strError;
  stringstream ssText;
  ssText << char(3) << "13,06 " << ((!strAction.empty())?strAction:"actions") << " " << char(3);
  // }}}
  // {{{ storage
  if (strAction == "storage")
  {
    if (isLocalAdmin(strIdent, "Radial", bAdmin, auth))
    {
      string strJson = var("Json", ptData);
      if (!strJson.empty())
      {
        Json *ptJson = new Json(strJson);
        if (hub("storage", ptJson, strError))
        {
          if (ptJson->m.find("Response") != ptJson->m.end() && (!ptJson->m["Response"]->l.empty() || !ptJson->m["Response"]->m.empty() || !ptJson->m["Response"]->v.empty()))
          {
            ssText << ":  " << ptJson->m["Response"];
          }
          else
          {
            ssText << ":  Completed request.";
          }
        }
        else
        {
          ssText << " error:  " << strError;
        }
        delete ptJson;
      }
      else
      {
        ssText << ":  The storage action is used to send a JSON formatted request to common storage within the Bridge.  Please provide a JSON formatted request immediately following the action.";
      }
    }
    else
    {
      ssText << " error:  You are not authorized to access common storage.  You must be registered as a local administrator for the Bridge.";
    }
  }
  // }}}
  // {{{ invalid
  else
  {
    vector<string> actions = {"storage"};
    ssText << ":  Please provide an Action:  ";
    for (size_t i = 0; i < actions.size(); i++)
    {
      ssText << ((i > 0)?", ":"") << actions[i];
    }
    ssText << ".";
    actions.clear();
  }
  // }}}
  // {{{ post work
  chat(strTarget, ssText.str());
  // }}}
}
// }}}
// {{{ autoMode()
void Irc::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Irc::autoMode()";
  ssMessage << strPrefix << " [" << strOldMaster << "," << strNewMaster << "]:  " << ((strNewMaster == m_strNode)?"Set":"Unset") << " master mode.";
  log(ssMessage.str());
  threadDecrement();
}
// }}}
// {{{ bot()
void Irc::bot(string strPrefix)
{
  // {{{ prep work
  string strError;
  stringstream ssMessage;
  SSL_CTX *ctx;
  strPrefix += "->Irc::bot()";
  // }}}
  if ((ctx = m_pUtility->sslInitClient(strError)) != NULL)
  {
    // {{{ prep work
    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
    // }}}
    while (!shutdown())
    {
      // {{{ prep work
      bool bExit = false, bRegistering = false;
      int fdSocket = -1, nReturn;
      pollfd *fds;
      size_t unIndex = 0, unPosition;
      string strBuffer[2], strName = "Radial", strMessage, strNick, strNickBase = "radial_bot", strUser = "radial_bot";
      SSL *ssl = NULL;
      time_t CTime[2] = {0, 0};
      // }}}
      while (!shutdown() && isMasterSettled() && isMaster() && !bExit)
      {
        // {{{ prep work
        if (enabled())
        {
          time(&(CTime[1]));
          if ((CTime[1] - CTime[0]) > 120)
          {
            monitorChannels(strPrefix);
            CTime[0] = CTime[1];
          }
        }
        // {{{ connect
        if (fdSocket == -1)
        {
          addrinfo hints, *result;
          memset(&hints, 0, sizeof(addrinfo));
          hints.ai_family = AF_UNSPEC;
          hints.ai_socktype = SOCK_STREAM;
          if ((nReturn = getaddrinfo(m_strServer.c_str(), m_strPort.c_str(), &hints, &result)) == 0)
          {
            bool bConnected[3] = {false, false, false};
            for (addrinfo *rp = result; !bConnected[2] && rp != NULL; rp = rp->ai_next)
            {
              bConnected[0] = bConnected[1] = false;
              if ((fdSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) >= 0)
              {
                bConnected[0] = true;
                if (connect(fdSocket, rp->ai_addr, rp->ai_addrlen) == 0)
                {
                  bConnected[1] = true;
                  if ((ssl = m_pUtility->sslConnect(ctx, fdSocket, strError)) != NULL)
                  {
                    bConnected[2] = true;
                  }
                  else
                  {
                    close(fdSocket);
                    fdSocket = -1;
                  }
                }
                else
                {
                  close(fdSocket);
                  fdSocket = -1;
                }
              }
            }
            freeaddrinfo(result);
            if (bConnected[2])
            {
              ssMessage.str("");
              ssMessage << strPrefix << "->Utility::sslConnect() [" << m_strServer << ":" << m_strPort << "]:  Connected to IRC server.";
              log(ssMessage.str());
            }
            else if (!bConnected[1])
            {
              ssMessage.str("");
              ssMessage << strPrefix << "->" << ((!bConnected[0])?"socket":"connect") << "(" << errno << ") error:  " << strerror(errno);
              log(ssMessage.str());
            }
            else if (!bConnected[2])
            {
              ssMessage.str("");
              ssMessage << strPrefix << "->Utility::sslConnect() error:  " << strError;
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
        // }}}
        fds = new pollfd[1];
        fds[0].fd = fdSocket;
        fds[0].events = POLLIN;
        if (fdSocket != -1 && !enabled() && !bRegistering)
        {
          stringstream ssBuffer;
          bRegistering = true;
          strNick = strNickBase;
          if (unIndex > 0)
          {
            stringstream ssIndex;
            ssIndex << unIndex;
            strNick += ssIndex.str();
          }
          ssBuffer << "NICK " << strNick << "\r\n";
          ssBuffer << "USER " << strUser << " 0 * :" << strName << "\r\n";
          strBuffer[1] = ssBuffer.str();
        }
        while (message(strMessage))
        {
          strBuffer[1].append(strMessage);
        }
        if (!strBuffer[1].empty())
        {
          fds[0].events |= POLLOUT;
        }
        // }}}
        if ((nReturn = poll(fds, 1, 250)) > 0)
        {
          // {{{ read
          if (fds[0].revents & POLLIN)
          {
            if (m_pUtility->sslRead(ssl, strBuffer[0], nReturn))
            {
              // {{{ prep work
              while ((unPosition = strBuffer[0].find("\r")) != string::npos)
              {
                strBuffer[0].erase(unPosition, 1);
              }
              // }}}
              while ((unPosition = strBuffer[0].find("\n")) != string::npos)
              {
                // {{{ prep work
                strMessage = strBuffer[0].substr(0, unPosition);
                strBuffer[0].erase(0, (unPosition + 1));
                // }}}
                // {{{ PING
                if (strMessage.size() >= 4 && strMessage.substr(0, 4) == "PING")
                {
                  string strPong = strMessage;
                  strPong[1] = 'O';
                  strBuffer[1].append(strPong + "\r\n");
                }
                // }}}
                else
                {
                  // {{{ prep work
                  string strCommand, strID, strIdent, strLine;
                  stringstream ssCommand, ssSubMessage(strMessage);
                  ssSubMessage >> strID >> strCommand;
                  if (!strID.empty() && strID[0] == ':')
                  {
                    strID.erase(0, 1);
                  }
                  if ((unPosition = strID.find("@")) != string::npos)
                  {
                    strID.erase(unPosition, (strID.size() - unPosition));
                  }
                  if ((unPosition = strID.find("!")) != string::npos && (unPosition + 1) < strID.size())
                  {
                    strIdent = strID.substr((unPosition + 1), strID.size() - (unPosition + 1));
                    strID.erase(unPosition, (strID.size() - unPosition));
                  }
                  else
                  {
                    strIdent = strID;
                  }
                  // }}}
                  if (!strCommand.empty())
                  {
                    // {{{ numeric
                    if (m_manip.isNumeric(strCommand))
                    {
                      size_t unCommand;
                      ssCommand.str(strCommand);
                      ssCommand >> unCommand;
                      switch (unCommand)
                      {
                        case 1:
                        {
                          stringstream ssBuffer;
                          bRegistering = false;
                          strNick = strNickBase;
                          if (unIndex > 0)
                          {
                            stringstream ssIndex;
                            ssIndex << unIndex;
                            strNick += ssIndex.str();
                          }
                          enable(strNick);
                          ssMessage.str("");
                          ssMessage << strPrefix << " [" << m_strServer << ":" << m_strPort << "," << strNick << "]:  Registered on IRC server.";
                          log(ssMessage.str());
                          break;
                        }
                        case 433:
                        case 436:
                        {
                          bRegistering = false;
                          unIndex++;
                          break;
                        }
                      }
                    }
                    // }}}
                    // {{{ PRIVMSG
                    else if (strCommand == "PRIVMSG")
                    {
                      bool bChannel = false;
                      string strMessage, strTarget;
                      stringstream ssBuffer;
                      ssSubMessage >> strTarget;
                      getline(ssSubMessage, strMessage);
                      if (strMessage.substr(0, 2) == " :")
                      {
                        strMessage.erase(0, 2);
                      }
                      if (!strTarget.empty() && strTarget[0] == '#')
                      {
                        bChannel = true;
                        analyze(strID, strTarget, strMessage);
                      }
                      // {{{ !r || !radial
                      if (!bChannel || strMessage == "!r" || (strMessage.size() > 3 && strMessage.substr(0, 3) == "!r ") || strMessage == "!radial" || (strMessage.size() > 8 && strMessage.substr(0, 8) == "!radial "))
                      {
                        string strChannel;
                        stringstream ssData(strMessage), ssPrefix;
                        if (strMessage == "!r" || (strMessage.size() > 3 && strMessage.substr(0, 3) == "!r ") || strMessage == "!radial" || (strMessage.size() > 8 && strMessage.substr(0, 8) == "!radial "))
                        {
                          string strValue;
                          ssData >> strValue;
                        }
                        analyze(strPrefix, ((bChannel)?strTarget:strID), strID, strIdent, ssData);
                      }
                      // }}}
                    }
                    // }}}
                  }
                }
              }
            }
            else
            {
              bExit = true;
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslRead(" << SSL_get_error(ssl, nReturn) << ") error:  " << m_pUtility->sslstrerror();
                log(ssMessage.str());
              }
            }
          }
          // }}}
          // {{{ write
          if (fds[0].revents & POLLOUT)
          {
            if (!m_pUtility->sslWrite(ssl, strBuffer[1], nReturn))
            {
              bExit = true;
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslWrite(" << SSL_get_error(ssl, nReturn) << ") error:  " << m_pUtility->sslstrerror();
                log(ssMessage.str());
              }
            }
          }
          // }}}
        }
        else if (nReturn < 0)
        {
          bExit = true;
          ssMessage.str("");
          ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
          log(ssMessage.str());
        }
        // {{{ post work
        delete[] fds;
        // }}}
      }
      // {{{ post work
      if (ssl != NULL)
      {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = NULL;
      }
      if (fdSocket != -1)
      {
        close(fdSocket);
        fdSocket = -1;
        disable();
        ssMessage.str("");
        ssMessage << strPrefix << " [" << m_strServer << ":" << m_strPort << "," << strNick << "]:  Exited IRC server.";
        log(ssMessage.str());
      }
      if (!shutdown())
      {
        for (size_t i = 0; !shutdown() && i < 20; i++)
        {
          msleep(250);
        }
      }
      // }}}
    }
    // {{{ post work
    SSL_CTX_free(ctx);
    // }}}
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->Utility::sslInitClient() error:  " << strError;
    notify(ssMessage.str());
  }
  // {{{ post work
  m_pUtility->sslDeinit();
  // }}}
}
// }}}
// {{{ callback()
void Irc::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;

  threadIncrement();
  strPrefix += "->Irc::callback()";
  if (ptJson->m.find("Function") != ptJson->m.end() && !ptJson->m["Function"]->v.empty())
  {
    if (ptJson->m["Function"]->v == "chat")
    {
      if (ptJson->m.find("Message") != ptJson->m.end() && !ptJson->m["Message"]->v.empty())
      {
        if (ptJson->m.find("Target") != ptJson->m.end() && !ptJson->m["Target"]->v.empty())
        {
          size_t unCount = 0;
          while (unCount++ < 40 && !isMasterSettled())
          {
            msleep(250);
          }
          if (isMasterSettled())
          {
            if (isMaster())
            {
              unCount = 0;
              while (unCount++ < 40 && !enabled())
              {
                msleep(250);
              }
              if (enabled())
              {
                if (chat(ptJson->m["Target"]->v, ptJson->m["Message"]->v, strError))
                {
                  bResult = true;
                }
              }
              else
              {
                strError = "IRC disabled.";
              }
            }
            else
            {
              Json *ptLink = new Json(ptJson);
              ptLink->i("Node", master());
              if (hub("link", ptLink, strError))
              {
                bResult = true;
              }
              delete ptLink;
            }
          }
          else
          {
            strError = "Master not known.";
          }
        }
        else
        {
          strError = "Please provide the Target.";
        }
      }
      else
      {
        strError = "Please provide the Message.";
      }
    }
    else
    {
      strError = "Please provide a valid Function:  chat.";
    }
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
    hub(ptJson, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
// {{{ chat()
void Irc::chat(const string strTarget, const string strMessage)
{
  size_t unMaxLength = 450;
  string strLine;
  stringstream ssLines(strMessage), ssMessage, ssPrefix;

  ssPrefix << ":" << m_strNick << " PRIVMSG " << strTarget << " :";
  while (getline(ssLines, strLine))
  {
    while ((ssPrefix.str().size() + strLine.size() + 2) > unMaxLength)
    {
      ssMessage.str("");
      ssMessage << ssPrefix.str() << strLine.substr(0, (unMaxLength - (ssPrefix.str().size() + 2))) << "\r\n";
      push(ssMessage.str());
      strLine.erase(0, (unMaxLength - (ssPrefix.str().size() + 2)));
    }
    if (!strLine.empty())
    {
      ssMessage.str("");
      ssMessage << ssPrefix.str() << strLine << "\r\n";
      push(ssMessage.str());
    }
  }
}
// }}}
// {{{ disable()
void Irc::disable()
{
  if (enabled())
  {
    if (m_ptMonitor != NULL)
    {
      for (auto &i : m_ptMonitor->m)
      {
        part(i.first);
      }
    }
    m_strNick.clear();
    m_bEnabled = false;
  }
}
// }}}
// {{{ enable()
void Irc::enable(const string strNick)
{
  if (!enabled())
  {
    m_bEnabled = true;
    if (!m_messages.empty())
    {
      m_messages.clear();
    }
    m_strNick = strNick;
    if (m_ptMonitor != NULL)
    {
      for (auto &i : m_ptMonitor->m)
      {
        join(i.first);
      }
    }
  }
}
// }}}
// {{{ enabled()
bool Irc::enabled()
{
  return m_bEnabled;
}
// }}}
// {{{ isLocalAdmin()
bool Irc::isLocalAdmin(const string strUserID, string strApplication, const bool bAdmin, map<string, bool> auth)
{
  return (bAdmin || (!strUserID.empty() && auth.find(strApplication) != auth.end() && auth[strApplication]));
}
// }}}
// {{{ join()
void Irc::join(const string strChannel)
{
  stringstream ssMessage;

  ssMessage << ":" << m_strNick << " JOIN :" << strChannel << "\r\n";
  push(ssMessage.str());
}
// }}}
// {{{ lock()
void Irc::lock()
{
  m_mutex.lock();
}
// }}}
// {{{ message()
bool Irc::message(string &strMessage)
{
  bool bResult = false;

  lock();
  if (!m_messages.empty())
  {
    bResult = true;
    strMessage = m_messages.front();
    m_messages.pop_front();
  }
  unlock();

  return bResult;
}
// }}}
// {{{ monitor()
Json *Irc::monitor()
{
  Json *ptMonitor = NULL;

  if (m_ptMonitor != NULL)
  {
    ptMonitor = new Json(m_ptMonitor);
  }

  return ptMonitor;
}
// }}}
// {{{ monitorChannels()
void Irc::monitorChannels(string strPrefix)
{
  struct stat tStat;
  stringstream ssFile, ssMessage;

  strPrefix += "->Irc::monitorChannels()";
  ssFile << m_strData << "/irc/monitor.channels";
  if (stat(ssFile.str().c_str(), &tStat) == 0)
  {
    if (tStat.st_mtime > m_CMonitorChannelsModify)
    {
      ifstream inMonitor;
      inMonitor.open(ssFile.str().c_str());
      if (inMonitor)
      {
        string strLine;
        stringstream ssJson;
        m_CMonitorChannelsModify = tStat.st_mtime;
        while (getline(inMonitor, strLine))
        {
          ssJson << strLine;
        }
        if (!ssJson.str().empty())
        {
          if (m_ptMonitor != NULL)
          {
            if (enabled())
            {
              for (auto &i : m_ptMonitor->m)
              {
                part(i.first);
              }
            }
            delete m_ptMonitor;
          }
          m_ptMonitor = new Json(ssJson.str());
          if (!m_ptMonitor->m.empty())
          {
            if (enabled())
            {
              for (auto &i : m_ptMonitor->m)
              {
                join(i.first);
              }
            }
          }
          else
          {
            ssMessage.str("");
            ssMessage << strPrefix << " error [" << ssFile.str() << "]:  JSON is empty.";
            log(ssMessage.str());
          }
        }
        else
        {
          ssMessage.str("");
          ssMessage << strPrefix << " error [" << ssFile.str() << "]:  File is empty.";
          log(ssMessage.str());
        }
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->ifstream::open(" << errno << ") error [" << ssFile.str() << "]:  " << strerror(errno);
        log(ssMessage.str());
      }
      inMonitor.close();
    }
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->stat(" << errno << ") error [" << ssFile.str() << "]:  " << strerror(errno);
    log(ssMessage.str());
  }
}
// }}}
// {{{ part()
void Irc::part(const string strChannel)
{
  stringstream ssMessage;

  ssMessage << ":" << m_strNick << " PART :" << strChannel << "\r\n";
  push(ssMessage.str());
}
// }}}
// {{{ push()
void Irc::push(const string strMessage)
{
  lock();
  m_messages.push_back(strMessage);
  unlock();
}
// }}}
// {{{ unlock()
void Irc::unlock()
{
  m_mutex.unlock();
}
// }}}
// {{{ var()
string Irc::var(const string strName, Json *ptData)
{
  string strValue;

  if (ptData->m.find(strName) != ptData->m.end() && !ptData->m[strName]->v.empty())
  {
    strValue = ptData->m[strName]->v;
  }

  return strValue;
}
// }}}
}
}
