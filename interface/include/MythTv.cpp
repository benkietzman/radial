// -*- C++ -*-
// Radial
// -------------------------------------
// file       : MythTv.cpp
// author     : Ben Kietzman
// begin      : 2025-04-24
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "MythTv"
// }}}
extern "C++"
{
namespace radial
{
// {{{ MythTv()
MythTv::MythTv(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "mythtv", argc, argv, pCallback)
{
  m_strPort = "6544";
  m_strServer = "localhost";
  m_pUtility->setReadSize(8388608);
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg.size() > 7 && strArg.substr(0, 7) == "--port=")
    {
      m_strPort = strArg.substr(7, strArg.size() - 7);
      m_manip.purgeChar(m_strPort, m_strPort, "'");
      m_manip.purgeChar(m_strPort, m_strPort, "\"");
    }
    else if (strArg.size() > 9 && strArg.substr(0, 9) == "--server=")
    {
      m_strServer = strArg.substr(9, strArg.size() - 9);
      m_manip.purgeChar(m_strServer, m_strServer, "'");
      m_manip.purgeChar(m_strServer, m_strServer, "\"");
    }
  }
  // }}}
  // {{{ functions
  m_functions["action"] = &MythTv::action;
  m_functions["backend"] = &MythTv::backend;
  m_functions["status"] = &MythTv::status;
  // }}}
  m_pThreadSchedule = new thread(&MythTv::schedule, this, strPrefix);
  pthread_setname_np(m_pThreadSchedule->native_handle(), "schedule");
}
// }}}
// {{{ ~MythTv()
MythTv::~MythTv()
{
  m_pThreadSchedule->join();
  delete m_pThreadSchedule;
}
// }}}
// {{{ callback()
void MythTv::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->MythTv::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    string strFunction = ptJson->m["Function"]->v;
    radialUser d;
    userInit(ptJson, d);
    if (m_functions.find(strFunction) != m_functions.end())
    {
      if ((this->*m_functions[strFunction])(d, strError))
      {
        bResult = true;
      }
    }
    else
    {
      strError = "Please provide a valid Function.";
    }
    if (bResult)
    {
      if (exist(ptJson, "Response"))
      {
        delete ptJson->m["Response"];
      }
      ptJson->m["Response"] = d.p->m["o"];
      d.p->m.erase("o");
    }
    userDeinit(d);
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
    ptJson->j(p.p);
    hub(p, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
// {{{ backend()
bool MythTv::backend(radialUser &d, string &e)
{
  bool b = false;
  string strLiveID, strRequestID;
  stringstream m;
  Json *i = d.p->m["i"];

  if (!empty(i, "Transfer") && i->m["Transfer"]->v == "live" && !empty(d.r, "wsRequestID") && !empty(i, "LiveID"))
  {
    strLiveID = i->m["LiveID"]->v;
    strRequestID = d.r->m["wsRequestID"]->v;
  }
  if (dep({"Service", "Command"}, i, e))
  {
    int fdSocket;
    if (m_pUtility->connect(m_strServer, m_strPort, fdSocket, e))
    {
      bool bExit = false, bValid = false;;
      int nReturn;
      map<string, string> data, headers;
      size_t unLength = 0, unPosition, unRead = 0;
      string strBase64, strBuffers[2];
      stringstream ssReq;
      ssReq << "GET /" << i->m["Service"]->v << "/" << i->m["Command"]->v << " HTTP/1.1" << endl;
      ssReq << "Host: " << m_strServer << endl << endl;
      strBuffers[1] = ssReq.str();
      if (!strLiveID.empty())
      {
        data["Action"] = "mythtv";
        data["Service"] = i->m["Service"]->v;
        data["Command"] = i->m["Command"]->v;
        data["LiveID"] = strLiveID;
      }
      while (!bExit && !shutdown())
      {
        pollfd fds[1];
        fds[0].fd = fdSocket;
        fds[0].events = POLLIN;
        if (!strBuffers[1].empty())
        {
          fds[0].events |= POLLOUT;
        }
        if ((nReturn = poll(fds, 1, 250)) > 0)
        {
          if (fds[0].revents & POLLIN)
          {
            if (m_pUtility->fdRead(fds[0].fd, strBuffers[0], nReturn))
            {
              if (bValid)
              {
                unRead += strBuffers[0].size();
                if (unRead >= unLength)
                {
                  b = bExit = true;
                }
                if (!strRequestID.empty())
                {
                  m_manip.encodeBase64(strBuffers[0], strBase64);
                  strBuffers[0].clear();
                  data["Data"] = strBase64;
                  live(strRequestID, data, true);
                }
              }
              else if ((unPosition = strBuffers[0].find("\n\n")) != string::npos)
              {
                string strHeader;
                stringstream ssHeaders(strBuffers[0].substr(0, unPosition));
                strBuffers[0].erase(0, (unPosition + 2));
                while (getline(ssHeaders, strHeader))
                {
                  if (strHeader.size() > 9 && strHeader.substr(0, 9) == "HTTP/1.1")
                  {
                    string strStatus = strHeader.substr(9, (strHeader.size() - 9));
                    if (strStatus == "200 OK")
                    {
                      bValid = true;
                    }
                    else
                    {
                      e = strStatus;
                    }
                  }
                  else if ((unPosition = strHeader.find(": ")) != string::npos && unPosition > 0)
                  {
                    headers[strHeader.substr(0, unPosition)] = strHeader.substr((unPosition + 2), (strHeader.size() - (unPosition + 2)));
                  }
                }
                if (bValid)
                {
                  if (headers.find("Content-Length") != headers.end() && !headers["Content-Length"].empty())
                  {
                    stringstream ssLength(headers["Content-Length"]);
                    ssLength >> unLength;
                  }
                  else
                  {
                    bValid = false;
                    m.str("");
                    m << "Missing Content-Length header in response.";
                    e = m.str();
                  }
                }
              }
            }
            else
            {
              bExit = true;
              if (nReturn == 0)
              {
                b = true;
              }
              else
              {
                m.str("");
                m << "Utility::fdRead(" << errno << ") error:  " << strerror(errno);
                e = m.str();
              }
            }
          }
          if ((fds[0].revents & POLLOUT) && !m_pUtility->fdWrite(fds[0].fd, strBuffers[1], nReturn))
          {
            bExit = true;
            if (nReturn == 0)
            {
              b = true;
            }
            else
            {
              m.str("");
              m << "Utility::fdWrite(" << errno << ") error:  " << strerror(errno);
              e = m.str();
            }
          }
          if (fds[0].revents & POLLERR)
          {
            bExit = true;
            m.str("");
            m << "poll() Encountered a POLLERR.";
            e = m.str();
          }
          if (fds[0].revents & POLLNVAL)
          {
            bExit = true;
            m.str("");
            m << "poll() Encountered a POLLNVAL.";
            e = m.str();
          }
        }
        else if (nReturn < 0)
        {
          bExit = true;
          m.str("");
          m << "poll(" << errno << ") error:  " << strerror(errno);
          e = m.str();
        }
      }
      if (b && strRequestID.empty())
      {
        email("ben@kietzman.org", "ben@kietzman.org", "MythTV Debug", strBuffers[0], "");
        if ((unPosition = strBuffers[0].find("?>")) != string::npos)
        {
          string strJson;
          Json *j = new Json(strBuffers[0].substr((unPosition + 2), (strBuffers[0].size() - (unPosition + 2))));
          d.p->i("o", j->j(strJson));
          delete j;
        }
        else
        {
          b = false;
          e = "Invalid response.";
        }
      }
      close(fdSocket);
    }
  }

  return b;
}
// }}}
// {{{ schedule()
void MythTv::schedule(string strPrefix)
{
  list<string> removals;
  time_t CTime[2];

  threadIncrement();
  strPrefix += "->MythTv::schedule()";
  time(&(CTime[0]));
  while (!shutdown())
  {
    time(&(CTime[1]));
    if ((CTime[1] - CTime[0]) > 300)
    {
      CTime[0] = CTime[1];
    }
    msleep(1000);
  }
  threadDecrement();
}
// }}}
}
}
