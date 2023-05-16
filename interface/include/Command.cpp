// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Command.cpp
// author     : Ben Kietzman
// begin      : 2023-05-16
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Command"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Command()
Command::Command(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "command", argc, argv, pCallback)
{
}
// }}}
// {{{ ~Command()
Command::~Command()
{
}
// }}}
// {{{ callback()
void Command::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;

  threadIncrement();
  strPrefix += "->Command::callback()";
  if (!empty(ptJson, "Command"))
  {
    char *args[100], *pszArgument;
    int readpipe[2] = {-1, -1}, writepipe[2] = {-1, -1};
    pid_t execPid;
    size_t unIndex = 0;
    string strArgument;
    stringstream ssCommand;
    ssCommand.str(ptJson->m["Command"]->v);
    while (ssCommand >> strArgument)
    {
      pszArgument = new char[strArgument.size() + 1];
      strcpy(pszArgument, strArgument.c_str());
      args[unIndex++] = pszArgument;
    }
    if (exist(ptJson, "Arguments"))
    {
      for (auto &i : ptJson->m["Arguments"]->l)
      {
        if (!i->v.empty())
        {
          pszArgument = new char[i->v.size() + 1];
          strcpy(pszArgument, i->v.c_str());
          args[unIndex++] = pszArgument;
        }
      }
    }
    args[unIndex] = NULL;
    if (pipe(readpipe) == 0)
    {
ssMessage.str("");
ssMessage << strPrefix << "->pipe():  Created readpipe.";
log(ssMessage.str());
      if (pipe(writepipe) == 0)
      {
ssMessage.str("");
ssMessage << strPrefix << "->pipe():  Created writepipe.";
log(ssMessage.str());
        if ((execPid = fork()) == 0)
        {
          close(readpipe[0]);
          close(writepipe[1]);
          dup2(writepipe[0], 0);
          close(writepipe[0]);
          dup2(readpipe[1], 1);
          close(readpipe[1]);
          execve(args[0], args, environ);
          _exit(1);
        }
        else if (execPid > 0)
        {
          bool bExit = false, bKill = false;
          int nReturn;
          long lArg;
          size_t unDuration;
          string strBuffer[2];
          stringstream ssDuration;
          timespec start, stop;
ssMessage.str("");
ssMessage << strPrefix << "->fork():  Forked process.";
log(ssMessage.str());
          bResult = true;
          close(writepipe[0]);
          close(readpipe[1]);
          if ((lArg = fcntl(readpipe[0], F_GETFL, NULL)) >= 0)
          {
            lArg |= O_NONBLOCK;
            fcntl(readpipe[0], F_SETFL, lArg);
          }
          if ((lArg = fcntl(writepipe[1], F_GETFL, NULL)) >= 0)
          {
            lArg |= O_NONBLOCK;
            fcntl(writepipe[1], F_SETFL, lArg);
          }
          if (!empty(ptJson, "Input"))
          {
            strBuffer[1] = ptJson->m["Input"]->v;
          }
          clock_gettime(CLOCK_REALTIME, &start);
          while (!bExit)
          {
            pollfd fds[2];
            fds[0].fd = readpipe[0];
            fds[0].events = POLLIN;
            fds[1].fd = -1;
            if (!strBuffer[1].empty())
            {
              fds[1].fd = writepipe[1];
              fds[1].events = POLLOUT;
            }
            if ((nReturn = poll(fds, 2, 500)) > 0)
            {
              if (fds[0].revents & (POLLHUP | POLLIN))
              {
                if (!m_pUtility->fdRead(readpipe[0], strBuffer[0], nReturn))
                {
                  bExit = true;
                  if (nReturn < 0)
                  {
                    ssMessage.str("");
                    ssMessage << "read(" << errno << ") " << strerror(errno);
                    strError = ssMessage.str();
                  }
                }
              }
              if (fds[1].revents & POLLOUT)
              {
                if (!m_pUtility->fdWrite(writepipe[1], strBuffer[1], nReturn))
                {
                  bExit = true;
                  if (nReturn < 0)
                  {
                    ssMessage.str("");
                    ssMessage << "write(" << errno << ") " << strerror(errno);
                    strError = ssMessage.str();
                  }
                }
              }
            }
            else if (nReturn < 0)
            {
              bExit = true;
              ssMessage.str("");
              ssMessage << "poll(" << errno << ") " << strerror(errno);
              strError = ssMessage.str();
            }
            clock_gettime(CLOCK_REALTIME, &stop);
            if ((stop.tv_sec - start.tv_sec) > 1800)
            {
              bExit = true;
            }
          }
          close(readpipe[0]);
          close(writepipe[1]);
ssMessage.str("");
ssMessage << strPrefix << ":  Completed command.";
log(ssMessage.str());
          unDuration = ((stop.tv_sec - start.tv_sec) * 1000) + ((stop.tv_nsec - start.tv_nsec) / 1000000);
          ssDuration << unDuration;
          ptJson->insert("Duration", ssDuration.str(), 'n');
          ptJson->i("Output", strBuffer[0]);
          if (bKill)
          {
            pid_t retWait;
            size_t unAttempts = 0;
            kill(execPid, SIGTERM);
            while ((retWait = waitpid(execPid, NULL, WNOHANG)) == 0 && unAttempts++ < 10)
            {
              msleep(1000);
            }
            if (retWait == 0)
            {
              kill(execPid, SIGKILL);
            }
            strError = "Terminated the child process due to crossing the 30 minute timeout.";
          }
        }
        else
        {
          ssMessage.str("");
          ssMessage << "fork(" << errno << ") " << strerror(errno);
          strError = ssMessage.str();
        }
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
      ssMessage.str("");
      ssMessage << "pipe(" << errno << ") " << strerror(errno);
      strError = ssMessage.str();
    }
  }
  else
  {
    strError = "Please provide the Command.";
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
}
}
