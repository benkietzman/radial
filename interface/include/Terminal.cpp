// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Terminal.cpp
// author     : Ben Kietzman
// begin      : 2023-12-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Terminal"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Terminal()
Terminal::Terminal(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "terminal", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Terminal()
Terminal::~Terminal()
{
}
// }}}
// {{{ callback()
void Terminal::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Terminal::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Session"))
  {
    string strNode;
    stringstream ssSession(ptJson->m["Session"]->v);
    getline(ssSession, strNode, '_');
    if (!strNode.empty())
    {
      if (strNode == m_strNode)
      {
        radialTerminal *t = NULL;
        m_mutex.lock();
        if (m_sessions.find(ptJson->m["Session"]->v) != m_sessions.end())
        {
          t = m_sessions[ptJson->m["Session"]->v];
          t->bActive = true;
          time(&(t->CTime));
        }
        m_mutex.unlock();
        if (t != NULL)
        {
          if (!empty(ptJson, "Function"))
          {
            bool bScreen = false;
            string strInvalid = "Please provide a valid Function:  disconnect, getSocketTimeout, screen, send, sendCtrl, sendDown, sendEnter, sendEscape, sendFunction, sendHome, sendKey, sendKeypadEnter, sendLeft, sendRight, sendShiftFunction, sendTab, sendUp, sendWait, setSocketTimeout, wait.";
            if (exist(ptJson, "Response"))
            {
              delete ptJson->m["Response"];
            }
            ptJson->m["Response"] = new Json;
            if (exist(ptJson, "Request") && !empty(ptJson->m["Request"], "Screen") && (ptJson->m["Request"]->m["Screen"]->t == '1' || ptJson->m["Request"]->m["Screen"]->v == "yes"))
            {
              bScreen = true;
            }
            t->m.lock();
            // {{{ disconnect
            if (ptJson->m["Function"]->v == "disconnect")
            {
              bResult = true;
              t->t.disconnect();
              m_mutex.lock();
              delete t;
              t = NULL;
              m_sessions.erase(ptJson->m["Session"]->v);
              m_mutex.unlock();
              delete ptJson->m["Session"];
              ptJson->m.erase("Session");
            }
            // }}}
            // {{{ getSocketTimeout
            else if (ptJson->m["Function"]->v == "getSocketTimeout")
            {
              int nLong, nShort;
              stringstream ssLong, ssShort;
              bResult = true;
              t->t.getSocketTimeout(nShort, nLong);
              ssLong << nLong;
              ssShort << nShort;
              ptJson->m["Response"]->insert("Long", ssLong.str(), 'n');
              ptJson->m["Response"]->insert("Short", ssShort.str(), 'n');
            }
            // }}}
            // {{{ screen
            else if (ptJson->m["Function"]->v == "screen")
            {
              bResult = bScreen = true;
            }
            // }}}
            // {{{ send*
            else if (ptJson->m["Function"]->v.size() >= 4 && ptJson->m["Function"]->v.substr(0, 4) == "send")
            {
              bool bWait = false;
              size_t unCount = 1;
              string strData;
              if (exist(ptJson, "Request"))
              {
                if (!empty(ptJson->m["Request"], "Data"))
                {
                  strData = ptJson->m["Request"]->m["Data"]->v;
                }
                if (!empty(ptJson->m["Request"], "Count"))
                {
                  stringstream ssCount(ptJson->m["Request"]->m["Count"]->v);
                  ssCount >> unCount;
                }
                if (!empty(ptJson->m["Request"], "Wait") && (ptJson->m["Request"]->t == '1' || ptJson->m["Request"]->m["Wait"]->v == "yes"))
                {
                  bWait = true;
                }
              }
              // {{{ send
              if (ptJson->m["Function"]->v == "send")
              {
                if (t->t.send(strData, unCount))
                {
                  bResult = true;
                }
                else
                {
                  strError = t->t.error();
                }
              }
              // }}}
              // {{{ sendCtrl
              else if (ptJson->m["Function"]->v == "sendCtrl")
              {
                if (strData.size() == 1)
                {
                  if (t->t.sendCtrl(strData[0], bWait))
                  {
                    bResult = true;
                  }
                  else
                  {
                    strError = t->t.error();
                  }
                }
                else
                {
                  strError = "Data should contain a single character for this Function.";
                }
              }
              // }}}
              // {{{ sendDown
              else if (ptJson->m["Function"]->v == "sendDown")
              {
                if (t->t.sendDown(unCount, bWait))
                {
                  bResult = true;
                }
                else
                {
                  strError = t->t.error();
                }
              }
              // }}}
              // {{{ sendEnter
              else if (ptJson->m["Function"]->v == "sendEnter")
              {
                if (t->t.sendEnter(bWait))
                {
                  bResult = true;
                }
                else
                {
                  strError = t->t.error();
                }
              }
              // }}}
              // {{{ sendEscape
              else if (ptJson->m["Function"]->v == "sendEscape")
              {
                if (t->t.sendEscape(bWait))
                {
                  bResult = true;
                }
                else
                {
                  strError = t->t.error();
                }
              }
              // }}}
              // {{{ sendFunction
              else if (ptJson->m["Function"]->v == "sendFunction")
              {
                stringstream ssKey(strData);
                int nKey = 0;
                ssKey >> nKey;
                if (nKey >= 1 && nKey <= 12)
                {
                  if (t->t.sendFunction(nKey))
                  {
                    bResult = true;
                  }
                  else
                  {
                    strError = t->t.error();
                  }
                }
                else
                {
                  strError = "Please provide a Data value between 1 and 12.";
                }
              }
              // }}}
              // {{{ sendHome
              else if (ptJson->m["Function"]->v == "sendHome")
              {
                if (t->t.sendHome(bWait))
                {
                  bResult = true;
                }
                else
                {
                  strError = t->t.error();
                }
              }
              // }}}
              // {{{ sendKey
              else if (ptJson->m["Function"]->v == "sendKey")
              {
                if (strData.size() == 1)
                {
                  if (t->t.sendKey(strData[0], unCount, bWait))
                  {
                    bResult = true;
                  }
                  else
                  {
                    strError = t->t.error();
                  }
                }
                else
                {
                  strError = "Data should contain a single character for this Function.";
                }
              }
              // }}}
              // {{{ sendKeypadEnter
              else if (ptJson->m["Function"]->v == "sendKeypadEnter")
              {
                if (t->t.sendKeypadEnter(bWait))
                {
                  bResult = true;
                }
                else
                {
                  strError = t->t.error();
                }
              }
              // }}}
              // {{{ sendLeft
              else if (ptJson->m["Function"]->v == "sendLeft")
              {
                if (t->t.sendLeft(unCount, bWait))
                {
                  bResult = true;
                }
                else
                {
                  strError = t->t.error();
                }
              }
              // }}}
              // {{{ sendRight
              else if (ptJson->m["Function"]->v == "sendRight")
              {
                if (t->t.sendRight(unCount, bWait))
                {
                  bResult = true;
                }
                else
                {
                  strError = t->t.error();
                }
              }
              // }}}
              // {{{ sendShiftFunction
              else if (ptJson->m["Function"]->v == "sendShiftFunction")
              {
                stringstream ssKey(strData);
                int nKey = 0;
                ssKey >> nKey;
                if (nKey >= 1 && nKey <= 12)
                {
                  if (t->t.sendShiftFunction(nKey))
                  {
                    bResult = true;
                  }
                  else
                  {
                    strError = t->t.error();
                  }
                }
                else
                {
                  strError = "Please provide a Data value between 1 and 12.";
                }
              }
              // }}}
              // {{{ sendTab
              else if (ptJson->m["Function"]->v == "sendTab")
              {
                if (t->t.sendTab(unCount, bWait))
                {
                  bResult = true;
                }
                else
                {
                  strError = t->t.error();
                }
              }
              // }}}
              // {{{ sendUp
              else if (ptJson->m["Function"]->v == "sendUp")
              {
                if (t->t.sendUp(unCount, bWait))
                {
                  bResult = true;
                }
                else
                {
                  strError = t->t.error();
                }
              }
              // }}}
              // {{{ sendWait
              else if (ptJson->m["Function"]->v == "sendWait")
              {
                if (t->t.sendWait(strData, unCount))
                {
                  bResult = true;
                }
                else
                {
                  strError = t->t.error();
                }
              }
              // }}}
              // {{{ invalid
              else
              {
                strError = strInvalid;
              }
              // }}}
            }
            // }}}
            // {{{ setSocketTimeout
            else if (ptJson->m["Function"]->v == "setSocketTimeout")
            {
              if (exist(ptJson, "Request"))
              {
                if (!empty(ptJson->m["Request"], "Long"))
                {
                  int nLong;
                  stringstream ssLong(ptJson->m["Request"]->m["Long"]->v);
                  ssLong >> nLong;
                  if (!empty(ptJson->m["Request"], "Short"))
                  {
                    int nShort;
                    stringstream ssShort(ptJson->m["Request"]->m["Short"]->v);
                    ssShort >> nShort;
                    bResult = true;
                    t->t.setSocketTimeout(nShort, nLong);
                  }
                  else
                  {
                    strError = "Please provide the Short within the Request.";
                  }
                }
                else
                {
                  strError = "Please provide the Long within the Request.";
                }
              }
              else
              {
                strError = "Please provide the Request.";
              }
            }
            // }}}
            // {{{ wait
            else if (ptJson->m["Function"]->v == "wait")
            {
              bool bWait = false;
              if (exist(ptJson, "Request") && !empty(ptJson->m["Request"], "Wait") && (ptJson->m["Request"]->m["Wait"]->t == '1' || ptJson->m["Request"]->m["Wait"]->v == "yes"))
              {
                bWait = true;
              }
              if (t->t.wait(bWait))
              {
                bResult = true;
              }
              else
              {
                strError = t->t.error();
              }
            }
            // }}}
            // {{{ invalid
            else
            {
              strError = strInvalid;
            }
            // }}}
            if (bScreen)
            {
              stringstream ssValue;
              vector<string> screen;
              t->t.screen(screen);
              ptJson->m["Response"]->m["Screen"] = new Json;
              for (size_t i = 0; i < screen.size(); i++)
              {
                ptJson->m["Response"]->m["Screen"]->pb(screen[i]);
              }
              ssValue.str("");
              ssValue << t->t.col();
              ptJson->m["Response"]->insert("Col", ssValue.str(), 'n');
              ssValue.str("");
              ssValue << t->t.cols();
              ptJson->m["Response"]->insert("Cols", ssValue.str(), 'n');
              ssValue.str("");
              ssValue << t->t.row();
              ptJson->m["Response"]->insert("Row", ssValue.str(), 'n');
              ssValue.str("");
              ssValue << t->t.rows();
              ptJson->m["Response"]->insert("Rows", ssValue.str(), 'n');
            }
            t->m.unlock();
          }
          else
          {
            strError = "Please provide the Function.";
          }
          if (t != NULL)
          {
            t->bActive = false;
          }
        }
        else
        {
          strError = "Please provide a valid Session.";
        }
      }
      else
      {
        Json *ptLink = new Json(ptJson);
        ptLink->i("Interface", "terminal");
        ptLink->i("Node", strNode);
        if (hub("link", ptLink, strError))
        {
          bResult = true;
          if (exist(ptLink, "Response"))
          {
            ptJson->i("Response", ptLink->m["Response"]);
          }
        }
        delete ptLink;
      }
    }
    else
    {
      strError = "Please provide a valid Session.";
    }
  }
  else if (!empty(ptJson, "Function"))
  {
    if (ptJson->m["Function"]->v == "connect")
    {
      if (exist(ptJson, "Request"))
      {
        if (!empty(ptJson->m["Request"], "Server"))
        {
          if (!empty(ptJson->m["Request"], "Port"))
          {
            radialTerminal *t = new radialTerminal;
            if (!empty(ptJson->m["Request"], "Cols"))
            {
              size_t unCols;
              stringstream ssCols(ptJson->m["Request"]->m["Cols"]->v);
              ssCols >> unCols;
              t->t.cols(unCols);
            }
            if (!empty(ptJson->m["Request"], "Rows"))
            {
              size_t unRows;
              stringstream ssRows(ptJson->m["Request"]->m["Rows"]->v);
              ssRows >> unRows;
              t->t.rows(unRows);
            }
            if (!empty(ptJson->m["Request"], "Type"))
            {
              t->t.type(ptJson->m["Request"]->m["Type"]->v);
            }
            if (t->t.connect(ptJson->m["Request"]->m["Server"]->v, ptJson->m["Request"]->m["Port"]->v))
            {
              stringstream ssSession;
              bResult = true;
              ssSession << m_strNode << "_" << getpid() << "_" << syscall(SYS_gettid) << "_" << t;
              ptJson->i("Session", ssSession.str());
              m_mutex.lock();
              m_sessions[ssSession.str()] = t;
              m_mutex.unlock();
              if (!empty(ptJson->m["Request"], "Screen") && (ptJson->m["Request"]->m["Screen"]->t == '1' || ptJson->m["Request"]->m["Screen"]->v == "yes"))
              {
                stringstream ssValue;
                vector<string> screen;
                t->t.screen(screen);
                if (exist(ptJson, "Response"))
                {
                  delete ptJson->m["Response"];
                }
                ptJson->m["Response"] = new Json;
                ptJson->m["Response"]->m["Screen"] = new Json;
                for (size_t i = 0; i < screen.size(); i++)
                {
                  ptJson->m["Response"]->m["Screen"]->pb(screen[i]);
                }
                ssValue.str("");
                ssValue << t->t.col();
                ptJson->m["Response"]->insert("Col", ssValue.str(), 'n');
                ssValue.str("");
                ssValue << t->t.cols();
                ptJson->m["Response"]->insert("Cols", ssValue.str(), 'n');
                ssValue.str("");
                ssValue << t->t.row();
                ptJson->m["Response"]->insert("Row", ssValue.str(), 'n');
                ssValue.str("");
                ssValue << t->t.rows();
                ptJson->m["Response"]->insert("Rows", ssValue.str(), 'n');
              }
            }
            else
            {
              strError = t->t.error();
            }
            if (!bResult)
            {
              delete t;
            }
          }
          else
          {
            strError = "Please provide the Port within the Request.";
          }
        }
        else
        {
          strError = "Please provide the Server within the Request.";
        }
      }
      else
      {
        strError = "Please provide the Request.";
      }
    }
    else
    {
      strError = "Please provide a valid Function:  connect.";
    }
  }
  else
  {
    strError = "Please provide the Function or Session.";
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
  threadDecrement();
}
// }}}
// {{{ schedule()
void Terminal::schedule(string strPrefix)
{
  list<string> removals;
  time_t CTime[2];

  threadIncrement();
  strPrefix += "->Terminal::schedule()";
  time(&(CTime[0]));
  while (!shutdown())
  {
    time(&(CTime[1]));
    if ((CTime[1] - CTime[0]) > 300)
    {
      CTime[0] = CTime[1];
      m_mutex.lock();
      for (auto &session : m_sessions)
      {
        if (!session.second->bActive && (CTime[1] - session.second->CTime) > 300)
        {
          removals.push_back(session.first);
        }
      }
      while (!removals.empty())
      {
        m_sessions[removals.front()]->m.lock();
        m_sessions[removals.front()]->t.disconnect();
        m_sessions[removals.front()]->m.unlock();
        delete m_sessions[removals.front()];
        removals.pop_front();
      }
      m_mutex.unlock();
    }
    msleep(1000);
  }
  threadDecrement();
}
// }}}
}
}