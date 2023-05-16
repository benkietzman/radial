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
// {{{ process()
void Command::process(string strPrefix)
{
  // {{{ prep work
  bool bExit = false;
  int nReturn;
  list<list<radialCommand *>::iterator> removals;
  list<radialCommand *> commands;
  pollfd *fds;
  size_t unIndex, unPosition;
  string strError, strLine;
  stringstream ssMessage;

  strPrefix += "->Command::process()";
  m_pUtility->fdNonBlocking(0, strError);
  m_pUtility->fdNonBlocking(1, strError);
  // }}}
  while (!bExit)
  {
    // {{{ prep work
    fds = new pollfd[(commands.size() * 2) + 2];
    unIndex = 0;
    // {{{ stdin
    fds[unIndex].fd = 0;
    fds[unIndex].events = POLLIN;
    unIndex++;
    // }}}
    // {{{ stdout
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
    // }}}
    // {{{ commands
    for (auto &command : commands)
    {
      fds[unIndex].fd = command->fdRead;
      fds[unIndex].events = POLLIN;
      unIndex++;
      fds[unIndex].fd = command->fdWrite;
      fds[unIndex].events |= POLLOUT;
      unIndex++;
    }
    // }}}
    if (fds[0].fd != 0 || (fds[1].fd != -1 && fds[1].fd != 1))
    {
      bExit = true;
    }
    // }}}
    if (!bExit && (nReturn = poll(fds, unIndex, 500)) > 0)
    {
      // {{{ stdin
      if (fds[0].revents & (POLLHUP | POLLIN))
      {
        if (m_pUtility->fdRead(fds[0].fd, m_strBuffers[0], nReturn))
        {
          while ((unPosition = m_strBuffers[0].find("\n")) != string::npos)
          {
            Json *ptJson;
            strLine = m_strBuffers[0].substr(0, unPosition);
            m_strBuffers[0].erase(0, (unPosition + 1));
            ptJson = new Json(strLine);
            if (exist(ptJson, "_s") && ptJson->m["_s"]->v == "hub")
            {
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
                // {{{ invalid
                else
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << " error [stdin,hub," << ptJson->m["Function"]->v << "]:  Please provide a valid Function:  interfaces, shutdown.";
                  log(ssMessage.str());
                }
                // }}}
              }
              else
              {
                ssMessage.str("");
                ssMessage << strPrefix << " error [stdin,hub]:  Please provide a Function.";
                log(ssMessage.str());
              }
            }
            else if (!empty(ptJson, "Command"))
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
                if (pipe(writepipe) == 0)
                {
                  if ((execPid = fork()) == 0)
                  {
                    close(readpipe[0]);
                    close(writepipe[1]);
                    dup2(writepipe[0], 0);
                    close(writepipe[0]);
                    dup2(readpipe[1], 1);
                    close(readpipe[1]);
                    execve(args[0], args, environ);
                    if (!empty(ptJson, "Format") && ptJson->m["Format"]->v == "json")
                    {
                      string strOut;
                      Json *ptOut = new Json;
                      ptOut->i("Status", "error");
                      ssMessage.str("");
                      ssMessage << "execve(" << errno << ") " << strerror(errno);
                      ptOut->i("Error", ssMessage.str());
                      ptOut->j(strOut);
                      write(1, strOut.c_str(), strOut.size());
                    }
                    else
                    {
                      ssMessage.str("");
                      ssMessage << "execve(" << errno << ") " << strerror(errno);
                      write(1, ssMessage.str().c_str(), ssMessage.str().size());
                    }
                    _exit(1);
                  }
                  else if (execPid > 0)
                  {
                    radialCommand *ptCommand = new radialCommand;
                    long lArg;
                    close(writepipe[0]);
                    close(readpipe[1]);
                    ptCommand->bJson = false;
                    ptCommand->bProcessed = false;
                    ptCommand->fdRead = readpipe[0];
                    ptCommand->fdWrite = writepipe[1];
                    if ((lArg = fcntl(ptCommand->fdRead, F_GETFL, NULL)) >= 0)
                    {
                      lArg |= O_NONBLOCK;
                      fcntl(ptCommand->fdRead, F_SETFL, lArg);
                    }
                    if ((lArg = fcntl(ptCommand->fdWrite, F_GETFL, NULL)) >= 0)
                    {
                      lArg |= O_NONBLOCK;
                      fcntl(ptCommand->fdWrite, F_SETFL, lArg);
                    }
                    if (!empty(ptJson, "Format") && ptJson->m["Format"]->v == "json")
                    {
                      ptCommand->bJson = true;
                    }
                    if (exist(ptJson, "Input"))
                    {
                      if (ptCommand->bJson)
                      {
                        ptJson->m["Input"]->j(ptCommand->strBuffer[1]);
                      }
                      else
                      {
                        ptCommand->strBuffer[1] = ptJson->m["Input"]->v;
                      }
                    }
                    if (ptCommand->strBuffer[1].empty())
                    {
                      close(ptCommand->fdWrite);
                      ptCommand->fdWrite = -1;
                    }
                    clock_gettime(CLOCK_REALTIME, &(ptCommand->start));
                    ptCommand->ptJson = new Json(ptJson);
                    commands.push_back(ptCommand);
                  }
                  else
                  {
                    ssMessage.str("");
                    ssMessage << "fork(" << errno << ") " << strerror(errno);
                    strError = ssMessage.str();
                    ptJson->i("Status", "error");
                    ptJson->i("Error", strError);
                    hub(ptJson, false);
                  }
                }
                else
                {
                  ssMessage.str("");
                  ssMessage << "pipe(" << errno << ") " << strerror(errno);
                  strError = ssMessage.str();
                  ptJson->i("Status", "error");
                  ptJson->i("Error", strError);
                  hub(ptJson, false);
                }
              }
              else
              {
                ssMessage.str("");
                ssMessage << "pipe(" << errno << ") " << strerror(errno);
                strError = ssMessage.str();
                ptJson->i("Status", "error");
                ptJson->i("Error", strError);
                hub(ptJson, false);
              }
            }
            else
            {
              ptJson->i("Status", "error");
              ptJson->i("Error", "Please provide the Command.");
              hub(ptJson, false);
            }
            delete ptJson;
          }
        }
        else
        {
          bExit = true;
          if (nReturn < 0 && errno != 104)
          {
            ssMessage.str("");
            ssMessage << strPrefix << "->Utility::fdRead(" << errno << ") [stdin," << fds[0].fd << "]:  " << strerror(errno);
            log(ssMessage.str());
          }
        }
      }
      // }}}
      // {{{ stdout
      if (fds[1].revents & POLLOUT)
      {
        if (!m_pUtility->fdWrite(fds[1].fd, m_strBuffers[1], nReturn))
        {
          bExit = true;
          if (nReturn < 0)
          {
            ssMessage.str("");
            ssMessage << strPrefix << "->Utility::fdWrite(" << errno << ") [stdout," << fds[1].fd << "]:  " << strerror(errno);
            log(ssMessage.str());
          }
        }
      }
      // }}}
      // {{{ commands
      for (size_t i = 2; i < unIndex; i++)
      {
        bool bRemoved = false;
        for (auto j = commands.begin(); j != commands.end(); j++)
        {
          if (fds[i].fd == (*j)->fdRead)
          {
            if (fds[i].revents & (POLLHUP | POLLIN))
            {
              if (!m_pUtility->fdRead((*j)->fdRead, (*j)->strBuffer[0], nReturn))
              {
                if (!bRemoved)
                {
                  removals.push_back(j);
                }
                if (nReturn == 0)
                {
                  (*j)->bProcessed = true;
                }
                else
                {
                  ssMessage.str("");
                  ssMessage << "Utility::fdRead(" << errno << ") " << strerror(errno);
                  (*j)->strError = ssMessage.str();
                }
              }
            }
          }
          else if (fds[i].fd == (*j)->fdWrite)
          {
            if (fds[i].revents & POLLOUT)
            {
              if (m_pUtility->fdWrite((*j)->fdWrite, (*j)->strBuffer[1], nReturn))
              {
                if ((*j)->strBuffer[1].empty())
                {
                  close((*j)->fdWrite);
                  (*j)->fdWrite = -1;
                }
              }
              else
              {
                if (!bRemoved)
                {
                  removals.push_back(j);
                }
                if (nReturn < 0)
                {
                  ssMessage.str("");
                  ssMessage << "Utility::fdWrite(" << errno << ") " << strerror(errno);
                  (*j)->strError = ssMessage.str();
                }
              }
            }
          }
          clock_gettime(CLOCK_REALTIME, &((*j)->stop));
          if (!bRemoved && ((*j)->stop.tv_sec - (*j)->start.tv_sec) > 1800)
          {
            removals.push_back(j);
          }
        }
      }
      // }}}
    }
    else if (!bExit && nReturn < 0 && errno != EINTR)
    {
      bExit = true;
      ssMessage.str("");
      ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
      notify(ssMessage.str());
    }
    // {{{ post work
    delete[] fds;
    for (auto &i : removals)
    {
      pid_t retWait;
      size_t unDuration = (((*i)->stop.tv_sec - (*i)->start.tv_sec) * 1000) + (((*i)->stop.tv_nsec - (*i)->start.tv_nsec) / 1000000);
      stringstream ssDuration;
      close((*i)->fdRead);
      if ((*i)->fdWrite != -1)
      {
        close((*i)->fdWrite);
      }
      ssDuration << unDuration;
      (*i)->ptJson->insert("Duration", ssDuration.str(), 'n');
      if ((*i)->bJson)
      {
        Json *ptOutput = new Json((*i)->strBuffer[0]);
        (*i)->ptJson->i("Output", ptOutput);
        delete ptOutput;
      }
      else
      {
        (*i)->ptJson->i("Output", (*i)->strBuffer[0]);
      }
      if ((retWait = waitpid((*i)->execPid, NULL, WNOHANG)) == 0)
      {
        size_t unAttempts = 0;
        kill((*i)->execPid, SIGTERM);
        while ((retWait = waitpid((*i)->execPid, NULL, WNOHANG)) == 0 && unAttempts++ < 5)
        {
          msleep(1000);
        }
        if (retWait == 0)
        {
          kill((*i)->execPid, SIGKILL);
        }
        (*i)->strError = "Terminated the child process due to crossing the 30 minute timeout.";
      }
      (*i)->ptJson->i("Status", (((*i)->bProcessed)?"okay":"error"));
      if (!(*i)->strError.empty())
      {
        (*i)->ptJson->i("Error", (*i)->strError);
      }
      hub((*i)->ptJson, false);
      delete (*i)->ptJson;
      delete *i;
      commands.erase(i);
    }
    removals.clear();
    if (shutdown())
    {
      bExit = true;
    }
    // }}}
  }
  // {{{ post work
  for (auto &i : commands)
  {
    close(i->fdRead);
    if (i->fdWrite != -1)
    {
      close(i->fdWrite);
    }
    if (waitpid(i->execPid, NULL, WNOHANG) == 0)
    {
      kill(i->execPid, SIGKILL);
    }
    delete i->ptJson;
    delete i;
  }
  setShutdown();
  // }}}
}
// }}}
}
}
