// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Logger.cpp
// author     : Ben Kietzman
// begin      : 2022-05-17
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Logger"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Logger()
Logger::Logger(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "logger", argc, argv, pCallback)
{
  string strError;

  if (m_pWarden != NULL)
  {
    Json *ptLogger = new Json;
    if (m_pWarden->vaultRetrieve({"logger"}, ptLogger, strError))
    {
      for (auto &app : ptLogger->m)
      {
        if (!empty(app.second, "Password") && !empty(app.second, "User"))
        {
          m_logger[app.first] = new common::Logger(strError);
          m_logger[app.first]->setCredentials(app.first, app.second->m["User"]->v, app.second->m["Password"]->v);
          m_logger[app.first]->setTimeout("10");
          m_logger[app.first]->setThrottle(100);
          m_logger[app.first]->useSingleSocket(true);
        }
      }
    }
    delete ptLogger;
  }
}
// }}}
// {{{ ~Logger()
Logger::~Logger()
{
  for (auto &app : m_logger)
  {
    delete app.second;
  }
}
// }}}
// {{{ callback()
void Logger::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strApplication = "Radial", strError;
  stringstream ssMessage;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Logger::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    if (ptJson->m["Function"]->v == "applications")
    {
      bResult = true;
      if (exist(ptJson, "Response"))
      {
        delete ptJson->m["Response"];
      }
      ptJson->m["Response"] = new Json;
      for (auto &i : m_logger)
      {
        ptJson->m["Response"]->pb(i.first);
      }
    }
    else if (ptJson->m["Function"]->v == "log" || ptJson->m["Function"]->v == "message")
    {
      if (exist(ptJson, "Request"))
      {
        if (!empty(ptJson->m["Request"], "Application"))
        {
          strApplication = ptJson->m["Request"]->m["Application"]->v;
        }
        if (m_logger.find(strApplication) != m_logger.end())
        {
          if (exist(ptJson->m["Request"], "Label"))
          {
            if (!empty(ptJson->m["Request"], "Message"))
            {
              map<string, string> label;
              ptJson->m["Request"]->m["Label"]->flatten(label, true, false);
              if ((ptJson->m["Function"]->v == "log" && m_logger[strApplication]->log(label, ptJson->m["Request"]->m["Message"]->v, strError)) || (ptJson->m["Function"]->v == "message" && m_logger[strApplication]->message(label, ptJson->m["Request"]->m["Message"]->v, strError)))
              {
                bResult = true;
              }
            }
            else
            {
              string strJson;
              strError = "Please provide the Message within the Request.";
            }
          }
          else
          {
            strError = "Please provide the Label within the Request.";
          }
        }
        else
        {
          strError = "Please provide a valid Application within the Request.";
        }
      }
      else
      {
        strError = "Please provide the Request.";
      }
    }
    else
    {
      strError = "Please provide a valid Function:  applications, log, message.";
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
}
// }}}
}
}
