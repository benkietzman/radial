// -*- C++ -*-
// Emulator
// -------------------------------------
// file       : emulator.cpp
// author     : Ben Kietzman
// begin      : 2026-02-05
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include <cerrno>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <poll.h>
#include <string>
using namespace std;
#include <Json>
#include <Radial>
#include <SignalHandling>
#include <StringManip>
#include <Warden>
using namespace common;
// }}}
// {{{ defines
#define PARENT_READ  readpipe[0]
#define CHILD_WRITE  readpipe[1]
#define CHILD_READ   writepipe[0]
#define PARENT_WRITE writepipe[1]
// }}}
// {{{ structs
struct command
{
  int r;
  int w;
  string b[2];
};
// }}}
// {{{ global variables
bool gbShutdown = false;
// }}}
// {{{ prototypes
void shutdown();
void sighandle(const int nSignal);
// }}}
// {{{ main()
int main(int argc, char *argv[])
{
  string e, p = "main()", strPassword, strUser;
  Radial radial(e);
  StringManip manip;
  Warden warden("Emulator", "/data/warden/socket", e);

  sethandles(sighandle);
  signal(SIGBUS, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGCONT, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGSEGV, SIG_IGN);
  signal(SIGWINCH, SIG_IGN);
  warden.vaultRetrieve({"user"}, strUser, e);
  warden.vaultRetrieve({"password"}, strPassword, e);
  radial.setCredentials(strUser, strPassword);
  if (radial.connect(e))
  {
    bool bShutdown = false;
    char szBuffer[1024];
    int nIndex, nReturn;
    list<string> removals;
    list<Json *> messages;
    map<string, command *> commands;
    pollfd *fds;
    cout << p << ":  Connected to Radial." << endl;
    while (!gbShutdown)
    {
      if (radial.getMessages(messages, e))
      {
        for (auto &m : messages)
        {
          bool bProcessed = false;
          string strError;
          if (m->m.find("wsRequestID") != m->m.end() && !m->m["wsRequestID"]->v.empty())
          {
            string w = m->m["wsRequestID"]->v;
            if (m->m.find("Function") != m->m.end() && !m->m["Function"]->v.empty())
            {
              string f = m->m["Function"]->v;
              if (f == "launch" || commands.find(w) != commands.end())
              {
                if (f == "data")
                {
                  if (m->m.find("Data") != m->m.end() && !m->m["Data"]->v.empty())
                  {
                    commands[w]->b[1].append(m->m["Data"]->v);
                  }
                  else
                  {
                    strError = "Radial::getMessages() Please provide the Data.";
                  }
                }
                else if (f == "launch")
                {
                  if (m->m.find("Command") != m->m.end() && !m->m["Command"]->v.empty())
                  {
                    char *args[100], *pszArgument;
                    int readpipe[2] = {-1, -1}, writepipe[2] = {-1, -1};
                    pid_t childPid;
                    string strArgument;
                    stringstream ssCommand(m->m["Command"]->v);
                    unsigned int unIndex = 0;
                    while (ssCommand >> strArgument)
                    {
                      pszArgument = new char[strArgument.size() + 1];
                      strcpy(pszArgument, strArgument.c_str());
                      args[unIndex++] = pszArgument;
                    }
                    args[unIndex] = NULL;
                    if (pipe(readpipe) == 0)
                    {
                      if (pipe(writepipe) == 0)
                      {
                        if ((childPid = fork()) == 0)
                        {
                          close(PARENT_WRITE);
                          close(PARENT_READ);
                          cout << p << " [" << m->m["Command"]->v << "]:  Launched command." << endl;
                          dup2(CHILD_READ, 0);
                          close(CHILD_READ);
                          dup2(CHILD_WRITE, 1);
                          close(CHILD_WRITE);
                          execvpe(args[0], args, environ);
                          _exit(1);
                        }
                        else if (childPid > 0)
                        {
                          command *ptCommand = new command;
                          close(CHILD_READ);
                          close(CHILD_WRITE);
                          bProcessed = true;
                          ptCommand->r = PARENT_READ;
                          ptCommand->w = PARENT_WRITE;
                          commands[m->m["wsRequestID"]->v] = ptCommand;
                        }
                        else
                        {
                          strError = (string)"fork() " + strerror(errno);
                        }
                      }
                      else
                      {
                        strError = (string)"pipe() [write] " + strerror(errno);
                      }
                    }
                    else
                    {
                      strError = (string)"->pipe() [read] " + strerror(errno);
                    }
                    for (unsigned int i = 0; i < unIndex; i++)
                    {
                      delete[] args[i];
                    }
                  }
                  else
                  {
                    strError = "Radial::getMessages() Please provide the Command.";
                  }
                }
                else
                {
                  strError = "Radial::getMessages() Please provide a valid Function:  data, launch.";
                }
              }
              else
              {
                strError = "Radial::getMessages() error:  wsRequestID does not exist.";
              }
            }
            else
            {
              strError = "Radial::getMessages() error:  Please provide the Function.";
            }
          }
          else
          {
            strError = "Radial::getMessages() error:  Please provide the wsRequestID.";
          }
          m->i("Status", ((bProcessed)?"okay":"error"));
          if (!strError.empty())
          {
            m->i("Error", strError);
          }
        }
        radial.putMessages(messages);
      }
      else
      {
        bShutdown = true;
        cerr << p << "->Radial::getMessages() error:  " << e << endl;
      }
      fds = new pollfd[commands.size()*2];
      nIndex = 0;
      for (auto &i : commands)
      {
        fds[nIndex].fd = i.second->r;
        fds[nIndex].events = POLLIN;
        nIndex++;
        fds[nIndex].fd = ((!i.second->b[1].empty())?i.second->w:-1);
        fds[nIndex].events = POLLOUT;
        nIndex++;
      }
      if (nIndex > 0)
      {
        if ((nReturn = poll(fds, nIndex, 10)) > 0)
        {
          for (int i = 0; i < nIndex; i++)
          {
            for (auto &j : commands)
            {
              if (fds[i].fd == j.second->r)
              {
                if (fds[i].revents & (POLLHUP | POLLIN))
                {
                  if ((nReturn = read(fds[i].fd, szBuffer, 1024)) > 0)
                  {
cout.write(szBuffer, nReturn);
                    j.second->b[0].append(szBuffer, nReturn);
                  }
                  else
                  {
                    removals.push_back(j.first);
                  }
                }
              }
              else if (fds[i].fd == j.second->w)
              {
                if (fds[i].revents & POLLOUT)
                {
                  if ((nReturn = write(fds[i].fd, j.second->b[1].c_str(), j.second->b[1].size())) > 0)
                  {
                    j.second->b[1].erase(0, nReturn);
                  }
                  else
                  {
                    removals.push_back(j.first);
                  }
                }
              }
            }
          }
        }
        else if (nReturn < 0 && errno != EINTR)
        {
          shutdown();
          cerr << p << "->poll(" << errno << ") error:  " << strerror(errno) << endl;
        }
      }
      delete[] fds;
      for (auto &i : commands)
      {
        if (!i.second->b[0].empty())
        {
          string strEncoded;
          Json *ptReq = new Json, *ptRes = new Json;
          ptReq->i("Interface", "live");
          ptReq->i("Function", "message");
          ptReq->m["Request"] = new Json;
          ptReq->m["Request"]->i("wsReqeustID", i.first);
          ptReq->m["Request"]->m["Message"] = new Json;
          ptReq->m["Request"]->m["Message"]->i("Action", "data");
          manip.encodeBase64(i.second->b[0], strEncoded);
          i.second->b[0].clear();
          ptReq->m["Request"]->m["Message"]->i("Data", strEncoded);
          ptReq->m["Request"]->i("Wait", "0", '0');
          if (!radial.request(ptReq, ptRes, e))
          {
            shutdown();
            cerr << p << "->Radial::request(live,message) error:  " << e << endl;
          }
          delete ptReq;
          delete ptRes;
        }
      }
      removals.sort();
      removals.unique();
      while (!removals.empty())
      {
        close(commands[removals.front()]->r);
        close(commands[removals.front()]->w);
        delete commands[removals.front()];
        commands.erase(removals.front());
        removals.pop_front();
      }
      if (bShutdown)
      {
        shutdown();
      }
    }
    radial.disconnect(e);
    cout << p << ":  Disconnected from Radial." << endl;
  }
  else
  {
    shutdown();
    cerr << p << "->Radial::connect() error:  " << e << endl;
  }

  return 0;
}
// }}}
// {{{ shutdown()
void shutdown()
{
  gbShutdown = true;
}
// }}}
// {{{ sighandle()
void sighandle(const int nSignal)
{
  string strSignal;

  sethandles(sigdummy);
  shutdown();
  cerr << "The program's signal handling caught a " << sigstring(strSignal, nSignal) << "(" << nSignal << ")!  Exiting..." << endl;
}
// }}}
