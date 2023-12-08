// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Interface.cpp
// author     : Ben Kietzman
// begin      : 2022-04-19
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Interface"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Interface()
Interface::Interface(string strPrefix, const string strName, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Base(argc, argv)
{
  string strError;

  strPrefix += "->Interface::Interface()";
  signal(SIGBUS, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGCONT, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGSEGV, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  signal(SIGWINCH, SIG_IGN);
  m_bCallbackPool = false;
  m_bMaster = false;
  m_bMasterSettled = false;
  m_bResponse = false;
  m_fdCallbackPool[0] = -1;
  m_fdCallbackPool[1] = -1;
  m_fdResponse[0] = -1;
  m_fdResponse[1] = -1;
  m_pAutoModeCallback = NULL;
  m_pCallback = pCallback;
  m_pJunction->setProgram(strName);
  m_strName = strName;
  m_pThreadCallbackPool = NULL;
  if (m_pWarden != NULL)
  {
    Json *ptAes = new Json, *ptJwt = new Json;
    if (m_pWarden->vaultRetrieve({"aes"}, ptAes, strError) && !empty(ptAes, "Secret"))
    {
      m_strAesSecret = ptAes->m["Secret"]->v;
    }
    delete ptAes;
    if (m_pWarden->vaultRetrieve({"jwt"}, ptJwt, strError) && !empty(ptJwt, "Secret") && !empty(ptJwt, "Signer"))
    {
      m_strJwtSecret = ptJwt->m["Secret"]->v;
      m_strJwtSigner = ptJwt->m["Signer"]->v;
    }
    delete ptJwt;
  }
}
// }}}
// {{{ ~Interface()
Interface::~Interface()
{
  if (m_pThreadCallbackPool)
  {
    m_pThreadCallbackPool->join();
    delete m_pThreadCallbackPool;
    m_pThreadCallbackPool = NULL;
  }
}
// }}}
// {{{ alert()
void Interface::alert(const string strMessage)
{
  log("alert", strMessage);
}
bool Interface::alert(const string strUser, const string strMessage, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ptJson->m["Request"] = new Json;
  ptJson->m["Request"]->i("User", strUser);
  ptJson->m["Request"]->i("Message", strMessage);
  if (hub("alert", ptJson, strError))
  {
    bResult = true;
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ auth()
bool Interface::auth(Json *ptJson, string &strError)
{
  bool bResult = false;
  string strTarget = "auth";
  Json *ptAuth = new Json(ptJson);

  keyRemovals(ptAuth);
  if (!empty(ptAuth, "Interface"))
  {
    if (exist(ptAuth, "Request"))
    {
      delete ptAuth->m["Request"];
    }
    ptAuth->m["Request"] = new Json;
    ptAuth->m["Request"]->i("Interface", ptAuth->m["Interface"]->v);
    ptAuth->i("Interface", "auth");
  }
  if (!empty(ptJson, "Node"))
  {
    strTarget = "link";
  }
  if (hub(strTarget, ptAuth, strError))
  {
    bResult = true;
  }
  delete ptAuth;

  return bResult;
}
// }}}
// {{{ callback
// {{{ callbackPool()
void Interface::callbackPool()
{
  char cChar;
  int nReturn;
  list<radialCallbackWorker *>::iterator workerIter;
  list<radialCallbackWorker *> workers;
  string strError, strPrefix = "Interface::callbackPool()";
  stringstream ssMessage;
  time_t CTime;
  radialCallback *ptCallback;

  threadIncrement();
  while (!shutdown())
  {
    pollfd fds[1];
    fds[0].fd = m_fdCallbackPool[0];
    fds[0].events = POLLIN;
    if ((nReturn = poll(fds, 1, 2000)) > 0)
    {
      if (fds[0].revents & (POLLHUP | POLLIN))
      {
        if (read(fds[0].fd, &cChar, 1) > 0)
        {
          m_mutexShare.lock();
          m_bCallbackPool = false;
          m_mutexShare.unlock();
        }
        else
        {
          ssMessage.str("");
          ssMessage << strPrefix << "->read(" << errno << "):  " << strerror(errno);
          notify(ssMessage.str());
          setShutdown();
        }
      }
    }
    else if (nReturn < 0 && errno != EINTR)
    {
      ssMessage.str("");
      ssMessage << strPrefix << "->poll(" << errno << "):  " << strerror(errno);
      notify(ssMessage.str());
      setShutdown();
    }
    m_mutexShare.lock();
    while (!m_callbacks.empty())
    {
      ptCallback = m_callbacks.front();
      m_callbacks.pop();
      workerIter = workers.end();
      for (auto i = workers.begin(); i != workers.end(); i++)
      {
        if ((*i)->fdWorker[1] != -1 && (workerIter == workers.end() || (*i)->callbacks.size() < (*workerIter)->callbacks.size()))
        {
          workerIter = i;
        }
      }
      if (workerIter != workers.end() && workers.size() >= m_unWorkers)
      {
        (*workerIter)->callbacks.push(ptCallback);
        if (!(*workerIter)->bWorker && (*workerIter)->fdWorker[1] != -1)
        {
          char cChar = '\n';
          (*workerIter)->mutexWorker.lock();
          (*workerIter)->bWorker = true;
          (*workerIter)->mutexWorker.unlock();
          write((*workerIter)->fdWorker[1], &cChar, 1);
        }
      }
      else
      {
        radialCallbackWorker *ptWorker = new radialCallbackWorker;
        ptWorker->bWorker = false;
        time(&(ptWorker->CTime));
        ptWorker->callbacks.push(ptCallback);
        if (pipe(ptWorker->fdWorker) == 0)
        {
          char cChar = '\n';
          m_pUtility->fdNonBlocking(ptWorker->fdWorker[0], strError);
          m_pUtility->fdNonBlocking(ptWorker->fdWorker[1], strError);
          ptWorker->bWorker = true;
          write(ptWorker->fdWorker[1], &cChar, 1);
        }
        workers.push_back(ptWorker);
        thread threadWorker(&Interface::callbackWorker, this, ptWorker);
        pthread_setname_np(threadWorker.native_handle(), "callbackWorker");
        threadWorker.detach();
      }
    }
    if (!workers.empty())
    {
      time(&CTime);
      workerIter = workers.begin();
      while (workerIter != workers.end())
      {
        workerIter = workers.end();
        for (auto i = workers.begin(); workerIter == workers.end() && i != workers.end(); i++)
        {
          if ((*i)->callbacks.empty() && ((*i)->fdWorker[0] == -1 || (*i)->fdWorker[1] == -1 || (CTime - (*i)->CTime) > 10))
          {
            workerIter = i;
          }
        }
        if (workerIter != workers.end())
        {
          if ((*workerIter)->fdWorker[1] != -1)
          {
            close((*workerIter)->fdWorker[1]);
            (*workerIter)->fdWorker[1] = -1;
          }
          workers.erase(workerIter);
        }
      }
    }
    m_mutexShare.unlock();
  }
  for (auto &worker : workers)
  {
    if (worker->fdWorker[1] != -1)
    {
      close(worker->fdWorker[1]);
      worker->fdWorker[1] = -1;
    }
  }
  close(m_fdCallbackPool[0]);
  m_fdCallbackPool[0] = -1;
  close(m_fdCallbackPool[1]);
  m_fdCallbackPool[1] = -1;
  threadDecrement();
}
// }}}
// {{{ callbackPush()
void Interface::callbackPush(string strPrefix, const string strPacket, const bool bResponse)
{
  radialCallback *ptCallback = new radialCallback;

  ptCallback->strPrefix = strPrefix;
  ptCallback->strPacket = strPacket;
  ptCallback->bResponse = bResponse;
  m_mutexShare.lock();
  m_callbacks.push(ptCallback);
  if (!m_bCallbackPool && m_fdCallbackPool[1] != -1)
  {
    char cChar = '\n';
    m_bCallbackPool = true;
    write(m_fdCallbackPool[1], &cChar, 1);
  }
  m_mutexShare.unlock();
}
// }}}
// {{{ callbackWorker()
void Interface::callbackWorker(radialCallbackWorker *ptWorker)
{
  bool bClose = false;
  char cChar;
  int nReturn;
  radialCallback *ptCallback;

  threadIncrement();
  while (ptWorker->fdWorker[1] != -1 || !ptWorker->callbacks.empty())
  {
    pollfd fds[1];
    fds[0].fd = ptWorker->fdWorker[0];
    fds[0].events = POLLIN;
    if ((nReturn = poll(fds, 1, 2000)) > 0)
    {
      if (fds[0].revents & (POLLHUP | POLLIN))
      {
        if (read(fds[0].fd, &cChar, 1) > 0)
        {
          ptWorker->mutexWorker.lock();
          ptWorker->bWorker = false;
          ptWorker->mutexWorker.unlock();
        }
        else
        {
          bClose = true;
        }
      }
      else if (fds[0].revents & POLLNVAL)
      {
        bClose = true;
      }
    }
    else if (nReturn < 0 && errno != EINTR)
    {
      bClose = true;
    }
    if (bClose && ptWorker->fdWorker[0] != -1)
    {
      close(ptWorker->fdWorker[0]);
      ptWorker->fdWorker[0] = -1;
    }
    while (!ptWorker->callbacks.empty())
    {
      ptCallback = ptWorker->callbacks.front();
      ptWorker->callbacks.pop();
      time(&(ptWorker->CTime));
      if (m_pCallback != NULL)
      {
        m_pCallback(ptCallback->strPrefix, ptCallback->strPacket, ptCallback->bResponse);
      }
      delete ptCallback;
    }
  }
  delete ptWorker;
  threadDecrement();
}
// }}}
// }}}
// {{{ centralmon()
bool Interface::centralmon(const string strServer, const string strProcess, Json *ptData, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ptJson->i("Function", ((!strProcess.empty())?"process":"system"));
  ptJson->i("Server", strServer);
  if (!strProcess.empty())
  {
    ptJson->i("Process", strProcess);
  }
  if (hub("centralmon", ptJson, strError))
  {
    bResult = true;
    if (exist(ptJson, "Response"))
    {
      ptData->merge(ptJson->m["Response"], true, false);
    }
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ chat()
bool Interface::chat(const string strTarget, const string strMessage)
{
  string strError;

  return chat(strTarget, strMessage, strError);
}
bool Interface::chat(const string strTarget, const string strMessage, string &strError)
{
  bool bResult = false;
  stringstream ssMessage;
  Json *ptJson = new Json;

  ssMessage << char(3) << "11,10 " << m_strNode << " " << char(3) << " " << char(3) << "07,05 " << m_strName << " " << char(3) << " " << strMessage;
  ptJson->i("Function", "chat");
  ptJson->i("Target", strTarget);
  ptJson->i("Message", ssMessage.str());
  if (hub("irc", ptJson, strError))
  {
    bResult = true;
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ command()
bool Interface::command(const string strCommand, list<string> arguments, const string strInput, string &strOutput, size_t &unDuration, string &strError, const time_t CTimeout, const string strNode)
{
  bool bRemote = false, bResult = false;
  Json *ptJson = new Json;

  if (!strNode.empty() && strNode != m_strNode)
  {
    bRemote = true;
    ptJson->i("Interface", "command");
    ptJson->i("Node", strNode);
  }
  ptJson->i("Command", strCommand);
  if (!arguments.empty())
  {
    ptJson->i("Arguments", arguments);
  }
  if (!strInput.empty())
  {
    ptJson->i("Input", strInput);
  }
  if (CTimeout > 0)
  {
    stringstream ssTimeout;
    ssTimeout << CTimeout;
    ptJson->i("Timeout", ssTimeout.str(), 'n');
  }
  unDuration = 0;
  if ((!bRemote && hub("command", ptJson, strError)) || (bRemote && hub("link", ptJson, strError)))
  {
    bResult = true;
    if (!empty(ptJson, "Duration"))
    {
      stringstream ssDuration(ptJson->m["Duration"]->v);
      ssDuration >> unDuration;
    }
    if (!empty(ptJson, "Output"))
    {
      strOutput = ptJson->m["Output"]->v;
    }
  }
  delete ptJson;

  return bResult;
}
bool Interface::command(const string strCommand, list<string> arguments, Json *ptInput, Json *ptOutput, size_t &unDuration, string &strError, const time_t CTimeout, const string strNode)
{
  bool bRemote = false, bResult = false;
  Json *ptJson = new Json;

  if (!strNode.empty() && strNode != m_strNode)
  {
    bRemote = true;
    ptJson->i("Interface", "command");
    ptJson->i("Node", strNode);
  }
  ptJson->i("Command", strCommand);
  if (!arguments.empty())
  {
    ptJson->i("Arguments", arguments);
  }
  ptJson->i("Format", "json");
  if (ptInput != NULL)
  {
    ptJson->i("Input", ptInput);
  }
  if (CTimeout > 0)
  {
    stringstream ssTimeout;
    ssTimeout << CTimeout;
    ptJson->i("Timeout", ssTimeout.str(), 'n');
  }
  unDuration = 0;
  if ((!bRemote && hub("command", ptJson, strError)) || (bRemote && hub("link", ptJson, strError)))
  {
    bResult = true;
    if (!empty(ptJson, "Duration"))
    {
      stringstream ssDuration(ptJson->m["Duration"]->v);
      ssDuration >> unDuration;
    }
    if (ptOutput != NULL && exist(ptJson, "Output"))
    {
      ptOutput->merge(ptJson->m["Output"], true, false);
    }
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ cron
// {{{ cron()
bool Interface::cron(time_t &CTime, string strValue, string &strError)
{
  bool bResult = false;
  list<string> cron;
  string strItem;
  stringstream ssCron(strValue);

  while (getline(ssCron, strItem, ' '))
  {
    m_manip.trim(strItem, strItem);
    if (!strItem.empty())
    {
      cron.push_back(strItem);
    }
  }
  if (cron.size() == 5)
  {
    map<size_t, list<int> > value;
    size_t unIndex = 0;
    bResult = true;
    for (auto i = cron.begin(); bResult && i != cron.end(); i++)
    {
      if (!cronParse(unIndex, (*i), value[unIndex], strError))
      {
        bResult = false;
      }
      unIndex++;
    }
    if (bResult)
    {
      bool bFound = false;
      struct tm tTime;
      time_t CMaxTime;
      time(&CTime);
      CTime += 60 - (CTime % 60);
      CMaxTime = CTime + 31536000;
      for (time_t i = CTime; !bFound && i < CMaxTime; i += 60)
      {
        localtime_r(&i, &tTime);
        for (auto min = value[0].begin(); !bFound && min != value[0].end(); min++)
        {
          if (tTime.tm_min == (*min))
          {
            for (auto hour = value[1].begin(); !bFound && hour != value[1].end(); hour++)
            {
              if (tTime.tm_hour == (*hour))
              {
                for (auto dom = value[2].begin(); !bFound && dom != value[2].end(); dom++)
                {
                  if (tTime.tm_mday == (*dom))
                  {
                    for (auto month = value[3].begin(); !bFound && month != value[3].end(); month++)
                    {
                      if ((tTime.tm_mon + 1) == (*month))
                      {
                        for (auto dow = value[4].begin(); !bFound && dow != value[4].end(); dow++)
                        {
                          if (tTime.tm_wday == (*dow))
                          {
                            bFound = true;
                            CTime = i;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      if (!bFound)
      {
        bResult = false;
        strError = "Failed to schedule cron within a one year span.";
      }
    }
  }
  else
  {
    strError = "Please provide a value in the same time format used by cron:  minute hour dom month dow.";
  }

  return bResult;
}
// }}}
// {{{ cronParse()
bool Interface::cronParse(const size_t unType, const string strValue, list<int> &value, string &strError)
{
  bool bResult = false;

  value.clear();
  if (unType <= 4)
  {
    if (!strValue.empty())
    {
      if (cronParseComma(unType, strValue, value, strError))
      {
        bResult = true;
      }
    }
    else
    {
      strError = "Please provide the value.";
    }
  }
  else
  {
    strError = "Please provide a valid type:  0 (minute), 1 (hour), 2 (dom), 3 (month), 4 (dow).";
  }

  return bResult;
}
// }}}
// {{{ cronParseComma()
bool Interface::cronParseComma(const size_t unType, const string strValue, list<int> &value, string &strError)
{
  bool bResult = false;

  if (!strValue.empty())
  {
    string strItem;
    stringstream ssValue(strValue);
    bResult = true;
    while (bResult && getline(ssValue, strItem, ','))
    {
      m_manip.trim(strItem, strItem);
      if (!strItem.empty())
      {
        if (!cronParseHyphen(unType, strItem, value, strError))
        {
          bResult = false;
        }
      }
      else
      {
        bResult = false;
        strError = "Please provide the value.";
      }
    }
  }
  else
  {
    strError = "Please provide the value.";
  }

  return bResult;
}
// }}}
// {{{ cronParseDow()
bool Interface::cronParseDow(string &strValue, string &strError)
{
  bool bResult = true;

  if (!strValue.empty())
  {
    if (!m_manip.isNumeric(strValue))
    {
      string strLower;
      m_manip.toLower(strLower, strValue);
      if (strLower == "sun" || strLower == "sunday")
      {
        strValue = "0";
      }
      else if (strLower == "mon" || strLower == "monday")
      {
        strValue = "1";
      }
      else if (strLower == "tue" || strLower == "tuesday")
      {
        strValue = "2";
      }
      else if (strLower == "wed" || strLower == "wednesday")
      {
        strValue = "3";
      }
      else if (strLower == "thu" || strLower == "thursday")
      {
        strValue = "4";
      }
      else if (strLower == "fri" || strLower == "friday")
      {
        strValue = "5";
      }
      else if (strLower == "sat" || strLower == "saturday")
      {
        strValue = "6";
      }
      else
      {
        bResult = false;
        strError = "Please provide a valid day of the week.";
      }
    }
    if (strValue == "7")
    {
      strValue = "0";
    }
  }
  else
  {
    bResult = false;
    strError = "Please provide a day of the week.";
  }

  return bResult;
}
// }}}
// {{{ cronParseHyphen()
bool Interface::cronParseHyphen(const size_t unType, string strValue, list<int> &value, string &strError)
{
  bool bResult = false;

  if (!strValue.empty())
  {
    size_t unPosition;
    string strItem;
    if ((unPosition = strValue.find("-")) != string::npos)
    {
      strItem = strValue.substr((unPosition + 1), (strValue.size() - (unPosition + 1)));
      strValue.erase(unPosition);
    }
    m_manip.trim(strValue, strValue);
    m_manip.trim(strItem, strItem);
    if (!strValue.empty())
    {
      int nValue, nItem = -1;
      if (strValue == "*")
      {
        if (strItem.empty())
        {
          strItem = "*";
        }
        strValue = ((unType == 0 || unType == 1 || unType == 4)?"0":"1");
      }
      if (strItem == "*")
      {
        switch (unType)
        {
          case 0: strItem = "59"; break;
          case 1: strItem = "23"; break;
          case 2: strItem = "31"; break;
          case 3: strItem = "12"; break;
          case 4: strItem = "6"; break;
        }
      }
      if (cronParseValue(unType, strValue, nValue, strError) && (strItem.empty() || cronParseValue(unType, strItem, nItem, strError)))
      {
        if (nItem == -1 || nValue <= nItem)
        {
          bResult = true;
          if (nItem == -1)
          {
            nItem = nValue;
          }
          for (int i = nValue; i <= nItem; i++)
          {
            value.push_back(i);
          }
        }
      }
    }
    else
    {
      strError = "Please provide a value.";
    }
  }
  else
  {
    strError = "Please provide a value.";
  }

  return bResult;
}
// }}}
// {{{ cronParseMonth()
bool Interface::cronParseMonth(string &strValue, string &strError)
{
  bool bResult = true;

  if (!strValue.empty())
  {
    if (!m_manip.isNumeric(strValue))
    {
      string strLower;
      m_manip.toLower(strLower, strValue);
      if (strLower == "jan" || strLower == "january")
      {
        strValue = "1";
      }
      else if (strLower == "feb" || strLower == "february")
      {
        strValue = "2";
      }
      else if (strLower == "mar" || strLower == "march")
      {
        strValue = "3";
      }
      else if (strLower == "apr" || strLower == "april")
      {
        strValue = "4";
      }
      else if (strLower == "may")
      {
        strValue = "5";
      }
      else if (strLower == "jun" || strLower == "june")
      {
        strValue = "6";
      }
      else if (strLower == "jul" || strLower == "july")
      {
        strValue = "7";
      }
      else if (strLower == "aug" || strLower == "august")
      {
        strValue = "8";
      }
      else if (strLower == "sep" || strLower == "september")
      {
        strValue = "9";
      }
      else if (strLower == "oct" || strLower == "october")
      {
        strValue = "10";
      }
      else if (strLower == "nov" || strLower == "november")
      {
        strValue = "11";
      }
      else if (strLower == "dec" || strLower == "december")
      {
        strValue = "12";
      }
      else
      {
        bResult = false;
        strError = "Please provide a valid month.";
      }
    }
  }
  else
  {
    bResult = false;
    strError = "Please provide a day of the week.";
  }

  return bResult;
}
// }}}
// {{{ cronParseValue()
bool Interface::cronParseValue(const size_t unType, string strValue, int &nValue, string &strError)
{
  bool bResult = false;

  if (!strValue.empty())
  {
    if ((unType != 3 || cronParseMonth(strValue, strError)) && (unType != 4 || cronParseDow(strValue, strError)))
    {
      if (m_manip.isNumeric(strValue))
      {
        int nItem;
        stringstream ssItem(strValue);
        ssItem >> nItem;
        if ((unType == 0 && nItem >= 0 && nItem <= 59) || (unType == 1 && nItem >= 0 && nItem <= 23) || (unType == 2 && nItem >= 1 && nItem <= 31) || (unType == 3 && nItem >= 1 && nItem <= 12) || (unType == 4 && nItem >= 0 && nItem <= 6))
        {
          bResult = true;
          nValue = nItem;
        }
        else
        {
          strError = "Please provide a valid numeric value.";
        }
      }
      else
      {
        strError = "Please provide a numeric value.";
      }
    }
  }
  else
  {
    strError = "Please provide a value.";
  }

  return bResult;
}
// }}}
// }}}
// {{{ curl()
bool Interface::curl(const string strURL, const string strType, Json *ptAuth, Json *ptGet, Json *ptPost, Json *ptPut, const string strProxy, string &strCookies, string &strHeader, string &strContent, string &strError, const string strUserAgent, const bool bMobile, const bool bFailOnError, const string strCustomRequest)
{
  bool bResult = false;
  list<Json *> in, out;
  Json *ptJson = new Json;

  ptJson->i("Service", "curl");
  in.push_back(ptJson);
  ptJson = new Json;
  ptJson->insert("URL", strURL);
  ptJson->insert("Display", "Content,Cookies,Header");
  if (!strCookies.empty())
  {
    ptJson->insert("Cookies", strCookies);
    strCookies.clear();
  }
  if (ptAuth != NULL)
  {
    ptJson->m["Auth"] = new Json(ptAuth);
  }
  if (!strContent.empty())
  {
    ptJson->insert("Content", strContent);
  }
  ptJson->insert("FailOnError", ((bFailOnError)?"yes":"no"));
  if (!strCustomRequest.empty())
  {
    ptJson->insert("CustomRequest", strCustomRequest);
  }
  if (ptGet != NULL)
  {
    ptJson->m["Get"] = new Json(ptGet);
  }
  if (!strHeader.empty())
  {
    ptJson->insert("Header", strHeader);
  }
  if (!strUserAgent.empty())
  {
    ptJson->insert("UserAgent", strUserAgent);
  }
  ptJson->insert("Mobile", ((bMobile)?"yes":"no"));
  if (ptPost != NULL)
  {
    ptJson->m["Post"] = new Json(ptPost);
  }
  if (ptPut != NULL)
  {
    ptJson->m["Put"] = new Json(ptPut);
  }
  if (!strProxy.empty())
  {
    ptJson->insert("Proxy", strProxy);
  }
  if (!strType.empty())
  {
    ptJson->insert("Type", strType);
  }
  in.push_back(ptJson);
  if (junction(in, out, strError))
  {
    map<string, string> requestResponse;
    out.front()->flatten(requestResponse, true);
    if (out.size() == 2)
    {
      if (out.back()->m.find("Cookies") != out.back()->m.end())
      {
        strCookies = out.back()->m["Cookies"]->v;
      }
      if (out.back()->m.find("Header") != out.back()->m.end())
      {
        strHeader = out.back()->m["Header"]->v;
      }
      if (out.back()->m.find("Content") != out.back()->m.end())
      {
        strContent = out.back()->m["Content"]->v;
      }
    }
    if (requestResponse.find("Status") != requestResponse.end() && requestResponse["Status"] == "okay")
    {
      bResult = true;
    }
    else if (requestResponse.find("Error") != requestResponse.end() && !requestResponse["Error"].empty())
    {
      strError = requestResponse["Error"];
    }
    else
    {
      strError = "[curl] Encountered an unknown error.";
    }
    requestResponse.clear();
  }
  for (auto &i : in)
  {
    delete i;
  }
  for (auto &i : out)
  {
    delete i;
  }

  return bResult;
}
// }}}
// {{{ db
// {{{ db()
bool Interface::db(const string f, Json *d, string &e)
{
  string q;

  return db(f, d, q, e);
}
bool Interface::db(const string f, Json *d, string &q, string &e)
{
  string id;

  return db(f, d, id, q, e);
}
bool Interface::db(const string f, Json *d, string &id, string &q, string &e)
{
  bool b = false, s = false;
  Json *i, *o;

  if (exist(d, "Request"))
  {
    s = true;
    i = d->m["Request"];
    if (exist(d, "Response"))
    {
      delete d->m["Response"];
      d->m.erase("Response");
    }
    d->m["Response"] = new Json;
    o = d->m["Response"];
  }
  else
  {
    i = d;
    o = new Json;
  }
  b = db(f, i, o, id, q, e);
  if (!s)
  {
    delete o;
  }

  return b;
}
bool Interface::db(const string f, Json *i, list<map<string, string> > &rs, string &e)
{
  string q;

  return db(f, i, rs, q, e);
}
bool Interface::db(const string f, Json *i, list<map<string, string> > &rs, string &q, string &e)
{
  bool b = false;
  Json *o = new Json;

  if ((b = db(f, i, o, q, e)))
  {
    if (!o->m.empty())
    {
      map<string, string> r;
      o->flatten(r, true, false);
      rs.push_back(r);
    }
    else
    {
      for (auto &a : o->l)
      {
        map<string, string> r;
        a->flatten(r, true, false);
        rs.push_back(r);
      }
    }
  }
  delete o;

  return b;
}
bool Interface::db(const string f, Json *i, map<string, string> &r, string &e)
{
  string q;

  return db(f, i, r, q, e);
}
bool Interface::db(const string f, Json *i, map<string, string> &r, string &q, string &e)
{
  bool b = false;
  list<map<string, string> > rs;

  if ((b = db(f, i, rs, q, e)))
  {
    if (!rs.empty())
    {
      r = rs.front();
    }
  }

  return b;
}
bool Interface::db(const string f, Json *i, Json *o, string &e)
{
  string q;

  return db(f, i, o, q, e);
}
bool Interface::db(const string f, Json *i, Json *o, string &q, string &e)
{
  string id;

  return db(f, i, o, id, q, e);
}
bool Interface::db(const string f, Json *i, Json *o, string &id, string &q, string &e)
{
  bool b = false;

  if (!f.empty())
  {
    string strJson;
    Json *j = new Json;
    j->i("Function", f);
    j->m["Request"] = new Json(i);
    j->m["Response"] = new Json(o);
    if (hub("db", j, e))
    {
      b = true;
      if (!empty(j, "ID"))
      {
        id = j->m["ID"]->v;
      }
      if (!empty(j, "Query"))
      {
        q = j->m["Query"]->v;
      }
    }
    i->parse(j->m["Request"]->j(strJson));
    o->parse(j->m["Response"]->j(strJson));
    delete j;
  }
  else
  {
    e = "Please provide the Function.";
  }

  return b;
}
// }}}
// {{{ dbfree()
void Interface::dbfree(list<map<string, string> > *rows)
{
  if (rows != NULL)
  {
    rows->clear();
    delete rows;
    rows = NULL;
  }
}
// }}}
// {{{ dbquery()
list<map<string, string> > *Interface::dbquery(const string strDatabase, const string strQuery, string &strError)
{
  unsigned long long ullRows;

  return dbquery(strDatabase, strQuery, ullRows, strError);
}
list<map<string, string> > *Interface::dbquery(const string strDatabase, const string strQuery, unsigned long long &ullRows, string &strError)
{
  list<map<string, string> > *rows = NULL;
  Json *ptJson = new Json;

  ullRows = 0;
  ptJson->i("Database", strDatabase);
  ptJson->i("Query", strQuery);
  if (hub("database", ptJson, strError))
  {
    if (exist(ptJson, "Response"))
    {
      rows = new list<map<string, string> >;
      for (auto &ptRow : ptJson->m["Response"]->l)
      {
        map<string, string> row;
        ptRow->flatten(row, true, false);
        rows->push_back(row);
      }
    }
    if (!empty(ptJson, "Rows"))
    {
      stringstream ssRows(ptJson->m["Rows"]->v);
      ssRows >> ullRows;
    }
  }
  delete ptJson;

  return rows;
}
// }}}
// {{{ dbupdate()
bool Interface::dbupdate(const string strDatabase, const string strUpdate, string &strError)
{
  unsigned long long ullID, ullRows;

  return dbupdate(strDatabase, strUpdate, ullID, ullRows, strError);
}
bool Interface::dbupdate(const string strDatabase, const string strUpdate, string &strID, string &strError)
{
  bool bResult = false;
  unsigned long long ullID, ullRows;

  if (dbupdate(strDatabase, strUpdate, ullID, ullRows, strError))
  {
    stringstream ssID;
    bResult = true;
    ssID << ullID;
    strID = ssID.str();
  }

  return bResult;
}
bool Interface::dbupdate(const string strDatabase, const string strUpdate, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ullID = ullRows = 0;
  ptJson->i("Database", strDatabase);
  ptJson->i("Update", strUpdate);
  if (hub("database", ptJson, strError))
  {
    bResult = true;
    if (!empty(ptJson, "ID"))
    {
      stringstream ssID(ptJson->m["ID"]->v);
      ssID >> ullID;
    }
    if (!empty(ptJson, "Rows"))
    {
      stringstream ssRows(ptJson->m["Rows"]->v);
      ssRows >> ullRows;
    }
  }
  delete ptJson;

  return bResult;
}
// }}}
// }}}
// {{{ email()
void Interface::email(const string strFrom, const string strTo, const string strSubject, const string strText, const string strHtml)
{
  list<string> to;

  to.push_back(strTo);
  email(strFrom, to, strSubject, strText, strHtml);
}
void Interface::email(const string strFrom, list<string> to, const string strSubject, const string strText, const string strHtml)
{
  list<string> bcc, cc;
  map<string, string> file;

  email(strFrom, to, cc, bcc, strSubject, strText, strHtml, file);
}
void Interface::email(const string strFrom, list<string> to, list<string> cc, list<string> bcc, const string strSubject, const string strText, const string strHtml, map<string, string> file)
{
  Json *ptJson = new Json, *ptReq = new Json;

  ptJson->m["Request"] = new Json;
  ptReq->i("Service", "email");
  ptReq->i("From", strFrom);
  if (!to.empty())
  {
    stringstream ssTo;
    for (auto i = to.begin(); i != to.end(); i++)
    {
      if (i != to.begin())
      {
        ssTo << ",";
      }
      ssTo << *i;
    }
    ptReq->i("To", ssTo.str());
  }
  if (!cc.empty())
  {
    stringstream ssCc;
    for (auto i = cc.begin(); i != cc.end(); i++)
    {
      if (i != cc.begin())
      {
        ssCc << ",";
      }
      ssCc << *i;
    }
    ptReq->i("CC", ssCc.str());
  }
  if (!bcc.empty())
  {
    stringstream ssBcc;
    for (auto i = bcc.begin(); i != bcc.end(); i++)
    {
      if (i != bcc.begin())
      {
        ssBcc << ",";
      }
      ssBcc << *i;
    }
    ptReq->i("BCC", ssBcc.str());
  }
  if (!strSubject.empty())
  {
    ptReq->i("Subject", strSubject);
  }
  if (!strText.empty())
  {
    ptReq->i("Text", strText);
  }
  if (!strHtml.empty())
  {
    ptReq->i("HTML", strHtml);
  }
  ptJson->m["Request"]->pb(ptReq);
  delete ptReq;
  if (!file.empty())
  {
    for (auto &i : file)
    {
      string strData;
      ptReq = new Json;
      m_manip.encodeBase64(i.second, strData);
      ptReq->i("Name", i.first);
      ptReq->i("Data", strData);
      ptReq->i("Encode", "base64");
      ptJson->m["Request"]->pb(ptReq);
      delete ptReq;
    }
  }
  hub("junction", ptJson, false);
  delete ptJson;
}
// }}}
// {{{ enableWorkers()
void Interface::enableWorkers()
{
  int nReturn;
  string strError;
  stringstream ssMessage;

  if ((nReturn = pipe(m_fdCallbackPool)) == 0)
  {
    m_pUtility->fdNonBlocking(m_fdCallbackPool[0], strError);
    m_pUtility->fdNonBlocking(m_fdCallbackPool[1], strError);
    m_pThreadCallbackPool = new thread(&Interface::callbackPool, this);
    pthread_setname_np(m_pThreadCallbackPool->native_handle(), "callbackPool");
  }
  else
  {
    ssMessage.str("");
    ssMessage << "Interface::enableWorkers()->pipe(" << errno << ") error:  " << strerror(errno);
    log(ssMessage.str());
    setShutdown();
  }
}
// }}}
// {{{ getUserEmail()
string Interface::getUserEmail(radialUser &d)
{
  string e, v;
  radialUser a;

  userInit(d, a);
  a.p->m["i"]->i("userid", d.u);
  if (user(a, e) && !empty(a.p->m["o"], "email"))
  {
    v = a.p->m["o"]->m["email"]->v;
  }
  userDeinit(a);

  return v;
}
// }}}
// {{{ getUserFirstName()
string Interface::getUserFirstName(radialUser &d)
{
  string e, v;
  radialUser a;

  userInit(d, a);
  a.p->m["i"]->i("userid", d.u);
  if (user(a, e) && !empty(a.p->m["o"], "first_name"))
  {
    v = a.p->m["o"]->m["first_name"]->v;
  }
  userDeinit(a);

  return v;
}
// }}}
// {{{ getUserLastName()
string Interface::getUserLastName(radialUser &d)
{
  string e, v;
  radialUser a;

  userInit(d, a);
  a.p->m["i"]->i("userid", d.u);
  if (user(a, e) && !empty(a.p->m["o"], "last_name"))
  {
    v = a.p->m["o"]->m["last_name"]->v;
  }
  userDeinit(a);

  return v;
}
// }}}
// {{{ getUserName()
string Interface::getUserName(radialUser &d)
{
  string e;
  stringstream v;
  radialUser a;

  userInit(d, a);
  a.p->m["i"]->i("userid", d.u);
  if (user(a, e) && !empty(a.p->m["o"], "first_name") && !empty(a.p->m["o"], "last_name"))
  {
    v << a.p->m["o"]->m["first_name"]->v << " " << a.p->m["o"]->m["last_name"]->v;
  }
  userDeinit(a);

  return v.str();
}
// }}}
// {{{ hub()
void Interface::hub(radialPacket &p, const bool bWait)
{
  string strValue;

  if (bWait)
  {
    int fdUnique[2] = {-1, -1}, nReturn;
    stringstream ssMessage;
    p.s = m_strName;
    if ((nReturn = pipe(fdUnique)) == 0)
    {
      bool bExit = false, bResult = false;
      size_t unPosition, unUnique = 0;
      string strBuffer, strError;
      stringstream ssUnique;
      time_t CTime[2];
      m_pUtility->fdNonBlocking(fdUnique[0], strError);
      m_pUtility->fdNonBlocking(fdUnique[1], strError);
      m_mutexShare.lock();
      ssUnique << m_strName << "_" << unUnique;
      while (m_waiting.find(ssUnique.str()) != m_waiting.end())
      {
        unUnique++;
        ssUnique.str("");
        ssUnique << m_strName << "_" << unUnique;
      }
      p.u = ssUnique.str();
      m_waiting[ssUnique.str()] = fdUnique[1];
      m_responses.push_back(pack(p, strValue));
      if (!m_bResponse && m_fdResponse[1] != -1)
      {
        m_bResponse = true;
        write(m_fdResponse[1], "\n", 1);
      }
      m_mutexShare.unlock();
      time(&(CTime[0]));
      while (!bExit)
      {
        pollfd fds[1];
        fds[0].fd = fdUnique[0];
        fds[0].events = POLLIN;
        if ((nReturn = poll(fds, 1, 2000)) > 0)
        {
          if (fds[0].revents & (POLLHUP | POLLIN))
          {
            if (!m_pUtility->fdRead(fds[0].fd, strBuffer, nReturn))
            {
              bExit = true;
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << "Utility::fdRead(" << errno << ") " << strerror(errno);
                strError = ssMessage.str();
              }
              else if ((unPosition = strBuffer.find("\n")) != string::npos)
              {
                bResult = true;
              }
              else if (!strBuffer.empty())
              {
                ssMessage.str("");
                ssMessage << "Invalid response. --- " << strBuffer;
                strError = ssMessage.str();
              }
              else
              {
                strError = "Failed to receive a response.";
              }
            }
          }
        }
        else if (nReturn < 0 && errno != EINTR)
        {
          bExit = true;
          ssMessage.str("");
          ssMessage << "poll(" << errno << ") " << strerror(errno);
          strError = ssMessage.str();
        }
        time(&(CTime[1]));
        if ((CTime[1] - CTime[0]) > 600)
        {
          bExit = true;
          ssMessage.str("");
          ssMessage << "Failed to receive a response within 10 minutes.";
          strError = ssMessage.str();
        }
      }
      close(fdUnique[0]);
      m_mutexShare.lock();
      m_waiting.erase(ssUnique.str());
      m_mutexShare.unlock();
      if (bResult)
      {
        unpack(strBuffer.substr(0, unPosition), p);
      }
      else
      {
        Json *ptJson = new Json(p.p);
        ptJson->i("Status", "error");
        ptJson->i("Error", ((!strError.empty())?strError:"Encountered an uknown error."));
        ptJson->j(p.p);
        delete ptJson;
      }
    }
    else
    {
      Json *ptJson = new Json(p.p);
      ssMessage.str("");
      ssMessage << "pipe(" << errno << ") " << strerror(errno);
      ptJson->i("Status", "error");
      ptJson->i("Error", ssMessage.str());
      ptJson->j(p.p);
      delete ptJson;
    }
  }
  else
  {
    m_mutexShare.lock();
    m_responses.push_back(pack(p, strValue));
    if (!m_bResponse && m_fdResponse[1] != -1)
    {
      m_bResponse = true;
      write(m_fdResponse[1], "\n", 1);
    }
    m_mutexShare.unlock();
  }
}
void Interface::hub(const string strTarget, Json *ptJson, const bool bWait)
{
  radialPacket p;

  ptJson->j(p.p);
  if (!strTarget.empty())
  {
    p.t = strTarget;
  }
  hub(p, bWait);
  if (bWait)
  {
    ptJson->parse(p.p);
  }
}
void Interface::hub(Json *ptJson, const bool bWait)
{
  hub("", ptJson, bWait);
}
bool Interface::hub(const string strTarget, Json *ptJson, string &strError)
{
  bool bResult = false;

  hub(strTarget, ptJson, true);
  if (exist(ptJson, "Status") && ptJson->m["Status"]->v == "okay")
  {
    bResult = true;
  }
  else if (!empty(ptJson, "Error"))
  {
    strError = ptJson->m["Error"]->v;
  }
  else
  {
    strError = (string)"[hub," + strTarget + "] Encountered an unknown error.";
  }

  return bResult;
}
bool Interface::hub(Json *ptJson, string &strError)
{
  return hub("", ptJson, strError);
}
// }}}
// {{{ interfaceAdd()
bool Interface::interfaceAdd(const string strInterface, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ptJson->i("Function", "add");
  ptJson->i("Name", strInterface);
  if (hub(ptJson, strError))
  {
    bResult = true;
  }
  delete ptJson;

  return bResult;
}
bool Interface::interfaceAdd(const string strNode, const string strInterface, string &strError)
{
  bool bResult = false;

  if (!strNode.empty() && strNode != m_strNode)
  {
    Json *ptJson = new Json;
    ptJson->i("Node", strNode);
    ptJson->i("Interface", "hub");
    ptJson->i("Function", "add");
    ptJson->i("Name", strInterface);
    if (hub("link", ptJson, strError))
    {
      bResult = true;
    }
    delete ptJson;
  }
  else if (interfaceAdd(strInterface, strError))
  {
    bResult = true;
  }

  return bResult;
}
// }}}
// {{{ interfaceRemove()
bool Interface::interfaceRemove(const string strInterface, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ptJson->i("Function", "remove");
  ptJson->i("Name", strInterface);
  if (hub(ptJson, strError))
  {
    bResult = true;
  }
  delete ptJson;

  return bResult;
}
bool Interface::interfaceRemove(const string strNode, const string strInterface, string &strError)
{
  bool bResult = false;

  if (!strNode.empty() && strNode != m_strNode)
  {
    Json *ptJson = new Json;
    ptJson->i("Node", strNode);
    ptJson->i("Interface", "hub");
    ptJson->i("Function", "remove");
    ptJson->i("Name", strInterface);
    if (hub("link", ptJson, strError))
    {
      bResult = true;
    }
    delete ptJson;
  }
  else if (interfaceRemove(strInterface, strError))
  {
    bResult = true;
  }

  return bResult;
}
// }}}
// {{{ interfaces()
void Interface::interfaces(string strPrefix, Json *ptJson)
{
  strPrefix += "->Interface::interfaces()";
  m_mutexShare.lock();
  for (auto &i : m_i)
  {
    delete i.second;
  }
  m_i.clear();
  if (exist(ptJson, "Interfaces"))
  {
    for (auto &interface : ptJson->m["Interfaces"]->m)
    {
      m_i[interface.first] = new radialInterface;
      if (!empty(interface.second, "AccessFunction"))
      {
        m_i[interface.first]->strAccessFunction = interface.second->m["AccessFunction"]->v;
      }
      if (!empty(interface.second, "Command"))
      {
        m_i[interface.first]->strCommand = interface.second->m["Command"]->v;
      }
      m_i[interface.first]->nPid = -1;
      if (!empty(interface.second, "PID"))
      {
        stringstream ssPid(interface.second->m["PID"]->v);
        ssPid >> m_i[interface.first]->nPid;
      }
      m_i[interface.first]->bRespawn = ((exist(interface.second, "Respawn") && interface.second->m["Respawn"]->v == "1")?true:false);
      m_i[interface.first]->bRestricted = ((exist(interface.second, "Restricted") && interface.second->m["Restricted"]->v == "1")?true:false);
    }
  }
  m_mutexShare.unlock();
}
// }}}
// {{{ isApplicationDeveloper()
bool Interface::isApplicationDeveloper(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    if (!d.u.empty())
    {
      q << "select a.id from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.application_id = " << i->m["id"]->v << " and b.type in ('Primary Developer', 'Backup Developer') and c.userid = '" << d.u << "'";
      auto g = dbquery("central_r", q.str(), e);
      if (g != NULL)
      {
        if (!g->empty())
        {
          b = true;
        }
        else
        {
          e = "You are not a developer for this application.";
        }
      }
      dbfree(g);
    }
    else
    {
      e = "You are not authorized to run this request.";
    }
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ isLocalAdmin()
bool Interface::isLocalAdmin(radialUser &d, const string strApplication, const bool bAny, const bool bLocal)
{
  bool bResult = false;

  if (!bLocal && d.g)
  {
    bResult = true;
  }
  else if ((bAny || !strApplication.empty()) && !d.u.empty())
  {
    for (auto &app : d.auth)
    {
      if ((bAny || app.first == strApplication) && app.second)
      {
        bResult = true;
      }
    }
  }

  return bResult;
}
// }}}
// {{{ isMaster()
bool Interface::isMaster()
{
  return m_bMaster;
}
// }}}
// {{{ isMasterSettled()
bool Interface::isMasterSettled()
{
  return m_bMasterSettled;
}
// }}}
// {{{ isServerAdmin()
bool Interface::isServerAdmin(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    if (!d.u.empty())
    {
      q << "select a.id from server_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and a.server_id = " << i->m["id"]->v << " and b.type in ('Primary Admin', 'Backup Admin') and c.userid = '" << d.u << "'";
      auto g = dbquery("central_r", q.str(), e);
      if (g != NULL)
      {
        if (!g->empty())
        {
          b = true;
        }
        else
        {
          e = "You are not an admin for this server.";
        }
      }
      dbfree(g);
    }
    else
    {
      e = "You are not authorized to run this request.";
    }
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ isValid()
bool Interface::isValid(radialUser &d, const string strApplication)
{
  bool bResult = false;

  if (d.g)
  {
    bResult = true;
  }
  else if (!d.u.empty())
  {
    if (!strApplication.empty())
    {
      for (auto &app : d.auth)
      {
        if (app.first == strApplication)
        {
          bResult = true;
        }
      }
    }
    else
    {
      bResult = true;
    }
  }

  return bResult;
}
// }}}
// {{{ junction()
bool Interface::junction(list<Json *> in, list<Json *> &out, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ptJson->m["Request"] = new Json;
  for (auto &i : in)
  {
    ptJson->m["Request"]->pb(i);
  }
  for (auto &i : out)
  {
    delete i;
  }
  out.clear();
  if (hub("junction", ptJson, strError))
  {
    if (exist(ptJson, "Response"))
    {
      if (!ptJson->m["Response"]->l.empty())
      {
        Json *ptStatus = ptJson->m["Response"]->l.front();
        if (exist(ptStatus, "Status") && ptStatus->m["Status"]->v == "okay")
        {
          bResult = true;
        }
        else if (!empty(ptStatus, "Error"))
        {
          strError = ptStatus->m["Error"]->v;
        }
        else
        {
          strError = "[junction] Encountered an unknown error.";
        }
        for (auto &i : ptJson->m["Response"]->l)
        {
          out.push_back(new Json(i));
        }
      }
      else
      {
        strError = "Failed to receive the status line of the Response.";
      }
    }
    else
    {
      strError = "Failed to receive the Response.";
    }
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ jwt()
bool Interface::jwt(const string strSigner, const string strSecret, string &strPayload, Json *ptPayload, string &strError)
{
  bool bDecode = false, bResult = false;
  Json *ptJson = new Json;

  if (!strPayload.empty())
  {
    bDecode = true;
    ptJson->i("Function", "decode");
  }
  else
  {
    ptJson->i("Function", "encode");
  }
  ptJson->i("Signer", strSigner);
  if (!strSecret.empty())
  {
    ptJson->i("Secret", strSecret);
  }
  if (bDecode)
  {
    ptJson->i("Payload", strPayload);
  }
  else
  {
    ptJson->m["Payload"] = new Json(ptPayload);
  }
  if (hub("jwt", ptJson, strError))
  {
    if (exist(ptJson, "Response"))
    {
      if (exist(ptJson->m["Response"], "Payload"))
      {
        if (bDecode)
        {
          bResult = true;
          ptPayload->merge(ptJson->m["Response"]->m["Payload"], true, false);
        }
        else if (!ptJson->m["Response"]->m["Payload"]->v.empty())
        {
          bResult = true;
          strPayload = ptJson->m["Response"]->m["Payload"]->v;
        }
        else
        {
          strError = "The Payload was empty within the Response.";
        }
      }
      else
      {
        strError = "Failed to find the Payload within the Response.";
      }
    }
    else
    {
      strError = "Failed to receive the Response.";
    }
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ keyRemovals()
void Interface::keyRemovals(Json *ptJson)
{
  list<string> removals;

  for (auto &i : ptJson->m)
  {
    if (!i.first.empty() && i.first[0] == '_')
    {
      removals.push_back(i.first);
    }
  }
  while (!removals.empty())
  {
    if (exist(ptJson, removals.front()))
    {
      delete ptJson->m[removals.front()];
      ptJson->m.erase(removals.front());
    }
    removals.pop_front();
  }
}
// }}}
// {{{ links()
void Interface::links(string strPrefix, Json *ptJson)
{
  strPrefix += "->Interface::links()";
  m_mutexShare.lock();
  for (auto &link : m_l)
  {
    for (auto &interface : link->interfaces)
    {
      delete interface.second;
    }
    link->interfaces.clear();
    delete link;
  }
  m_l.clear();
  if (exist(ptJson, "Links"))
  {
    for (auto &link : ptJson->m["Links"]->m)
    {
      radialLink *ptLink = new radialLink;
      ptLink->strNode = link.first;
      if (!empty(link.second, "Server"))
      {
        ptLink->strServer = link.second->m["Server"]->v;
      }
      if (!empty(link.second, "Port"))
      {
        ptLink->strPort = link.second->m["Port"]->v;
      }
      if (exist(link.second, "Interfaces"))
      {
        for (auto &interface : link.second->m["Interfaces"]->m)
        {
          ptLink->interfaces[interface.first] = new radialInterface;
          if (!empty(interface.second, "AccessFunction"))
          {
            ptLink->interfaces[interface.first]->strAccessFunction = interface.second->m["AccessFunction"]->v;
          }
          if (!empty(interface.second, "Command"))
          {
            ptLink->interfaces[interface.first]->strCommand = interface.second->m["Command"]->v;
          }
          ptLink->interfaces[interface.first]->nPid = -1;
          if (!empty(interface.second, "PID"))
          {
            stringstream ssPid(interface.second->m["PID"]->v);
            ssPid >> ptLink->interfaces[interface.first]->nPid;
          }
          ptLink->interfaces[interface.first]->bRespawn = ((exist(interface.second, "Respawn") && interface.second->m["Respawn"]->v == "1")?true:false);
          ptLink->interfaces[interface.first]->bRestricted = ((exist(interface.second, "Restricted") && interface.second->m["Restricted"]->v == "1")?true:false);
        }
      }
      m_l.push_back(ptLink);
    }
  }
  m_mutexShare.unlock();
}
// }}}
// {{{ live()
bool Interface::live(const string strApplication, const string strUser, map<string, string> message, string &strError)
{
  bool bResult = false;
  Json *ptMessage = new Json(message);

  if (live(strApplication, strUser, ptMessage, strError))
  {
    bResult = true;
  }
  delete ptMessage;

  return bResult;
}
bool Interface::live(const string strApplication, const string strUser, Json *ptMessage, string &strError)
{
  bool bResult = false;
  Json *ptJson = new Json;

  ptJson->i("Function", "message");
  ptJson->m["Request"] = new Json;
  if (!strApplication.empty())
  {
    ptJson->m["Request"]->i("Application", strApplication);
  }
  if (!strUser.empty())
  {
    ptJson->m["Request"]->i("User", strUser);
  }
  ptJson->m["Request"]->m["Message"] = new Json(ptMessage);
  if (hub("live", ptJson, strError))
  {
    bResult = true;
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ log()
void Interface::log(const string strMessage)
{
  log("log", strMessage);
}
void Interface::log(const string strFunction, const string strMessage)
{
  Json *ptJson = new Json;

  ptJson->i("Function", strFunction);
  ptJson->i("Message", strMessage);
  hub("log", ptJson, false);
  delete ptJson;
}
// }}}
// {{{ logger()
void Interface::logger(const string strApplication, const string strFunction, map<string, string> label, const string strMessage)
{
  Json *ptJson = new Json;

  ptJson->i("Function", strFunction);
  ptJson->m["Request"] = new Json;
  ptJson->m["Request"]->i("Application", strApplication);
  ptJson->m["Request"]->i("Label", label);
  ptJson->m["Request"]->i("Message", strMessage);
  hub("logger", ptJson, false);
  delete ptJson;
}
void Interface::logger(const string strFunction, map<string, string> label, const string strMessage)
{
  logger("Radial", strFunction, label, strMessage);
}
// }}}
// {{{ master()
string Interface::master()
{
  return m_strMaster;
}
// }}}
// {{{ mysql
// {{{ mysql()
bool Interface::mysql(const string strServer, const unsigned int unPort, const string strUser, const string strPassword, const string strDatabase, const string strType, const string strQuery, unsigned long long &ullID, unsigned long long &ullRows, list<map<string, string> > &rows, string &strError)
{
  bool bResult;
  stringstream ssPort;
  Json *ptJson = new Json;

  ptJson->i("Server", strServer);
  ssPort << unPort;
  ptJson->i("Port", ssPort.str(), 'n');
  ptJson->i("User", strUser);
  ptJson->i("Password", strPassword);
  ptJson->i("Database", strDatabase);
  ptJson->i(strType, strQuery);
  if (hub("mysql", ptJson, strError))
  {
    bResult = true;
    if (exist(ptJson, "Response"))
    {
      for (auto &ptRow : ptJson->m["Response"]->l)
      {
        map<string, string> row;
        ptRow->flatten(row, true, false);
        rows.push_back(row);
      }
    }
    if (!empty(ptJson, "ID"))
    {
      stringstream ssID(ptJson->m["ID"]->v);
      ssID >> ullID;
    }
    if (!empty(ptJson, "Rows"))
    {
      stringstream ssRows(ptJson->m["Rows"]->v);
      ssRows >> ullRows;
    }
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ mysqlQuery()
bool Interface::mysqlQuery(const string strServer, const unsigned int unPort, const string strUser, const string strPassword, const string strDatabase, const string strQuery, unsigned long long &ullRows, list<map<string, string> > &rows, string &strError)
{
  unsigned long long ullID;

  return mysql(strServer, unPort, strUser, strPassword, strDatabase, "Query", strQuery, ullID, ullRows, rows, strError);
}
// }}}
// {{{ mysqlUpdate()
bool Interface::mysqlUpdate(const string strServer, const unsigned int unPort, const string strUser, const string strPassword, const string strDatabase, const string strQuery, unsigned long long &ullID, unsigned long long &ullRows, string &strError)
{
  list<map<string, string> > rows;

  return mysql(strServer, unPort, strUser, strPassword, strDatabase, "Update", strQuery, ullID, ullRows, rows, strError);
}
// }}}
// }}}
// {{{ notify()
void Interface::notify(const string strMessage)
{
  log("notify", strMessage);
}
// }}}
// {{{ ny()
void Interface::ny(Json *ptJson, const string strField)
{
  if (ptJson != NULL)
  {
    if (exist(ptJson, strField))
    {
      if (ptJson->m[strField]->v == "1")
      {
        ptJson->m[strField]->i("name", "Yes");
        ptJson->m[strField]->i("value", "1", 'n');
      }
      else
      {
        ptJson->m[strField]->i("name", "No");
        ptJson->m[strField]->i("value", "0", 'n');
      }
    }
    else
    {
      ptJson->m[strField] = new Json;
      ptJson->m[strField]->i("name", "No");
      ptJson->m[strField]->i("value", "0", 'n');
    }
  }
}
// }}}
// {{{ page
// {{{ page()
bool Interface::page(const string strType, const string strTarget, const string strMessage, string &strError)
{
  bool bResult = false;
  list<Json *> in, out;
  Json *ptJson = new Json;

  ptJson->i("Service", "pager");
  ptJson->i(strType, strTarget);
  ptJson->i("Message", strMessage);
  in.push_back(ptJson);
  if (junction(in, out, strError))
  {
    bResult = true;
  }
  for (auto &i : in)
  {
    delete i;
  }
  for (auto &i : out)
  {
    delete i;
  }

  return bResult;
}
// }}}
// {{{ pageGroup()
bool Interface::pageGroup(const string strGroup, const string strMessage, string &strError)
{
  return page("Group", strGroup, strMessage, strError);
}
// }}}
// {{{ pageUser()
bool Interface::pageUser(const string strUser, const string strMessage, string &strError)
{
  return page("User", strUser, strMessage, strError);
}
// }}}
// }}}
// {{{ process()
void Interface::process(string strPrefix)
{
  int nReturn;
  string strError;
  stringstream ssMessage;

  strPrefix += "->Interface::process()";
  if ((nReturn = pipe(m_fdResponse)) == 0)
  {
    bool bExit = false;
    char cChar;
    list<int> uniqueRemovals;
    map<int, string> uniques;
    pollfd *fds;
    size_t unIndex, unPosition;
    string strJson, strLine;
    time_t CBroadcast, CMaster[2], CShutdown = 0, CThroughput, CTime, unBroadcastSleep = 15;
    m_pUtility->fdNonBlocking(0, strError);
    m_pUtility->fdNonBlocking(1, strError);
    time(&CBroadcast);
    CMaster[0] = CMaster[1] = CThroughput = CBroadcast;
    while (!bExit)
    {
      fds = new pollfd[uniques.size() + 3];
      unIndex = 0;
      fds[unIndex].fd = 0;
      fds[unIndex].events = POLLIN;
      unIndex++;
      fds[unIndex].fd = -1;
      fds[unIndex].events = POLLOUT;
      if (m_strBuffers[1].empty())
      {
        m_mutexShare.lock();
        while (!m_responses.empty())
        {
          m_strBuffers[1].append(m_responses.front() + "\n");
          m_responses.pop_front();
        }
        m_mutexShare.unlock();
      }
      if (!m_strBuffers[1].empty())
      {
        fds[unIndex].fd = 1;
      }
      unIndex++;
      fds[unIndex].fd = m_fdResponse[0];
      fds[unIndex].events = POLLIN;
      unIndex++;
      for (auto &unique : uniques)
      {
        fds[unIndex].fd = -1;
        if (!unique.second.empty())
        {
          fds[unIndex].fd = unique.first;
        }
        fds[unIndex].events = POLLOUT;
        unIndex++;
      }
      if (fds[0].fd != 0 || (fds[1].fd != -1 && fds[1].fd != 1))
      {
        bExit = true;
      }
      if (!bExit && (nReturn = poll(fds, unIndex, 2000)) > 0)
      {
        if (fds[0].revents & (POLLHUP | POLLIN))
        {
          if (m_pUtility->fdRead(fds[0].fd, m_strBuffers[0], nReturn))
          {
            while ((unPosition = m_strBuffers[0].find("\n")) != string::npos)
            {
              int fdUnique = -1;
              radialPacket p;
              strLine = m_strBuffers[0].substr(0, unPosition);
              m_strBuffers[0].erase(0, (unPosition + 1));
              unpack(strLine, p);
              if (p.s == m_strName && !p.u.empty())
              {
                m_mutexShare.lock();
                if (m_waiting.find(p.u) != m_waiting.end())
                {
                  fdUnique = m_waiting[p.u];
                }
                m_mutexShare.unlock();
              }
              if (fdUnique != -1)
              {
                uniques[fdUnique] = strLine + "\n";
              }
              else if (p.s == "hub")
              {
                Json *ptJson = new Json(p.p);
                if (!empty(ptJson, "Function"))
                {
                  // {{{ interfaces
                  if (ptJson->m["Function"]->v == "interfaces")
                  {
                    interfaces(strPrefix, ptJson);
                  }
                  // }}}
                  // {{{ links
                  else if (ptJson->m["Function"]->v == "links")
                  {
                    links(strPrefix, ptJson);
                  }
                  // }}}
                  // {{{ shutdown
                  else if (ptJson->m["Function"]->v == "shutdown")
                  {
                    ssMessage.str("");
                    ssMessage << strPrefix << ":  Shutting down.";
                    log(ssMessage.str());
                    setShutdown();
                  }
                  // }}}
                }
                delete ptJson;
              }
              else
              {
                Json *ptJson = new Json(p.p);
                if (exist(ptJson, "|function") && ptJson->m["|function"]->v == "master")
                {
                  if (!empty(ptJson, "Master"))
                  {
                    time(&CMaster[0]);
                    if (m_strMaster != ptJson->m["Master"]->v)
                    {
                      string strMaster = m_strMaster;
                      m_strMaster = ptJson->m["Master"]->v;
                      m_bMaster = ((m_strMaster == m_strNode)?true:false);
                      m_bMasterSettled = false;
                      time(&CMaster[1]);
                      if (m_pAutoModeCallback != NULL)
                      {
                        m_pAutoModeCallback(strPrefix, strMaster, m_strMaster);
                      }
                    }
                  }
                }
                else if (exist(ptJson, "|function") && ptJson->m["|function"]->v == "status")
                {
                  float fCpu = 0, fMem = 0;
                  pid_t nPid = getpid();
                  stringstream ssImage, ssPid, ssResident;
                  time_t CTime = 0;
                  unsigned long ulImage = 0, ulResident = 0;
                  ptJson->i("Status", "okay");
                  if (exist(ptJson, "Response"))
                  {
                    delete ptJson->m["Response"];
                  }
                  ptJson->m["Response"] = new Json;
                  m_pCentral->getProcessStatus(nPid, CTime, fCpu, fMem, ulImage, ulResident);
                  ptJson->m["Response"]->m["Memory"] = new Json;
                  ssImage << ulImage;
                  ptJson->m["Response"]->m["Memory"]->i("Image", ssImage.str(), 'n');
                  ssResident << ulResident;
                  ptJson->m["Response"]->m["Memory"]->i("Resident", ssResident.str(), 'n');
                  ssPid << nPid;
                  ptJson->m["Response"]->i("PID", ssPid.str(), 'n');
                  if (!m_strMaster.empty())
                  {
                    ptJson->m["Response"]->m["Master"] = new Json;
                    ptJson->m["Response"]->m["Master"]->i("Node", m_strMaster);
                    ptJson->m["Response"]->m["Master"]->i("Settled", ((m_bMasterSettled)?"1":"0"), ((m_bMasterSettled)?'1':'0'));
                  }
                  m_mutexBase.lock();
                  if (m_unThreads > 0)
                  {
                    stringstream ssThreads;
                    ssThreads << m_unThreads;
                    ptJson->m["Response"]->i("Threads", ssThreads.str(), 'n');
                  }
                  m_mutexBase.unlock();
                  ptJson->j(p.p);
                  hub(p, false);
                }
                else if (m_pCallback != NULL)
                {
                  if (!empty(ptJson, "wsRequestID"))
                  {
                    string strIdentity, strName, strNode;
                    stringstream ssRequestID(ptJson->m["wsRequestID"]->v);
                    ssRequestID >> strNode >> strName >> strIdentity;
                    if (strName != m_strName)
                    {
                      Json *ptLive = new Json;
                      ptLive->i("wsRequestID", ptJson->m["wsRequestID"]->v);
                      ptLive->i("radialProcess", m_strName);
                      ptLive->i("radialPrefix", strPrefix);
                      ptLive->i("radialPurpose", "status");
                      if (!empty(ptJson, "Interface"))
                      {
                        ptLive->i("radialInterface", ptJson->m["Interface"]->v);
                      }
                      if (!empty(ptJson, "Function"))
                      {
                        ptLive->i("radialFunction", ptJson->m["Function"]->v);
                      }
                      hub(strName, ptLive, false);
                      delete ptLive;
                    }
                  }
                  if (!shutdown())
                  {
                    if (m_pThreadCallbackPool != NULL)
                    {
                      callbackPush(strPrefix, strLine, true);
                    }
                    else
                    {
                      m_pCallback(strPrefix, strLine, true);
                    }
                  }
                  else
                  {
                    ptJson->i("Status", "error");
                    ptJson->i("Error", "Interface is shutting down.");
                    ptJson->j(p.p);
                    hub(p, false);
                  }
                }
                delete ptJson;
              }
            }
          }
          else
          {
            bExit = true;
          }
        }
        if (fds[1].revents & POLLOUT)
        {
          if (!m_pUtility->fdWrite(fds[1].fd, m_strBuffers[1], nReturn))
          {
            bExit = true;
          }
        }
        if (fds[2].revents & POLLIN)
        {
          if ((nReturn = read(fds[2].fd, &cChar, 1)) > 0)
          {
            m_mutexShare.lock();
            m_bResponse = false;
            m_mutexShare.unlock();
          }
          else
          {
            bExit = true;
          }
        }
        for (size_t i = 3; i < unIndex; i++)
        {
          if (fds[i].revents & POLLOUT)
          {
            if (m_pUtility->fdWrite(fds[i].fd, uniques[fds[i].fd], nReturn))
            {
              if (uniques[fds[i].fd].empty())
              {
                uniqueRemovals.push_back(fds[i].fd);
              }
            }
            else
            {
              uniqueRemovals.push_back(fds[i].fd);
              if (nReturn < 0)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->m_pUtility->fdWrite(" << errno << ") [" << fds[i].fd << "]:  " << strerror(errno);
                log(ssMessage.str());
              }
            }
          }
          else if (fds[i].revents & POLLNVAL)
          {
            uniqueRemovals.push_back(fds[i].fd);
            if (nReturn < 0)
            {
              ssMessage.str("");
              ssMessage << strPrefix << "->poll(POLLNVAL) [" << fds[i].fd << "]:  Invalid request.";
              log(ssMessage.str());
            }
          }
        }
      }
      else if (!bExit && nReturn < 0 && errno != EINTR)
      {
        bExit = true;
        ssMessage.str("");
        ssMessage << strPrefix << "->poll(" << errno << "):  " << strerror(errno);
        notify(ssMessage.str());
      }
      delete[] fds;
      uniqueRemovals.sort();
      uniqueRemovals.unique();
      while (!uniqueRemovals.empty())
      {
        uniques.erase(uniqueRemovals.front());
        close(uniqueRemovals.front());
        uniqueRemovals.pop_front();
      }
      time(&CTime);
      if (m_pAutoModeCallback != NULL)
      {
        if (!m_bMasterSettled && (CTime - CMaster[1]) > 120)
        {
          m_bMasterSettled = true;
        }
        if ((CTime - CBroadcast) > unBroadcastSleep)
        {
          string strMaster = m_strMaster;
          unsigned int unSeed = CTime + getpid();
          srand(unSeed);
          unBroadcastSleep = (rand_r(&unSeed) % 5) + 1;
          if (!m_strMaster.empty() && m_strMaster != m_strNode)
          {
            bool bFound = false;
            m_mutexShare.lock();
            for (auto linkIter = m_l.begin(); !bFound && linkIter != m_l.end(); linkIter++)
            {
              if ((*linkIter)->strNode == m_strMaster)
              {
                bFound = true;
              }
            }
            m_mutexShare.unlock();
            if (!bFound)
            {
              m_strMaster.clear();
            }
          }
          if ((CTime - CMaster[0]) > 60)
          {
            m_strMaster.clear();
          }
          if (m_strMaster.empty())
          {
            m_strMaster = m_strNode;
          }
          if (!m_strMaster.empty() && m_strMaster == m_strNode)
          {
            Json *ptJson = new Json;
            CMaster[0] = CTime;
            ptJson->i("Interface", m_strName);
            ptJson->i("|function", "master");
            ptJson->i("Master", m_strMaster);
            hub("link", ptJson, false);
            delete ptJson;
          }
          if (strMaster != m_strMaster)
          {
            m_bMaster = ((m_strMaster == m_strNode)?true:false);
            m_bMasterSettled = false;
            time(&CMaster[1]);
            m_pAutoModeCallback(strPrefix, strMaster, m_strMaster);
          }
          CBroadcast = CTime;
        }
      }
      if ((CTime - CThroughput) >= 60)
      {
        Json *ptJson = new Json;
        radialPacket p;
        CThroughput = CTime;
        p.s = m_strName;
        ptJson->i("Function", "throughput");
        ptJson->m["Response"] = new Json;
        m_mutexBase.lock();
        for (auto &i : m_throughput)
        {
          stringstream ssThroughput;
          ssThroughput << i.second;
          ptJson->m["Response"]->i(i.first, ssThroughput.str(), 'n');
        }
        m_throughput.clear();
        m_mutexBase.unlock();
        throughput(ptJson->m["Response"]);
        ptJson->j(p.p);
        delete ptJson;
        hub(p, false);
      }
      if (shutdown())
      {
        time(&CTime);
        if (CShutdown == 0)
        {
          CShutdown = CTime;
        }
        m_mutexShare.lock();
        if ((CTime - CShutdown) > 10 && m_strBuffers[0].empty() && m_strBuffers[1].empty() && m_responses.empty() && m_waiting.empty())
        {
          bExit = true;
        }
        m_mutexShare.unlock();
      }
    }
    for (auto &unique : uniques)
    {
      close(unique.first);
    }
    close(m_fdResponse[0]);
    m_fdResponse[0] = -1;
    close(m_fdResponse[1]);
    m_fdResponse[1] = -1;
  }
  setShutdown();
}
// }}}
// {{{ setAutoMode()
void Interface::setAutoMode(void (*pCallback)(string, const string, const string))
{
  m_pAutoModeCallback = pCallback;
}
// }}}
// {{{ status()
void Interface::status(Json *ptStatus)
{
  string strError;
  Json *ptThroughput = new Json;

  Base::status(ptStatus);
  if (!m_strMaster.empty())
  {
    ptStatus->m["Master"] = new Json;
    ptStatus->m["Master"]->i("Node", m_strMaster);
    ptStatus->m["Master"]->i("Settled", ((m_bMasterSettled)?"1":"0"), ((m_bMasterSettled)?'1':'0'));
  }
  if (storageRetrieve({"radial", "nodes", m_strNode, "interfaces", m_strName, "throughput"}, ptThroughput, strError))
  {
    ptStatus->m["Throughput"] = ptThroughput;
  }
  else
  {
    delete ptThroughput;
  }
}
// }}}
// {{{ storage
// {{{ storage()
bool Interface::storage(const string strFunction, const list<string> keys, Json *ptJson, string &strError)
{
  bool bResult = false;
  string strJson;
  Json *ptSubJson = new Json;

  ptSubJson->i("Function", strFunction);
  if (!keys.empty())
  {
    ptSubJson->i("Keys", keys);
  }
  if ((strFunction == "add" || strFunction == "update") && ptJson != NULL)
  {
    ptSubJson->i("Request", ptJson);
  }
  if (hub("storage", ptSubJson, strError))
  {
    bResult = true;
    if ((strFunction == "retrieve" || strFunction == "retrieveKeys") && exist(ptSubJson, "Response"))
    {
      ptSubJson->m["Response"]->j(strJson);
      ptJson->parse(strJson);
    }
  }
  delete ptSubJson;

  return bResult;
}
// }}}
// {{{ storageAdd()
bool Interface::storageAdd(const list<string> keys, Json *ptJson, string &strError)
{
  return storage("add", keys, ptJson, strError);
}
// }}}
// {{{ storageRemove()
bool Interface::storageRemove(const list<string> keys, string &strError)
{
  return storage("remove", keys, NULL, strError);
}
// }}}
// {{{ storageRetrieve()
bool Interface::storageRetrieve(Json *ptJson, string &strError)
{
  return storage("retrieve", {}, ptJson, strError);
}
bool Interface::storageRetrieve(const list<string> keys, Json *ptJson, string &strError)
{
  return storage("retrieve", keys, ptJson, strError);
}
// }}}
// {{{ storageRetrieveKeys()
bool Interface::storageRetrieveKeys(const list<string> keysIn, list<string> &keysOut, string &strError)
{
  bool bResult = false;
  Json *ptKeys = new Json;

  keysOut.clear();
  if (storage("retrieveKeys", keysIn, ptKeys, strError))
  {
    bResult = true;
    for (auto &key : ptKeys->l)
    {
      keysOut.push_back(key->v);
    }
  }
  delete ptKeys;

  return bResult;
}
// }}}
// {{{ storageUpdate()
bool Interface::storageUpdate(const list<string> keys, Json *ptJson, string &strError)
{
  return storage("update", keys, ptJson, strError);
}
// }}}
// }}}
// {{{ throughput()
void Interface::throughput(const string strType, const size_t unThroughput)
{
  m_mutexBase.lock();
  if (m_throughput.find(strType) == m_throughput.end())
  {
    m_throughput[strType] = 0;
  }
  m_throughput[strType] += unThroughput;
  m_mutexBase.unlock();
}
void Interface::throughput(Json *ptData)
{
  list<string> keys = {"radial", "nodes", m_strNode, "interfaces", m_strName, "throughput"};
  Json *ptJson = new Json;

  ptJson->i("Function", "add");
  ptJson->i("Keys", keys);
  ptJson->i("Request", ptData);
  hub("storage", ptJson, false);
  delete ptJson;
}
// }}}
// {{{ user()
bool Interface::user(radialUser &d, string &e)
{
  bool b = false;
  Json *i = d.p->m["i"];

  if (!empty(i, "id") || !empty(i, "userid"))
  {
    map<string, string> r;
    if (db("dbCentralUsers", i, r, e))
    {
      if (!r.empty())
      {
        Json *j = new Json(r);
        b = true;
        ny(j, "active");
        ny(j, "admin");
        ny(j, "alert_chat");
        ny(j, "alert_email");
        ny(j, "alert_live_audio");
        ny(j, "alert_live_message");
        ny(j, "alert_pager");
        if (exist(j, "alert_remote_auth_decrypted_password"))
        {
          j->i("alert_remote_auth_password", j->m["alert_remote_auth_decrypted_password"]->v);
          delete j->m["alert_remote_auth_decrypted_password"];
          j->m.erase("alert_remote_auth_decrypted_password");
        }
        if (!d.g && (r["userid"].empty() || d.u != r["userid"]) && exist(j, "alert_remote_auth_password"))
        {
          delete j->m["alert_remote_auth_password"];
          j->m.erase("alert_remote_auth_password");
        }
        ny(j, "locked");
        d.p->i("o", j);
        delete j;
      }
      else
      {
        e = "No results returned.";
      }
    }
  }
  else
  {
    e = "Please provide the id or userid.";
  }

  return b;
}
// }}}
// {{{ userDeinit()
void Interface::userDeinit(radialUser &d)
{
  delete d.p;
}
// }}}
// {{{ userInit()
void Interface::userInit(radialUser &d)
{
  d.g = false;
  d.p = new Json;
  d.p->m["i"] = new Json;
  d.p->m["o"] = new Json;
}
void Interface::userInit(radialUser &i, radialUser &o)
{
  userInit(o);
  o.auth = i.auth;
  o.g = i.g;
  o.u = i.u;
}
void Interface::userInit(Json *ptJson, radialUser &d)
{
  string strError, strJwt;

  userInit(d);
  if (exist(ptJson, "Request"))
  {
    d.p->insert("i", ptJson->m["Request"]);
  }
  if (!empty(ptJson, "Jwt"))
  {
    strJwt = ptJson->m["Jwt"]->v;
  }
  else if (!empty(ptJson, "wsJwt"))
  {
    strJwt = ptJson->m["wsJwt"]->v;
  }
  if (!strJwt.empty())
  {
    string strPayload, strValue;
    Json *ptJwt = new Json;
    m_manip.decryptAes(m_manip.decodeBase64(strJwt, strValue), m_strJwtSecret, strPayload, strError);
    if (strPayload.empty())
    {
      strPayload = strJwt;
    }
    if (jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
    {
      if (!empty(ptJwt, "sl_admin") && ptJwt->m["sl_admin"]->v == "1")
      {
        d.g = true;
      }
      if (exist(ptJwt, "sl_auth"))
      {
        for (auto &auth : ptJwt->m["sl_auth"]->m)
        {
          d.auth[auth.first] = (auth.second->v == "1");
        }
      }
      if (!empty(ptJwt, "sl_login"))
      {
        d.u = ptJwt->m["sl_login"]->v;
      }
    }
    delete ptJwt;
  }
}
// }}}
}
}
