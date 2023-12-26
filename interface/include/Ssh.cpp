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
          if (!empty(ptJson, "Function"))
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
            else if (ptJson->m["Function"]->v == "send")
            {
              string strData, strRequest;
              if (!empty(ptJson, "Request"))
              {
                strRequest = ptJson->m["Request"]->v;
              }
              if (transact(ptSsh, ptJson->m["Request"]->v, strData, strError))
              {
                bResult = true;
              }
              if (ptSsh->fdSocket == -1)
              {
                m_mutex.lock();
                delete ptSsh;
                ptSsh = NULL;
                m_sessions.erase(ptJson->m["Session"]->v);
                m_mutex.unlock();
                delete ptJson->m["Session"];
                ptJson->m.erase("Session");
              }
              if (!strData.empty())
              {
                ptJson->i("Response", strData);
              }
            }
            else
            {
              strError = "Please provide a valid Function:  disconnect, send.";
            }
          }
          else
          {
            strError = "Please provide the Function.";
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
      if (exist(ptJson, "Request"))
      {
        if (!empty(ptJson->m["Request"], "Server"))
        {
          string strPort = "22";
          if (!empty(ptJson->m["Request"], "Port"))
          {
            strPort = ptJson->m["Request"]->m["Port"]->v;
          }
          if (!empty(ptJson->m["Request"], "User"))
          {
            if (!empty(ptJson->m["Request"], "Password"))
            {
              radialSsh *ptSsh = new radialSsh;
              if ((ptSsh->session = ssh_new()) != NULL)
              {
                int nPort;
                stringstream ssPort(strPort);
                ssh_options_set(ptSsh->session, SSH_OPTIONS_HOST, ptJson->m["Request"]->m["Server"]->v.c_str());
                ssPort >> nPort;
                ssh_options_set(ptSsh->session, SSH_OPTIONS_PORT, &nPort);
                ssh_options_set(ptSsh->session, SSH_OPTIONS_USER, ptJson->m["Request"]->m["User"]->v.c_str());
                if (ssh_connect(ptSsh->session) == SSH_OK)
                {
                  int nMethod;
                  ptSsh->fdSocket = ssh_get_fd(ptSsh->session);
                  ssh_userauth_none(ptSsh->session, NULL);
                  nMethod = ssh_userauth_list(ptSsh->session, NULL);
                  if ((nMethod & SSH_AUTH_METHOD_NONE && authenticateNone(ptSsh->session) == SSH_AUTH_SUCCESS) || (nMethod & SSH_AUTH_METHOD_INTERACTIVE && authenticateKbdint(ptSsh->session, ptJson->m["Request"]->m["Password"]->v) == SSH_AUTH_SUCCESS) || (nMethod & SSH_AUTH_METHOD_PASSWORD && authenticatePassword(ptSsh->session, ptJson->m["Request"]->m["Password"]->v) == SSH_AUTH_SUCCESS))
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
                              string strData;
                              if (transact(ptSsh, "", strData, strError))
                              {
                                stringstream ssSession;
                                bResult = true;
                                ssSession << m_strNode << "_" << getpid() << "_" << syscall(SYS_gettid) << "_" << ptSsh->fdSocket;
                                ptJson->i("Session", ssSession.str());
                                m_mutex.lock();
                                m_sessions[ssSession.str()] = ptSsh;
                                m_mutex.unlock();
                                if (!strData.empty())
                                {
                                  ptJson->i("Response", strData);
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
              strError = "Please provide the Password within the Request.";
            }
          }
          else
          {
            strError = "Please provide the User within the Request.";
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
bool Ssh::transact(radialSsh *ptSsh, const string strCommand, string &strData, string &strError)
{
  bool bResult = false;

  strData.clear();
  if (ptSsh->fdSocket != -1)
  {
    bool bClose = false, bExit = false, bReading = true;
    int nReturn;
    char szBuffer[4096];
    string strBuffer;
    time_t CTime[2];
    if (!strCommand.empty())
    {
      bReading = false;
      strBuffer = strCommand;
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
      if (!strBuffer.empty())
      {
        fds[0].events |= POLLOUT;
      }
      if ((nReturn = poll(fds, 1, 100)) > 0)
      {
        if (fds[0].revents & POLLNVAL)
        {
          bExit = true;
          strError = (string)"poll() Invalid socket.";
        }
        if (fds[0].revents & POLLIN)
        {
          if ((nReturn = ssh_channel_read_nonblocking(ptSsh->channel, szBuffer, 4096, 0)) > 0)
          {
            bReading = true;
            strData.append(szBuffer, nReturn);
            if (strData.size() > 1048576)
            {
              bExit = true;
            }
          }
          else if (nReturn < 0 || ssh_channel_is_eof(ptSsh->channel))
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
          if ((nReturn = ssh_channel_write(ptSsh->channel, strBuffer.c_str(), strBuffer.size())) > 0)
          {
            strBuffer.erase(0, nReturn);
            if (strBuffer.empty())
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
        if ((CTime[1] - CTime[0]) > 5)
        {
          bClose = bExit = true;
          bResult = false;
          strError = "Command timed out after five seconds waiting for response.";
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
