// -*- C++ -*-
// Radial
// -------------------------------------
// file       : Feedback.cpp
// author     : Ben Kietzman
// begin      : 2023-06-02
// copyright  : kietzman.org
// email      : ben@kietzman.org
/***********************************************************************
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
***********************************************************************/
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
}
// }}}
// {{{ ~Feedback()
Feedback::~Feedback()
{
}
// }}}
// {{{ answers()
bool Feedback::answers(const string strQuestionID, list<map<string, string> > &getAnswer, string &strError)
{
  bool bResult = false;
  Json *ptAnswer = new Json;

  ptAnswer->i("question_id", strQuestionID);
  if (db("dbFeedbackAnswers", ptAnswer, getAnswer, strError))
  {
    bResult = true;
  }
  delete ptAnswer;

  return bResult;
}
// }}}
// {{{ questions()
bool Feedback::questions(const string strSurveyID, list<map<string, string> > &getQuestion, string &strError)
{
  bool bResult = false;
  Json *ptQuestion = new Json;

  ptQuestion->i("survey_id", strSurveyID);
  if (db("dbFeedbackQuestions", ptQuestion, getQuestion, strError))
  {
    bResult = true;
  }
  delete ptQuestion;

  return bResult;
}
// }}}
// {{{ callback()
void Feedback::callback(string strPrefix, const string strPacket, const bool bResponse)
{
  bool bResult = false;
  string strError;
  Json *ptJson;
  radialPacket p;

  threadIncrement();
  strPrefix += "->Feedback::callback()";
  unpack(strPacket, p);
  ptJson = new Json(p.p);
  if (!empty(ptJson, "Function"))
  {
    // {{{ answers
    if (ptJson->m["Function"]->v == "answers")
    {
      if (exist(ptJson, "Request"))
      {
        if (!empty(ptJson->m["Request"], "question_id"))
        {
          list<map<string, string> > getAnswer;
          if (answers(ptJson->m["Request"]->m["question_id"]->v, getAnswer, strError))
          {
            bResult = true;
            ptJson->m["Response"] = new Json;
            for (auto &getAnswerRow : getAnswer)
            {
              ptJson->m["Response"]->pb(getAnswerRow);
            }
          }
        }
        else
        {
          strError = "Please provide the question_id within the Request.";
        }
      }
      else
      {
        strError = "Please provide the Request.";
      }
    }
    // }}}
    // {{{ questions
    else if (ptJson->m["Function"]->v == "questions")
    {
      if (exist(ptJson, "Request"))
      {
        if (!empty(ptJson->m["Request"], "survey_id"))
        {
          list<map<string, string> > getQuestion;
          if (questions(ptJson->m["Request"]->m["survey_id"]->v, getQuestion, strError))
          {
            bResult = true;
            ptJson->m["Response"] = new Json;
            for (auto &getQuestionRow : getQuestion)
            {
              ptJson->m["Response"]->pb(getQuestionRow);
            }
          }
        }
        else
        {
          strError = "Please provide the survey_id within the Request.";
        }
      }
      else
      {
        strError = "Please provide the Request.";
      }
    }
    // }}}
    // {{{ results
    else if (ptJson->m["Function"]->v == "results")
    {
    }
    // }}}
    // {{{ resultAdd
    else if (ptJson->m["Function"]->v == "resultAdd")
    {
    }
    // }}}
    // {{{ survey
    else if (ptJson->m["Function"]->v == "survey")
    {
    }
    // }}}
    // {{{ surveyEdit
    else if (ptJson->m["Function"]->v == "surveyEdit")
    {
    }
    // }}}
    // {{{ surveyRemove
    else if (ptJson->m["Function"]->v == "surveyRemove")
    {
    }
    // }}}
    // {{{ surveys
    else if (ptJson->m["Function"]->v == "surveys")
    {
    }
    // }}}
    // {{{ type
    else if (ptJson->m["Function"]->v == "type")
    {
    }
    // }}}
    // {{{ types
    else if (ptJson->m["Function"]->v == "types")
    {
    }
    // }}}
    // {{{ invalid
    else
    {
      strError = "Please provide a valid Function:  answers, questions, results, resultAdd, survey, surveyEdit, surveyRemove, surveys, type, types.";
    }
    // }}}
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
