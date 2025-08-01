// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Feedback.cpp
// author     : Ben Kietzman
// begin      : 2023-06-02
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
// {{{ includes
#include "Feedback"
// }}}
extern "C++"
{
namespace radial
{
// {{{ Feedback()
Feedback::Feedback(string strPrefix, int argc, char **argv, void (*pCallback)(string, const string, const bool)) : Interface(strPrefix, "feedback", argc, argv, pCallback)
{
  m_pCallbackAddon = NULL;
  m_functions["action"] = &Feedback::action;
  m_functions["answers"] = &Feedback::answers;
  m_functions["questions"] = &Feedback::questions;
  m_functions["results"] = &Feedback::results;
  m_functions["resultAdd"] = &Feedback::resultAdd;
  m_functions["status"] = &Feedback::status;
  m_functions["survey"] = &Feedback::survey;
  m_functions["surveyEdit"] = &Feedback::surveyEdit;
  m_functions["surveyRemove"] = &Feedback::surveyRemove;
  m_functions["surveys"] = &Feedback::surveys;
  m_functions["type"] = &Feedback::type;
  m_functions["types"] = &Feedback::types;
  m_pThreadSchedule = new thread(&Feedback::schedule, this, strPrefix);
  pthread_setname_np(m_pThreadSchedule->native_handle(), "schedule");
}
// }}}
// {{{ ~Feedback()
Feedback::~Feedback()
{
  m_pThreadSchedule->join();
  delete m_pThreadSchedule;
}
// }}}
// {{{ answers()
bool Feedback::answers(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "question_id"))
  {
    q << "select id, sequence, answer from answer where question_id = " << i->m["question_id"]->v << " order by sequence, id";
    auto g = dbquery("feedback_r", q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
    }
    dbfree(g);
  }
  else
  {
    e = "Please provide the question_id.";
  }

  return b;
}
// }}}
// {{{ callback()
void Feedback::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  strPrefix += "->Feedback::callback()";
  throughput("callback");
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    bool bInvalid = true;
    string strFunction = ptJson->m["Function"]->v;
    radialUser d;
    userInit(ptJson, d);
    if (m_pCallbackAddon != NULL && m_pCallbackAddon(strFunction, d, strError, bInvalid))
    {
      bResult = true;
    }
    else if (bInvalid)
    {
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
}
// }}}
// {{{ questions()
bool Feedback::questions(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "survey_id"))
  {
    q << "select id, sequence, question, type_id, required from question where survey_id = " << i->m["survey_id"]->v << " order by sequence, id";
    auto g = dbquery("feedback_r", q.str(), e);
    if (g != NULL)
    {
      b = true;
      for (auto &r : *g)
      {
        o->pb(r);
      }
    }
    dbfree(g);
  }
  else
  {
    e = "Please provide the survey_id.";
  }

  return b;
}
// }}}
// {{{ results()
bool Feedback::results(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (!empty(i, "survey_id"))
  {
    if (!empty(i, "question_id"))
    {
      radialUser s;
      userInit(d, s);
      s.p->m["i"]->i("id", i->m["survey_id"]->v);
      if (survey(s, e))
      {
        bool bValid = (s.p->m["o"]->m["restrict"]->v == "0");
        if (!bValid && isValid(d, "Feedback") && exist(s.p->m["o"], "owner") && d.u == s.p->m["o"]->m["owner"]->m["userid"]->v)
        {
          bValid = true;
        }
        if (bValid)
        {
          q << "select " << ((s.p->m["o"]->m["anonymous"]->v == "0")?"a.application_contact_id, ":"") << "date_format(a.entry_date, '%Y-%m-%d %H:%i') entry_date, b.answer from result a, result_answer b where a.id = b.result_id and a.survey_id = " << i->m["survey_id"]->v << " and b.question_id = " << i->m["question_id"]->v << " order by a.entry_date";
          auto g = dbquery("feedback_r", q.str(), e);
          if (g != NULL)
          {
            b = true;
            for (auto &r : *g)
            {
              Json *ro = new Json(r);
              if (s.p->m["o"]->m["anonymous"]->v == "0" && !r["application_contact_id"].empty())
              {
                q.str("");
                q << "select b.userid, b.first_name, b.last_name from central.application_contact a, central.person b where a.contact_id = b.id and a.id = " << r["application_contact_id"];
                auto pg = dbquery("central_r", q.str(), e);
                if (pg != NULL && !pg->empty())
                {
                  ro->i("contact", pg->front());
                }
                dbfree(pg);
              }
              o->pb(ro);
              delete ro;
            }
          }
          dbfree(g);
        }
        else
        {
          e = "You are not authorized to view the survey results.";
        }
      }
      userDeinit(s);
    }
    else
    {
      e = "Please provide the question_id.";
    }
  }
  else
  {
    e = "Please provide the question_id.";
  }

  return b;
}
// }}}
// {{{ resultAdd()
bool Feedback::resultAdd(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (exist(i, "survey"))
  {
    if (!empty(i->m["survey"], "id"))
    {
      if (!empty(i->m["survey"], "hash"))
      {
        if (exist(i->m["survey"], "questions") && !i->m["survey"]->m["questions"]->l.empty())
        {
          bool bAnonymous = (!empty(i->m["survey"], "anonymous") && i->m["survey"]->m["anonymous"]->v == "1");
          if (bAnonymous || isValid(d))
          {
            bool bGood = true;
            string strApplicationContactID;
            if (!bAnonymous)
            {
              q << "select b.id from central.application a, central.application_contact b, central.person c where a.id = b.application_id and b.contact_id = c.id and a.name = 'Feedback' and c.userid = '" << d.u << "'";
              auto g = dbquery("central_r", q.str(), e);
              if (g != NULL && !g->empty())
              {
                strApplicationContactID = g->front()["id"];
              }
              else
              {
                bGood = false;
              }
              dbfree(g);
            }
            if (bGood)
            {
              for (auto j = i->m["survey"]->m["questions"]->l.begin(); bGood && j != i->m["survey"]->m["questions"]->l.end(); j++)
              {
                if (!empty((*j), "required") && (*j)->m["required"]->v == "1" && (!exist((*j), "answer") || (empty((*j), "answer") && (*j)->m["answer"]->l.empty() && (*j)->m["answer"]->m.empty())))
                {
                  bGood = false;
                }
              }
              if (bGood)
              {
                string strID;
                q.str("");
                q << "insert into result (survey_id" << ((bAnonymous)?"":", application_contact_id") << ", entry_date) values(" << i->m["survey"]->m["id"]->v << ((bAnonymous)?"":(string)", "+strApplicationContactID) << ", now())";
                if (dbupdate("feedback", q.str(), strID, e))
                {
                  size_t unQuestion = 1;
                  b = true;
                  if (exist(i->m["survey"], "owner") && !empty(i->m["survey"]->m["owner"], "email"))
                  {
                    stringstream ssSubject, ssText;
                    ssSubject << "Feedback Received:  " << ((!empty(i->m["survey"], "title"))?i->m["survey"]->m["title"]->v:"");
                    ssText << "Feedback has been received for the " << ((!empty(i->m["survey"], "title"))?i->m["survey"]->m["title"]->v:"") << " survey." << endl << endl << "https://" << m_strServer << "/feedback/#/results/" << i->m["survey"]->m["hash"]->v;
                    email("", i->m["survey"]->m["owner"]->m["email"]->v, ssSubject.str(), ssText.str(), "");
                  }
                  for (auto &j : i->m["survey"]->m["questions"]->l)
                  {
                    if (exist(j, "answer") && exist(j, "id"))
                    {
                      string strAnswer;
                      if (!empty(j, "answer"))
                      {
                        m_manip.trim(strAnswer, j->m["answer"]->v);
                      }
                      else if (!empty(j->m["answer"], "id"))
                      {
                        strAnswer = j->m["answer"]->m["id"]->v;
                      }
                      else if (!j->m["answer"]->l.empty())
                      {
                        for (auto k = j->m["answer"]->l.begin(); k != j->m["answer"]->l.end(); k++)
                        {
                          if (k != j->m["answer"]->l.begin())
                          {
                            strAnswer += ",";
                          }
                          strAnswer += (*k)->v;
                        }
                      }
                      if (!strAnswer.empty())
                      {
                        q.str("");
                        q << "insert into result_answer (result_id, question_id, answer) values (" << strID << ", " << j->m["id"]->v << ", '" << esc(strAnswer) << "')";
                        dbupdate("feedback", q.str(), e);
                      }
                    }
                    unQuestion++;
                  }
                }
                else
                {
                  e = "Failed to store feedback results.";
                }
              }
              else
              {
                e = "Please answer all required questions.";
              }
            }
            else
            {
              e = "Failed to retrieve your account information.";
            }
          }
          else
          {
            e = "You must be logged in for this survey.";
          }
        }
        else
        {
          e = "Please provide the suvey questions.";
        }
      }
      else
      {
        e = "Please provide the suvey hash.";
      }
    }
    else
    {
      e = "Please provide the survey id.";
    }
  }
  else
  {
    e = "Please provide the survey.";
  }

  return b;
}
// }}}
// {{{ schedule()
void Feedback::schedule(string strPrefix)
{ 
  // {{{ prep work
  string strError;
  time_t CTime[2];

  threadIncrement();
  strPrefix += "->Feedback::schedule()";
  time(&(CTime[0]));
  // }}}
  while (!shutdown())
  {
    time(&(CTime[1]));
    if ((CTime[1] - CTime[0]) >= 60)
    {
      Json *ptMessage = new Json;
      CTime[0] = CTime[1];
      ptMessage->i("Source", m_strNode);
      status(ptMessage);
      ptMessage->i("Action", "status");
      live("Feedback", "", ptMessage);
      delete ptMessage;
    }
    msleep(1000);
  }
  // {{{ post work
  setShutdown();
  threadDecrement();
  // }}}
}
// }}}
// {{{ setCallbackAddon()
void Feedback::setCallbackAddon(bool (*pCallback)(const string, radialUser &, string &, bool &))
{
  m_pCallbackAddon = pCallback;
}
// }}}
// {{{ survey()
bool Feedback::survey(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "hash") || !empty(i, "id"))
  {
    q << "select id, hash, application_contact_id, title, date_format(entry_date, '%Y-%m-%d %H:%i') entry_date, date_format(modified_date, '%Y-%m-%d %H:%i') modified_date, date_format(start_date, '%Y-%m-%d %H:%i') start_date, date_format(end_date, '%Y-%m-%d %H:%i') end_date, public, anonymous, `unique`, `restrict` from survey where ";
    if (!empty(i, "hash"))
    {
      q << "hash = '" << i->m["hash"]->v << "'";
    }
    else
    {
      q << "id = " << i->m["id"]->v;
    }
    auto g = dbquery("feedback_r", q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        auto r = g->front();
        stringstream ssDate;
        struct tm tTime;
        time_t CTime;
        b = true;
        d.p->i("o", r);
        if (!r["application_contact_id"].empty())
        {
          q.str("");
          q << "select b.first_name, b.last_name, b.userid, b.email from application_contact a, person b where a.contact_id = b.id and a.id = " << r["application_contact_id"];
          auto gp = dbquery("central_r", q.str(), e);
          if (gp != NULL && !gp->empty())
          {
            d.p->m["o"]->i("owner", gp->front());
          }
          dbfree(gp);
        }
        time(&CTime);
        localtime_r(&CTime, &tTime);
        ssDate << put_time(&tTime, "%Y-%m-%d %H:%M");
        d.p->m["o"]->i("now_date", ssDate.str());
      }
      else
      {
        e = "Survey does not exist.";
      }
    }
    dbfree(g);
  }
  else
  {
    e = "Please provide the hash or id.";
  }

  return b;
}
// }}}
// {{{ surveyEdit()
bool Feedback::surveyEdit(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  if (exist(i, "survey"))
  {
    if (isValid(d, "Feedback"))
    {
      q << "select b.id from application a, application_contact b, person c where a.id = b.application_id and b.contact_id = c.id and a.name = 'Feedback' and c.userid = '" << d.u << "'";
      auto g = dbquery("central_r", q.str(), e);
      if (g != NULL && !g->empty())
      {
        auto r = g->front();
        if (!exist(i->m["survey"], "id") || i->m["survey"]->m["id"]->v.empty())
        {
          const EVP_MD* md = EVP_md5();
          string strHash, strID, strTime;
          time_t CTime;
          unsigned char md_value[EVP_MAX_MD_SIZE];
          unsigned int md_len;
          EVP_MD_CTX *context = EVP_MD_CTX_new();
          time(&CTime);
          strTime = to_string(CTime);
          EVP_DigestInit_ex2(context, md, NULL);
          EVP_DigestUpdate(context, strTime.c_str(), strTime.length());
          EVP_DigestFinal_ex(context, md_value, &md_len);
          EVP_MD_CTX_free(context);
          strHash.resize(md_len * 2);
          for (unsigned int i = 0; i < md_len ; ++i)
          {
            sprintf(&strHash[i*2], "%02x", md_value[i]);
          }
          i->m["survey"]->i("application_contact_id", r["id"]);
          q.str("");
          q << "insert into survey (application_contact_id, hash) values (" << r["id"] << ", '" << strHash << "')";
          if (dbupdate("feedback", q.str(), strID, e))
          {
            i->m["survey"]->i("id", strID);
            i->m["survey"]->i("hash", strHash);
          }
        }
        if (!empty(i->m["survey"], "id") && !empty(i->m["survey"], "hash"))
        {
          o->i("id", i->m["survey"]->m["id"]->v);
          o->i("hash", i->m["survey"]->m["hash"]->v);
          q.str("");
          q << "select application_contact_id from survey where id = " << i->m["survey"]->m["id"]->v;
          auto gs = dbquery("feedback", q.str(), e);
          if (gs != NULL && !gs->empty())
          {
            auto sr = gs->front();
            if (sr["application_contact_id"] == r["id"])
            {
              q.str("");
              q << "update survey set ";
              q << "title = ";
              if (!empty(i->m["survey"], "title"))
              {
                q << "'" << esc(i->m["survey"]->m["title"]->v) << "'";
              }
              else
              {
                q << "null";
              }
              q << ", ";
              q << "modified_date = now(),";
              q << "public = " << ((!empty(i->m["survey"], "public"))?i->m["survey"]->m["public"]->v:"null") << ", ";
              q << "anonymous = " << ((!empty(i->m["survey"], "anonymous"))?i->m["survey"]->m["anonymous"]->v:"null") << ", ";
              q << "`unique` = " << ((!empty(i->m["survey"], "unique"))?i->m["survey"]->m["unique"]->v:"null") << ", ";
              q << "`restrict` = " << ((!empty(i->m["survey"], "restrict"))?i->m["survey"]->m["restrict"]->v:"null") << ", ";
              q << "start_date = ";
              if (!empty(i->m["survey"], "start_date"))
              {
                q << "'" << esc(i->m["survey"]->m["start_date"]->v) << "'";
              }
              else
              {
                q << "null";
              }
              q << ", ";
              q << "end_date = ";
              if (!empty(i->m["survey"], "end_date"))
              {
                q << "'" << esc(i->m["survey"]->m["end_date"]->v) << "'";
              }
              else
              {
                q << "null";
              }
              q << " where id = " << i->m["survey"]->m["id"]->v;
              if (dbupdate("feedback", q.str(), e))
              {
                b = true;
                if (exist(i->m["survey"], "questions"))
                {
                  for (auto &j : i->m["survey"]->m["questions"]->l)
                  {
                    if (empty(j, "id"))
                    {
                      string strID;
                      q.str("");
                      q << "insert into question (survey_id) values (" << i->m["survey"]->m["id"]->v << ")";
                      if (dbupdate("feedback", q.str(), strID, e))
                      {
                        j->i("id", strID);
                      }
                    }
                    if (!empty(j, "id"))
                    {
                      string strTypeID;
                      if (exist(j, "type") && !empty(j->m["type"], "id"))
                      {
                        q.str("");
                        q << "select id from type where id = " << j->m["type"]->m["id"]->v;
                        auto gt = dbquery("feedback_r", q.str(), e);
                        if (gt != NULL && !gt->empty())
                        {
                          strTypeID = gt->front()["id"];
                        }
                        dbfree(gt);
                      }
                      q.str("");
                      q << "update question set ";
                      q << "type_id = " << ((!strTypeID.empty())?strTypeID:"null") << ", ";
                      q << "sequence = " << ((!empty(j, "sequence"))?j->m["sequence"]->v:"null") << ", ";
                      q << "required = " << ((!empty(j, "required"))?j->m["required"]->v:"null") << ", ";
                      q << "question = ";
                      if (!empty(j, "question"))
                      {
                        q << "'" << esc(j->m["question"]->v) << "'";
                      }
                      else
                      {
                        q << "null";
                      }
                      q << " where id = " << j->m["id"]->v;
                      if (dbupdate("feedback", q.str(), e))
                      {
                        if (exist(j, "answers"))
                        {
                          for (auto &k : j->m["answers"]->l)
                          {
                            if (empty(k, "id"))
                            {
                              string strID;
                              q.str("");
                              q << "insert into answer (question_id) values (" << j->m["id"]->v << ")";
                              if (dbupdate("feedback", q.str(), strID, e))
                              {
                                k->i("id", strID);
                              }
                            }
                            if (!empty(k, "id"))
                            {
                              q.str("");
                              q << "update answer set ";
                              q << "sequence = " << ((!empty(k, "sequence"))?k->m["sequence"]->v:"null") << ", ";
                              q << "answer = ";
                              if (!empty(k, "answer"))
                              {
                                q << "'" << esc(k->m["answer"]->v) << "'";
                              }
                              else
                              {
                                q << "null";
                              }
                              q << " where id = " << k->m["id"]->v;
                              dbupdate("feedback", q.str(), e);
                            }
                          }
                          q.str("");
                          q << "select id from answer where question_id = " << j->m["id"]->v;
                          auto qa = dbquery("feedback", q.str(), e);
                          if (qa != NULL)
                          {
                            for (auto &ra : *qa)
                            {
                              bool bFound = false;
                              for (auto k = j->m["answers"]->l.begin(); !bFound && k != j->m["answers"]->l.end(); k++)
                              {
                                if (!empty((*k), "id") && ra["id"] == (*k)->m["id"]->v)
                                {
                                  bFound = true;
                                }
                              }
                              if (!bFound)
                              {
                                q.str("");
                                q << "delete from answer where id = " << ra["id"];
                                dbupdate("feedback", q.str(), e);
                              }
                            }
                          }
                          dbfree(qa);
                        }
                      }
                    }
                  }
                  q.str("");
                  q << "select id from question where survey_id = " << i->m["survey"]->m["id"]->v;
                  auto qq = dbquery("feedback", q.str(), e);
                  if (qq != NULL)
                  {
                    for (auto &rq : *qq)
                    {
                      bool bFound = false;
                      for (auto j = i->m["survey"]->m["questions"]->l.begin(); !bFound && j != i->m["survey"]->m["questions"]->l.end(); j++)
                      {
                        if (!empty((*j), "id") && rq["id"] == (*j)->m["id"]->v)
                        {
                          bFound = true;
                        }
                      }
                      if (!bFound)
                      {
                        q.str("");
                        q << "delete from question where id = " << rq["id"];
                        dbupdate("feedback", q.str(), e);
                      }
                    }
                  }
                  dbfree(qq);
                }
              }
              else
              {
                e = "Failed to update survey.";
              }
            }
            else
            {
              e = "You are not the owner of this survey.";
            }
          }
          else
          {
            e = "Failed to retrieve survey.";
          }
          dbfree(gs);
        }
        else if (e.empty())
        {
          e = "Please provide the id and hash.";
        }
      }
      else
      {
        e = "Failed to fetch contact information.";
      }
      dbfree(g);
    }
    else
    {
      e = "You are not authorized to perform this action.";
    }
  }
  else
  {
    e = "Please provide the survey.";
  }

  return b;
}
// }}}
// {{{ surveyRemove()
bool Feedback::surveyRemove(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "id"))
  {
    q << "select application_contact_id from survey where id = " << i->m["id"]->v;
    auto g = dbquery("feedback_r", q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        auto r = g->front();
        string strUserID;
        if (!r["application_contact_id"].empty())
        {
          q.str("");
          q << "select b.userid from application_contact a, person b where a.contact_id = b.id and a.id = " << r["application_contact_id"];
          auto *gp = dbquery("central_r", q.str(), e);
          if (gp != NULL && !gp->empty())
          {
            strUserID = gp->front()["userid"];
          }
          dbfree(gp);
        }
        if (isLocalAdmin(d, "Feedback") || (isValid(d, "Feedback") && !strUserID.empty() && d.u == strUserID))
        {
          q.str("");
          q << "delete from survey where id = " << i->m["id"]->v;
          if (dbupdate("feedback", q.str(), e))
          {
            b = true;
          }
          else
          {
            e = "Failed to remove survey.";
          }
        }
        else
        {
          e = "You are not authorized to remove the survey.";
        }
      }
      else
      {
        e = "Survey does not exist.";
      }
    }
    dbfree(g);
  }
  else
  {
    e = "Please provide the id.";
  }

  return b;
}
// }}}
// {{{ surveys()
bool Feedback::surveys(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"], *o = d.p->m["o"];

  q << "select a.id, a.application_contact_id, a.title, a.public, a.anonymous, a.unique, a.restrict, date_format(a.entry_date, '%Y-%m-%d %H:%i') entry_date, date_format(a.modified_date, '%Y-%m-%d %H:%i') modified_date, date_format(a.start_date, '%Y-%m-%d %H:%i') start_date, date_format(a.end_date, '%Y-%m-%d %H:%i') end_date, a.hash from survey a";
  if (!empty(i, "type") && i->m["type"]->v == "Your Surveys")
  {
    q << ", central.application_contact b, central.person c";
  }
  q << " where ";
  if (!empty(i, "type") && i->m["type"]->v == "Public Surveys")
  {
    q << "a.public = 1 and (a.start_date is null or a.start_date <= now()) and (a.start_date is null  or a.end_date >= now())";
  }
  else
  {
    q << "a.application_contact_id = b.id and b.contact_id = c.id and c.userid = '" << d.u << "'";
  }
  q << " order by a.entry_date desc";
  auto g = dbquery("feedback_r", q.str(), e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      stringstream ssDate;
      struct tm tTime;
      time_t CTime;
      Json *j;
      time(&CTime);
      localtime_r(&CTime, &tTime);
      ssDate << put_time(&tTime, "%Y-%m-%d %H:%M");
      r["open"] = (((r["start_date"].empty() || r["start_date"] <= ssDate.str()) && (r["end_date"].empty() || r["end_date"] >= ssDate.str()))?"1":"0");
      j = new Json(r);
      if (!r["application_contact_id"].empty())
      {
        q.str("");
        q << "select b.first_name, b.last_name, b.userid, b.email from application_contact a, person b where a.contact_id = b.id and a.id = " << r["application_contact_id"];
        auto gp = dbquery("central_r", q.str(), e);
        if (gp != NULL && !gp->empty())
        {
          j->i("owner", gp->front());
        }
        dbfree(gp);
      }
      q.str("");
      q << "select count(*) numResults from result where survey_id = " << r["id"];
      auto gr = dbquery("feedback_r", q.str(), e);
      if (gr != NULL && !gr->empty())
      {
        j->i("numResults", gr->front()["numResults"]);
      }
      dbfree(gr);
      o->pb(j);
      delete j;
    }
  }
  dbfree(g);

  return b;
}
// }}}
// {{{ type()
bool Feedback::type(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *i = d.p->m["i"];

  if (!empty(i, "type_id"))
  {
    q << "select id, name from type where id = " << i->m["type_id"]->v;
    auto g = dbquery("feedback_r", q.str(), e);
    if (g != NULL)
    {
      if (!g->empty())
      {
        b = true;
        d.p->i("o", g->front());
      }
      else
      {
        e = "Type does not exist.";
      }
    }
    dbfree(g);
  }
  else
  {
    e = "Please provide the type_id.";
  }

  return b;
}
// }}}
// {{{ types()
bool Feedback::types(radialUser &d, string &e)
{
  bool b = false;
  stringstream q;
  Json *o = d.p->m["o"];

  q << "select id, name from type order by name";
  auto g = dbquery("feedback_r", q.str(), e);
  if (g != NULL)
  {
    b = true;
    for (auto &r : *g)
    {
      o->pb(r);
    }
  }
  dbfree(g);

  return b;
}
// }}}
}
}
