// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Auth.cpp
// author     : Ben Kietzman
// begin      : 2022-05-27
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Auth"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Auth()
Auth::Auth(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "auth", argc, argv, pCallback)
{
  m_pAnalyzeCallback = NULL;
}
// }}}
// {{{ ~Auth()
Auth::~Auth()
{
}
// }}}
// {{{ callback()
void Auth::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Auth::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "User"))
  {
    if (!empty(ptJson, "Password"))
    {
      if (exist(ptJson, "Request"))
      {
        if (!empty(ptJson, "Function"))
        {
          string strFunction = ptJson->m["Function"]->v;
          if (strFunction == "password")
          {
            if (!empty(ptJson->m["Request"], "Action"))
            {
              if (m_pWarden != NULL && m_pWarden->radial(ptJson->m["User"]->v, ptJson->m["Password"]->v, strError))
              {
                string strAction = ptJson->m["Request"]->m["Action"]->v;
                if (strAction == "get" || strAction == "pop" || strAction == "push" || strAction == "put")
                {
                  Json *ptData = new Json;
                  if (m_pWarden->vaultRetrieve({"radial", ptJson->m["User"]->v, "Password"}, ptData, strError))
                  {
                    if (strAction == "pop" || strAction == "push" || strAction == "put")
                    {
                      if (!ptData->v.empty())
                      {
                        ptData->pb(ptData->v);
                      }
                      if (strAction == "push")
                      {
                        if (!empty(ptJson->m["Request"], "Password"))
                        {
                          ptData->pb(ptJson->m["Request"]->m["Password"]->v);
                        }
                      }
                      else if (strAction == "put")
                      {
                        if (exist(ptJson->m["Request"], "Password"))
                        {
                          Json *ptPut = new Json;
                          if (!empty(ptJson->m["Request"], "Password"))
                          {
                            ptPut->pb(ptJson->m["Request"]->m["Password"]->v);
                          }
                          else
                          {
                            for (auto &i : ptJson->m["Request"]->m["Password"]->l)
                            {
                              if (!i->v.empty())
                              {
                                ptPut->pb(i->v);
                              }
                            }
                          }
                          if (!ptPut->l.empty())
                          {
                            ptData->i("Password", ptPut);
                          }
                          delete ptPut;
                        }
                      }
                      else if (ptData->l.size() > 1)
                      {
                        delete ptData->l.back();
                        ptData->l.pop_back();
                      }
                      if (m_pWarden->vaultAdd({"radial", ptJson->m["User"]->v, "Password"}, ptData, strError))
                      {
                        ofstream outCred(m_strData + "/.cred");
                        outCred.close();
                        bResult = true;
                        if (empty(ptJson, "Node"))
                        {
                          list<string> nodes;
                          m_mutexShare.lock();
                          for (auto &link : m_l)
                          {
                            if (link->strNode != m_strNode && link->interfaces.find("auth") != link->interfaces.end())
                            {
                              nodes.push_back(link->strNode);
                            }
                          }
                          m_mutexShare.unlock();
                          for (auto &node : nodes)
                          {
                            Json *ptLink = new Json(ptJson);
                            ptLink->m["Request"]->i("Action", "put");
                            ptLink->m["Request"]->i("Password", ptData);
                            ptLink->i("Node", node);
                            delete ptLink;
                          }
                        }
                      }
                    }
                    else
                    {
                      bResult = true;
                      ptJson->i("Resposne", ptData);
                    }
                  }
                  delete ptData;
                }
                else
                {
                  strError = "Please provide a valid Action within the Request:  get. pop, push, put.";
                }
              }
              else if (m_pWarden == NULL)
              {
                strError = "Please initialize Warden.";
              }
            }
            else
            {
              strError = "Please provide the Action within the Request.";
            }
          }
          else
          {
            strError = "Please provide a valid Function:  password.";
          }
        }
        else if (!empty(ptJson->m["Request"], "Interface"))
        {
          Json *ptData = new Json(ptJson);
          if (m_pWarden != NULL && m_pWarden->authz(ptData, strError))
          {
            if (exist(ptData, "radial") && exist(ptData->m["radial"], "Access") && exist(ptData->m["radial"]->m["Access"], ptJson->m["Request"]->m["Interface"]->v))
            {
              string strAccessFunction = "Function";
              if (m_accessFunctions.find(ptJson->m["Request"]->m["Interface"]->v) != m_accessFunctions.end() && m_accessFunctions[ptJson->m["Request"]->m["Interface"]->v] != "Function")
              {
                strAccessFunction = m_accessFunctions[ptJson->m["Request"]->m["Interface"]->v];
              }
              if (ptData->m["radial"]->m["Access"]->m[ptJson->m["Request"]->m["Interface"]->v]->v == "all")
              {
                bResult = true;
              }
              else if (!empty(ptJson, strAccessFunction))
              {
                if (ptData->m["radial"]->m["Access"]->m[ptJson->m["Request"]->m["Interface"]->v]->v == ptJson->m[strAccessFunction]->v)
                {
                  bResult = true;
                }
                else
                {
                  for (auto &access : ptData->m["radial"]->m["Access"]->m[ptJson->m["Request"]->m["Interface"]->v]->l)
                  {
                    if (access->v == ptJson->m[strAccessFunction]->v)
                    {
                      bResult = true;
                    }
                  }
                }
              }
            }
            if (!bResult)
            {
              if (m_pAnalyzeCallback != NULL)
              {
                bResult = m_pAnalyzeCallback(strPrefix, ptJson, ptData, strError);
              }
              else
              {
                strError = "Authorization denied.";
              }
            }
          }
          else if (m_pWarden == NULL)
          {
            strError = "Please initialize Warden.";
          }
          delete ptData;
        }
        else
        {
          strError = "Please provide the Function or the Interface within the Request.";
        }
      }
      else
      {
        strError = "Please provide the Request.";
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
// {{{ init()
bool Auth::init()
{
  bool bResult = false;
  string strError;
  Json *ptJson = new Json;

  ptJson->i("Function", "list");
  if (hub(ptJson, strError))
  {
    if (exist(ptJson, "Response"))
    {
      bResult = true;
      for (auto &interface : ptJson->m["Response"]->m)
      {
        m_accessFunctions[interface.first] = ((!empty(interface.second, "AccessFunction"))?interface.second->m["AccessFunction"]->v:"Function");
      }
    }
  }
  delete ptJson;

  return bResult;
}
// }}}
// {{{ setAnalyze()
void Auth::setAnalyze(bool (*pCallback)(string, Json *, Json *, string &))
{
  m_pAnalyzeCallback = pCallback;
}
// }}}
}
}
