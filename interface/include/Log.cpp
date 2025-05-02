// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Log.cpp
// author     : Ben Kietzman
// begin      : 2022-04-19
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Log"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Log()
Log::Log(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "log", argc, argv, pCallback)
{
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-e" || (strArg.size() > 8 && strArg.substr(0, 8) == "--email="))
    {
      if (strArg == "-e" && i + 1 < argc && argv[i+1][0] != '-')
      {
        m_strEmail = argv[++i];
      }
      else
      {
        m_strEmail = strArg.substr(8, strArg.size() - 8);
      }
      m_manip.purgeChar(m_strEmail, m_strEmail, "'");
      m_manip.purgeChar(m_strEmail, m_strEmail, "\"");
    }
  }
  // }}}
  m_pCentral->setEmail(m_strEmail);
  m_pCentral->setLog(m_strData, "radial_", "monthly", true, true);
}
// }}}
// {{{ ~Log()
Log::~Log()
{
}
// }}}
// {{{ callback()
void Log::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  stringstream ssMessage;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Log::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    if (!empty(ptJson, "Message"))
    {
      if (ptJson->m["Function"]->v == "alert")
      {
        if (m_pCentral->notify("", ptJson->m["Message"]->v, strError))
        {
          stringstream ssQuery;
          ssQuery << "select distinct d.userid from application a, application_contact b, contact_type c, person d where a.id=b.application_id and b.type_id=c.id and b.contact_id=d.id and a.name = 'Radial' and b.notify = 1 and (c.type = 'Primary Developer' or c.type = 'Backup Developer') and d.userid is not null";
          auto getUser = dbquery("central_r", ssQuery.str(), strError);
          if (getUser != NULL && !getUser->empty())
          {
            bResult = true;
            for (auto &getUserRow : *getUser)
            {
              if (!alert(getUserRow["userid"], (string)"Radial:  " + ptJson->m["Message"]->v, strError))
              {
                bResult = false;
              }
            }
          }
          dbfree(getUser);
        }
      }
      else if (ptJson->m["Function"]->v == "log")
      {
        if (m_pCentral->log(ptJson->m["Message"]->v, strError))
        {
          bResult = true;
        }
      }
      else if (ptJson->m["Function"]->v == "notify")
      {
        if (m_pCentral->notify("", ptJson->m["Message"]->v, strError))
        {
          bResult = true;
        }
      }
      else
      {
        strError = "Please provide a valid Function:  alert, log, notify.";
      }
    }
    else
    {
      strError = "Please provide the Message.";
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
    ptJson->j(p.p);
    hub(p, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
}
}
