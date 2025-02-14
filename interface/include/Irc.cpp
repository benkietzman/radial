// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Irc.cpp
// author     : Ben Kietzman
// begin      : 2022-12-27
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
// {{{ includes
#include "Irc"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Irc()
Irc::Irc(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool), void (*pCallbackInotify)(string, const string, const string)) : Interface(strPrefix, "irc", argc, argv, pCallback)
{
  map<string, list<string> > watches;
  string strError;
  Json *ptAes = new Json, *ptJwt = new Json;

  m_pAnalyzeCallback1 = NULL;
  m_pAnalyzeCallback2 = NULL;
  // {{{ command line arguments
  for (int i = 1; i < argc; i++)
  {
    string strArg = argv[i];
    if (strArg == "-p" || (strArg.size() > 7 && strArg.substr(0, 7) == "--port="))
    {
      if (strArg == "-p" && i + 1 < argc && argv[i+1][0] != '-')
      {
        m_strPort = argv[++i];
      }
      else
      {
        m_strPort = strArg.substr(7, strArg.size() - 7);
      }
      m_manip.purgeChar(m_strPort, m_strPort, "'");
      m_manip.purgeChar(m_strPort, m_strPort, "\"");
    }
    else if (strArg == "-s" || (strArg.size() > 9 && strArg.substr(0, 9) == "--server="))
    {
      if (strArg == "-s" && i + 1 < argc && argv[i+1][0] != '-')
      {
        m_strServer = argv[++i];
      }
      else
      {
        m_strServer = strArg.substr(9, strArg.size() - 9);
      }
      m_manip.purgeChar(m_strServer, m_strServer, "'");
      m_manip.purgeChar(m_strServer, m_strServer, "\"");
    }
  }
  // }}}
  m_bEnabled = false;
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"aes"}, ptAes, strError))
  { 
    if (!empty(ptAes, "Secret"))
    {
      m_strAesSecret = ptAes->m["Secret"]->v;
    }
  }
  delete ptAes; 
  if (m_pWarden != NULL && m_pWarden->vaultRetrieve({"jwt"}, ptJwt, strError))
  {
    if (!empty(ptJwt, "Secret"))
    {
      m_strJwtSecret = ptJwt->m["Secret"]->v;
    }
    if (!empty(ptJwt, "Signer"))
    {
      m_strJwtSigner = ptJwt->m["Signer"]->v;
    }
  }
  delete ptJwt;
  monitorChannels(strPrefix, true);
  watches[m_strData + "/irc"] = {"monitor.channels"};
  m_pThreadInotify = new thread(&Irc::inotify, this, strPrefix, watches, pCallbackInotify);
  pthread_setname_np(m_pThreadInotify->native_handle(), "inotify");
}
// }}}
// {{{ ~Irc()
Irc::~Irc()
{
  m_pThreadInotify->join();
  delete m_pThreadInotify;
}
// }}}
// {{{ analyze()
void Irc::analyze(const string strNick, const string strTarget, const string strMessage)
{
  if (m_ptMonitor != NULL && strNick != m_strNick)
  {
    string strMessageLower;
    stringstream ssText;
    m_manip.toLower(strMessageLower, m_manip.trim(strMessageLower, strMessage));
    ssText << char(3) << "08,03 " << strNick << " @ " << strTarget << " " << char(3) << " " << strMessage;
    for (auto &i : m_ptMonitor->m)
    {
      if (i.first != strTarget && exist(i.second, "Alerts"))
      {
        for (auto &j : i.second->m["Alerts"]->l)
        {
          string strAlertLower;
          m_manip.toLower(strAlertLower, m_manip.trim(strAlertLower, j->v));
          if (strMessageLower.size() >= (strAlertLower.size() + 1) && strMessageLower.substr(0, (strAlertLower.size() + 1)) == (strAlertLower + (string)" "))
          {
            chat(i.first, ssText.str());
          }
        }
      }
    }
  }
}
void Irc::analyze(string strPrefix, const string strTarget, const string strUserID, const string strIdent, const string strFirstName, const string strLastName, const bool bAdmin, map<string, bool> &auth, stringstream &ssData, const string strSource)
{
  list<string> actions = {"alert", "central", "centralmon", "command", "database", "date", "db", "feedback (fb)", "interface", "irc", "live", "math", "radial", "sqlite (sql)", "ssh (s)", "storage", "terminal (t)"};
  string strAction;
  Json *ptRequest = new Json;

  strPrefix += "->analyze()";
  ssData >> strAction;
  if (!strAction.empty())
  {
    ptRequest->i("Action", strAction);
    if (m_pAnalyzeCallback1 == NULL || !m_pAnalyzeCallback1(strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, actions, strAction, ssData, ptRequest, strSource))
    {
      // {{{ alert
      if (strAction == "alert")
      {
        string strUser;
        ssData >> strUser;
        if (!strUser.empty())
        {
          string strMessage;
          ptRequest->i("User", strUser);
          getline(ssData, strMessage);
          m_manip.trim(strMessage, strMessage);
          if (!strMessage.empty())
          {
            ptRequest->i("Message", strMessage);
          }
        }
      }
      // }}}
      // {{{ central
      else if (strAction == "central")
      {
        string strFunction;
        ssData >> strFunction;
        if (!strFunction.empty())
        {
          ptRequest->i("Function", strFunction);
          // {{{ application
          if (strFunction == "application")
          {
            string strSubData;
            getline(ssData, strSubData);
            m_manip.trim(strSubData, strSubData);
            if (!strSubData.empty())
            {
              string strApplicationID;
              stringstream ssSubData(strSubData);
              ssSubData >> strApplicationID;
              if (!strApplicationID.empty())
              {
                ptRequest->i("ApplicationID", strApplicationID);
                if (strApplicationID == "account")
                {
                  string strMechID;
                  ssSubData >> strMechID;
                  if (!strMechID.empty())
                  {
                    ptRequest->i("MechID", strMechID);
                  }
                }
                else if (m_manip.isNumeric(strApplicationID))
                {
                  string strForm;
                  ssSubData >> strForm;
                  if (!strForm.empty())
                  {
                    ptRequest->i("Form", strForm);
                    if (strForm == "notify")
                    {
                      string strMessage;
                      getline(ssSubData, strMessage);
                      m_manip.trim(strMessage, strMessage);
                      if (!strMessage.empty())
                      {
                        ptRequest->i("Message", strMessage);
                      }
                    }
                  }
                }
                else
                {
                  ptRequest->i("ApplicationID", strSubData);
                }
              }
            }
          }
          // }}}
          // {{{ group
          else if (strFunction == "group")
          {
            string strSubData;
            getline(ssData, strSubData);
            m_manip.trim(strSubData, strSubData);
            if (!strSubData.empty())
            {
              string strGroupID;
              stringstream ssSubData(strSubData);
              ssSubData >> strGroupID;
              if (!strGroupID.empty())
              {
                ptRequest->i("GroupID", strGroupID);
                if (m_manip.isNumeric(strGroupID))
                {
                  string strForm;
                  ssSubData >> strForm;
                  if (!strForm.empty())
                  {
                    ptRequest->i("Form", strForm);
                    if (strForm == "notify")
                    {
                      string strMessage;
                      getline(ssSubData, strMessage);
                      m_manip.trim(strMessage, strMessage);
                      if (!strMessage.empty())
                      {
                        ptRequest->i("Message", strMessage);
                      }
                    }
                  }
                }
                else
                {
                  ptRequest->i("GroupID", strSubData);
                }
              }
            }
          }
          // }}}
          // {{{ server
          else if (strFunction == "server")
          {
            string strSubData;
            getline(ssData, strSubData);
            m_manip.trim(strSubData, strSubData);
            if (!strSubData.empty())
            {
              string strServerID;
              stringstream ssSubData(strSubData);
              ssSubData >> strServerID;
              if (!strServerID.empty())
              {
                ptRequest->i("ServerID", strServerID);
                if (m_manip.isNumeric(strServerID))
                {
                  string strForm;
                  ssSubData >> strForm;
                  if (!strForm.empty())
                  {
                    ptRequest->i("Form", strForm);
                    if (strForm == "notify")
                    {
                      string strMessage;
                      getline(ssSubData, strMessage);
                      m_manip.trim(strMessage, strMessage);
                      if (!strMessage.empty())
                      {
                        ptRequest->i("Message", strMessage);
                      }
                    }
                  }
                }
                else
                {
                  ptRequest->i("ServerID", strSubData);
                }
              }
            }
          }
          // }}}
          // {{{ user
          else if (strFunction == "user")
          {
            string strUser;
            ssData >> strUser;
            if (!strUser.empty())
            {
              string strForm;
              ptRequest->i("User", strUser);
              ssData >> strForm;
              if (!strForm.empty())
              {
                ptRequest->i("Form", strForm);
                if (strForm == "notify")
                {
                  string strMessage;
                  getline(ssData, strMessage);
                  m_manip.trim(strMessage, strMessage);
                  if (!strMessage.empty())
                  {
                    ptRequest->i("Message", strMessage);
                  }
                }
                else
                {
                  string strType;
                  getline(ssData, strType);
                  m_manip.trim(strType, strType);
                  if (strType.empty())
                  {
                    strType = "all";
                  }
                  ptRequest->i("Type", strType);
                }
              }
            }
          }
          // }}}
        }
      }
      // }}}
      // {{{ centralmon
      else if (strAction == "centralmon")
      {
        string strServer;
        ssData >> strServer;
        if (!strServer.empty())
        {
          string strProcess;
          ssData >> strProcess;
          ptRequest->i("Server", strServer);
          if (!strProcess.empty())
          {
            ptRequest->i("Process", strProcess);
          }
        }
      }
      // }}}
      // {{{ command
      else if (strAction == "command")
      {
        string strNode;
        ssData >> strNode;
        if (!strNode.empty())
        {
          string strCommand;
          getline(ssData, strCommand);
          ptRequest->i("Node", strNode);
          if (!strCommand.empty())
          {
            ptRequest->i("Command", strCommand);
          }
        }
      }
      // }}}
      // {{{ database
      else if (strAction == "database")
      {
        string strDatabase;
        ssData >> strDatabase;
        if (!strDatabase.empty())
        {
          string strQuery;
          ptRequest->i("Database", strDatabase);
          getline(ssData, strQuery);
          m_manip.trim(strQuery, strQuery);
          if (!strQuery.empty())
          {
            ptRequest->i("Query", strQuery);
          }
        }
      }
      // }}}
      // {{{ date
      else if (strAction == "date")
      {
        string strDate;
        ssData >> strDate;
        if (!strDate.empty())
        {
          string strTime;
          ptRequest->i("Date", strDate);
          ssData >> strTime;
          if (!strTime.empty())
          {
            ptRequest->i("Time", strTime);
          }
        }
      }
      // }}}
      // {{{ db
      else if (strAction == "db")
      {
        string strFunction;
        ssData >> strFunction;
        if (!strFunction.empty())
        {
          string strRequest;
          ptRequest->i("Function", strFunction);
          getline(ssData, strRequest);
          m_manip.trim(strRequest, strRequest);
          if (!strRequest.empty())
          {
            ptRequest->i("Request", strRequest);
          }
        }
      }
      // }}}
      // {{{ feedback || fb
      else if (strAction == "feedback" || strAction == "fb")
      {
        string strData;
        getline(ssData, strData);
        m_manip.trim(strData, strData);
        if (!strData.empty())
        {
          ptRequest->i("Data", strData);
        }
      }
      // }}}
      // {{{ interface
      else if (strAction == "interface")
      {
        string strInterface;
        ssData >> strInterface;
        if (!strInterface.empty())
        {
          string strFunction;
          ptRequest->i("Interface", strInterface);
          ssData >> strFunction;
          if (!strFunction.empty())
          {
            string strNode;
            ssData >> strNode;
            ptRequest->i("Function", strFunction);
            if (!strNode.empty())
            {
              ptRequest->i("Node", strNode);
            }
          }
        }
      }
      // }}}
      // {{{ irc
      else if (strAction == "irc")
      {
        string strFunction;
        ssData >> strFunction;
        if (!strFunction.empty())
        {
          string strSubTarget;
          ptRequest->i("Function", strFunction);
          ssData >> strSubTarget;
          if (!strSubTarget.empty())
          {
            string strMessage;
            ptRequest->i("Target", strSubTarget);
            getline(ssData, strMessage);
            m_manip.trim(strMessage, strMessage);
            if (!strMessage.empty())
            {
              ptRequest->i("Message", strMessage);
            }
          }
        }
      }
      // }}}
      // {{{ live
      else if (strAction == "live")
      {
        string strJson, strRequest;
        getline(ssData, strRequest);
        m_manip.trim(strJson, strRequest);
        if (!strJson.empty())
        {
          ptRequest->i("Json", strJson);
        }
      }
      // }}}
      // {{{ math
      else if (strAction == "math")
      {
        string strEquation;
        getline(ssData, strEquation);
        if (!strEquation.empty())
        {
          ptRequest->i("Equation", strEquation);
        }
      }
      // }}}
      // {{{ radial
      else if (strAction == "radial")
      {
        string strInterface;
        ssData >> strInterface;
        if (!strInterface.empty())
        {
          string strJson, strRequest;
          if (strInterface == "hub")
          {
            strInterface.clear();
          }
          ptRequest->i("Interface", strInterface);
          getline(ssData, strRequest);
          m_manip.trim(strJson, strRequest);
          if (!strJson.empty())
          {
            ptRequest->i("Json", strJson);
          }
        }
      }
      // }}}
      // {{{ sqlite || sql
      else if (strAction == "sqlite" || strAction == "sql")
      {
        string strFunction;
        ssData >> strFunction;
        if (!strFunction.empty())
        {
          string strRemainder;
          ptRequest->i("Function", strFunction);
          getline(ssData, strRemainder);
          if (!strRemainder.empty())
          {
            ptRequest->i("Remainder", strRemainder);
          }
        }
      }
      // }}}
      // {{{ ssh || s
      else if (strAction == "ssh" || strAction == "s")
      {
        string strFunction;
        ssData >> strFunction;
        if (!strFunction.empty())
        {
          string strCommand;
          ptRequest->i("Function", strFunction);
          getline(ssData, strCommand);
          m_manip.trim(strCommand, strCommand);
          if (!strCommand.empty())
          {
            ptRequest->i("Command", strCommand);
          }
        }
      }
      // }}}
      // {{{ storage
      else if (strAction == "storage")
      {
        string strJson, strRequest;
        getline(ssData, strRequest);
        m_manip.trim(strJson, strRequest);
        if (!strJson.empty())
        {
          ptRequest->i("Json", strJson);
        }
      }
      // }}}
      // {{{ terminal || t
      else if (strAction == "terminal" || strAction == "t")
      {
        string strFunction;
        ssData >> strFunction;
        if (!strFunction.empty())
        {
          string strCommand;
          ptRequest->i("Function", strFunction);
          getline(ssData, strCommand);
          m_manip.trim(strCommand, strCommand);
          if (!strCommand.empty())
          {
            ptRequest->i("Command", strCommand);
          }
        }
      }
      // }}}
    }
  }
  else if (m_pAnalyzeCallback1 != NULL)
  {
    m_pAnalyzeCallback1(strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, actions, strAction, ssData, ptRequest, strSource);
  }
  analyze(strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, actions, ptRequest, strSource);
  delete ptRequest;
}
void Irc::analyze(string strPrefix, const string strTarget, const string strUserID, const string strIdent, const string strFirstName, const string strLastName, const bool bAdmin, map<string, bool> &auth, list<string> &actions, Json *ptData, const string strSource)
{
  // {{{ prep work
  string strAction = var("Action", ptData), strError, strQuery, strValue;
  stringstream ssQuery, ssText;
  throughput("analyze");
  ssText << char(3) << "11,10 " << m_strNode << " " << char(3) << " " << char(3) << "07,05 " << m_strName << " " << char(3) << " " << char(3) << "13,06 " << ((!strAction.empty())?strAction:"actions") << " " << char(3);
  // }}}
  // {{{ callback
  if (m_pAnalyzeCallback2 != NULL && m_pAnalyzeCallback2(strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, strAction, ptData, ssText, strSource))
  {
  }
  // }}}
  // {{{ alert
  else if (strAction == "alert")
  {
    string strUser = var("User", ptData);
    if (!strUser.empty())
    {
      string strMessage = var("Message", ptData);
      ssText << " " << char(3) << "00,14 " << strUser << " " << char(3);
      if (!strMessage.empty())
      {
        stringstream ssMessage;
        ssMessage << strMessage << endl << endl << " -- " << strFirstName << " " << strLastName << " (" << strIdent << ")" << endl;
        if (alert(strUser, ssMessage.str(), strError))
        {
          ssText << ":  Successfully sent the alert to " << strUser << ".";
        }
        else
        {
          ssText << " error:  " << strError;
        }
      }
      else
      {
        ssText << ":  Please provide the message immediately following the user.";
      }
    }
    else
    {
      ssText << ":  The alert action is used to send an alert message through the Alert framework.  Please provide the user immediately following the action.";
    }
  }
  // }}}
  // {{{ central
  else if (strAction == "central")
  {
    string strFunction = var("Function", ptData);
    // {{{ application
    if (strFunction == "application")
    {
      string strApplicationID = var("ApplicationID", ptData);
      ssText << " " << char(3) << "00,14 " << strFunction << " " << char(3);
      // {{{ account
      if (strApplicationID == "account")
      {
        string strMechID = var("MechID", ptData);
        ssText << " " << char(3) << "00,14 " << strApplicationID << " " << char(3);
        if (!strMechID.empty())
        {
          ssText << " " << char(3) << "00,14 " << strMechID << " " << char(3);
          ssQuery.str("");
          ssQuery << "select a.id, a.name, b.description from application a, application_account b where a.id = b.application_id and lower(b.user_id) = lower('" << m_manip.escape(strMechID, strValue) << "') order by a.name";
          auto getApplicationAccount = dbquery("central_r", ssQuery.str(), strError);
          if (getApplicationAccount != NULL)
          {
            for (auto &getApplicationAccountRow : *getApplicationAccount)
            {
              ssText << endl << "Application:  " << getApplicationAccountRow["name"] << " [" << getApplicationAccountRow["id"] << "], Description:  " << getApplicationAccountRow["description"];
            }
          }
          else
          {
            ssText << " error:  " << strError;
          }
          dbfree(getApplicationAccount);
        }
        else
        {
          ssText << ":  Please provide the MechID immediately following the account.";
        }
      }
      // }}}
      // {{{ application
      else if (!strApplicationID.empty())
      {
        // {{{ id
        if (m_manip.isNumeric(strApplicationID))
        {
          string strForm = var("Form", ptData);
          ssText << " " << char(3) << "00,14 " << strApplicationID << " " << char(3);
          // {{{ general
          if (strForm == "general")
          {
            ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
            ssQuery.str("");
            ssQuery << "select name, date_format(creation_date, '%Y-%m-%d') creation_date, website, date_format(retirement_date, '%Y-%m-%d') retirement_date, description from application where id = " << strApplicationID;
            list<map<string, string> > *getApplication = dbquery("central_r", ssQuery.str(), strError);
            if (getApplication != NULL)
            {
              if (!getApplication->empty())
              {
                map<string, string> getApplicationRow = getApplication->front();
                ssText << " Application:  " << getApplicationRow["name"];
                ssText << ", Website:  " << getApplicationRow["website"];
                if (!getApplicationRow["retirement_date"].empty())
                {
                  ssText << ", Retired:  " << getApplicationRow["retirement_date"];
                }
                ssText << ", Description:  " << getApplicationRow["description"];
                getApplicationRow.clear();
              }
              else
              {
                ssText << " error:  The application does not exist.";
              }
            }
            else
            {
              ssText << " error:  " << strError;
            }
            dbfree(getApplication);
          }
          // }}}
          // {{{ developers
          else if (strForm == "developers")
          {
            ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
            ssQuery.str("");
            ssQuery << "select b.type, c.id, c.last_name, c.first_name, c.userid from application_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and (b.type = 'Primary Developer' or b.type = 'Backup Developer') and a.application_id = " << strApplicationID << " order by b.id, c.last_name, c.first_name, c.userid";
            auto getApplicationContact = dbquery("central_r", ssQuery.str(), strError);
            if (getApplicationContact != NULL)
            {
              for (auto &getApplicationContactRow : *getApplicationContact)
              {
                ssText << endl << "Type:  " << getApplicationContactRow["type"] << ", Name:  " << getApplicationContactRow["last_name"] << ", " << getApplicationContactRow["first_name"] << " (" << getApplicationContactRow["userid"] << ")";
              }
            }
            else
            {
              ssText << " error:  " << strError;
            }
            dbfree(getApplicationContact);
          }
          // }}}
          // {{{ notify
          else if (strForm == "notify")
          {
            string strMessage = var("Message", ptData);
            if (!strMessage.empty())
            {
              string strPayload, strValue;
              stringstream ssTime;
              time_t CTime;
              Json *ptJwt = new Json;
              ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
              ptJwt->i("sl_first_name", strFirstName);
              ptJwt->i("sl_last_name", strLastName);
              ptJwt->i("sl_login", strIdent);
              time(&CTime);
              ssTime << CTime;
              ptJwt->i("exp", ssTime.str(), 'n');
              ptJwt->i("sl_admin", ((bAdmin)?"1":"0"), ((bAdmin)?'1':'0'));
              ptJwt->m["sl_auth"] = new Json;
              for (auto &i : auth)
              { 
                ptJwt->m["sl_auth"]->i(i.first, ((i.second)?"1":"0"), ((i.second)?'1':'0'));
              }
              if (jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
              {
                Json *ptJson = new Json;
                ptJson->i("Function", "applicationNotify");
                ptJson->m["Request"] = new Json;
                ptJson->m["Request"]->i("id", strApplicationID);
                ptJson->m["Request"]->i("notification", strMessage);
                ptJson->i("Jwt", m_manip.encodeBase64(m_manip.encryptAes(strPayload, m_strJwtSecret, strValue, strError), strValue));
                if (hub("central", ptJson, strError))
                {
                  ssText << ": done";
                }
                else
                {
                  ssText << " error:  " << strError;
                }
                delete ptJson;
              }
              else
              {
                ssText << " error:  " << strError;
              }
              delete ptJwt;
            }
            else
            {
              ssText << " error:  Please provide the message following notify.";
            }
          }
          // }}}
          // {{{ invalid
          else
          {
            ssText << ":  Please provide the Form immediately following the ApplicationID.";
          }
          // }}}
        }
        // }}}
        // {{{ name
        else
        {
          ssQuery.str("");
          ssQuery << "select id from application where name = '" << m_manip.escape(strApplicationID, strValue) << "'";
          list<map<string, string> > *getApplication = dbquery("central_r", ssQuery.str(), strError);
          if (getApplication != NULL)
          {
            if (!getApplication->empty())
            {
              map<string, string> getApplicationRow = getApplication->front();
              ssText << ":  The ApplicationID is " << getApplicationRow["id"] << ".";
              getApplicationRow.clear();
            }
            else
            {
              ssText << " error:  The application does not exist.";
            }
          }
          else
          {
            ssText << " error:  " << strError;
          }
          dbfree(getApplication);
        }
        // }}}
      }
      // }}}
      // {{{ invalid
      else
      {
        ssText << ":  Please provide the ApplicationID immediately following the application.";
      }
      // }}}
    }
    // }}}
    // {{{ group
    else if (strFunction == "group")
    {
      string strGroupID = var("GroupID", ptData);
      ssText << " " << char(3) << "00,14 " << strFunction << " " << char(3);
      // {{{ group
      if (!strGroupID.empty())
      {
        // {{{ id
        if (m_manip.isNumeric(strGroupID))
        {
          string strForm = var("Form", ptData);
          ssText << " " << char(3) << "00,14 " << strGroupID << " " << char(3);
          // {{{ general
          if (strForm == "general")
          {
            ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
            ssQuery.str("");
            ssQuery << "select name, date_format(creation_date, '%Y-%m-%d') creation_date, description from `group` where id = " << strGroupID;
            list<map<string, string> > *getGroup = dbquery("central_r", ssQuery.str(), strError);
            if (getGroup != NULL)
            {
              if (!getGroup->empty())
              {
                map<string, string> getGroupRow = getGroup->front();
                ssText << " Group:  " << getGroupRow["name"];
                ssText << ", Description:  " << getGroupRow["description"];
                getGroupRow.clear();
              }
              else
              {
                ssText << " error:  The group does not exist.";
              }
            }
            else
            {
              ssText << " error:  " << strError;
            }
            dbfree(getGroup);
          }
          // }}}
          // {{{ notify
          else if (strForm == "notify")
          {
            string strMessage = var("Message", ptData);
            if (!strMessage.empty())
            {
              string strPayload, strValue;
              stringstream ssTime;
              time_t CTime;
              Json *ptJwt = new Json;
              ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
              ptJwt->i("sl_first_name", strFirstName);
              ptJwt->i("sl_last_name", strLastName);
              ptJwt->i("sl_login", strIdent);
              time(&CTime);
              ssTime << CTime;
              ptJwt->i("exp", ssTime.str(), 'n');
              ptJwt->i("sl_admin", ((bAdmin)?"1":"0"), ((bAdmin)?'1':'0'));
              ptJwt->m["sl_auth"] = new Json;
              for (auto &i : auth)
              { 
                ptJwt->m["sl_auth"]->i(i.first, ((i.second)?"1":"0"), ((i.second)?'1':'0'));
              }
              if (jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
              {
                Json *ptJson = new Json;
                ptJson->i("Function", "groupNotify");
                ptJson->m["Request"] = new Json;
                ptJson->m["Request"]->i("id", strGroupID);
                ptJson->m["Request"]->i("notification", strMessage);
                ptJson->i("Jwt", m_manip.encodeBase64(m_manip.encryptAes(strPayload, m_strJwtSecret, strValue, strError), strValue));
                if (hub("central", ptJson, strError))
                {
                  ssText << ": done";
                }
                else
                {
                  ssText << " error:  " << strError;
                }
                delete ptJson;
              }
              else
              {
                ssText << " error:  " << strError;
              }
              delete ptJwt;
            }
            else
            {
              ssText << " error:  Please provide the message following notify.";
            }
          }
          // }}}
          // {{{ owners
          else if (strForm == "owners")
          {
            ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
            ssQuery.str("");
            ssQuery << "select b.type, c.id, c.last_name, c.first_name, c.userid from group_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and (b.type = 'Primary Owner' or b.type = 'Backup Owner') and a.group_id = " << strGroupID << " order by b.id, c.last_name, c.first_name, c.userid";
            auto getGroupContact = dbquery("central_r", ssQuery.str(), strError);
            if (getGroupContact != NULL)
            {
              for (auto &getGroupContactRow : *getGroupContact)
              {
                ssText << endl << "Type:  " << getGroupContactRow["type"] << ", Name:  " << getGroupContactRow["last_name"] << ", " << getGroupContactRow["first_name"] << " (" << getGroupContactRow["userid"] << ")";
              }
            }
            else
            {
              ssText << " error:  " << strError;
            }
            dbfree(getGroupContact);
          }
          // }}}
          // {{{ invalid
          else
          {
            ssText << ":  Please provide the Form immediately following the GroupID.";
          }
          // }}}
        }
        // }}}
        // {{{ name
        else
        {
          ssQuery.str("");
          ssQuery << "select id from `group` where name = '" << m_manip.escape(strGroupID, strValue) << "'";
          list<map<string, string> > *getGroup = dbquery("central_r", ssQuery.str(), strError);
          if (getGroup != NULL)
          {
            if (!getGroup->empty())
            {
              map<string, string> getGroupRow = getGroup->front();
              ssText << ":  The GroupID is " << getGroupRow["id"] << ".";
              getGroupRow.clear();
            }
            else
            {
              ssText << " error:  The group does not exist.";
            }
          }
          else
          {
            ssText << " error:  " << strError;
          }
          dbfree(getGroup);
        }
        // }}}
      }
      // }}}
      // {{{ invalid
      else
      {
        ssText << ":  Please provide the GroupID immediately following the group.";
      }
      // }}}
    }
    // }}}
    // {{{ server
    else if (strFunction == "server")
    {
      string strServerID = var("ServerID", ptData);
      ssText << " " << char(3) << "00,14 " << strFunction << " " << char(3);
      // {{{ server
      if (!strServerID.empty())
      {
        // {{{ id
        if (m_manip.isNumeric(strServerID))
        {
          string strForm = var("Form", ptData);
          ssText << " " << char(3) << "00,14 " << strServerID << " " << char(3);
          // {{{ general
          if (strForm == "general")
          {
            ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
            ssQuery.str("");
            ssQuery << "select name, description from server where id = " << strServerID;
            list<map<string, string> > *getServer = dbquery("central_r", ssQuery.str(), strError);
            if (getServer != NULL)
            {
              if (!getServer->empty())
              {
                map<string, string> getServerRow = getServer->front();
                ssText << " Server:  " << getServerRow["name"] << ", Description:  " << getServerRow["description"];
                getServerRow.clear();
              }
              else
              {
                ssText << " error:  The server does not exist.";
              }
            }
            else
            {
              ssText << " error:  " << strError;
            }
            dbfree(getServer);
          }
          // }}}
          // {{{ admins
          else if (strForm == "admins")
          {
            ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
            ssQuery.str("");
            ssQuery << "select b.type, c.id, c.last_name, c.first_name, c.userid from server_contact a, contact_type b, person c where a.type_id = b.id and a.contact_id = c.id and (b.type = 'Primary Admin' or b.type = 'Backup Admin') and a.server_id = " << strServerID << " order by b.id, c.last_name, c.first_name, c.userid";
            auto getServerContact = dbquery("central_r", ssQuery.str(), strError);
            if (getServerContact != NULL)
            {
              for (auto &getServerContactRow : *getServerContact)
              {
                ssText << endl << " Type:  " << getServerContactRow["type"] << ", Name:  " << getServerContactRow["last_name"] << ", " << getServerContactRow["first_name"] << "(" << getServerContactRow["userid"] << ")";
              }
            }
            else
            {
              ssText << " error:  " << strError;
            }
            dbfree(getServerContact);
          }
          // }}}
          // {{{ notify
          else if (strForm == "notify")
          {
            string strMessage = var("Message", ptData);
            if (!strMessage.empty())
            {
              string strPayload, strValue;
              stringstream ssTime;
              time_t CTime;
              Json *ptJwt = new Json;
              ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
              ptJwt->i("sl_first_name", strFirstName);
              ptJwt->i("sl_last_name", strLastName);
              ptJwt->i("sl_login", strIdent);
              time(&CTime);
              ssTime << CTime;
              ptJwt->i("exp", ssTime.str(), 'n');
              ptJwt->i("sl_admin", ((bAdmin)?"1":"0"), ((bAdmin)?'1':'0'));
              ptJwt->m["sl_auth"] = new Json;
              for (auto &i : auth)
              { 
                ptJwt->m["sl_auth"]->i(i.first, ((i.second)?"1":"0"), ((i.second)?'1':'0'));
              }
              if (jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
              {
                Json *ptJson = new Json;
                ptJson->i("Function", "serverNotify");
                ptJson->m["Request"] = new Json;
                ptJson->m["Request"]->i("id", strServerID);
                ptJson->m["Request"]->i("notification", strMessage);
                ptJson->i("Jwt", m_manip.encodeBase64(m_manip.encryptAes(strPayload, m_strJwtSecret, strValue, strError), strValue));
                if (hub("central", ptJson, strError))
                {
                  ssText << ": done";
                }
                else
                {
                  ssText << " error:  " << strError;
                }
                delete ptJson;
              }
              else
              {
                ssText << " error:  " << strError;
              }
              delete ptJwt;
            }
            else
            {
              ssText << " error:  Please provide the message following notify.";
            }
          }
          // }}}
          // {{{ invalid
          else
          {
            ssText << ":  Please provide the Form immediately following the ServerID.";
          }
          // }}}
        }
        // }}}
        // {{{ name
        else
        {
          ssQuery.str("");
          ssQuery << "select id from server where name = '" << m_manip.escape(strServerID, strValue) << "'";
          list<map<string, string> > *getServer = dbquery("central_r", ssQuery.str(), strError);
          if (getServer != NULL)
          {
            if (!getServer->empty())
            {
              map<string, string> getServerRow = getServer->front();
              ssText << ":  The ServerID is " << getServerRow["id"] << ".";
              getServerRow.clear();
            }
            else
            {
              ssText << " error:  The server does not exist.";
            }
          }
          else
          {
            ssText << " error:  " << strError;
          }
          dbfree(getServer);
        }
        // }}}
      }
      // }}}
      // {{{ invalid
      else
      {
        ssText << ":  Please provide the ServerID immediately following the server.";
      }
      // }}}
    }
    // }}}
    // {{{ user
    else if (strFunction == "user")
    {
      string strUser = var("User", ptData);
      ssText << " " << char(3) << "00,14 " << strFunction << " " << char(3);
      if (!strUser.empty())
      {
        string strForm = var("Form", ptData);
        ssText << " " << char(3) << "00,14 " << strUser << " " << char(3);
        // {{{ general
        if (strForm == "general")
        {
          ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
          ssQuery.str("");
          ssQuery << "select id, last_name, first_name, email, pager, active, admin, locked from person where userid = '" << strUser << "'";
          list<map<string, string> > *getPerson = dbquery("central_r", ssQuery.str(), strError);
          if (getPerson != NULL)
          {
            if (!getPerson->empty())
            {
              map<string, string> getPersonRow = getPerson->front();
              ssText << "Name:  " << getPersonRow["last_name"] << ", " << getPersonRow["first_name"] << "(" << strUser << "), Email:  " << getPersonRow["email"] << ", Pager:  " << getPersonRow["pager"] << ", Active:  " << ((getPersonRow["active"] == "1")?"Yes":"No") << ", Admin:  " << ((getPersonRow["admin"] == "1")?"Yes":"No") << ", Locked:  " << ((getPersonRow["locked"] == "1")?"Yes":"No");
              getPersonRow.clear();
            }
            else
            {
              ssText << " error:  The user does not exist.";
            }
          }
          else
          {
            ssText << " error:  " << strError;
          }
          dbfree(getPerson);
        }
        // }}}
        // {{{ applications
        else if (strForm == "applications")
        {
          bool bFound = false;
          list<string> types = {"all", "primary developer", "backup developer", "primary contact", "contact"};
          string strType = var("Type", ptData);
          ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
          for (auto i = types.begin(); !bFound && i != types.end(); i++)
          {
            if (strType == (*i))
            {
              bFound = true;
            }
          }
          if (bFound)
          {
            ssText << " " << char(3) << "00,14 " << strType << " " << char(3);
            ssQuery.str("");
            ssQuery << "select a.id, a.name, c.type from application a, application_contact b, contact_type c, person d where a.id = b.application_id and b.type_id = c.id and b.contact_id = d.id and d.userid = '" << strUser << "'";
            if (strType != "all")
            {
              ssQuery << " and c.type = '" << strType << "'";
            }
            ssQuery << " order by a.name";
            auto getApplication = dbquery("central_r", ssQuery.str(), strError);
            if (getApplication != NULL)
            {
              for (auto &getApplicationRow : *getApplication)
              {
                ssText << endl << "Application:  " << getApplicationRow["name"] << ", Contact Type:  " << getApplicationRow["type"];
              }
            }
            else
            {
              ssText << " error:  " << strError;
            }
            dbfree(getApplication);
          }
          else
          {
            ssText << ":  Please provide the Contact Type immediately following applications.";
          }
          types.clear();
        }
        // }}}
        // {{{ notify
        else if (strForm == "notify")
        {
          string strMessage = var("Message", ptData);
          if (!strMessage.empty())
          {
            string strPayload, strValue;
            stringstream ssTime;
            time_t CTime;
            Json *ptJwt = new Json;
            ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
            ptJwt->i("sl_first_name", strFirstName);
            ptJwt->i("sl_last_name", strLastName);
            ptJwt->i("sl_login", strIdent);
            time(&CTime);
            ssTime << CTime;
            ptJwt->i("exp", ssTime.str(), 'n');
            ptJwt->i("sl_admin", ((bAdmin)?"1":"0"), ((bAdmin)?'1':'0'));
            ptJwt->m["sl_auth"] = new Json;
            for (auto &i : auth)
            { 
              ptJwt->m["sl_auth"]->i(i.first, ((i.second)?"1":"0"), ((i.second)?'1':'0'));
            }
            if (jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
            {
              Json *ptJson = new Json;
              ptJson->i("Function", "userNotify");
              ptJson->m["Request"] = new Json;
              ptJson->m["Request"]->i("userid", strUser);
              ptJson->m["Request"]->i("notification", strMessage);
              ptJson->i("Jwt", m_manip.encodeBase64(m_manip.encryptAes(strPayload, m_strJwtSecret, strValue, strError), strValue));
              if (hub("central", ptJson, strError))
              {
                ssText << ": done";
              }
              else
              {
                ssText << " error:  " << strError;
              }
              delete ptJson;
            }
            else
            {
              ssText << " error:  " << strError;
            }
            delete ptJwt;
          }
          else
          {
            ssText << " error:  Please provide the message following notify.";
          }
        }
        // }}}
        // {{{ reminders
        else if (strForm == "reminders")
        {
          map<string, string> getUserRow;
          Json *ptUser = new Json;
          ptUser->i("userid", strIdent);
          if (db("dbCentralUsers", ptUser, getUserRow, strQuery, strError))
          {
            string strPayload, strValue;
            stringstream ssTime;
            time_t CTime;
            Json *ptJwt = new Json;
            ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
            ptJwt->i("sl_first_name", strFirstName);
            ptJwt->i("sl_last_name", strLastName);
            ptJwt->i("sl_login", strIdent);
            time(&CTime);
            ssTime << CTime;
            ptJwt->i("exp", ssTime.str(), 'n');
            ptJwt->i("sl_admin", ((bAdmin)?"1":"0"), ((bAdmin)?'1':'0'));
            ptJwt->m["sl_auth"] = new Json;
            for (auto &i : auth)
            { 
              ptJwt->m["sl_auth"]->i(i.first, ((i.second)?"1":"0"), ((i.second)?'1':'0'));
            }
            if (jwt(m_strJwtSigner, m_strJwtSecret, strPayload, ptJwt, strError))
            {
              Json *ptJson = new Json;
              ptJson->i("Function", "userReminders");
              ptJson->m["Request"] = new Json;
              ptJson->m["Request"]->i("person_id", getUserRow["id"]);
              ptJson->i("Jwt", m_manip.encodeBase64(m_manip.encryptAes(strPayload, m_strJwtSecret, strValue, strError), strValue));
              if (hub("central", ptJson, strError))
              {
                ssText << ": done";
                if (exist(ptJson, "Response"))
                {
                  for (auto &reminder : ptJson->m["Response"]->l)
                  {
                    if (!empty(reminder, "title"))
                    {
                      ssText << endl;
                      if (exist(reminder, "cron") && !empty(reminder->m["cron"], "value") && !empty(reminder, "sched"))
                      {
                        ssText << char(3) << ((reminder->m["cron"]->m["value"]->v == "0")?"00,14":"07,05") << " " << reminder->m["sched"]->v << " " << char(3) << " ";
                      }
                      ssText << reminder->m["title"]->v;
                    }
                  }
                }
              }
              else
              {
                ssText << " error:  " << strError;
              }
              delete ptJson;
            }
            delete ptJwt;
          }
          else
          {
            ssText << " error:  " << strError;
          }
          delete ptUser;
        }
        // }}}
        // {{{ servers
        else if (strForm == "servers")
        {
          bool bFound = false;
          list<string> types = {"all", "primary admin", "backup admin", "primary contact", "contact"};
          string strType = var("Type", ptData);
          ssText << " " << char(3) << "00,14 " << strForm << " " << char(3);
          for (auto i = types.begin(); !bFound && i != types.end(); i++)
          {
            if (strType == (*i))
            {
              bFound = true;
            }
          }
          if (bFound)
          {
            ssText << " " << char(3) << "00,14 " << strType << " " << char(3);
            ssQuery.str("");
            ssQuery << "select a.id, a.name, c.type from server a, server_contact b, contact_type c, person d where a.id = b.server_id and b.type_id = c.id and b.contact_id = d.id and d.userid = '" << strUser << "'";
            if (strType != "all")
            {
              ssQuery << " and c.type = '" << strType << "'";
            }
            ssQuery << " order by a.name";
            list<map<string, string> > *getServer = dbquery("central_r", ssQuery.str(), strError);
            if (getServer != NULL)
            {
              for (auto &getServerRow : *getServer)
              {
                ssText << endl << "Server:  " << getServerRow["name"] << ", Contact Type:  " << getServerRow["type"];
              }
            }
            else
            {
              ssText << " error:  " << strError;
            }
            dbfree(getServer);
          }
          else
          {
            ssText << ":  Please provide the Contact Type immediately following servers.";
          }
          types.clear();
        }
        // }}}
        // {{{ invalid
        else
        {
          ssText << ":  Please provide the Form immediately following the User:  general, applications, reminders, servers.";
        }
        // }}}
      }
      else
      {
        ssText << ":  Please provide the User immediately following the user.";
      }
    }
    // }}}
    // {{{ invalid
    else
    {
      ssText << ":  The central action is used to access Central functionalities.  Please provide one of the following functions immediately following the action:  application, group, server, user." << endl;
    }
    // }}}
  }
  // }}}
  // {{{ centralmon
  else if (strAction == "centralmon")
  {
    string strServer = var("Server", ptData);
    if (!strServer.empty())
    {
      string strProcess = var("Process", ptData);
      Json *ptResponse = new Json;
      ssText << " " << char(3) << "00,14 " << strServer << " " << char(3);
      if (!strProcess.empty())
      {
        ssText << " " << char(3) << "00,14 " << strProcess << " " << char(3);
      }
      if (centralmon(strServer, strProcess, ptResponse, strError))
      {
        if (!strProcess.empty())
        {
          if (!empty(ptResponse, "StartTime"))
          {
            ssText << endl << "Start Time:  " << ptResponse->m["StartTime"]->v;
          }
          if (!empty(ptResponse, "NumberOfProcesses"))
          {
            ssText << endl << "# Processes:  " << ptResponse->m["NumberOfProcesses"]->v;
          }
          if (!empty(ptResponse, "ImageSize"))
          {
            ssText << endl << "Image Size:  " << ptResponse->m["ImageSize"]->v << " KB";
          }
          if (!empty(ptResponse, "ResidentSize"))
          {
            ssText << endl << "Resident Size:  " << ptResponse->m["ResidentSize"]->v << " KB";
          }
          if (exist(ptResponse, "Owners") && !ptResponse->m["Owners"]->m.empty())
          {
            bool bFirst = true;
            ssText << endl << "Process Owners:  (";
            for (auto &i : ptResponse->m["Owners"]->m)
            {
              ssText << ((bFirst)?"":", ") << i.first << ":  " << i.second->v << " processes";
              bFirst = false;
            }
            ssText << ")";
          }
        }
        else
        {
          if (!empty(ptResponse, "OperatingSystem"))
          {
            ssText << endl << "Operating System:  " << ptResponse->m["OperatingSystem"]->v;
          }
          if (!empty(ptResponse, "SystemRelease"))
          {
            ssText << endl << "System Release:  " << ptResponse->m["SystemRelease"]->v;
          }
          if (!empty(ptResponse, "UpTime"))
          {
            ssText << endl << "Uptime:  " << ptResponse->m["UpTime"]->v << " days";
          }
          if (!empty(ptResponse, "MainUsed") && !empty(ptResponse, "MainTotal"))
          {
            ssText << endl << "Memory Used:  " << ptResponse->m["MainUsed"]->v << " of " << ptResponse->m["MainTotal"]->v << " MB";
          }
          if (!empty(ptResponse, "SwapUsed") && !empty(ptResponse, "SwapTotal"))
          {
            ssText << endl << "Swap Used:  " << ptResponse->m["SwapUsed"]->v << " of " << ptResponse->m["SwapTotal"]->v << " MB";
          }
          if (!empty(ptResponse, "NumberOfProcessors"))
          {
            ssText << endl << "# Processors:  " << ptResponse->m["NumberOfProcessors"]->v;
          }
          if (!empty(ptResponse, "CpuSpeed"))
          {
            ssText << endl << "CPU Speed:  " << ptResponse->m["CpuSpeed"]->v << " MHz";
          }
          if (!empty(ptResponse, "CpuUsage"))
          {
            ssText << endl << "CPU Usage:  " << ptResponse->m["CpuUsage"]->v << "%";
          }
          if (!empty(ptResponse, "NumberOfProcesses"))
          {
            ssText << endl << "# Processes:  " << ptResponse->m["NumberOfProcesses"]->v;
          }
          if (exist(ptResponse, "Partitions") && !ptResponse->m["Partitions"]->m.empty())
          {
            bool bFirst = true;
            ssText << endl << "Partitions:  (";
            for (auto &i : ptResponse->m["Partitions"]->m)
            {
              ssText << ((bFirst)?"":", ") << i.first << ":  " << i.second->v << "% full";
              bFirst = false;
            }
            ssText << ")";
          }
        }
        if (exist(ptResponse, "Alarms") && (!ptResponse->m["Alarms"]->l.empty() || !ptResponse->m["Alarms"]->v.empty()))
        {
          ssText << endl << "Alarms:  (";
          if (!ptResponse->m["Alarms"]->l.empty())
          {
            bool bFirst = true;
            for (auto &i : ptResponse->m["Alarms"]->l)
            {
              ssText << char(3) << "04" << ((bFirst)?"":", ") << i->v << char(3);
              bFirst = false;
            }
            ssText << ")";
          }
          else
          {
            ssText << char(3) << "04" << ptResponse->m["Alarms"]->v << char(3);
          }
          ssText << ")";
        }
      }
      else
      {
        ssText << " error:  " << strError;
      }
      delete ptResponse;
    }
    else
    {
      ssText << ":  The centralmon action is used to obtain status information about a running system or process.  Please provide the server immediately following the action and follow the server if you would like to obtain process information instead of system information.";
    }
  }
  // }}}
  // {{{ command
  else if (strAction == "command")
  {
    if (isLocalAdmin(strIdent, "Radial", bAdmin, auth))
    {
      string strNode = var("Node", ptData);
      if (!strNode.empty())
      {
        string strCommand = var("Command", ptData);
        ssText << " " << char(3) << "00,14 " << strNode << " " << char(3);
        if (!strCommand.empty())
        {
          size_t unDuration = 0;
          string strOutput;
          if (command(strCommand, {}, "", strOutput, unDuration, strError, 300, strNode))
          {
            ssText << ":  Command ran for " << unDuration << " seconds.";
            ssText << endl << strOutput;
          }
          else
          {
            ssText << " error:  " << strError;
          }
        }
        else
        {
          ssText << ":  Please provide the command immediately following the server.";
        }
      }
      else
      {
        ssText << ":  The centralmon action is used to obtain status information about a running system or process.  Please provide the server immediately following the action and follow the server if you would like to obtain process information instead of system information.";
      }
    }
    else
    {
      ssText << " error:  You are not authorized to access command.  You must be registered as a local administrator for Radial.";
    }
  }
  // }}}
  // {{{ database
  else if (strAction == "database")
  {
    if (isLocalAdmin(strIdent, "Radial", bAdmin, auth))
    {
      string strDatabase = var("Database", ptData);
      if (!strDatabase.empty())
      {
        string strQuery = var("Query", ptData);
        if (!strQuery.empty())
        {
          auto get = dbquery(strDatabase, strQuery, strError);
          if (get != NULL)
          {
            ssText << ":  Query sent.";
            for (auto &row : (*get))
            {
              Json *ptRow = new Json(row);
              ssText << endl << ptRow;
              delete ptRow;
            }
          }
          else
          {
            ssText << ":  " << strError;
          }
        }
        else
        {
          ssText << ":  Please provide a Query immediately following the Database.";
        }
      }
      else
      {
        ssText << ":  The database action is used to send a Query to a Database.  Please provide a Database immediately following the action.";
      }
    }
    else
    {
      ssText << " error:  You are not authorized to access database.  You must be registered as a local administrator for Radial.";
    }
  }
  // }}}
  // {{{ date
  else if (strAction == "date")
  {
    string strDate = var("Date", ptData);
    struct tm tTime;
    time_t CTime;
    if (!strDate.empty())
    {
      string strTime = var("Time", ptData);
      if (!strTime.empty())
      {
        string strDay, strHour, strMinute, strMonth, strSecond, strYear;
        stringstream ssDate(strDate), ssTime(strTime);
        if (strDate.find("-") != string::npos)
        {
          getline(ssDate, strYear, '-');
          getline(ssDate, strMonth, '-');
          getline(ssDate, strDay, '-');
        }
        else
        {
          getline(ssDate, strMonth, '/');
          getline(ssDate, strDay, '/');
          getline(ssDate, strYear, '/');
        }
        getline(ssTime, strHour, ':');
        getline(ssTime, strMinute, ':');
        getline(ssTime, strSecond, ':');
        tTime.tm_year = atoi(strYear.c_str()) - 1900;
        tTime.tm_mon = atoi(strMonth.c_str()) - 1;
        tTime.tm_mday = atoi(strDay.c_str());
        tTime.tm_hour = atoi(strHour.c_str());
        tTime.tm_min = atoi(strMinute.c_str());
        tTime.tm_sec = atoi(strSecond.c_str());
        tTime.tm_isdst = -1;
        CTime = mktime(&tTime);
      }
      else
      {
        stringstream ssTime(strDate);
        ssTime >> CTime;
      }
    }
    else
    {
      time(&CTime);
    }
    localtime_r(&CTime, &tTime);
    ssText << ":  " << put_time(&tTime, "%Y-%m-%d %H:%M:%S %s");
  }
  // }}}
  // {{{ db
  else if (strAction == "db")
  {
    if (isLocalAdmin(strIdent, "Radial", bAdmin, auth))
    {
      string strFunction = var("Function", ptData);
      if (!strFunction.empty())
      {
        string strRequest = var("Request", ptData);
        if (!strRequest.empty())
        {
          string strID, strQuery;
          Json *ptIn = new Json(strRequest), *ptOut = new Json;
          if (db(strFunction, ptIn, ptOut, strID, strQuery, strError))
          {
            ssText << ":  Request sent.";
            if (!strID.empty())
            {
              ssText << endl << "ID:  " << strID;
            }
            for (auto &row : ptOut->l)
            {
              Json *ptRow = new Json(row);
              ssText << endl << ptRow;
              delete ptRow;
            }
          }
          else
          {
            ssText << ":  " << strError;
          }
        }
        else
        {
          ssText << ":  Please provide a Request immediately following the Function.";
        }
      }
      else
      {
        ssText << ":  The db action is used to call a database function.  Please provide a Function immediately following the action.";
      }
    }
    else
    {
      ssText << " error:  You are not authorized to access db.  You must be registered as a local administrator for Radial.";
    }
  }
  // }}}
  // {{{ feedback || fb
  else if (strAction == "feedback" || strAction == "fb")
  {
    string strData = var("Data", ptData);
    lock();
    if (m_feedbackClients.find(strIdent) != m_feedbackClients.end())
    {
      if (!strData.empty())
      {
        m_feedbackClients[strIdent].push_back(strData);
      }
      else
      {
        ssText << ":  Please provide an answer to submit an answer or provide exit to abandon the survey.";
      }
    }
    else if (!strData.empty())
    {
      m_feedbackClients[strIdent] = {};
      thread threadFeedback(&Irc::feedback, this, strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, strData, strSource);
      pthread_setname_np(threadFeedback.native_handle(), "feedback");
      threadFeedback.detach();
    }
    else
    {
      ssText << ":  The feedback action is used to take a feedback survey.  Please provide a hash immediately following the action.";
    }
    unlock();
  }
  // }}}
  // {{{ interface
  else if (strAction == "interface")
  {
    string strInterface = var("Interface", ptData);
    if (!strInterface.empty())
    {
      string strFunction = var("Function", ptData);
      ssText << " " << char(3) << "00,14 " << strInterface << " " << char(3);
      if (!strFunction.empty())
      {
        if (strFunction == "restart" || strFunction == "start" || strFunction == "status" || strFunction == "stop")
        {
          ssText << " " << char(3) << "00,14 " << strFunction << " " << char(3);
          if (strFunction == "status" || isLocalAdmin(strIdent, "Radial", bAdmin, auth))
          {
            list<string> nodes;
            map<string, string> results;
            string strNode = var("Node", ptData);
            if (!strNode.empty())
            {
              nodes.push_back(strNode);
            }
            else
            {
              m_mutexShare.lock();
              for (auto &link : m_l)
              {
                nodes.push_back(link->strNode);
              }
              m_mutexShare.unlock();
              nodes.push_back(m_strNode);
            }
            if (!nodes.empty())
            {
              for (auto &node : nodes)
              {
                bool bSubResult = false;
                string strResult = "done";
                stringstream ssSubText;
                if (strFunction == "status")
                {
                  bool bFoundInterface = false;
                  m_mutexShare.lock();
                  if (node == m_strNode)
                  {
                    if (m_i.find(strInterface) != m_i.end())
                    {
                      bFoundInterface = true;
                    }
                  }
                  else
                  {
                    bool bFoundNode = false;
                    for (auto l = m_l.begin(); !bFoundNode && l != m_l.end(); l++)
                    {
                      if ((*l)->strNode == node)
                      {
                        bFoundNode = true;
                        if ((*l)->interfaces.find(strInterface) != (*l)->interfaces.end())
                        {
                          bFoundInterface = true;
                        }
                      }
                    }
                  }
                  m_mutexShare.unlock();
                  if (bFoundInterface)
                  {
                    string strT = strInterface;
                    Json *ptJson = new Json;
                    if (node != m_strNode)
                    {
                      ptJson->i("Interface", strInterface);
                      ptJson->i("Node", node);
                      strT = "link";
                    }
                    ptJson->i("|function", "status");
                    if (hub(strT, ptJson, strError))
                    {
                      bSubResult = true;
                      if (exist(ptJson, "Response"))
                      {
                        ptJson->m["Response"]->j(strResult);
                      }
                    }
                    delete ptJson;
                  }
                }
                else if (strInterface == "irc" && node == m_strNode && (strFunction == "restart" || strFunction == "stop"))
                {
                  bSubResult = true;
                  setShutdown();
                }
                else if (strFunction == "start" || interfaceRemove(node, strInterface, strError) || strError.find("Encountered an unknown error.") != string::npos || strError == "Interface not found.")
                {
                  bool bStopped = false;
                  time_t CTime[2];
                  time(&(CTime[0]));
                  CTime[1] = CTime[0];
                  while (!bStopped && (CTime[1] - CTime[0]) < 40)
                  {
                    m_mutexShare.lock();
                    if (node == m_strNode)
                    {
                      if (m_i.find(strInterface) == m_i.end())
                      {
                        bStopped = true;
                      }
                    }
                    else
                    {
                      auto linkIter = m_l.end();
                      for (auto i = m_l.begin(); linkIter == m_l.end() && i != m_l.end(); i++)
                      {
                        if ((*i)->strNode == node)
                        {
                          linkIter = i;
                        }
                      }
                      if (linkIter != m_l.end())
                      {
                        if ((*linkIter)->interfaces.find(strInterface) == (*linkIter)->interfaces.end())
                        {
                          bStopped = true;
                        }
                      }
                      else
                      {
                        bStopped = true;
                      }
                    }
                    m_mutexShare.unlock();
                    if (strFunction == "start")
                    {
                      CTime[1] += 40;
                    }
                    else
                    {
                      msleep(250);
                      time(&(CTime[1]));
                    }
                  }
                  if (bStopped)
                  {
                    if (strFunction == "stop" || interfaceAdd(node, strInterface, strError))
                    {
                      bSubResult = true;
                    }
                  }
                  else if (strFunction == "start")
                  {
                    strError = "Already started.";
                  }
                  else
                  {
                    strError = "Failed to stop.";
                  }
                }
                ssSubText << node << ":  " << ((bSubResult)?strResult:strError);
                chat(strTarget, ssSubText.str(), strSource);
              }
              ssText << ":  done";
            }
            else
            {
              ssText << ":  Interface does not exist.";
            }
          }
          else
          {
            ssText << " error:  You are not authorized to restart/start/stop an interface.  You must be registered as a local administrator for Radial.";
          }
        }
        else
        {
          ssText << ":  Please provide a valid Function immediately following the Interface:  restart, start, status, stop.";
        }
      }
      else
      {
        bool bFirst = true, bFound = false;
        ssText << ":  ";
        m_mutexShare.lock();
        for (auto interface = m_i.begin(); !bFound && interface != m_i.end(); interface++)
        {
          if (interface->first == strInterface)
          {
            bFound = true;
          }
        }
        if (bFound)
        {
          if (bFirst)
          {
            bFirst = false;
          }
          else
          {
            ssText << ", ";
          }
          ssText << m_strNode;
        }
        for (auto &link : m_l)
        {
          bFound = false;
          for (auto interface = link->interfaces.begin(); !bFound && interface != link->interfaces.end(); interface++)
          {
            if (interface->first == strInterface)
            {
              bFound = true;
            }
          }
          if (bFound)
          {
            if (bFirst)
            {
              bFirst = false;
            }
            else
            {
              ssText << ", ";
            }
            ssText << link->strNode;
          }
        }
        m_mutexShare.unlock();
      }
    }
    else
    {
      ssText << ":";
      m_mutexShare.lock();
      ssText << endl << m_strNode << ":  ";
      for (auto interface = m_i.begin(); interface != m_i.end(); interface++)
      {
        if (interface != m_i.begin())
        {
          ssText << ", ";
        }
        ssText << interface->first;
      }
      for (auto &link : m_l)
      {
        ssText << endl << link->strNode << ":  ";
        for (auto interface = link->interfaces.begin(); interface != link->interfaces.end(); interface++)
        {
          if (interface != link->interfaces.begin())
          {
            ssText << ", ";
          }
          ssText << interface->first;
        }
      }
      m_mutexShare.unlock();
    }
  }
  // }}}
  // {{{ irc
  else if (strAction == "irc")
  {
    string strFunction = var("Function", ptData);
    if (strFunction == "channels")
    {
      ssText << ":  ";
      for (auto i = m_channels.begin(); i != m_channels.end(); i++)
      {
        if (i != m_channels.begin())
        {
          ssText << ", ";
        }
        if (i->second)
        {
          ssText << char(2);
        }
        ssText << i->first;
        if (i->second)
        {
          ssText << char(2);
        }
      }
    }
    else if (strFunction == "chat" || strFunction == "join" || strFunction == "part")
    {
      string strSubTarget = var("Target", ptData);
      if (!strSubTarget.empty())
      {
        if (strFunction == "chat")
        {
          string strMessage = var("Message", ptData);
          if (!strMessage.empty())
          {
            stringstream ssMessage;
            ssMessage << char(3) << "08,03 " << strUserID;
            if (strUserID != strTarget)
            {
              ssMessage << " @ " << strTarget;
            }
            ssMessage << " " << char(3) << " " << strMessage;
            chat(strSubTarget, ssMessage.str());
            ssText << ":  Message sent.";
          }
          else
          {
            ssText << ":  Please provide a Message immediately following the Target.";
          }
        }
        else
        {
          if (m_channels.find(strSubTarget) != m_channels.end() && m_channels[strSubTarget])
          {
            if (strFunction == "join")
            {
              ssText << ":  Already joined channel.";
            }
            else
            {
              part(strSubTarget);
              ssText << ":  Parted channel.";
            }
          }
          else if (strFunction == "join")
          {
            join(strSubTarget);
            ssText << ":  Joined channel.";
          }
          else
          {
            ssText << ":  Already parted channel.";
          }
        }
      }
      else
      {
        ssText << ":  Please provide a Target immediately following the action.";
      }
    }
    else
    {
      ssText << ":  The irc action is used to interface with IRC via the chatbot.  Please provide one of the following functions immediately following the action:  channels, chat, join, part.";
    }
  }
  // }}}
  // {{{ live
  else if (strAction == "live")
  {
    string strApplication, strClass, strJson = var("Json", ptData), strMessage, strTitle, strUser;
    if (!strJson.empty())
    {
      Json *ptJson = new Json(strJson);
      if (!empty(ptJson, "Application"))
      {
        strApplication = ptJson->m["Application"]->v;
      }
      if (!empty(ptJson, "Class"))
      {
        strClass = ptJson->m["Class"]->v;
      }
      if (!empty(ptJson, "Message"))
      {
        strMessage = ptJson->m["Message"]->v;
      }
      if (!empty(ptJson, "Title"))
      {
        strTitle = ptJson->m["Title"]->v;
      }
      if (!empty(ptJson, "User"))
      {
        strUser = ptJson->m["User"]->v;
      }
      delete ptJson;
    }
    if (!strMessage.empty())
    {
      if (isLocalAdmin(strIdent, "Radial", bAdmin, auth) || (!strApplication.empty() && isLocalAdmin(strIdent, strApplication, bAdmin, auth)))
      {
        live(strApplication, strUser, {{"Action", "message"}, {"Class", ((!strClass.empty())?strClass:"info")}, {"Title", ((!strTitle.empty())?strTitle:strApplication)}, {"Body", strMessage}});
        ssText << ":  The message has been sent.";
      }
      else
      {
        ssText << " error:  You are not authorized to send a message to the Application.  You must be registered as a local administrator for the Application.";
      }
    }
    else
    {
      Json *ptFormat = new Json;
      ptFormat->i("Application", "");
      ptFormat->i("Class", "");
      ptFormat->i("Message", "");
      ptFormat->i("Title", "");
      ptFormat->i("User", "");
      ssText << ":  The live action is used to send a JSON formatted request to Radial Live.  You must provide both an Application and a Message.  Here are the available values for Class:  danger, dark, info, light, primary, secondary, success, and warning.  FORMAT:  " << ptFormat;
      delete ptFormat;
    }
  }
  // }}}
  // {{{ math
  else if (strAction == "math")
  {
    string strEquation = var("Equation", ptData);
    if (!strEquation.empty())
    {
      auto max{numeric_limits<long double>::digits10 + 1};
      long double ldResult;
      string strItem;
      stringstream ssEquation(strEquation);
      vector<string> equation;
      while (ssEquation >> strItem)
      {
        equation.push_back(strItem);
      }
      if (math(equation, ldResult, strError))
      {
        ssText << ":  " << setprecision(max) << ldResult;
      }
      else
      {
        ssText << ":  " << strError;
      }
    }
    else
    {
      ssText << ":  The math action is used to perform basic mathematics.  The following Functions are available:  abs, acos, asin, atan, cbrt, ceil, cos, exp, floor, sin, sqrt, tan.  Each item in the provided equation must be space delimited.  All sub-operations must be wrapped in parenthesis.  EX:  ( ( -1 * ( ( ( ( sqrt 64 ) + 4 ) / 2 ) ^ 3 ) ) + ( floor 300.47 ) ) % 10";
    }
  }
  // }}}
  // {{{ radial
  else if (strAction == "radial")
  {
    if (isLocalAdmin(strIdent, "Radial", bAdmin, auth))
    {
      string strInterface = var("Interface", ptData), strJson = var("Json", ptData);
      if (!strJson.empty())
      {
        Json *ptJson = new Json(strJson);
        if (hub(strInterface, ptJson, strError))
        {
          ssText << ":  " << ptJson;
        }
        else
        {
          ssText << " error:  " << strError;
        }
        delete ptJson;
      }
      else
      {
        ssText << ":  The radial action is used to send a JSON formatted request to Radial.  Please provide a JSON formatted request immediately following the Interface.";
      }
    }
    else
    {
      ssText << " error:  You are not authorized to access radial.  You must be registered as a local administrator for Radial.";
    }
  }
  // }}}
  // {{{ sqlite || sql
  else if (strAction == "sqlite" || strAction == "sql")
  {
    if (isLocalAdmin(strIdent, "Radial", bAdmin, auth) || isValid(strIdent, "Sqlite", bAdmin, auth))
    {
      string strFunction = var("Function", ptData);
      if (!strFunction.empty())
      {
        string strRemainder = var("Remainder", ptData);
        ssText << " " << char(3) << "00,14 " << strFunction << " " << char(3);
        if (strFunction == ".create")
        {
          string strDatabase, strNode;
          stringstream ssRemainder(strRemainder);
          ssRemainder >> strDatabase >> strNode;
          if (sqliteCreate(strDatabase, strNode, strError))
          {
            ssText << ":  Database created.";
          }
          else
          {
            ssText << ":  " << strError;
          }
        }
        else if (strFunction == ".databases")
        {
          map<string, map<string, string> > databases;
          if (sqliteDatabases(databases, strError))
          {
            list<map<string, string> > resultSet;
            ssText << ":  okay";
            for (auto &database : databases)
            {
              map<string, string> row;
              stringstream ssNodes;
              row["name"] = database.first;
              for (auto node = database.second.begin(); node != database.second.end(); node++)
              {
                if (node != database.second.begin())
                {
                  ssNodes << ", ";
                }
                ssNodes << node->first;
                if (node->second == "master")
                {
                  ssNodes << " (master)";
                }
              }
              row["nodes"] = ssNodes.str();
              resultSet.push_back(row);
            }
            ssText << sqliteDisplay(resultSet);
          }
          else
          {
            ssText << ":  " << strError;
          }
        }
        else if (strFunction == ".drop")
        {
          string strDatabase, strNode;
          stringstream ssRemainder(strRemainder);
          ssRemainder >> strDatabase >> strNode;
          if (sqliteDrop(strDatabase, strNode, strError))
          {
            ssText << ":  Database dropped.";
          }
          else
          {
            ssText << ":  " << strError;
          }
        }
        else
        {
          string strSubFunction;
          stringstream ssRemainder(strRemainder);
          ssRemainder >> strSubFunction;
          if (strSubFunction == ".desc")
          {
            string strTable;
            ssRemainder >> strTable;
            if (!strTable.empty())
            {
              list<map<string, string> > resultSet;
              size_t unID = 0, unRows = 0;
              stringstream ssStatement;
              ssStatement << "select sql from sqlite_master where name = '" << m_manip.escape(strTable, strValue) << "'";
              if (sqliteQuery(strFunction, ssStatement.str(), resultSet, unID, unRows, strError))
              {
                ssText << ":  okay";
                ssText << endl << "# of Rows Returned:  " << unRows;
                ssText << sqliteDisplay(resultSet);
              }
              else
              {
                ssText << ":  " << strError;
              }
            }
            else
            {
              ssText << ":  Please provide the table following the .desc.";
            }
          }
          else if (strSubFunction == ".tables")
          {
            list<map<string, string> > resultSet;
            size_t unID = 0, unRows = 0;
            if (sqliteQuery(strFunction, "select name from sqlite_master where type='table'", resultSet, unID, unRows, strError))
            {
              ssText << ":  okay";
              ssText << endl << "# of Rows Returned:  " << unRows;
              ssText << sqliteDisplay(resultSet);
            }
            else
            {
              ssText << ":  " << strError;
            }
          }
          else if (!strSubFunction.empty())
          {
            list<map<string, string> > resultSet;
            size_t unID = 0, unRows = 0;
            string strExtra, strLower;
            stringstream ssStatement;
            m_manip.toLower(strLower, strSubFunction);
            getline(ssRemainder, strExtra);
            ssStatement << strSubFunction << " " << strExtra;
            if (sqliteQuery(strFunction, ssStatement.str(), resultSet, unID, unRows, strError))
            {
              ssText << ":  okay";
              if (strLower == "select")
              {
                ssText << endl << "# of Rows Returned:  " << unRows;
                ssText << sqliteDisplay(resultSet);
              }
              else if (strLower == "delete" || strLower == "insert" || strLower == "update")
              {
                ssText << endl << "# of Rows " << ((strLower == "delete")?"Deleted":((strLower == "insert")?"Inserted":"Updated")) << ":  " << unRows;
                if (strLower == "insert")
                {
                  ssText << endl << "Lastest ID:  " << unID;
                }
              }
            }
            else
            {
              ssText << ":  " << strError;
            }
          }
          else
          {
            ssText << ":  Please provide an SQL statement or one of the following sub-Functions following the database:  .desc, .tables.";
          }
        }
      }
      else
      {
        ssText << ":  The sqlite action is used to interface with SQLite databases managed by Radial.  The following Functions are available:  .create, .databases, .drop, <database>.";
      }
    }
    else
    {
      ssText << " error:  You are not authorized to access radial.  You must be registered as a local administrator for Radial.";
    }
  }
  // }}}
  // {{{ ssh || s
  else if (strAction == "ssh" || strAction == "s")
  {
    string strFunction = var("Function", ptData);
    if (!strFunction.empty())
    {
      if (strFunction == "connect" || strFunction == "c")
      {
        string strCommand = var("Command", ptData), strConnect, strPassword, strPort, strServer, strUser;
        stringstream ssBuffer(strCommand), ssConnect, ssServer;
        ssBuffer >> strConnect;
        ssConnect.str(strConnect);
        getline(ssBuffer, strPassword);
        m_manip.trim(strPassword, strPassword);
        getline(ssConnect, strUser, '@');
        getline(ssConnect, strServer, '@');
        ssServer.str(strServer);
        getline(ssServer, strServer, ':');
        getline(ssServer, strPort, ':');
        if (!strServer.empty() && !strUser.empty())
        {
          lock();
          if (m_sshClients.find(strIdent) == m_sshClients.end())
          {
            m_sshClients[strIdent] = {};
            thread threadSsh(&Irc::ssh, this, strPrefix, strTarget, strUserID, strIdent, strServer, strPort, strUser, strPassword, strSource);
            pthread_setname_np(threadSsh.native_handle(), "ssh");
            threadSsh.detach();
          }
          else
          {
            ssText << " error:  You already have a session open.  Use the following action to close the session:  exit.";
          }
          unlock();
        }
        else
        {
          ssText << " error:  Please provide the connection string following the connect Function:  [user]@[server]<:port> <password>.";
        }
      }
      else if (strFunction == "ctrl-c")
      {
        stringstream ssCommand;
        ssCommand << char(3);
        lock();
        if (m_sshClients.find(strIdent) != m_sshClients.end())
        {
          m_sshClients[strIdent].push_back(ssCommand.str());
        }
        else
        {
          ssText << " error:  Please provide connect following the action.";
        }
        unlock();
      }
      else if (strFunction == "disconnect" || strFunction == "d")
      {
        lock();
        if (m_sshClients.find(strIdent) != m_sshClients.end())
        {
          m_sshClients.erase(strIdent);
        }
        else
        {
          ssText << " error:  Please provide connect following the action.";
        }
        unlock();
      }
      else if (strFunction == "send" || strFunction == "s" || strFunction == "sendenter" || strFunction == "se")
      {
        string strCommand = var("Command", ptData);
        if (strFunction == "sendenter" || strFunction == "se")
        {
          strCommand += "\n";
        }
        lock();
        if (m_sshClients.find(strIdent) != m_sshClients.end())
        {
          m_sshClients[strIdent].push_back(strCommand);
        }
        else
        {
          ssText << " error:  Please provide connect following the action.";
        }
        unlock();
      }
      else
      {
        ssText << " error:  Please provide a valid Function following the action:  connect (c), ctrl-c, disconnect (d), send (s), sendenter (se).";
      }
    }
    else
    {
      ssText << ":  The ssh action is used to establish and maintain an SSH session.  Please provide the connection string immediately following the action:  [user]@[server]<:port> <password>.";
    }
  }
  // }}}
  // {{{ storage
  else if (strAction == "storage")
  {
    if (isLocalAdmin(strIdent, "Radial", bAdmin, auth))
    {
      string strJson = var("Json", ptData);
      if (!strJson.empty())
      {
        Json *ptJson = new Json(strJson);
        if (hub("storage", ptJson, strError))
        {
          if (exist(ptJson, "Response") && (!ptJson->m["Response"]->l.empty() || !ptJson->m["Response"]->m.empty() || !ptJson->m["Response"]->v.empty()))
          {
            ssText << ":  " << ptJson->m["Response"];
          }
          else
          {
            ssText << ":  Completed request.";
          }
        }
        else
        {
          ssText << " error:  " << strError;
        }
        delete ptJson;
      }
      else
      {
        ssText << ":  The storage action is used to send a JSON formatted request to common storage within Radial.  Please provide a JSON formatted request immediately following the action.";
      }
    }
    else
    {
      ssText << " error:  You are not authorized to access storage.  You must be registered as a local administrator for Radial.";
    }
  }
  // }}}
  // {{{ terminal || t
  else if (strAction == "terminal" || strAction == "t")
  {
    string strFunction = var("Function", ptData);
    if (!strFunction.empty())
    {
      string strCommand = var("Command", ptData);
      if (strFunction == "connect" || strFunction == "c")
      {
        string strPort, strServer;
        stringstream ssConnect(strCommand);
        getline(ssConnect, strServer, ':');
        getline(ssConnect, strPort, ':');
        if (!strServer.empty() && !strPort.empty())
        {
          lock();
          if (m_terminalClients.find(strIdent) == m_terminalClients.end())
          {
            m_terminalClients[strIdent] = {};
            thread threadTerminal(&Irc::terminal, this, strPrefix, strTarget, strIdent, strServer, strPort, strSource);
            pthread_setname_np(threadTerminal.native_handle(), "terminal");
            threadTerminal.detach();
          }
          else
          {
            ssText << " error:  You already have a session open.  Use the following action to close the session:  exit.";
          }
          unlock();
        }
        else
        {
          ssText << " error:  Please provide the connection string following the connect Function:  [server]:[port].";
        }
      }
      else
      {
        lock();
        if (m_terminalClients.find(strIdent) != m_terminalClients.end())
        {
          stringstream ssCommand;
          if (!strCommand.empty())
          {
            ssCommand << strFunction << " " << strCommand;
          }
          else
          {
            ssCommand << strFunction;
          }
          m_terminalClients[strIdent].push_back(ssCommand.str());
        }
        else
        {
          ssText << " error:  Please provide connect following the action.";
        }
        unlock();
      }
    }
    else
    {
      ssText << ":  The terminal action is used to establish and maintain a terminal session.  Please provide the following immediately following the action:  connect (c) [server]:[port].";
    }
  }
  // }}}
  // {{{ invalid
  else
  {
    actions.sort();
    actions.unique();
    ssText << ":  Please provide an Action:  ";
    for (auto actionIter = actions.begin(); actionIter != actions.end(); actionIter++)
    {
      ssText << ((actionIter != actions.begin())?", ":"") << (*actionIter);
    }
    ssText << ".";
  }
  // }}}
  // {{{ post work
  chat(strTarget, ssText.str(), strSource);
  // }}}
}
// }}}
// {{{ analyzer()
void Irc::analyzer(string strPrefix, const string strTarget, const string strUserID, const string strIdent, const string strData, const string strSource)
{
  bool bAdmin = false;
  map<string, bool> auth;
  string strError, strFirstName, strLastName;
  stringstream ssData(strData), ssQuery;

  strPrefix += "->analyze()";
  ssQuery.str("");
  ssQuery << "select id, admin, first_name, last_name from person where userid = '" << strIdent << "' and active = 1 and locked = 0";
  auto getPerson = dbquery("central_r", ssQuery.str(), strError);
  if (getPerson != NULL)
  {
    if (!getPerson->empty())
    {
      map<string, string> getPersonRow = getPerson->front();
      strFirstName = getPersonRow["first_name"];
      strLastName = getPersonRow["last_name"];
      if (getPersonRow["admin"] == "1")
      {
        bAdmin = true;
      }
      else
      {
        ssQuery.str("");
        ssQuery << "select a.name, b.admin from application a, application_contact b where a.id = b.application_id and b.contact_id = " << getPersonRow["id"] << " and b.locked = 0";
        auto getApplicationContact = dbquery("central_r", ssQuery.str(), strError);
        if (getApplicationContact != NULL)
        {
          if (!getApplicationContact->empty())
          {
            for (auto &getApplicationContactRow : *getApplicationContact)
            {
              auth[getApplicationContactRow["name"]] = ((getApplicationContactRow["admin"] == "1")?true:false);
            }
          }
        }
        m_pCentral->free(getApplicationContact);
      }
    }
  }
  m_pCentral->free(getPerson);
  analyze(strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, ssData, strSource);
}
// }}}
// {{{ autoMode()
void Irc::autoMode(string strPrefix, const string strOldMaster, const string strNewMaster)
{
  threadIncrement();
  strPrefix += "->Irc::autoMode()";
  if (strOldMaster != strNewMaster)
  {
    stringstream ssMessage;
    ssMessage << strPrefix << " [" << strNewMaster << "]:  Updated master.";
    log(ssMessage.str());
  }
  threadDecrement();
}
// }}}
// {{{ bot()
void Irc::bot(string strPrefix)
{
  // {{{ prep work
  string strError;
  stringstream ssMessage;
  SSL_CTX *ctx;
  strPrefix += "->Irc::bot()";
  // }}}
  if ((ctx = m_pUtility->sslInitClient(strError)) != NULL)
  {
    // {{{ prep work
    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
    // }}}
    while (!shutdown())
    {
      if (!shutdown() && isMasterSettled() && isMaster())
      {
        // {{{ prep work
        bool bExit = false, bRegistering = false;
        int fdSocket = -1, nReturn;
        list<string> channels;
        pollfd *fds;
        size_t unIndex = 0, unPosition;
        string strBuffer[2], strName = "Radial", strMessage, strNick, strNickBase = "radial_bot", strUser = "radial_bot";
        SSL *ssl = NULL;
        // }}}
        while (!bExit)
        {
          // {{{ prep work
          // {{{ connect
          if (fdSocket == -1)
          {
            if (m_pUtility->connect(m_strServer, m_strPort, fdSocket, strError, true))
            {
              if ((ssl = m_pUtility->sslConnect(ctx, fdSocket, strError)) != NULL)
              {
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslConnect() [" << m_strServer << ":" << m_strPort << "]:  Connected to IRC server.";
                log(ssMessage.str());
              }
              else
              {
                close(fdSocket);
                fdSocket = -1;
                ssMessage.str("");
                ssMessage << strPrefix << "->Utility::sslConnect() error:  " << strError;
                log(ssMessage.str());
              }
            }
            else
            {
              fdSocket = -1;
              ssMessage.str("");
              ssMessage << strPrefix << "->Utility::connect() error:  " << strError;
              log(ssMessage.str());
            }
          }
          // }}}
          fds = new pollfd[1];
          fds[0].fd = fdSocket;
          fds[0].events = POLLIN;
          if (fdSocket != -1 && !enabled() && !bRegistering)
          {
            stringstream ssBuffer;
            bRegistering = true;
            strNick = strNickBase;
            if (unIndex > 0)
            {
              stringstream ssIndex;
              ssIndex << unIndex;
              strNick += ssIndex.str();
            }
            ssBuffer << "NICK " << strNick << "\r\n";
            ssBuffer << "USER " << strUser << " 0 * :" << strName << "\r\n";
            strBuffer[1] = ssBuffer.str();
          }
          while (message(strMessage))
          {
            strBuffer[1].append(strMessage);
          }
          if (!strBuffer[1].empty())
          {
            fds[0].events |= POLLOUT;
          }
          // }}}
          if ((nReturn = poll(fds, 1, 500)) > 0)
          {
            // {{{ read
            if (fds[0].revents & (POLLHUP | POLLIN))
            {
              if (m_pUtility->sslRead(ssl, strBuffer[0], nReturn))
              {
                // {{{ prep work
                while ((unPosition = strBuffer[0].find("\r")) != string::npos)
                {
                  strBuffer[0].erase(unPosition, 1);
                }
                // }}}
                while ((unPosition = strBuffer[0].find("\n")) != string::npos)
                {
                  // {{{ prep work
                  strMessage = strBuffer[0].substr(0, unPosition);
                  strBuffer[0].erase(0, (unPosition + 1));
                  // }}}
                  // {{{ PING
                  if (strMessage.size() >= 4 && strMessage.substr(0, 4) == "PING")
                  {
                    string strPong = strMessage;
                    strPong[1] = 'O';
                    strBuffer[1].append(strPong + "\r\n");
                  }
                  // }}}
                  else
                  {
                    // {{{ prep work
                    string strCommand, strID, strIdent, strLine;
                    stringstream ssCommand, ssSubMessage(strMessage);
                    ssSubMessage >> strID >> strCommand;
                    if (!strID.empty() && strID[0] == ':')
                    {
                      strID.erase(0, 1);
                    }
                    if ((unPosition = strID.find("@")) != string::npos)
                    {
                      strID.erase(unPosition, (strID.size() - unPosition));
                    }
                    if ((unPosition = strID.find("!")) != string::npos && (unPosition + 1) < strID.size())
                    {
                      strIdent = strID.substr((unPosition + 1), strID.size() - (unPosition + 1));
                      strID.erase(unPosition, (strID.size() - unPosition));
                    }
                    else
                    {
                      strIdent = strID;
                    }
                    // }}}
                    if (!strCommand.empty())
                    {
                      // {{{ numeric
                      if (m_manip.isNumeric(strCommand))
                      {
                        size_t unCommand;
                        ssCommand.str(strCommand);
                        ssCommand >> unCommand;
                        switch (unCommand)
                        {
                          case 1:
                          {
                            stringstream ssBuffer;
                            bRegistering = false;
                            strNick = strNickBase;
                            if (unIndex > 0)
                            {
                              stringstream ssIndex;
                              ssIndex << unIndex;
                              strNick += ssIndex.str();
                            }
                            enable(strNick);
                            ssMessage.str("");
                            ssMessage << strPrefix << " [" << m_strServer << ":" << m_strPort << "," << strNick << "]:  Registered on IRC server.";
                            log(ssMessage.str());
                            break;
                          }
                          case 322:
                          {
                            string strChannel;
                            ssSubMessage >> strChannel >> strChannel;
                            if (!strChannel.empty())
                            {
                              channels.push_back(strChannel);
                            }
                            break;
                          }
                          case 323:
                          {
                            list<string> removals;
                            for (auto &channel : channels)
                            {
                              if (m_channels.find(channel) == m_channels.end())
                              {
                                m_channels[channel] = false;
                              }
                            }
                            for (auto &channel : m_channels)
                            {
                              bool bFound = false;
                              for (auto i = channels.begin(); !bFound && i != channels.end(); i++)
                              {
                                if ((*i) == channel.first)
                                {
                                  bFound = true;
                                }
                              }
                              if (!bFound)
                              {
                                removals.push_back(channel.first);
                              }
                            }
                            while (!removals.empty())
                            {
                              if (m_channels[removals.front()])
                              {
                                join(removals.front());
                              }
                              else
                              {
                                m_channels.erase(removals.front());
                              }
                              removals.pop_front();
                            }
                            channels.clear();
                            break;
                          }
                          case 433:
                          case 436:
                          {
                            bRegistering = false;
                            unIndex++;
                            break;
                          }
                        }
                      }
                      // }}}
                      // {{{ JOIN
                      else if (strCommand == "JOIN")
                      {
                        string strChannel;
                        ssSubMessage >> strChannel;
                        if (!strChannel.empty() && strChannel[0] == ':')
                        {
                          strChannel.erase(0, 1);
                        }
                        if (strID == strNick)
                        {
													m_channels[strChannel] = true;
                          if (strChannel == "#radial")
                          {
                            ssMessage.str("");
                            ssMessage << char(3) << "11,10 " << m_strNode << " " << char(3) << " " << char(3) << "07,05 " << m_strName << " " << char(3) << " Switched to master mode.";
                            chat("#radial", ssMessage.str());
                          }
                        }
                      }
                      // }}}
                      // {{{ PART
                      else if (strCommand == "PART")
                      {
                        string strChannel;
                        ssSubMessage >> strChannel;
                        if (!strChannel.empty() && strChannel[0] == ':')
                        {
                          strChannel.erase(0, 1);
                        }
                        if (strID == strNick)
                        {
                          if (m_channels.find(strChannel) != m_channels.end() && m_channels[strChannel])
                          {
                            m_channels[strChannel] = false;
                          }
                        }
                      }
                      // }}}
                      // {{{ PRIVMSG
                      else if (strCommand == "PRIVMSG")
                      {
                        bool bChannel = false;
                        string strMessage, strTarget;
                        stringstream ssBuffer;
                        ssSubMessage >> strTarget;
                        getline(ssSubMessage, strMessage);
                        if (strMessage.substr(0, 2) == " :")
                        {
                          strMessage.erase(0, 2);
                        }
                        if (!strTarget.empty() && strTarget[0] == '#')
                        {
                          bChannel = true;
                          analyze(strID, strTarget, strMessage);
                        }
                        // {{{ !r || !radial
                        if (!bChannel || strMessage == "!r" || (strMessage.size() > 3 && strMessage.substr(0, 3) == "!r ") || strMessage == "!radial" || (strMessage.size() > 8 && strMessage.substr(0, 8) == "!radial "))
                        {
                          string strChannel, strData;
                          stringstream ssData(strMessage), ssPrefix;
                          if (strMessage == "!r" || (strMessage.size() > 3 && strMessage.substr(0, 3) == "!r ") || strMessage == "!radial" || (strMessage.size() > 8 && strMessage.substr(0, 8) == "!radial "))
                          {
                            string strValue;
                            ssData >> strValue;
                          }
                          getline(ssData, strData);
                          m_manip.trim(strData, strData);
                          thread threadAnalyzer(&Irc::analyzer, this, strPrefix, ((bChannel)?strTarget:strID), strID, strIdent, strData, "");
                          pthread_setname_np(threadAnalyzer.native_handle(), "analyzer");
                          threadAnalyzer.detach();
                        }
                        // }}}
                      }
                      // }}}
                      // {{{ QUIT
                      else if (strCommand == "QUIT")
                      {
                        if (strID == strNick)
                        {
                          bExit = true;
                        }
                      }
                      // }}}
                    }
                  }
                }
              }
              else
              {
                bExit = true;
                if (nReturn < 0)
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Utility::sslRead(" << SSL_get_error(ssl, nReturn) << ") error:  " << m_pUtility->sslstrerror();
                  log(ssMessage.str());
                }
              }
            }
            // }}}
            // {{{ write
            if (fds[0].revents & POLLOUT)
            {
              if (!m_pUtility->sslWrite(ssl, strBuffer[1], nReturn))
              {
                bExit = true;
                if (nReturn < 0)
                {
                  ssMessage.str("");
                  ssMessage << strPrefix << "->Utility::sslWrite(" << SSL_get_error(ssl, nReturn) << ") error:  " << m_pUtility->sslstrerror();
                  log(ssMessage.str());
                }
              }
            }
            // }}}
            // {{{ error
            if (fds[0].revents & POLLERR)
            {
              bExit = true;
              ssMessage.str("");
              ssMessage << strPrefix << "->poll() error:  Encountered a POLLERR.";
              log(ssMessage.str());
            }
            if (fds[0].revents & POLLNVAL)
            {
              bExit = true;
              ssMessage.str("");
              ssMessage << strPrefix << "->poll() error:  Encountered a POLLNVAL.";
              log(ssMessage.str());
            }
            // }}}
          }
          else if (nReturn < 0)
          {
            bExit = true;
            ssMessage.str("");
            ssMessage << strPrefix << "->poll(" << errno << ") error:  " << strerror(errno);
            log(ssMessage.str());
          }
          // {{{ post work
          delete[] fds;
          if (shutdown() || !isMasterSettled() || !isMaster())
          {
            quit();
          }
          // }}}
        }
        // {{{ post work
        if (ssl != NULL)
        {
          SSL_shutdown(ssl);
          SSL_free(ssl);
          ssl = NULL;
        }
        if (fdSocket != -1)
        {
          close(fdSocket);
          fdSocket = -1;
          disable();
          ssMessage.str("");
          ssMessage << strPrefix << " [" << m_strServer << ":" << m_strPort << "," << strNick << "]:  Exited IRC server.";
          log(ssMessage.str());
        }
        // }}}
      }
      if (!shutdown())
      {
        for (size_t i = 0; !shutdown() && i < 20; i++)
        {
          msleep(250);
        }
      }
    }
    // {{{ post work
    SSL_CTX_free(ctx);
    // }}}
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->Utility::sslInitClient() error:  " << strError;
    notify(ssMessage.str());
  }
  // {{{ post work
  m_pUtility->sslDeinit();
  // }}}
}
// }}}
// {{{ callback()
void Irc::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  size_t unCount = 0;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Irc::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  while (unCount++ < 40 && !isMasterSettled())
  {
    msleep(250);
  }
  if (isMasterSettled() && isMaster())
  {
    if (!empty(ptJson, "Function"))
    {
      // {{{ analyze
      if (ptJson->m["Function"]->v == "analyze")
      {
        if (exist(ptJson, "Request"))
        {
          if (!empty(ptJson->m["Request"], "Source"))
          {
            if (!empty(ptJson->m["Request"], "Target"))
            {
              if (!empty(ptJson->m["Request"], "UserID"))
              {
                if (!empty(ptJson->m["Request"], "Ident"))
                {
                  if (!empty(ptJson->m["Request"], "Message"))
                  {
                    bResult = true;
                    thread threadAnalyzer(&Irc::analyzer, this, strPrefix, ptJson->m["Request"]->m["Target"]->v, ptJson->m["Request"]->m["UserID"]->v, ptJson->m["Request"]->m["Ident"]->v, ptJson->m["Request"]->m["Message"]->v, ptJson->m["Request"]->m["Source"]->v);
                    pthread_setname_np(threadAnalyzer.native_handle(), "analyzer");
                    threadAnalyzer.detach();
                  }
                  else
                  {
                    strError = "Please provide the Message within the Request.";
                  }
                }
                else
                {
                  strError = "Please provide the Ident within the Request.";
                }
              }
              else
              {
                strError = "Please provide the UserID within the Request.";
              }
            }
            else
            {
              strError = "Please provide the Target within the Request.";
            }
          }
          else
          {
            strError = "Please provide the Source within the Request.";
          }
        }
        else
        {
          strError = "Please provide the Request.";
        }
      }
      // }}}
      // {{{ channels
      else if (ptJson->m["Function"]->v == "channels")
      {
        if (enabled())
        {
          bResult = true;
          if (ptJson->m.find("Response") != ptJson->m.end())
          {
            delete ptJson->m["Response"];
          }
          ptJson->m["Response"] = new Json;
          for (auto &channel : m_channels)
          {
            ptJson->m["Response"]->i(channel.first, ((channel.second)?"1":"0"), ((channel.second)?'1':'0'));
          }
        }
        else
        {
          strError = "IRC disabled.";
        }
      }
      // }}}
      // {{{ chat
      else if (ptJson->m["Function"]->v == "chat")
      {
        if (!empty(ptJson, "Message"))
        {
          if (!empty(ptJson, "Target"))
          {
            size_t unCount = 0;
            while (unCount++ < 40 && !isMasterSettled())
            {
              msleep(250);
            }
            if (isMasterSettled())
            {
              if (isMaster())
              {
                size_t unPosition;
                string strMessage = ptJson->m["Message"]->v;
                stringstream ssEtx;
                ssEtx << char(3);
                while ((unPosition = strMessage.find("<ETX>")) != string::npos)
                {
                  strMessage.replace(unPosition, 5, ssEtx.str());
                }
                unCount = 0;
                while (unCount++ < 40 && !enabled())
                {
                  msleep(250);
                }
                if (enabled())
                {
                  bResult = true;
                  chat(ptJson->m["Target"]->v, strMessage, ((!empty(ptJson, "Source"))?ptJson->m["Source"]->v:""));
                }
                else
                {
                  strError = "IRC disabled.";
                }
              }
              else
              {
                Json *ptLink = new Json(ptJson);
                ptLink->i("Interface", "irc");
                ptLink->i("Node", master());
                if (hub("link", ptLink, strError))
                {
                  bResult = true;
                }
                delete ptLink;
              }
            }
            else
            {
              strError = "Master not known.";
            }
          }
          else
          {
            strError = "Please provide the Target.";
          }
        }
        else
        {
          strError = "Please provide the Message.";
        }
      }
      // }}}
      // {{{ invalid
      else
      {
        strError = "Please provide a valid Function:  analyze, channels, chat.";
      }
      // }}}
    }
    else
    {
      strError = "Please provide the Function.";
    }
  }
  else if (isMasterSettled())
  {
    Json *ptLink = new Json(ptJson);
    ptLink->i("Interface", "irc");
    ptLink->i("Node", master());
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
  else
  {
    strError = "Master not known.";
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
// {{{ callbackInotify()
void Irc::callbackInotify(string strPrefix, const string strPath, const string strFile)
{
  string strError;
  stringstream ssMessage;

  strPrefix += "->Irc::callbackInotify()";
  if (strPath == (m_strData + "/irc") && strFile == "monitor.channels")
  {
    monitorChannels(strPrefix);
  }
}
// }}}
// {{{ chat()
void Irc::chat(const string strTarget, const string strMessage, const string strSource)
{
  if (strSource == "live")
  {
    Json *ptLive = new Json;
    ptLive->i("Function", "message");
    ptLive->m["Request"] = new Json;
    ptLive->m["Request"]->i("User", strTarget);
    ptLive->m["Request"]->m["Message"] = new Json;
    ptLive->m["Request"]->m["Message"]->i("Action", "chat");
    ptLive->m["Request"]->m["Message"]->i("Message", strMessage);
    ptLive->m["Request"]->m["Message"]->i("User", "radial_bot");
    hub("live", ptLive, false);
    delete ptLive;
  }
  else
  {
    bool bPart = false;
    size_t unMaxLength = 450;
    string strLine;
    stringstream ssLines(strMessage), ssMessage, ssPrefix;
    if (m_channels.find(strTarget) == m_channels.end() || !m_channels[strTarget])
    {
      bPart = true;
      join(strTarget);
    }
    ssPrefix << ":" << m_strNick << " PRIVMSG " << strTarget << " :";
    while (getline(ssLines, strLine))
    {
      while ((ssPrefix.str().size() + strLine.size() + 2) > unMaxLength)
      {
        ssMessage.str("");
        ssMessage << ssPrefix.str() << strLine.substr(0, (unMaxLength - (ssPrefix.str().size() + 2))) << "\r\n";
        push(ssMessage.str());
        strLine.erase(0, (unMaxLength - (ssPrefix.str().size() + 2)));
      }
      if (!strLine.empty())
      {
        ssMessage.str("");
        ssMessage << ssPrefix.str() << strLine << "\r\n";
        push(ssMessage.str());
      }
    }
    if (bPart)
    {
      part(strTarget);
    }
  }
}
// }}}
// {{{ disable()
void Irc::disable()
{
  if (enabled())
  {
    if (m_ptMonitor != NULL)
    {
      for (auto &i : m_ptMonitor->m)
      {
        part(i.first);
      }
    }
    m_strNick.clear();
    m_bEnabled = false;
  }
}
// }}}
// {{{ enable()
void Irc::enable(const string strNick)
{
  if (!enabled())
  {
    m_bEnabled = true;
    if (!m_messages.empty())
    {
      m_messages.clear();
    }
    m_strNick = strNick;
    if (m_ptMonitor != NULL)
    {
      for (auto &i : m_ptMonitor->m)
      {
        join(i.first);
      }
    }
  }
}
// }}}
// {{{ enabled()
bool Irc::enabled()
{
  return m_bEnabled;
}
// }}}
// {{{ feedback()
void Irc::feedback(string strPrefix, const string strTarget, const string strUserID, const string strIdent, const string strFirstName, const string strLastName, const bool bAdmin, map<string, bool> auth, const string strHash, const string strSource)
{
  string strError;
  Json *ptSurvey = new Json;

  strPrefix += "->feedback()";
  chat(strTarget, string(1, char(2)) + string(1, char(3)) + (string)"07SESSION STARTED" + string(1, char(3)) + string(1, char(2)), strSource);
  if (feedbackSurvey(strHash, ptSurvey, strError))
  {
    bool bExit = false, bProcess;
    stringstream ssText;
    if (!empty(ptSurvey, "title"))
    {
      ssText << "SURVEY:  " << ptSurvey->m["title"]->v << endl;
    }
    ssText << "This survey was created by ";
    if (exist(ptSurvey, "owner") && !empty(ptSurvey->m["owner"], "first_name") && !empty(ptSurvey->m["owner"], "last_name"))
    {
      ssText << ptSurvey->m["owner"]->m["first_name"]->v << " " << ptSurvey->m["owner"]->m["last_name"]->v;
    }
    else
    {
      ssText << "an unknown person";
    }
    if (!empty(ptSurvey, "now_date") && !empty(ptSurvey, "start_date") && ptSurvey->m["now_date"]->v < ptSurvey->m["start_date"]->v)
    {
      bExit = true;
      ssText << " and opens " << ptSurvey->m["start_date"]->v;
    }
    if (!empty(ptSurvey, "end_date"))
    {
      if (!empty(ptSurvey, "now_date") && ptSurvey->m["now_date"]->v <= ptSurvey->m["end_date"]->v)
      {
        ssText << " and expires " << ptSurvey->m["end_date"]->v;
      }
      else
      {
        bExit = true;
        ssText << " and has expired";
      }
    }
    else
    {
      ssText << " and is open indefinitely";
    }
    ssText << ".";
    chat(strTarget, ssText.str(), strSource);
    if (!empty(ptSurvey, "id"))
    {
      if (exist(ptSurvey, "questions"))
      {
        delete ptSurvey->m["questions"];
      }
      ptSurvey->m["questions"] = new Json;
      if (feedbackQuestions(ptSurvey->m["id"]->v, ptSurvey->m["questions"], strError))
      {
        bool bSubmit = false;
        list<Json *>::iterator q = ptSurvey->m["questions"]->l.begin();
        string strData;
        while (!bExit)
        {
          bProcess = false;
          if (q != ptSurvey->m["questions"]->l.end())
          {
            if (!exist((*q), "type"))
            {
              ssText.str("");
              ssText << "QUESTION:  " << (*q)->m["question"]->v;
              chat(strTarget, ssText.str(), strSource);
              (*q)->m["type"] = new Json;
              if (feedbackType((*q)->m["type_id"]->v, (*q)->m["type"], strError))
              {
                if (!empty((*q)->m["type"], "name"))
                {
                  if ((*q)->m["type"]->m["name"]->v == "checkbox" || (*q)->m["type"]->m["name"]->v == "radio" || (*q)->m["type"]->m["name"]->v == "select")
                  {
                    if (exist((*q), "answers"))
                    {
                      delete ((*q)->m["answers"]);
                    }
                    (*q)->m["answers"] = new Json;
                    if (feedbackAnswers((*q)->m["id"]->v, (*q)->m["answers"], strError))
                    {
                      ssText.str("");
                      ssText << "ANSWERS:" << endl;
                      for (auto &ptAnswer : (*q)->m["answers"]->l)
                      {
                        if (!empty(ptAnswer, "id") && !empty(ptAnswer, "answer") && !empty(ptAnswer, "sequence"))
                        {
                          ssText << ptAnswer->m["sequence"]->v << ") " << ptAnswer->m["answer"]->v << endl;
                        }
                      }
                      if ((*q)->m["type"]->m["name"]->v == "checkbox")
                      {
                        ssText << "Please provide space delimited answers by number.";
                      }
                      else
                      {
                        ssText << "Please provide an answer by number.";
                      }
                      chat(strTarget, ssText.str(), strSource);
                    }
                    else
                    {
                      bExit = true;
                      chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  Interface::feedbackAnswers() " + strError + string(1, char(3)), strSource);
                      strError.clear();
                    }
                  }
                  else
                  {
                    ssText.str("");
                    ssText << "Please provide your answer.";
                    chat(strTarget, ssText.str(), strSource);
                  }
                }
                else
                {
                  bExit = true;
                  chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  Interface::feedbackType() Type name not returned." + string(1, char(3)), strSource);
                  strError.clear();
                }
              }
              else
              {
                bExit = true;
                chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  Interface::feedbackType() " + strError + string(1, char(3)), strSource);
                strError.clear();
              }
            }
          }
          else
          {
            bExit = bSubmit = true;
          }
          lock();
          if (m_feedbackClients.find(strIdent) != m_feedbackClients.end())
          {
            if (!m_feedbackClients[strIdent].empty())
            {
              bProcess = true;
              strData = m_feedbackClients[strIdent].front();
              m_feedbackClients[strIdent].pop_front();
            }
          }
          else
          {
            bExit = true;
          }
          unlock();
          if (bProcess)
          {
            if (strData == "exit")
            {
              bExit = true;
            }
            else if (exist((*q), "type") && !empty((*q)->m["type"], "name") && ((*q)->m["type"]->m["name"]->v == "checkbox" || (*q)->m["type"]->m["name"]->v == "radio" || (*q)->m["type"]->m["name"]->v == "select") && exist((*q), "answers"))
            {
              if ((*q)->m["type"]->m["name"]->v == "checkbox")
              {
                string strAnswer;
                stringstream ssAnswers(strData);
                (*q)->m["answer"] = new Json;
                while (ssAnswers >> strAnswer)
                {
                  (*q)->m["answer"]->pb(strAnswer);
                }
                if ((*q)->m["answer"]->l.empty())
                {
                  delete (*q)->m["answer"];
                  (*q)->m.erase("answer");
                }
              }
              else
              {
                string strAnswerID;
                for (auto a = (*q)->m["answers"]->l.begin(); strAnswerID.empty() && a != (*q)->m["answers"]->l.end(); a++)
                {
                  if (!empty((*a), "sequence") && (*a)->m["sequence"]->v == strData && !empty((*a), "id"))
                  {
                    strAnswerID = (*a)->m["id"]->v;
                  }
                }
                if (!strAnswerID.empty())
                {
                  (*q)->i("answer", strAnswerID);
                }
              }
            }
            else
            {
              (*q)->i("answer", strData);
            }
            if (exist((*q), "answer"))
            {
              if (!empty((*q), "answer"))
              {
                q++;
              }
              else
              {
                delete (*q)->m["answer"];
                (*q)->m.erase("answer");
              }
            }
            if (!strError.empty())
            {
              chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  " + strError + string(1, char(3)), strSource);
              strError.clear();
            }
          }
          else
          {
            msleep(100);
          }
        }
        if (bSubmit)
        {
          if (feedbackResultAdd(strUserID, strFirstName, strLastName, bAdmin, auth, ptSurvey, strError))
          {
            ssText.str("");
            ssText << "Thank you for providing your feedback to this survey.";
            chat(strTarget, ssText.str(), strSource);
          }
          else
          {
            chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  Interface::feedbackResultAdd() " + strError + string(1, char(3)), strSource);
          }
        }
      }
      else
      {
        chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  Interface::feedbackQuestions() " + strError + string(1, char(3)), strSource);
      }
    }
    else
    {
      chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  Survey ID was not returned." + string(1, char(3)), strSource);
    }
  }
  else
  {
    chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  Interface::feedbackSurvey() " + strError + string(1, char(3)), strSource);
  }
  delete ptSurvey;
  chat(strTarget, string(1, char(2)) + string(1, char(3)) + (string)"07SESSION ENDED" + string(1, char(3)) + string(1, char(2)), strSource);
  lock();
  if (m_feedbackClients.find(strIdent) != m_feedbackClients.end())
  {
    m_feedbackClients[strIdent].clear();
    m_feedbackClients.erase(strIdent);
  }
  unlock();
}
// }}}
// {{{ isLocalAdmin()
bool Irc::isLocalAdmin(const string strUserID, string strApplication, const bool bAdmin, map<string, bool> auth)
{
  return (bAdmin || (!strUserID.empty() && auth.find(strApplication) != auth.end() && auth[strApplication]));
}
// }}}
// {{{ isValid()
bool Irc::isValid(const string strUserID, string strApplication, const bool bAdmin, map<string, bool> auth)
{
  return (bAdmin || (!strUserID.empty() && auth.find(strApplication) != auth.end()));
}
// }}}
// {{{ join()
void Irc::join(const string strChannel)
{
  stringstream ssMessage;

  ssMessage << ":" << m_strNick << " JOIN :" << strChannel << "\r\n";
  push(ssMessage.str());
}
// }}}
// {{{ listChannels()
void Irc::listChannels()
{
  stringstream ssMessage;

  ssMessage << ":" << m_strNick << " LIST\r\n";
  push(ssMessage.str());
}
// }}}
// {{{ lock()
void Irc::lock()
{
  m_mutex.lock();
}
// }}}
// {{{ math()
bool Irc::math(vector<string> e, long double &r, string &strError)
{
  bool bResult = true;
  ssize_t a, b, d;

  r = 0;
  do
  {
    a = b = -1;
    d = 0;
    for (ssize_t i = 0; b == -1 && i < (ssize_t)e.size(); i++)
    {
      if (e[i] == "(")
      {
        if (++d == 1)
        {
          a = i;
        }
      }
      else if (e[i] == ")")
      {
        if (d-- == 1)
        {
          b = i;
        }
      }
    }
    if (b != -1)
    {
      long double t;
      stringstream s;
      vector<string> f;
      for (ssize_t i = (a + 1); i < b; i++)
      {
        f.push_back(e[i]);
      }
      if (math(f, t, strError))
      {
        s << t;
        f.clear();
        for (ssize_t i = 0; i < (ssize_t)e.size(); i++)
        {
          if (i == a)
          {
            f.push_back(s.str());
          }
          else if (i < a || i > b)
          {
            f.push_back(e[i]);
          }
        }
        e = f;
      }
      else
      {
        bResult = false;
      }
    }
  } while (bResult && b != -1);
  if (bResult)
  {
    if (e.size() == 2 || e.size() == 3)
    {
      long double dA, dB;
      string strA = e[0], strB = e[1], strC, strF;
      stringstream ssA, ssB;
      if (e.size() == 3)
      {
        strC = e[2];
      }
      if (m_manip.isNumeric(strA))
      {
        strF = strB;
        strB = strC;
      }
      else
      {
        strF = strA;
        strA = strB;
        strB = strC;
      }
      ssA.str(strA);
      ssA >> dA;
      ssB.str(strB);
      ssB >> dB;
      if (strF == "+")
      {
        r = dA + dB;
      }
      else if (strF == "-")
      {
        r = dA - dB;
      }
      else if (strF == "-")
      {
        r = dA - dB;
      }
      else if (strF == "*")
      {
        r = dA * dB;
      }
      else if (strF == "/")
      {
        if (dB != 0)
        {
          r = dA / dB;
        }
        else
        {
          bResult = false;
          strError = "Result is undefined.";
        }
      }
      else if (strF == "%")
      {
        r = (int)dA % (int)dB;
      }
      else if (strF == "^")
      {
        r = pow(dA, dB);
      }
      else if (strF == "abs")
      {
        r = abs(dA);
      }
      else if (strF == "acos")
      {
        r = acos(dA);
      }
      else if (strF == "asin")
      {
        r = asin(dA);
      }
      else if (strF == "atan")
      {
        r = atan(dA);
      }
      else if (strF == "cbrt")
      {
        r = cbrt(dA);
      }
      else if (strF == "ceil")
      {
        r = ceil(dA);
      }
      else if (strF == "cos")
      {
        r = cos(dA);
      }
      else if (strF == "exp")
      {
        r = exp(dA);
      }
      else if (strF == "floor")
      {
        r = floor(dA);
      }
      else if (strF == "sin")
      {
        r = sin(dA);
      }
      else if (strF == "sqrt")
      {
        r = sqrt(dA);
      }
      else if (strF == "tan")
      {
        r = tan(dA);
      }
      else
      {
        bResult = false;
        strError = "Please provide a valid Function:  +, -, *, /, %, ^, abs, acos, asin, atan, cbrt, ceil, cos, exp, floor, sin, sqrt, tan";
      }
    }
    else
    {
      bResult = false;
      strError = "Invalid number of items.";
    }
  }
  if (!bResult)
  {
    stringstream ssError;
    ssError << strError << " ---";
    for (auto i : e)
    {
      ssError << " " << i;
    }
    strError = ssError.str();
  }

  return bResult;
}
// }}}
// {{{ message()
bool Irc::message(string &strMessage)
{
  bool bResult = false;

  lock();
  if (!m_messages.empty())
  {
    bResult = true;
    strMessage = m_messages.front();
    m_messages.pop_front();
  }
  unlock();

  return bResult;
}
// }}}
// {{{ monitor()
Json *Irc::monitor()
{
  Json *ptMonitor = NULL;

  if (m_ptMonitor != NULL)
  {
    ptMonitor = new Json(m_ptMonitor);
  }

  return ptMonitor;
}
// }}}
// {{{ monitorChannels()
void Irc::monitorChannels(string strPrefix, const bool bSilent)
{
  stringstream ssFile, ssMessage;

  strPrefix += "->Irc::monitorChannels()";
  ssFile << m_strData << "/irc/monitor.channels";
  ifstream inMonitor;
  inMonitor.open(ssFile.str());
  if (inMonitor)
  {
    string strLine;
    stringstream ssJson;
    while (getline(inMonitor, strLine))
    {
      ssJson << strLine;
    }
    if (!ssJson.str().empty())
    {
      if (m_ptMonitor != NULL)
      {
        if (enabled())
        {
          auto subChannels = m_channels;
          for (auto &channel : subChannels)
          {
            if (channel.second)
            {
              part(channel.first);
            }
          }
        }
        delete m_ptMonitor;
      }
      m_ptMonitor = new Json(ssJson.str());
      if (!m_ptMonitor->m.empty())
      {
        if (enabled())
        {
          for (auto &i : m_ptMonitor->m)
          {
            join(i.first);
          }
        }
      }
      else if (!bSilent)
      {
        ssMessage.str("");
        ssMessage << strPrefix << " error [" << ssFile.str() << "]:  JSON is empty.";
        log(ssMessage.str());
      }
    }
    else if (!bSilent)
    {
      ssMessage.str("");
      ssMessage << strPrefix << " error [" << ssFile.str() << "]:  File is empty.";
      log(ssMessage.str());
    }
  }
  else if (!bSilent)
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->ifstream::open(" << errno << ") error [" << ssFile.str() << "]:  " << strerror(errno);
    log(ssMessage.str());
  }
  inMonitor.close();
}
// }}}
// {{{ part()
void Irc::part(const string strChannel)
{
  stringstream ssMessage;

  ssMessage << ":" << m_strNick << " PART :" << strChannel << "\r\n";
  push(ssMessage.str());
}
// }}}
// {{{ push()
void Irc::push(const string strMessage)
{
  lock();
  m_messages.push_back(strMessage);
  unlock();
}
// }}}
// {{{ quit()
void Irc::quit()
{
  stringstream ssMessage;

  for (auto &channel : m_channels)
  {
    if (channel.second)
    {
      part(channel.first);
    }
  }
  ssMessage << ":" << m_strNick << " QUIT :Quitting.\r\n";
  push(ssMessage.str());
}
// }}}
// {{{ setAnalyze()
void Irc::setAnalyze(bool (*pCallback1)(string, const string, const string, const string, const string, const string, const bool, map<string, bool> &, list<string> &, const string, stringstream &, Json *, const string), bool (*pCallback2)(string, const string, const string, const string, const string, const string, const bool, map<string, bool> &, const string, Json *, stringstream &, const string))
{
  m_pAnalyzeCallback1 = pCallback1;
  m_pAnalyzeCallback2 = pCallback2;
}
// }}}
// {{{ sqliteDisplay()
string Irc::sqliteDisplay(list<map<string, string> > &resultSet)
{
  stringstream ssText;

  if (!resultSet.empty())
  {
    map<string, size_t> pad;
    for (auto &col : resultSet.front())
    {
      pad[col.first] = col.first.size();
    }
    for (auto &row : resultSet)
    {
      for (auto &col : row)
      {
        if (col.second.size() > pad[col.first])
        {
          pad[col.first] = col.second.size();
        }
      }
    }
    ssText << endl;
    for (auto &col : resultSet.front())
    {
      ssText << "  " << left << setw(pad[col.first]) << setfill(' ') << col.first;
    }
    ssText << endl;
    for (auto &col : resultSet.front())
    {
      ssText << "  " << left << setw(pad[col.first]) << setfill('-') << '-';;
    }
    for (auto &row : resultSet)
    {
      ssText << endl;
      for (auto &col : row)
      {
        ssText << "  " << left << setw(pad[col.first]) << setfill(' ') << col.second;
      }
    }
  }

  return ssText.str();
}
// }}}
// {{{ ssh
// {{{ ssh()
void Irc::ssh(string strPrefix, const string strTarget, const string strUserID, const string strIdent, const string strServer, const string strPort, const string strUser, string strPassword, const string strSource)
{
  string strData, strError, strSession;

  strPrefix += "->ssh()";
  if (strPassword.empty())
  {
    bool bExit = false;
    size_t unAttempts = 0;
    chat(strTarget, string(1, char(2)) + string(1, char(3)) + (string)"07WAITING ON PASSWORD" + string(1, char(3)) + string(1, char(2)), strSource);
    chat(strUserID, string(1, char(2)) + string(1, char(3)) + (string)"07Please type the following:  ssh send [password]" + string(1, char(3)) + string(1, char(2)), strSource);
    while (!bExit && unAttempts++ < 1200)
    {
      lock();
      if (!m_sshClients[strIdent].empty())
      {
        strPassword = m_sshClients[strIdent].front();
        m_sshClients[strIdent].pop_front();
      }
      unlock();
      if (!strPassword.empty())
      {
        bExit = true;
      }
      else
      {
        msleep(250);
      }
    }
  }
  chat(strTarget, string(1, char(2)) + string(1, char(3)) + (string)"03SESSION STARTED" + string(1, char(3)) + string(1, char(2)), strSource);
  if (sshConnect(strServer, strPort, strUser, strPassword, strSession, strData, strError))
  {
    bool bExit = false, bCommand;
    string strCommand;
    if (!strData.empty())
    {
      sshConvert(strData);
      chat(strTarget, strData, strSource);
      strData.clear();
    }
    while (!bExit)
    {
      bCommand = false;
      lock();
      if (m_sshClients.find(strIdent) != m_sshClients.end())
      {
        if (!m_sshClients[strIdent].empty())
        {
          bCommand = true;
          strCommand = m_sshClients[strIdent].front();
          m_sshClients[strIdent].pop_front();
        }
      }
      else
      {
        bExit = true;
      }
      unlock();
      if (bCommand)
      {
        if (sshSend(strSession, strCommand, strData, strError))
        {
          if (strSession.empty())
          {
            bExit = true;
          }
        }
        else
        {
          bExit = true;
          chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  Interface::sshSend() " + strError + string(1, char(3)), strSource);
        }
        strCommand.clear();
      }
      else
      {
        msleep(100);
      }
      if (!strData.empty())
      {
        sshConvert(strData);
        chat(strTarget, strData, strSource);
        strData.clear();
      }
    }
    if (!strSession.empty())
    {
      sshDisconnect(strSession, strError);
    }
  }
  else
  {
    chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  Interface::sshConnect() " + strError + string(1, char(3)), strSource);
  }
  chat(strTarget, string(1, char(2)) + string(1, char(3)) + (string)"07SESSION ENDED" + string(1, char(3)) + string(1, char(2)), strSource);
  lock();
  if (m_sshClients.find(strIdent) != m_sshClients.end())
  {
    m_sshClients.erase(strIdent);
  }
  unlock();
}
// }}}
// {{{ sshConvert()
void Irc::sshConvert(string &strData)
{
  size_t unPosition;
  string strLine;
  stringstream ssData;

  while ((unPosition = strData.find("\n")) != string::npos)
  {
    strLine = strData.substr(0, (unPosition + 1));
    strData.erase(0, (unPosition + 1));
    sshConvertLine(strLine);
    ssData << strLine;
  }
  if (!strData.empty())
  {
    sshConvertLine(strData);
    ssData << strData;
  }
  strData = ssData.str();
}
// }}}
// {{{ sshConvertLine()
void Irc::sshConvertLine(string &strData)
{
  size_t unPosition;
  stringstream ssData;

  while ((unPosition = strData.find("\r\n")) != string::npos)
  {
    strData.erase(unPosition, 1);
  }
  while ((unPosition = strData.find("\r")) != string::npos)
  {
    strData.erase(0, (unPosition + 1));
  }
  while (!strData.empty())
  {
    if (strData[0] == char(27) && (unPosition = strData.find("m")) != string::npos)
    {
      strData.erase(0, (unPosition + 1));
    }
    else
    {
      ssData << strData[0];
      strData.erase(0, 1);
    }
  }
  strData = ssData.str();
}
// }}}
// }}}
// {{{ terminal()
void Irc::terminal(string strPrefix, const string strTarget, const string strIdent, string strServer, string strPort, const string strSource)
{
  string strError;
  radialTerminalInfo tInfo;

  strPrefix += "->terminal()";
  chat(strTarget, string(1, char(2)) + string(1, char(3)) + (string)"07SESSION STARTED" + string(1, char(3)) + string(1, char(2)), strSource);
  if (terminalConnect(tInfo, strServer, strPort, true, strError))
  {
    bool bExit = false, bProcess;
    string strData;
    stringstream ssText;
    for (size_t i = 0; i < tInfo.screen.size(); i++)
    {
      if (i == tInfo.unRow && tInfo.unCol < tInfo.screen[i].size())
      {
        stringstream ssCursor;
        ssCursor << char(3) << "08,03" << tInfo.screen[i][tInfo.unCol] << char(3);
        tInfo.screen[i].replace(tInfo.unCol, 1, ssCursor.str());
      }
      ssText << tInfo.screen[i] << endl;
    }
    chat(strTarget, ssText.str(), strSource);
    while (!bExit)
    {
      bProcess = false;
      lock();
      if (m_terminalClients.find(strIdent) != m_terminalClients.end())
      {
        if (!m_terminalClients[strIdent].empty())
        {
          bProcess = true;
          strData = m_terminalClients[strIdent].front();
          m_terminalClients[strIdent].pop_front();
        }
      }
      else
      {
        bExit = true;
      }
      unlock();
      if (bProcess)
      {
        string strFunction;
        stringstream ssData(strData);
        ssData >> strFunction;
        if (!strFunction.empty())
        {
          size_t unCount = 1;
          string strData, strWait, strInvalid = "Please provide a valid Function:  ctrl, disconnect (d), down, enter (e), escape, function (f), home, key, keypadEnter (ke), left, right, send (s), shiftFunction (sf), tab (t), up, wait (w).";
          // {{{ ctrl
          if (strFunction == "ctrl")
          {
            ssData >> strData;
            if (strData.size() == 1)
            {
              terminalCtrl(tInfo, strData[0], true, strError);
            }
            else
            {
              strError = "Data should contain a single character for this Function.";
            }
          }
          // }}}
          // {{{ disconnect
          else if (strFunction == "disconnect" || strFunction == "d")
          {
            bExit = true;
          }
          // }}}
          // {{{ down
          else if (strFunction == "down")
          {
            string strCount;
            ssData >> strCount;
            if (!strCount.empty())
            {
              stringstream ssCount(strCount);
              ssCount >> unCount;
            }
            terminalDown(tInfo, unCount, true, strError);
          }
          // }}}
          // {{{ enter
          else if (strFunction == "enter" || strFunction == "e")
          {
            terminalEnter(tInfo, true, strError);
          }
          // }}}
          // {{{ escape
          else if (strFunction == "escape")
          {
            terminalEscape(tInfo, true, strError);
          }
          // }}}
          // {{{ function
          else if (strFunction == "function" || strFunction == "f")
          {
            int nKey;
            ssData >> nKey;
            terminalFunction(tInfo, nKey, strError);
          }
          // }}}
          // {{{ home
          else if (strFunction == "home")
          {
            terminalHome(tInfo, true, strError);
          }
          // }}}
          // {{{ key
          else if (strFunction == "key")
          {
            string strCount;
            ssData >> strData >> strCount;
            if (!strCount.empty())
            {
              stringstream ssCount(strCount);
              ssCount >> unCount;
            }
            if (strData.size() == 1)
            {
              terminalKey(tInfo, strData[0], unCount, true, strError);
            }
            else
            {
              strError = "Data should contain a single character for this Function.";
            }
          }
          // }}}
          // {{{ keypadEnter
          else if (strFunction == "keypadEnter" || strFunction == "ke")
          {
            terminalKeypadEnter(tInfo, true, strError);
          }
          // }}}
          // {{{ left
          else if (strFunction == "left")
          {
            string strCount;
            ssData >> strCount;
            if (!strCount.empty())
            {
              stringstream ssCount(strCount);
              ssCount >> unCount;
            }
            terminalLeft(tInfo, unCount, true, strError);
          }
          // }}}
          // {{{ right
          else if (strFunction == "right")
          {
            string strCount;
            ssData >> strCount;
            if (!strCount.empty())
            {
              stringstream ssCount(strCount);
              ssCount >> unCount;
            }
            terminalRight(tInfo, unCount, true, strError);
          }
          // }}}
          // {{{ send
          else if (strFunction == "send" || strFunction == "s")
          {
            string strTrimmed;
            getline(ssData, strData);
            m_manip.ltrim(strTrimmed, strData);
            terminalSend(tInfo, strTrimmed, unCount, true, strError);
          }
          // }}}
          // {{{ shiftFunction
          else if (strFunction == "shiftFunction" || strFunction == "sf")
          {
            int nKey;
            ssData >> nKey;
            terminalShiftFunction(tInfo, nKey, strError);
          }
          // }}}
          // {{{ tab
          else if (strFunction == "tab" || strFunction == "t")
          {
            string strCount;
            ssData >> strCount;
            if (!strCount.empty())
            {
              stringstream ssCount(strCount);
              ssCount >> unCount;
            }
            terminalTab(tInfo, unCount, true, strError);
          }
          // }}}
          // {{{ up
          else if (strFunction == "up")
          {
            string strCount;
            ssData >> strCount;
            if (!strCount.empty())
            {
              stringstream ssCount(strCount);
              ssCount >> unCount;
            }
            terminalUp(tInfo, unCount, true, strError);
          }
          // }}}
          // {{{ wait
          else if (strFunction == "wait" || strFunction == "w")
          {
            terminalWait(tInfo, true, strError);
          }
          // }}}
          // {{{ invalid
          else
          {
            strError = strInvalid;
          }
          // }}}
          if (!tInfo.strSession.empty())
          {
            ssText.str("");
            for (size_t i = 0; i < tInfo.screen.size(); i++)
            {
              if (i == tInfo.unRow && tInfo.unCol < tInfo.screen[i].size())
              {
                stringstream ssCursor;
                ssCursor << char(3) << "08,03" << tInfo.screen[i][tInfo.unCol] << char(3);
                tInfo.screen[i].replace(tInfo.unCol, 1, ssCursor.str());
              }
              ssText << tInfo.screen[i] << endl;
            }
            chat(strTarget, ssText.str(), strSource);
          }
          else if (tInfo.strSession.empty())
          {
            bExit = true;
          }
        }
        else
        {
          strError = "Please provide the Function.";
        }
        if (!strError.empty())
        {
          chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  " + strError + string(1, char(3)), strSource);
          strError.clear();
        }
      }
      else
      {
        msleep(100);
      }
    }
    if (!tInfo.strSession.empty())
    {
      terminalDisconnect(tInfo, strError);
    }
  }
  else
  {
    chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  Interface::terminalConnect() " + strError + string(1, char(3)), strSource);
  }
  chat(strTarget, string(1, char(2)) + string(1, char(3)) + (string)"07SESSION ENDED" + string(1, char(3)) + string(1, char(2)), strSource);
  lock();
  if (m_terminalClients.find(strIdent) != m_terminalClients.end())
  {
    m_terminalClients[strIdent].clear();
    m_terminalClients.erase(strIdent);
  }
  unlock();
}
// }}}
// {{{ unlock()
void Irc::unlock()
{
  m_mutex.unlock();
}
// }}}
// {{{ var()
string Irc::var(const string strName, Json *ptData)
{
  string strValue;

  if (!empty(ptData, strName))
  {
    strValue = ptData->m[strName]->v;
  }

  return strValue;
}
// }}}
}
}
