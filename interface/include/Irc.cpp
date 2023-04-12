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
Irc::Irc(string strPrefix, int argc, char **argv, void (*pCallback)(string, Json *, const bool)) : Interface(strPrefix, "irc", argc, argv, pCallback)
{
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
  m_CMonitorChannelsModify = 0;
}
// }}}
// {{{ ~Irc()
Irc::~Irc()
{
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
void Irc::analyze(string strPrefix, const string strTarget, const string strUserID, const string strIdent, const string strFirstName, const string strLastName, const bool bAdmin, map<string, bool> &auth, stringstream &ssData)
{
  list<string> actions = {"central", "centralmon", "database", "interface", "irc", "live", "radial", "ssh (s)", "storage", "terminal (t)"};
  string strAction;
  Json *ptRequest = new Json;

  strPrefix += "->analyze()";
  ssData >> strAction;
  if (!strAction.empty())
  {
    ptRequest->i("Action", strAction);
    if (m_pAnalyzeCallback1 == NULL || !m_pAnalyzeCallback1(strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, actions, strAction, ssData, ptRequest))
    {
      // {{{ central
      if (strAction == "central")
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
                string strType;
                ptRequest->i("Form", strForm);
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
        string strSubTarget;
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
      // {{{ ssh
      else if (strAction == "ssh" || strAction == "s")
      {
        string strFunction;
        ssData >> strFunction;
        if (!strFunction.empty())
        {
          ptRequest->i("Function", strFunction);
          if (strFunction == "connect")
          {
            string strPassword, strPort, strServer, strUser;
            ssData >> strServer >> strPort >> strUser >> strPassword;
            ptRequest->i("Server", strServer);
            ptRequest->i("Port", strPort);
            ptRequest->i("User", strUser);
            ptRequest->i("Password", strPassword);
          }
          else
          {
            lock();
            if (m_sshClients.find(strIdent) != m_sshClients.end())
            {
              string strCommand;
              getline(ssData, strCommand);
              if (!strCommand.empty())
              {
                strCommand = strFunction + (string)" " + strCommand;
              }
              else
              {
                strCommand = strFunction;
              }
              ptRequest->i("Command", strCommand);
            }
            unlock();
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
          ptRequest->i("Function", strFunction);
          if (strFunction == "connect")
          {
            string strPort, strServer;
            ssData >> strServer >> strPort;
            ptRequest->i("Server", strServer);
            ptRequest->i("Port", strPort);
          }
          else
          {
            lock();
            if (m_terminalClients.find(strIdent) != m_terminalClients.end())
            {
              string strCommand;
              getline(ssData, strCommand);
              if (!strCommand.empty())
              {
                strCommand = strFunction + (string)" " + strCommand;
              }
              else
              {
                strCommand = strFunction;
              }
              ptRequest->i("Command", strCommand);
            }
            unlock();
          }
        }
      }
      // }}}
    }
  }
  analyze(strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, actions, ptRequest);
  delete ptRequest;
}
void Irc::analyze(string strPrefix, const string strTarget, const string strUserID, const string strIdent, const string strFirstName, const string strLastName, const bool bAdmin, map<string, bool> &auth, list<string> &actions, Json *ptData)
{
  // {{{ prep work
  string strAction = var("Action", ptData), strError, strValue;
  stringstream ssQuery, ssText;
  ssText << char(3) << "11,10 " << m_strNode << " " << char(3) << " " << char(3) << "07,05 " << m_strName << " " << char(3) << " " << char(3) << "13,06 " << ((!strAction.empty())?strAction:"actions") << " " << char(3);
  // }}}
  // {{{ callback
  if (m_pAnalyzeCallback2 != NULL && m_pAnalyzeCallback2(strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, strAction, ptData, ssText))
  {
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
            ssQuery << "select name, date_format(creation_date, '%Y-%m-%d') creation_date, website, wiki, date_format(retirement_date, '%Y-%m-%d') retirement_date, description from application where id = " << strApplicationID;
            list<map<string, string> > *getApplication = dbquery("central_r", ssQuery.str(), strError);
            if (getApplication != NULL)
            {
              if (!getApplication->empty())
              {
                map<string, string> getApplicationRow = getApplication->front();
                ssText << " Application:  " << getApplicationRow["name"];
                ssText << ", Website:  " << getApplicationRow["website"];
                if (getApplicationRow["wiki"] == "1")
                {
                  ssText << ", WIKI:  https://kietzman.org/wiki/index.php/" << getApplicationRow["name"];
                }
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
    // {{{ server
    else if (strFunction == "server")
    {
      string strServerID = var("ServerID", ptData);
      ssText << " " << char(3) << "00,14 " << strFunction << " " << char(3);
      // {{{ server
      if (!strServerID.empty())
      {
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
          // {{{ invalid
          else
          {
            ssText << ":  Please provide the Form immediately following the ServerID.";
          }
          // }}}
        }
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
          ssText << ":  Please provide the Form immediately following the User:  general, applications, servers.";
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
      ssText << ":  The central action is used to access Central functionalities.  Please provide one of the following functions immediately following the action:  application, server, user." << endl;
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
  // {{{ interface
  else if (strAction == "interface")
  {
    string strInterface = var("Interface", ptData);
    if (!strInterface.empty())
    {
      string strFunction = var("Function", ptData);
      if (!strFunction.empty())
      {
        if (strFunction == "restart" || strFunction == "start" || strFunction == "stop")
        {
          if (isLocalAdmin(strIdent, "Radial", bAdmin, auth))
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
              for (auto &link : m_links)
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
                stringstream ssSubText;
                if (strFunction == "start" || interfaceRemove(node, strInterface, strError) || strError == "Encountered an unknown error." || strError == "Interface not found.")
                {
                  bool bStopped = true;
                  if (strFunction == "restart" || strFunction == "stop")
                  {
                    time_t CTime[2];
                    bStopped = false;
                    time(&(CTime[0]));
                    CTime[1] = CTime[0];
                    while (!bStopped && (CTime[1] - CTime[0]) < 40)
                    {
                      m_mutexShare.lock();
                      if (node == m_strNode)
                      {
                        if (m_interfaces.find(strInterface) == m_interfaces.end())
                        {
                          bStopped = true;
                        }
                      }
                      else
                      {
                        auto linkIter = m_links.end();
                        for (auto i = m_links.begin(); linkIter == m_links.end() && i != m_links.end(); i++)
                        {
                          if ((*i)->strNode == node)
                          {
                            linkIter = i;
                          }
                        }
                        if (linkIter != m_links.end())
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
                  else
                  {
                    strError = "Failed to stop.";
                  }
                }
                ssSubText << node << ":  " << ((bSubResult)?"done":strError);
                chat(strTarget, ssSubText.str());
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
          ssText << ":  Please provide a valid Function immediately following the Interface:  restart, start, stop.";
        }
      }
      else
      {
        bool bFirst = true;
        ssText << ":  ";
        m_mutexShare.lock();
        for (auto &link : m_links)
        {
          bool bFound = false;
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
      for (auto &link : m_links)
      {
        ssText << link->strNode;
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
    string strSubTarget = var("Target", ptData);
    if (!strSubTarget.empty())
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
      ssText << ":  The irc action is used to send IRC messages.  Please provide a Target immediately following the action.";
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
    if (!strApplication.empty())
    {
      if (isLocalAdmin(strIdent, strApplication, bAdmin, auth))
      {
        if (live(strApplication, strUser, {{"Action", "message"}, {"Class", ((!strClass.empty())?strClass:"info")}, {"Title", ((!strTitle.empty())?strTitle:strApplication)}, {"Body", strMessage}}, strError))
        {
          ssText << ":  The message has been sent.";
        }
        else
        {
          ssText << " error:  " << strError;
        }
      }
      else
      {
        ssText << " error:  You are not authorized to send a message to the Application.  You must be a registered as a local administrator for the Application.";
      }
    }
    else
    {
      ssText << ":  The live action is used to send a JSON formatted request to Radial Live.  Please provide the Application within the JSON request.";
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
  // {{{ ssh || s
  else if (strAction == "ssh" || strAction == "s")
  {
    string strFunction = var("Function", ptData);
    if (!strFunction.empty())
    {
      string strCommand = var("Command", ptData);
      if (strFunction == "connect")
      {
        string strPassword = var("Password", ptData), strPort = var("Port", ptData), strServer = var("Server", ptData), strUser = var("User", ptData);
        if (!strServer.empty() && !strPort.empty() && !strUser.empty() && !strPassword.empty())
        {
          lock();
          if (m_sshClients.find(strIdent) == m_sshClients.end())
          {
            thread threadSsh(&Irc::ssh, this, strPrefix, strTarget, strIdent, strServer, strPort, strUser, strPassword);
            pthread_setname_np(threadSsh.native_handle(), "ssh");
            threadSsh.detach();
          }
          else
          {
            ssText << " error:  You already have a session open.  Use the following action to close the session:  exit.";
          }
          unlock();
        }
      }
      else if (!strCommand.empty())
      {
        lock();
        if (m_sshClients.find(strIdent) != m_sshClients.end())
        {
          m_sshClients[strIdent].push_back(strCommand);
        }
        else
        {
          ssText << " error:  Please provide a valid function following the action:  connect.";
        }
        unlock();
      }
    }
    else
    {
      ssText << ":  The ssh action is used to establish and maintain an SSH session.  Please provide the following immediately following the action:  connect [server] [port] [user] [password].";
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
      if (strFunction == "connect")
      {
        string strPort = var("Port", ptData), strServer = var("Server", ptData);
        if (!strPort.empty() || !strServer.empty())
        {
          lock();
          if (m_terminalClients.find(strIdent) == m_terminalClients.end())
          {
            thread threadTerminal(&Irc::terminal, this, strPrefix, strTarget, strIdent, strServer, strPort);
            pthread_setname_np(threadTerminal.native_handle(), "terminal");
            threadTerminal.detach();
          }
          else
          {
            ssText << " error:  You already have a session open.  Use the following action to close the session:  exit.";
          }
          unlock();
        }
      }
      else if (!strCommand.empty())
      {
        lock();
        if (m_terminalClients.find(strIdent) != m_terminalClients.end())
        {
          m_terminalClients[strIdent].push_back(strCommand);
        }
        else
        {
          ssText << " error:  Please provide a valid function following the action:  connect.";
        }
        unlock();
      }
    }
    else
    {
      ssText << ":  The terminal action is used to establish and maintain a terminal session.  Please provide the following immediately following the action:  connect [server] [port].";
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
  chat(strTarget, ssText.str());
  // }}}
}
// }}}
// {{{ analyzer()
void Irc::analyzer(string strPrefix, const string strTarget, const string strUserID, const string strIdent, const string strData)
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
  analyze(strPrefix, strTarget, strUserID, strIdent, strFirstName, strLastName, bAdmin, auth, ssData);
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
        pollfd *fds;
        size_t unIndex = 0, unPosition;
        string strBuffer[2], strName = "Radial", strMessage, strNick, strNickBase = "radial_bot", strUser = "radial_bot";
        SSL *ssl = NULL;
        time_t CTime[2] = {0, 0};
        // }}}
        while (!bExit)
        {
          // {{{ prep work
          if (enabled())
          {
            time(&(CTime[1]));
            if ((CTime[1] - CTime[0]) > 120)
            {
              monitorChannels(strPrefix);
              CTime[0] = CTime[1];
            }
          }
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
            if (fds[0].revents & POLLIN)
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
                          m_channels.push_back(strChannel);
                          m_channels.sort();
                          m_channels.unique();
                          if (strChannel == "#radial")
                          {
                            ssMessage.str("");
                            ssMessage << char(3) << "11,10 " << m_strNode << " " << char(3) << " " << char(3) << "07,05 " << m_strName << " " << char(3) << " " << strPrefix << ":  Assumed master role.";
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
                          auto channelIter = m_channels.end();
                          for (auto i = m_channels.begin(); channelIter == m_channels.end() && i != m_channels.end(); i++)
                          {
                            if ((*i) == strChannel)
                            {
                              channelIter = i;
                            }
                          }
                          if (channelIter != m_channels.end())
                          {
                            m_channels.erase(channelIter);
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
                          thread threadAnalyzer(&Irc::analyzer, this, strPrefix, ((bChannel)?strTarget:strID), strID, strIdent, strData);
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
void Irc::callback(string strPrefix, Json *ptJson, const bool bResponse)
{
  bool bResult = false;
  string strError;

  threadIncrement();
  strPrefix += "->Irc::callback()";
  if (!empty(ptJson, "Function"))
  {
    if (ptJson->m["Function"]->v == "chat")
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
              unCount = 0;
              while (unCount++ < 40 && !enabled())
              {
                msleep(250);
              }
              if (enabled())
              {
                bResult = true;
                chat(ptJson->m["Target"]->v, ptJson->m["Message"]->v);
              }
              else
              {
                strError = "IRC disabled.";
              }
            }
            else
            {
              Json *ptLink = new Json(ptJson);
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
    else
    {
      strError = "Please provide a valid Function:  chat.";
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
    hub(ptJson, false);
  }
  delete ptJson;
  threadDecrement();
}
// }}}
// {{{ chat()
void Irc::chat(const string strTarget, const string strMessage)
{
  auto channelIter = m_channels.end();
  bool bPart = false;
  size_t unMaxLength = 450;
  string strLine;
  stringstream ssLines(strMessage), ssMessage, ssPrefix;

  for (auto i = m_channels.begin(); channelIter == m_channels.end() && i != m_channels.end(); i++)
  {
    if ((*i) == strTarget)
    {
      channelIter = i;
    }
  }
  if (channelIter == m_channels.end())
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
// {{{ isLocalAdmin()
bool Irc::isLocalAdmin(const string strUserID, string strApplication, const bool bAdmin, map<string, bool> auth)
{
  return (bAdmin || (!strUserID.empty() && auth.find(strApplication) != auth.end() && auth[strApplication]));
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
// {{{ lock()
void Irc::lock()
{
  m_mutex.lock();
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
void Irc::monitorChannels(string strPrefix)
{
  struct stat tStat;
  stringstream ssFile, ssMessage;

  strPrefix += "->Irc::monitorChannels()";
  ssFile << m_strData << "/irc/monitor.channels";
  if (stat(ssFile.str().c_str(), &tStat) == 0)
  {
    if (tStat.st_mtime > m_CMonitorChannelsModify)
    {
      ifstream inMonitor;
      inMonitor.open(ssFile.str().c_str());
      if (inMonitor)
      {
        string strLine;
        stringstream ssJson;
        m_CMonitorChannelsModify = tStat.st_mtime;
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
              list<string> subChannels = m_channels;
              for (auto &channel : subChannels)
              {
                part(channel);
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
          else
          {
            ssMessage.str("");
            ssMessage << strPrefix << " error [" << ssFile.str() << "]:  JSON is empty.";
            log(ssMessage.str());
          }
        }
        else
        {
          ssMessage.str("");
          ssMessage << strPrefix << " error [" << ssFile.str() << "]:  File is empty.";
          log(ssMessage.str());
        }
      }
      else
      {
        ssMessage.str("");
        ssMessage << strPrefix << "->ifstream::open(" << errno << ") error [" << ssFile.str() << "]:  " << strerror(errno);
        log(ssMessage.str());
      }
      inMonitor.close();
    }
  }
  else
  {
    ssMessage.str("");
    ssMessage << strPrefix << "->stat(" << errno << ") error [" << ssFile.str() << "]:  " << strerror(errno);
    log(ssMessage.str());
  }
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
    part(channel);
  }
  ssMessage << ":" << m_strNick << " QUIT :Quitting.\r\n";
  push(ssMessage.str());
}
// }}}
// {{{ setAnalyze()
void Irc::setAnalyze(bool (*pCallback1)(string, const string, const string, const string, const string, const string, const bool, map<string, bool> &, list<string> &, const string, stringstream &, Json *), bool (*pCallback2)(string, const string, const string, const string, const string, const string, const bool, map<string, bool> &, const string, Json *, stringstream &))
{
  m_pAnalyzeCallback1 = pCallback1;
  m_pAnalyzeCallback2 = pCallback2;
}
// }}}
// {{{ sshClient()
void Irc::ssh(string strPrefix, const string strTarget, const string strUserID, const string strServer, const string strPort, const string strUser, const string strPassword)
{
  string strError;
  ssh_session session;

  strPrefix += "->ssh()";
  chat(strTarget, string(1, char(2)) + string(1, char(3)) + (string)"03SESSION STARTED" + string(1, char(3)) + string(1, char(2)));
  if ((session = ssh_new()) != NULL)
  {
    int nPort;
    stringstream ssPort(strPort);
    ssh_options_set(session, SSH_OPTIONS_HOST, strServer.c_str());
    ssPort >> nPort;
    ssh_options_set(session, SSH_OPTIONS_PORT, &nPort);
    ssh_options_set(session, SSH_OPTIONS_USER, strUser.c_str());
    if (ssh_connect(session) == SSH_OK)
    {
      int fdSocket = ssh_get_fd(session), nMethod;
      ssh_userauth_none(session, NULL);
      nMethod = ssh_userauth_list(session, NULL);
      if ((nMethod & SSH_AUTH_METHOD_NONE && sshAuthenticateNone(session) == SSH_AUTH_SUCCESS) || (nMethod & SSH_AUTH_METHOD_INTERACTIVE && sshAuthenticateKbdint(session, strPassword) == SSH_AUTH_SUCCESS) || (nMethod & SSH_AUTH_METHOD_PASSWORD && sshAuthenticatePassword(session, strPassword) == SSH_AUTH_SUCCESS))
      {
        ssh_channel channel;
        if ((channel = ssh_channel_new(session)) != NULL)
        {
          if (ssh_channel_open_session(channel) == SSH_OK)
          {
            if (ssh_channel_request_pty(channel) == SSH_OK)
            {
              if (ssh_channel_change_pty_size(channel, 80, 24) == SSH_OK)
              {
                if (ssh_channel_request_shell(channel) == SSH_OK)
                {
                  bool bExit = false, bReading;
                  int nReturn;
                  char szBuffer[4096];
                  list<string> messages;
                  size_t unPosition;
                  string strBuffer[2];
                  while (!bExit)
                  {
                    pollfd fds[1];
                    fds[0].fd = fdSocket;
                    fds[0].events = POLLIN;
                    if (strBuffer[1].empty())
                    {
                      lock();
                      while (!m_sshClients[strUserID].empty())
                      {
                        strBuffer[1].append(m_sshClients[strUserID].front() + "\n");
                        m_sshClients[strUserID].pop_front();
                      }
                      unlock();
                    }
                    if (!strBuffer[1].empty())
                    {
                      fds[0].events |= POLLOUT;
                    }
                    bReading = false;
                    if ((nReturn = poll(fds, 1, 500)) > 0)
                    {
                      if (fds[0].revents & POLLIN)
                      {
                        if ((nReturn = ssh_channel_read_nonblocking(channel, szBuffer, 4096, 0)) > 0)
                        {
                          bReading = true;
                          strBuffer[0].append(szBuffer, nReturn);
                          while ((unPosition = strBuffer[0].find("\n")) != string::npos)
                          {
                            messages.push_back(strBuffer[0].substr(0, unPosition));
                            strBuffer[0].erase(0, (unPosition + 1));
                          }
                        }
                        else
                        {
                          bExit = true;
                        }
                      }
                      if (fds[0].revents & POLLOUT)
                      {
                        if ((nReturn = ssh_channel_write(channel, strBuffer[1].c_str(), strBuffer[1].size())) > 0)
                        {
                          strBuffer[1].erase(0, nReturn);
                        }
                        else
                        {
                          bExit = true;
                        }
                      }
                    }
                    else if (nReturn < 0)
                    {
                      chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  poll() " + strerror(errno) + string(1, char(3)));
                    }
                    if ((bExit || !bReading) && !messages.empty())
                    {
                      stringstream ssTexts;
                      while (!messages.empty())
                      {
                        string strBuffer = messages.front();
                        stringstream ssBuffer;
                        while (!strBuffer.empty())
                        {
                          if (strBuffer[0] == '\033' && strBuffer.size() > 1)
                          {
                            if (strBuffer[1] == '[' && strBuffer.size() > 2)
                            {
                              char cEnd = '\0', cLast = '\0';
                              for (size_t i = 2; cEnd == '\0' && i < strBuffer.size(); i++)
                              {
                                unPosition = i;
                                if (isalpha(strBuffer[i]))
                                {
                                  cEnd = strBuffer[i];
                                }
                                cLast = strBuffer[i];
                              }
                              if (cEnd != '\0' || cLast == ';')
                              {
                                strBuffer.erase(0, unPosition);
                              }
                            }
                          }
                          else if (strBuffer[0] != '\r')
                          {
                            ssBuffer << strBuffer[0];
                          }
                          strBuffer.erase(0, 1);
                        }
                        ssTexts << ssBuffer.str() << endl;
                        messages.pop_front();
                      }
                      chat(strTarget, ssTexts.str());
                    }
                  }
                }
                else
                {
                  chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  ssh_channel_request() " + ssh_get_error(session) + string(1, char(3)));
                }
              }
              else
              {
                chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  ssh_channel_change_pty_size() " + ssh_get_error(session) + string(1, char(3)));
              }
            }
            else
            {
              chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  ssh_channel_request_pty() " + ssh_get_error(session) + string(1, char(3)));
            }
          }
          else
          {
            chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  ssh_channel_open_session() " + ssh_get_error(session) + string(1, char(3)));
          }
          ssh_channel_free(channel);
        }
        else
        {
          chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  ssh_channel_new() " + ssh_get_error(session) + string(1, char(3)));
        }
      }
      else
      {
        chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  sshAuthenticate*() " + ssh_get_error(session) + string(1, char(3)));
      }
      ssh_disconnect(session);
    }
    else
    {
      chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  ssh_connect() " + ssh_get_error(session) + string(1, char(3)));
    }
    ssh_free(session);
  }
  else
  {
    chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  ssh_new() Failed to initialize SSH session." + string(1, char(3)));
  }
  chat(strTarget, string(1, char(2)) + string(1, char(3)) + (string)"07SESSION ENDED" + string(1, char(3)) + string(1, char(2)));
  lock();
  m_sshClients[strUserID].clear();
  m_sshClients.erase(strUserID);
  unlock();
}
// }}}
// {{{ sshAuthenticateNone()
int Irc::sshAuthenticateNone(ssh_session session)
{
  return ssh_userauth_none(session, NULL);
}
// }}}
// {{{ sshAuthenticatePassword()
int Irc::sshAuthenticatePassword(ssh_session session, const string strPassword)
{
  return ssh_userauth_password(session, NULL, strPassword.c_str());
}
// }}}
// {{{ sshAuthenticateKbdint()
int Irc::sshAuthenticateKbdint(ssh_session session, const string strPassword)
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
// {{{ terminal()
void Irc::terminal(string strPrefix, const string strTarget, const string strUserID, string strServer, string strPort)
{
  bool bConnected = false;
  string strError;
  Terminal *pTerminal = new Terminal;

  strPrefix += "->terminal()";
  chat(strTarget, string(1, char(2)) + string(1, char(3)) + (string)"07SESSION STARTED" + string(1, char(3)) + string(1, char(2)));
  // {{{ initialize
  if (pTerminal->connect(strServer, strPort))
  {
    bConnected = true;
  }
  else
  {
    strError = (string)"Terminal::connect() " + pTerminal->error();
  }
  // }}}
  if (bConnected)
  {
    bool bExit = false, bProcess = false;
    string strData;
    stringstream ssTexts;
    time_t CTime[2] = {0, 0};
    vector<string> vecScreen;
    pTerminal->screen(vecScreen);
    for (size_t i = 0; i < vecScreen.size(); i++)
    {
      ssTexts << vecScreen[i] << endl;
    }
    vecScreen.clear();
    chat(strTarget, ssTexts.str());
    time(&(CTime[0]));
    while (!bExit)
    {
      bProcess = false;
      strError.clear();
      lock();
      if (!m_terminalClients[strUserID].empty())
      {
        bProcess = true;
        strData = m_terminalClients[strUserID].front();
        m_terminalClients[strUserID].pop_front();
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
          string strData, strWait, strInvalid = "Please provide a valid Function:  ctrl, disconnect, down, enter, escape, function, home, key, keypadEnter, left, right, send, shiftFunction, tab, up, wait.";
          // {{{ ctrl
          if (strFunction == "ctrl")
          {
            ssData >> strData;
            if (strData.size() == 1)
            {
              if (!pTerminal->sendCtrl(strData[0], true))
              {
                strError = pTerminal->error();
              }
            }
            else
            {
              strError = "Data should contain a single character for this Function.";
            }
          }
          // }}}
          // {{{ disconnect
          else if (strFunction == "disconnect" || strFunction == "exit")
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
            if (!pTerminal->sendDown(unCount, true))
            {
              strError = pTerminal->error();
            }
          }
          // }}}
          // {{{ enter
          else if (strFunction == "enter")
          {
            if (!pTerminal->sendEnter(true))
            {
              strError = pTerminal->error();
            }
          }
          // }}}
          // {{{ escape
          else if (strFunction == "escape")
          {
            if (!pTerminal->sendEscape(true))
            {
              strError = pTerminal->error();
            }
          }
          // }}}
          // {{{ function
          else if (strFunction == "function" || strFunction == "f")
          {
            int nKey = 0;
            ssData >> nKey;
            if (nKey >= 1 && nKey <= 12)
            {
              if (!pTerminal->sendFunction(nKey))
              {
                strError = pTerminal->error();
              }
            }
            else
            {
              strError = "Please provide a Data value between 1 and 12.";
            }
          }
          // }}}
          // {{{ home
          else if (strFunction == "home")
          {
            if (!pTerminal->sendHome(true))
            {
              strError = pTerminal->error();
            }
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
              if (!pTerminal->sendKey(strData[0], unCount, true))
              {
                strError = pTerminal->error();
              }
            }
            else
            {
              strError = "Data should contain a single character for this Function.";
            }
          }
          // }}}
          // {{{ keypadEnter
          else if (strFunction == "keypadEnter")
          {
            if (!pTerminal->sendKeypadEnter(true))
            {
              strError = pTerminal->error();
            }
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
            if (!pTerminal->sendLeft(unCount, true))
            {
              strError = pTerminal->error();
            }
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
            if (!pTerminal->sendRight(unCount, true))
            {
              strError = pTerminal->error();
            }
          }
          // }}}
          // {{{ send
          else if (strFunction == "send")
          {
            string strTrimmed;
            getline(ssData, strData);
            m_manip.ltrim(strTrimmed, strData);
            if (!pTerminal->sendWait(strTrimmed, unCount))
            {
              strError = pTerminal->error();
            }
          }
          // }}}
          // {{{ shiftFunction
          else if (strFunction == "shiftFunction" || strFunction == "sf")
          {
            int nKey = 0;
            ssData >> nKey;
            if (nKey >= 1 && nKey <= 12)
            {
              if (!pTerminal->sendShiftFunction(nKey))
              {
                strError = pTerminal->error();
              }
            }
            else
            {
              strError = "Please provide a Data value between 1 and 12.";
            }
          }
          // }}}
          // {{{ tab
          else if (strFunction == "tab")
          {
            string strCount;
            ssData >> strCount;
            if (!strCount.empty())
            {
              stringstream ssCount(strCount);
              ssCount >> unCount;
            }
            if (!pTerminal->sendTab(unCount, true))
            {
              strError = pTerminal->error();
            }
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
            if (!pTerminal->sendUp(unCount, true))
            {
              strError = pTerminal->error();
            }
          }
          // }}}
          // {{{ wait
          else if (strFunction == "wait")
          {
            if (!pTerminal->wait(true))
            {
              strError = pTerminal->error();
            }
          }
          // }}}
          // {{{ invalid
          else
          {
            strError = strInvalid;
          }
          // }}}
          if (!bExit)
          {
            pTerminal->screen(vecScreen);
            ssTexts.str("");
            for (size_t i = 0; i < vecScreen.size(); i++)
            {
              ssTexts << vecScreen[i] << endl;
            }
            vecScreen.clear();
            chat(strTarget, ssTexts.str());
          }
        }
        else
        {
          strError = "Please provide the Function.";
        }
        if (!strError.empty())
        {
          chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  " + strError + string(1, char(3)));
        }
        time(&(CTime[0]));
      }
      else
      {
        msleep(250);
      }
      time(&(CTime[1]));
      if ((CTime[1] - CTime[0]) > 3600)
      {
        bExit = true;
        chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  The session timed out due to inactivity." + string(1, char(3)));
      }
    }
    pTerminal->disconnect();
  }
  else if (!strError.empty())
  {
    chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  " + strError + string(1, char(3)));
  }
  else
  {
    chat(strTarget, string(1, char(3)) + (string)"04" + string(1, char(2)) + (string)"ERROR:" + string(1, char(2)) + (string)"  Encountered an unknown error." + string(1, char(3)));
  }
  delete pTerminal;
  chat(strTarget, string(1, char(2)) + string(1, char(3)) + (string)"07SESSION ENDED" + string(1, char(3)) + string(1, char(2)));
  lock();
  m_terminalClients[strUserID].clear();
  m_terminalClients.erase(strUserID);
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
