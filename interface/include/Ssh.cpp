// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Ssh.cpp
// author     : Ben Kietzman
// begin      : 2023-12-21
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Ssh"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Ssh()
Ssh::Ssh(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "ssh", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Ssh()
Ssh::~Ssh()
{
}
// }}}
// {{{ authenticate
// {{{ authenticateNone()
int Ssh::authenticateNone(ssh_session session)
{
  return ssh_userauth_none(session, NULL);
} 
// }}}
// {{{ authenticatePassword()
int Ssh::authenticatePassword(ssh_session session, const string strPassword)
{
  return ssh_userauth_password(session, NULL, strPassword.c_str());
} 
// }}}
// {{{ authenticateKbdint()
int Ssh::authenticateKbdint(ssh_session session, const string strPassword)
{   
  int nReturn; 
    
  while ((nReturn = ssh_userauth_kbdint(session, NULL, NULL)) == SSH_AUTH_INFO)
  {   
    int nPrompts = ssh_userauth_kbdint_getnprompts(session);
    for (int i = 0; i < nPrompts; i++)
    {   
      char cEcho;
      string strPrompt = ssh_userauth_kbdint_getprompt(session, i, &cEcho);
      if (strPrompt == "Password: " && ssh_userauth_kbdint_setanswer(session, i, strPassword.c_str()) < 0)
      {
        return SSH_AUTH_ERROR;
      }
    }
  }

  return nReturn;
}
// }}}
// }}}
// {{{ callback()
void Ssh::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Ssh::callback()";
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
        radialSsh *ptSsh = NULL;
        m_mutex.lock();
        if (m_sessions.find(ptJson->m["Session"]->v) != m_sessions.end())
        {
          ptSsh = m_sessions[ptJson->m["Session"]->v];
          ptSsh->bActive = true;
          time(&(ptSsh->CTime));
        }
        m_mutex.unlock();
        if (ptSsh != NULL)
        {
          if (!empty(ptJson, "Command"))
          {
            list<string> messages;
            if (transact(ptSsh, ptJson->m["Command"]->v, messages, strError))
            {
              bResult = true;
            }
            else if (ptSsh->fdSocket == -1)
            {
              m_mutex.lock();
              delete ptSsh;
              ptSsh = NULL;
              m_sessions.erase(ptJson->m["Session"]->v);
              m_mutex.unlock();
              delete ptJson->m["Session"];
              ptJson->m.erase("Session");
            }
            if (!messages.empty())
            {
              if (exist(ptJson, "Response"))
              {
                delete ptJson->m["Response"];
              }
              ptJson->m["Response"] = new Json;
              while (!messages.empty())
              {
                ptJson->m["Response"]->pb(messages.front());
                messages.pop_front();
              }
            }
          }
          else if (!empty(ptJson, "Function"))
          {
            if (ptJson->m["Function"]->v == "disconnect")
            {
              bResult = true;
              ssh_channel_free(ptSsh->channel);
              ssh_disconnect(ptSsh->session);
              ssh_free(ptSsh->session);
              m_mutex.lock();
              delete ptSsh;
              ptSsh = NULL;
              m_sessions.erase(ptJson->m["Session"]->v);
              m_mutex.unlock();
              delete ptJson->m["Session"];
              ptJson->m.erase("Session");
            }
            else
            {
              strError = "Please provide a valid Function:  disconnect.";
            }
          }
          else
          {
            strError = "Please provide the Command or Function.";
          }
          if (ptSsh != NULL)
          {
            ptSsh->bActive = false;
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
        ptLink->i("Interface", "ssh");
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
      if (!empty(ptJson, "Server"))
      {
        string strPort = "22";
        if (!empty(ptJson, "Port"))
        {
          strPort = ptJson->m["Port"]->v;
        }
        if (!empty(ptJson, "User"))
        {
          if (!empty(ptJson, "Password"))
          {
            radialSsh *ptSsh = new radialSsh;
            if ((ptSsh->session = ssh_new()) != NULL)
            {
              int nPort;
              stringstream ssPort(strPort);
              ssh_options_set(ptSsh->session, SSH_OPTIONS_HOST, ptJson->m["Server"]->v.c_str());
              ssPort >> nPort;
              ssh_options_set(ptSsh->session, SSH_OPTIONS_PORT, &nPort);
              ssh_options_set(ptSsh->session, SSH_OPTIONS_USER, ptJson->m["User"]->v.c_str());
              if (ssh_connect(ptSsh->session) == SSH_OK)
              {
                int nMethod;
                ptSsh->fdSocket = ssh_get_fd(ptSsh->session);
                ssh_userauth_none(ptSsh->session, NULL);
                nMethod = ssh_userauth_list(ptSsh->session, NULL);
                if ((nMethod & SSH_AUTH_METHOD_NONE && authenticateNone(ptSsh->session) == SSH_AUTH_SUCCESS) || (nMethod & SSH_AUTH_METHOD_INTERACTIVE && authenticateKbdint(ptSsh->session, ptJson->m["Password"]->v) == SSH_AUTH_SUCCESS) || (nMethod & SSH_AUTH_METHOD_PASSWORD && authenticatePassword(ptSsh->session, ptJson->m["Password"]->v) == SSH_AUTH_SUCCESS))
                {
                  if ((ptSsh->channel = ssh_channel_new(ptSsh->session)) != NULL)
                  {
                    if (ssh_channel_open_session(ptSsh->channel) == SSH_OK)
                    {
                      if (ssh_channel_request_pty(ptSsh->channel) == SSH_OK)
                      { 
                        if (ssh_channel_change_pty_size(ptSsh->channel, 80, 24) == SSH_OK)
                        {
                          if (ssh_channel_request_shell(ptSsh->channel) == SSH_OK)
                          {
                            list<string> messages;
                            if (transact(ptSsh, "", messages, strError))
                            {
                              stringstream ssSession;
                              bResult = true;
                              ssSession << m_strNode << "_" << getpid() << "_" << syscall(SYS_gettid) << "_" << ptSsh->fdSocket;
                              ptJson->i("Session", ssSession.str());
                              m_mutex.lock();
                              m_sessions[ssSession.str()] = ptSsh;
                              m_mutex.unlock();
                              if (!messages.empty())
                              {
                                if (exist(ptJson, "Response"))
                                {
                                  delete ptJson->m["Response"];
                                }
                                ptJson->m["Response"] = new Json;
                                while (!messages.empty())
                                {
                                  ptJson->m["Response"]->pb(messages.front());
                                  messages.pop_front();
                                }
                              }
                            }
                          }
                          else
                          {
                            strError = (string)"ssh_channel_request_shell() " + ssh_get_error(ptSsh->session);
                          }
                        }
                        else
                        {
                          strError = (string)"ssh_channel_change_pty_size() " + ssh_get_error(ptSsh->session);
                        }
                      }
                      else
                      {
                        strError = (string)"ssh_channel_request_pty() " + ssh_get_error(ptSsh->session);
                      }
                    }
                    else
                    {
                      strError = (string)"ssh_channel_open_session() " + ssh_get_error(ptSsh->session);
                    }
                    if (!bResult)
                    {
                      ssh_channel_free(ptSsh->channel);
                    }
                  }
                  else
                  {
                    strError = (string)"ssh_channel_new() " + ssh_get_error(ptSsh->session);
                  }
                }
                else
                {
                  strError = (string)"authenticate*() " + ssh_get_error(ptSsh->session);
                }
                if (!bResult)
                {
                  ssh_disconnect(ptSsh->session);
                }
              }
              else
              {
                strError = (string)"ssh_connect() " + ssh_get_error(ptSsh->session);
              }
              if (!bResult)
              {
                ssh_free(ptSsh->session);
              }
            }
            else
            {
              strError = "ssh_new() Failed to initialize SSH session.";
            }
            if (!bResult)
            {
              delete ptSsh;
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
void Ssh::schedule(string strPrefix)
{
  list<string> removals;
  time_t CTime[2];

  threadIncrement();
  strPrefix += "->Ssh::schedule()";
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
        ssh_channel_free(m_sessions[removals.front()]->channel);
        ssh_disconnect(m_sessions[removals.front()]->session);
        ssh_free(m_sessions[removals.front()]->session);
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
// {{{ transact()
bool Ssh::transact(radialSsh *ptSsh, const string strCommand, list<string> &messages, string &strError)
{
  bool bResult = false;

  if (ptSsh->fdSocket != -1)
  {
    bool bClose = false, bExit = false, bReading = true;
    int nReturn;
    char szBuffer[4096];
    size_t unPosition;
    string strBuffer[2];
    time_t CTime[2];
    if (!strCommand.empty())
    {
      bReading = false;
      strBuffer[1] = strCommand + "\n";
    }
    else
    {
      bResult = true;
    }
    while (!bExit)
    {
      pollfd fds[1];
      fds[0].fd = ptSsh->fdSocket;
      fds[0].events = POLLIN;
      if (!strBuffer[1].empty())
      {
        fds[0].events |= POLLOUT;
      }
      if ((nReturn = poll(fds, 1, 500)) > 0)
      {
        if (fds[0].revents & POLLIN)
        {
          if ((nReturn = ssh_channel_read_nonblocking(ptSsh->channel, szBuffer, 4096, 0)) > 0)
          {
            bReading = true;
            strBuffer[0].append(szBuffer, nReturn);
            while ((unPosition = strBuffer[0].find("\n")) != string::npos)
            {
              string strLine = strBuffer[0].substr(0, unPosition);
              stringstream ssLine;
              strBuffer[0].erase(0, (unPosition + 1));
              while (!strLine.empty())
              {
                if (strLine[0] == '\033' && strLine.size() > 1)
                {
                  if (strLine[1] == '[' && strLine.size() > 2)
                  {
                    char cEnd = '\0', cLast = '\0';
                    size_t unPosition; 
                    for (size_t i = 2; cEnd == '\0' && i < strLine.size(); i++)
                    {
                      unPosition = i;
                      if (isalpha(strLine[i]))
                      {
                        cEnd = strLine[i];
                      }
                      cLast = strLine[i];
                    }
                    if (cEnd != '\0' || cLast == ';')
                    {
                      strLine.erase(0, unPosition);
                    }
                  }
                }
                else if (strLine[0] != '\r')
                {
                  ssLine << strLine[0];
                }
                strLine.erase(0, 1);
              }
              messages.push_back(ssLine.str());
            }
          }
          else
          {
            bClose = bExit = true;
            if (strlen(ssh_get_error(ptSsh->session)) != 0)
            {
              bResult = false;
              strError = (string)"ssh_channel_read() " + ssh_get_error(ptSsh->session);
            }
          }
        }
        if (fds[0].revents & POLLOUT)
        {
          if ((nReturn = ssh_channel_write(ptSsh->channel, strBuffer[1].c_str(), strBuffer[1].size())) > 0)
          {
            strBuffer[1].erase(0, nReturn);
            if (strBuffer[1].empty())
            {
              bResult = true;
              time(&(CTime[0]));
            }
          }
          else
          {
            bClose = bExit = true;
            if (strlen(ssh_get_error(ptSsh->session)) != 0)
            {
              bResult = false;
              strError = (string)"ssh_channel_write() " + ssh_get_error(ptSsh->session);
            }
          }
        }
      }
      else if (nReturn < 0)
      {
        bClose = bExit = true;
        bResult = false;
        strError = (string)"poll() " + strerror(errno);
      }
      else if (bReading)
      {
        bExit = true;
      }
      else if (bResult)
      {
        time(&(CTime[1]));
        if ((CTime[1] - CTime[0]) > 60)
        {
          bClose = bExit = true;
          bResult = false;
          strError = "Command timed out after 60 seconds waiting for response.";
        }
      }
    }
    if (bClose)
    {
      ssh_channel_free(ptSsh->channel);
      ssh_disconnect(ptSsh->session);
      ssh_free(ptSsh->session);
      ptSsh->fdSocket = -1;
    }
  }
  else
  {
    strError = "SSH session already closed.";
  }

  return bResult;
}
// }}}
}
}
