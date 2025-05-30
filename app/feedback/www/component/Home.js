// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-08-08
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // [[[ controller()
  controller(id, nav)
  {
    // [[[ prep work
    let a = app;
    let c = common;
    let s = c.scope('Home',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Home');
      },
      // ]]]
      a: a,
      c: c,
      bLoadedSurveys: {'Public Surveys': false, 'Your Surveys': false},
      categories: {},
      survey: {questions: []},
      types: []
    });
    // ]]]
    // [[[ addAnswer()
    s.addAnswer = (nQ) =>
    {
      let answer = {nIndex: s.survey.questions[nQ].answers.length};
      s.survey.questions[nQ].answers.push(answer);
      c.loadModal('Home', 'loadModalSurvey', true);
    };
    // ]]]
    // [[[ addQuestion()
    s.addQuestion = () =>
    {
      let question = {nIndex: s.survey.questions.length, type: s.types[2], required: 0, answers: []};
      s.survey.questions.push(question);
      c.loadModal('Home', 'loadModalSurvey', true);
    };
    // ]]]
    // [[[ getSurvey()
    s.getSurvey = (strAction, strHash) =>
    {
      s.survey = {action: strAction, title: null, info: null, error: null, 'public': 0, anonymous: 0, unique: 0, restrict: 0, questions: [], disabled: true};
      if (strHash != null)
      {
        c.loadModal('Home', 'loadModalSurvey', true);
        s.survey.info.v = 'Fetching survey...';
        let request = {Interface: 'feedback', 'Function': 'survey', Request: {action: strAction, hash: strHash}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.survey.info.v = null;
          if (c.wsResponse(response, error))
          {
            let bUnique = false;
            s.survey = response.Response;
            s.survey.disabled = true;
            s.survey.action = response.Request.action;
            if (s.survey.unique == 1)
            {
              let cookies = [];
              if (c.isCookie('fb_unique'))
              {
                cookies = c.getCookie('fb_unique').split(',');
              }
              bUnique = true;
              for (let i = 0; bUnique && i < cookies.length; i++)
              {
                if (cookies[i] == s.survey.hash)
                {
                  bUnique = false;
                }
              }
            }
            else
            {
              bUnique = true;
            }
            c.loadModal('Home', 'loadModalSurvey', true);
            if (strAction != 'survey' || bUnique)
            {
              s.survey.info.v = 'Fetching questions...';
              var request = {Interface: 'feedback', 'Function': 'questions', Request: {'survey_id': s.survey.id}};
              c.wsRequest('radial', request).then((response) =>
              {
                let error = {};
                s.survey.info.v = null;
                if (c.wsResponse(response, error))
                {
                  s.survey.nQuestionCounter = 0;
                  s.survey.questions = response.Response;
                  if (s.survey.questions.length > 0)
                  {
                    c.loadModal('Home', 'loadModalSurvey', true);
                    for (let i = 0; i < s.survey.questions.length; i++)
                    {
                      s.survey.questions[i].nIndex = i;
                      s.survey.questions[i].info.v = 'Fetching type of answer...';
                      let request = {Interface: 'feedback', 'Function': 'type', Request: {i: i, 'type_id': s.survey.questions[i].type_id}};
                      c.wsRequest('radial', request).then((response) =>
                      {
                        let error = {};
                        if (c.wsResponse(response, error))
                        {
                          let bFoundType = false;
                          let i = response.Request.i;
                          s.survey.questions[i].type = response.Response;
                          for (let j = 0; !bFoundType && j < s.types.length; j++)
                          {
                            if (s.survey.questions[i].type.id == s.types[j].id)
                            {
                              bFoundType = true;
                              s.survey.questions[i].type = s.types[j];
                            }
                          }
                          s.survey.questions[i].info.v = 'Fetching answers...';
                          let request = {Interface: 'feedback', 'Function': 'answers', Request: {i: i, 'question_id': s.survey.questions[i].id}};
                          c.wsRequest('radial', request).then((response) =>
                          {
                            let error = {};
                            if (c.wsResponse(response, error))
                            {
                              let i = response.Request.i;
                              s.survey.questions[i].info.v = null;
                              s.survey.questions[i].answers = response.Response;
                              for (let j = 0; j < s.survey.questions[i].answers.length; j++)
                              {
                                s.survey.questions[i].answers[j].nIndex = j;
                              }
                              s.survey.disabled = ((++s.survey.nQuestionCounter < s.survey.questions.length || s.survey.action == 'results')?true:false);
                              if (s.survey.action == 'results')
                              {
                                c.loadModal('Home', 'loadModalSurvey', true);
                                s.survey.questions[i].info.v = 'Fetching results...';
                                let request = {Interface: 'feedback', 'Function': 'results', Request: {i: i, 'survey_id': s.survey.id, 'question_id': s.survey.questions[i].id, type: s.survey.questions[i].type, answers: s.survey.questions[i].answers}};
                                c.wsRequest('radial', request).then((response) =>
                                {
                                  let error = {};
                                  if (c.wsResponse(response, error))
                                  {
                                    let i = response.Request.i;
                                    s.survey.questions[i].info.v = null;
                                    s.survey.questions[i].results = response.Response;
                                    if (s.survey.questions[i].type.name != 'text')
                                    {
                                      for (let j = 0; j < s.survey.questions[i].results.length; j++)
                                      {
                                        let answers = s.survey.questions[i].results[j].answer.split(',');
                                        s.survey.questions[i].results[j].answer = '';
                                        for (let k = 0; k < answers.length; k++)
                                        {
                                          let bFound = false;
                                          if (k > 0)
                                          {
                                            s.survey.questions[i].results[j].answer += ', ';
                                          }
                                          for (let l = 0; !bFound && l < s.survey.questions[i].answers.length; l++)
                                          {
                                            if (answers[k] == s.survey.questions[i].answers[l].id)
                                            {
                                              bFound = true;
                                              s.survey.questions[i].results[j].answer += s.survey.questions[i].answers[l].answer;
                                            }
                                          }
                                        }
                                      }
                                    }
                                    c.loadModal('Home', 'loadModalSurvey', true);
                                  }
                                  else
                                  {
                                    s.survey.error.v = error.message;
                                  }
                                });
                              }
                              c.loadModal('Home', 'loadModalSurvey', true);
                            }
                            else
                            {
                              s.survey.error.v = error.message;
                            }
                          });
                          c.loadModal('Home', 'loadModalSurvey', true);
                        }
                        else
                        {
                          s.survey.error.v = error.message;
                        }
                      });
                    }
                  }
                  else
                  {
                    s.survey.disabled = false;
                  }
                  c.loadModal('Home', 'loadModalSurvey', true);
                }
                else
                {
                  s.survey.error.v = error.message;
                }
              });
            }
            else
            {
              s.survey.error.v = 'You cannot submit feedback more than once for this survey.';
            }
            c.loadModal('Home', 'loadModalSurvey', true);
          }
          else
          {
            s.survey.error.v = error.message;
          }
        });
      }
      else
      {
        s.survey.anonymous = 1;
        s.survey.disabled = false;
        c.loadModal('Home', 'loadModalSurvey', true);
      }
    };
    // ]]]
    // [[[ getSurveys()
    s.getSurveys = (strType) =>
    {
      if (!c.isDefined(s.categories[strType]))
      {
        s.categories[strType] = {surveys: null, showClosed: false};
        s.u();
      }
      s.categories[strType].info.v = 'Fetching surveys...';
      let request = {Interface: 'feedback', 'Function': 'surveys', Request: {type: strType}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.categories[strType].info.v = null;
        if (c.wsResponse(response, error))
        {
          s.categories[strType].surveys = response.Response;
          for (let i = 0; i < s.categories[strType].surveys.length; i++)
          {
            if (strType == 'Your Surveys' || s.categories[strType].surveys[i].restrict == 0 || (c.isDefined(s.categories[strType].surveys[i].owner) && c.getUserID() == s.categories[strType].surveys[i].owner.userid))
            {
              s.categories[strType].surveys[i].viewResults = true;
            }
          }
          s.u();
          s.bLoadedSurveys[strType] = true;
          c.dispatchEvent('loadedSurveys', null);
        }
        else
        {
          s.categories[strType].message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      let request = {Interface: 'feedback', 'Function': 'types'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.types = response.Response;
          s.loadSurveys();
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ loadSurveys()
    s.loadSurveys = () =>
    {
      if (c.isValid('Feedback'))
      {
        s.getSurveys('Your Surveys');
      }
      else
      {
        s.bLoadedSurveys['Your Surveys'] = true;
      }
      s.getSurveys('Public Surveys');
    }
    // ]]]
    // [[[ removeAnswer()
    s.removeAnswer = (nQ, nA) =>
    {
      s.survey.questions[nQ].answers.splice(nA, 1);
      for (let i = nA; i < s.survey.questions[nQ].answers.length; i++)
      {
        s.survey.questions[nQ].answers[i].nIndex = i;
      }
      c.loadModal('Home', 'loadModalSurvey', true);
    };
    // ]]]
    // [[[ removeQuestion()
    s.removeQuestion = (nQ) =>
    {
      s.survey.questions.splice(nQ, 1);
      for (let i = nQ; i < s.survey.questions.length; i++)
      {
        s.survey.questions[i].nIndex = i;
      }
      c.loadModal('Home', 'loadModalSurvey', true);
    };
    // ]]]
    // [[[ resultAdd()
    s.resultAdd = () =>
    {
      if (c.isDefined(s.survey.questions) && c.isArray(s.survey.questions))
      {
        s.survey.disabled = true;
        c.loadModal('Home', 'loadModalSurvey', true);
        s.survey.error.v = null;
        s.survey.info.v = 'Submitting feedback...';
        let request = {Interface: 'feedback', 'Function': 'resultAdd', Request: {survey: c.simplify(s.survey)}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.survey.info.v = null;
          if (c.wsResponse(response, error))
          {
            s.loadSurveys();
            if (s.survey.unique == 1)
            {
              let cookies = [];
              if (c.isCookie('fb_unique'))
              {
                cookies = c.getCookie('fb_unique').split(',');
              }
              cookies.push(s.survey.hash);
              c.setCookie('fb_unique', cookies.join(','));
            }
            alert("Feedback has been submitted.");
            c.loadModal('Home', 'loadModalSurvey', false);
          }
          else
          {
            s.survey.error.v = error.message;
            s.survey.disabled = false;
            c.loadModal('Home', 'loadModalSurvey', true);
          }
        });
      }
      else
      {
        alert('The questions are being fetched.  Please wait...');
      }
    };
    // ]]]
    // [[[ setClosed()
    s.setClosed = (strCategory, bShowClosed) =>
    {
      if (c.isDefined(s.categories[strCategory]))
      {
        s.categories[strCategory].showClosed = bShowClosed;
        s.u();
      }
    };
    // ]]]
    // [[[ surveyEdit()
    s.surveyEdit = () =>
    {
      s.survey.disabled = true;
      c.loadModal('Home', 'loadModalSurvey', true);
      s.survey.error.v = null;
      s.survey.info.v = 'Saving survey...';
      if (c.isDefined(s.survey.questions) && c.isArray(s.survey.questions))
      {
        let request = {Interface: 'feedback', 'Function': 'surveyEdit', Request: {survey: c.simplify(s.survey)}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.survey.info.v = null;
          if (c.wsResponse(response, error))
          {
            let e = document.querySelector('div.modal-backdrop');
            e.parentNode.removeChild(e);
            document.querySelector('body').style.overflow = 'auto';
            s.loadSurveys();
            if (c.isDefined(response.Response.hash))
            {
              s.getSurvey('edit', response.Response.hash);
              alert("Survey has been saved.");
            }
            else
            {
              alert("Survey has been saved.");
              c.loadModal('Home', 'loadModalSurvey', false);
            }
          }
          else
          {
            s.survey.error.v = error.message;
            s.survey.disabled = false;
          }
          c.loadModal('Home', 'loadModalSurvey', true);
        });
      }
      else
      {
        alert('The questions are being fetched.  Please wait...');
      }
    };
    // ]]]
    // [[[ surveyRemove()
    s.surveyRemove = (nID) =>
    {
      if (confirm('Are you sure you want to remove this survey?'))
      {
        let request = {Interface: 'feedback', 'Function': 'surveyRemove', Request: {id: nID}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.loadSurveys();
            alert("Survey has been removed.");
          }
          else
          {
            s.survey.message.v = error.message;
          }
        });
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Home');
    s.u();
    if (a.ready())
    {
      s.init();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.init();
    });
    c.attachEvent('loadedSurveys', (data) =>
    {
      if (s.bLoadedSurveys['Public Surveys'] && s.bLoadedSurveys['Your Surveys'] && c.isParam(nav, 'hash'))
      {
        let loc = location.hash.split('/');
        if (loc.length >= 3)
        {
          s.getSurvey(loc[1], c.getParam(nav, 'hash'));
        }
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div c-model="info" class="text-warning"></div>
  <!-- [[[ surveys -->
  <div class="row">
    <div class="col-md-8">
      {{#each categories}}
      <button class="btn btn-sm btn-primary bi bi-lightbulb{{#if showClosed}}-off{{/if}} float-end" style="margin-left: 10px;" c-click="setClosed('{{@key}}', {{#if showClosed}}false{{else}}true{{/if}})"> {{#if showClosed}}Hide{{else}}Show{{/if}} Closed</button>
      {{#ifCond @key "==" "Your Surveys"}}
      <button class="btn btn-sm btn-success bi bi-plus-circle float-end" c-click="getSurvey('edit', null)"> Add Survey</button>
      {{/ifCond}}
      <h5 class="page-header">{{@key}}</h5>
      <div c-model="categories.['{{@key}}'].info" class="text-warning"></div>
      <div c-model="categories.['{{@key}}'].message" class="text-danger" style="font-weight:bold;"></div>
      <div class="card-group">
      {{#each surveys}}
      {{#showClosedOrOpen ../showClosed open}}
      <div class="card border border-primary-subtle" style="margin: 10px; min-width: 400px; max-width: 400px;">
        <div class="card-header bg-primary fw-bold">
          <div class="row">
            <div class="col-md-2">
              {{#ifCond @../key "==" "Your Surveys"}}
              <button class="btn btn-sm btn-warning bi bi-pencil" c-click="getSurvey('edit', '{{../../hash}}')" title="Edit"></button>
              {{/ifCond}}
            </div>
            <div class="col-md-8" style="text-align: center;">
              {{../title}}
            </div>
            <div class="col-md-2">
              {{#ifCond @../key "==" "Your Surveys"}}
              <button class="btn btn-sm btn-danger bi bi-trash float-end" c-click="surveyRemove('{{../../id}}')" title="Remove"></button>
              {{/ifCond}}
            </div>
          </div>
        </div>
        <div class="card-body bg-primary-subtle">
          <div class="row">
            <div class="col-md-6 text-center">
              {{#if ../viewResults}}
              <button class="btn btn-sm btn-info bi bi-bar-chart-steps" c-click="getSurvey('results', '{{../hash}}')"> View Results ({{numberShort ../numResults 0}})</button>
              <br>
              <a href="#/results/{{../hash}}">external link</a>
              {{/if}}
            </div>
            <div class="col-md-6 text-center">
              {{#ifCond ../open "==" 1}}
              {{#isValid}}
              <button class="btn btn-sm btn-success bi bi-person-raised-hand" c-click="getSurvey('survey', '{{../../hash}}')"> Take Survey</button>
              <br>
              <a href="#/survey/{{../../hash}}">external link</a>
              {{else}}
              <a class="btn btn-link" href="#/Login"><b>login first</b></a>
              {{/isValid}}
              {{/ifCond}}
            </div>
          </div>
          <div class="row">
            <div class="col-md-12">
              <div class="card card-body text-center" style="margin-top: 20px;">
                <small>hash: {{../hash}}</small>
              </div>
            </div>
          </div>
        </div>
        <div class="card-footer">
          <div class="row">
            <div class="col-md-7">
              <small>{{#ifCond ../open "==" 1}}open {{#haveDate ../../end_date}}until {{../../../end_date}}{{else}}indefinitely{{/haveDate}}{{else}}closed{{#haveDate ../../end_date}} {{../../../end_date}}{{/haveDate}}{{/ifCond}}</small>
            </div>
            <div class="col-md-5" style="text-align: right;">
              {{#ifCond @../key "==" "Public Surveys"}}
              {{#if ../../owner}}
              <small>{{../../owner.first_name}} {{../../owner.last_name}}</small>
              {{/if}}
              {{/ifCond}}
            </div>
          </div>
        </div>
      </div>
      {{/showClosedOrOpen}}
      {{/each}}
      </div>
      {{/each}}
    </div>
    <div class="col-md-4">
      <div class="card border border-info-subtle">
        <div class="card-body bg-info-subtle">
          <p class="text-warning">
            Welcome to the Feedback website.  You can use this website to build custom suveys as well as provide feedback for surveys.
          </p>
          <p class="text-info">
            Surveys can be submitted either on this website or via a conversation with the Radial chatbot.  The Radial chatbot is known as radial_bot and can be accessed either via Internet Relay Chat (IRC) or by logging into the website and clicking the blue chat tab on the right.
          </p>
          <p class="text-info">
            The chatbot responds to keywords called actions.  Provide the feedback (aka: fb) action in order to access Feedback functionalities.  Surveys are uniquely identified using hashes.  Send the following message to the chatbot in order to begin a survey:
          </p>
          <p class="text-success">
            fb [hash]
          </p>
          <p class="text-info">
            Replace [hash] with the hash of the survey.  Provide answers to the survey questions using the following format:
          </p>
          <p class="text-success">
            fb [answer]
          </p>
          <p class="text-info">
            Continue to answer questions until the survey is complete.
          </p>
        </div>
      </div>
    </div>
  </div>
  <!-- ]]] -->
  <!-- [[[ survey -->
  <div id="loadModalSurvey" class="modal modal-lg">
    <div class="modal-dialog modal-dialog-scrollable">
      <div class="modal-content">
        <div class="modal-header">
          {{#ifCond @root.survey.action "==" "edit"}}
          <div class="input-group input-group-sm"><span class="input-group-text">Title</span><input type="text" class="form-control" c-model="survey.title"></div>
          {{else}}
          <b>{{@root.survey.title}}</b>
          {{/ifCond}}
          {{#ifCond @root.survey.anonymous "==" 1}}
          <div onclick="alert('Anonymous Survey')" style="display: inline-block; border-style: solid; border-width: 2px; border-color: green; border-radius: 10px; margin: 0px 10px; padding: 0px 4px; background: #eeeeee; text-align: center; font-size: 11px; font-weight: bold; color: green; cursor: help;">A</div>
          {{/ifCond}}
          {{#ifCond @root.survey.action "==" "results"}}
          <div onclick="alert('Survey Results')" style="display: inline-block; border-style: solid; border-width: 2px; border-color: purple; border-radius: 10px; margin: 0px 10px; padding: 0px 4px; background: #eeeeee; text-align: center; font-size: 11px; font-weight: bold; color: purple; cursor: help;">R</div>
          {{/ifCond}}
          <button type="button" class="btn-close" data-bs-dismiss="modal"></button>
        </div>
        <div class="modal-body">
          <div c-model="survey.info" class="text-warning"></div>
          <div c-model="survey.error" class="text-danger" style="font-weight:bold;"></div>
          {{#ifCond @root.survey.action "==" "edit"}}
          {{#ifCond @root.survey.entry_date "||" @root.survey.modified_date}}
          <div class="row">
            <div class="col-md-6"><div class="input-group input-group-sm"><span class="input-group-text">Created</span><input type="text" class="form-control" c-model="survey.entry_date" disabled></div></div>
            <div class="col-md-6"><div class="input-group input-group-sm"><span class="input-group-text">Modified</span><input type="text" class="form-control" c-model="survey.modified_date" disabled></div></div>
          </div>
          {{/ifCond}}
          <div class="row">
            <div class="col-md-6"><div class="input-group input-group-sm"><span class="input-group-text" onclick="alert('Surveys can be set to be open for input between a specified start and end date/time.')" style="cursor:help;">Start</span><input type="text" class="form-control" c-model="survey.start_date" placeholder="YYYY-MM-DD HH:MM"></div></div>
            <div class="col-md-6"><div class="input-group input-group-sm"><span class="input-group-text" onclick="alert('Surveys can be set to be open for input between a specified start and end date/time.')" style="cursor:help;">End</span><input type="text" class="form-control" c-model="survey.end_date" placeholder="YYYY-MM-DD HH:MM"></div></div>
          </div>
          <div class="row">
            <div class="col-md-6"><div class="input-group input-group-sm"><span class="input-group-text" onclick="alert('Surveys can be registered as public in order to list them in the Public Surveys section of this main page.')" style="cursor:help;">Public</span><select class="form-control" c-model="survey.public"><option value="0">no</option><option value="1">yes</option></select></div></div>
            <div class="col-md-6"><div class="input-group input-group-sm"><span class="input-group-text" onclick="alert('Surveys can be registered as anonymous so that all provided feedback is obtained and stored anonymously.')" style="cursor:help;">Anonymous</span><select class="form-control" c-model="survey.anonymous"><option value="0">no</option><option value="1">yes</option></select></div></div>
          </div>
          <div class="row">
            <div class="col-md-6"><div class="input-group input-group-sm"><span class="input-group-text" onclick="alert('Survey results can only be viewed by the owner of the survey.')" style="cursor:help;">Restrict</span><select class="form-control" c-model="survey.restrict"><option value="0">no</option><option value="1">yes</option></select></div></div>
            <div class="col-md-6"><div class="input-group input-group-sm"><span class="input-group-text" onclick="alert('Surveys can be registered as unique so that multiple entries by the same person are restricted.  Anonymous surveys are restricted using a cookie pushed to the client\'s browser while maintaining user anonymity.  Non-anonymous surveys are restricted via the user ID of the person logged in.')" style="cursor:help;">Unique</span><select class="form-control" c-model="survey.unique"><option value="0">no</option><option value="1">yes</option></select></div></div>
          </div>
          {{else}}
          This survey was created by
          {{#if @root.survey.owner}}
          {{@root.survey.owner.first_name}} {{@root.survey.owner.last_name}}
          {{else}}
          an unknown person
          {{/if}}
          {{#haveDate @root.survey.start_date}}
            {{#ifCond @root.survey.now_date "<" @root.survey.start_date}}
              and opens {{@root.survey.start_date}}
            {{/ifCond}}
          {{/haveDate}}
          {{#haveDate @root.survey.end_date}}
            {{#ifCond @root.survey.now_date "<=" @root.survey.end_date}}
              and expires {{@root.survey.end_date}}
            {{else}}
              and has expired
            {{/ifCond}}
          {{else}}
            and is open indefinitely
          {{/haveDate}}
          .
          {{/ifCond}}
          <ul class="list-group" style="border-style: none;">
            {{#each @root.survey.questions}}
            <li class="list-group-item" style="border-style: none;">
              {{#ifCond @root.survey.action "==" "edit"}}
              <div class="row">
                <div class="col-md-3"><div class="input-group input-group-sm"><span class="input-group-text" onclick="alert('Contains the sequence number which determines the order in which the question appear.')" style="cursor:help;">#</span><input type="text" class="form-control" c-model="survey.questions.[{{@index}}].sequence" placeholder="0"></div></div>
                <div class="col-md-7"><div class="input-group input-group-sm"><span class="input-group-text" onclick="alert('Contains the question.')" style="cursor:help;">?</span><input type="text" class="form-control" c-model="survey.questions.[{{@index}}].question"></div></div>
                <div class="col-md-2"><div class="input-group input-group-sm"><button class="btn btn-sm btn-danger bi bi-trash" c-click="removeQuestion({{@index}})" title="Remove"></button></div></div>
              </div>
              <div class="row">
                <div class="col-md-6"><div class="input-group input-group-sm"><span class="input-group-text">Type</span><select class="form-control" c-model="survey.questions.[{{@index}}].type" c-json>{{#each @root.types}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div></div>
                <div class="col-md-6"><div class="input-group input-group-sm"><span class="input-group-text">Required</span><select class="form-control" c-model="survey.questions.[{{@index}}].required"><option value="0">no</option><option value="1">yes</option></select></div></div>
              </div>
              {{else}}
              {{../question}}{{#ifCond ../required "==" "1"}}<span style="color:red;"> *</span>{{/ifCond}}
              {{/ifCond}}
              <div c-model="survey.questions.[{{@index}}].info" class="text-warning"></div>
              {{#if type}}
              <div style="margin-top: 10px;">
                {{#ifCond @root.survey.action "==" "edit"}}
                <ul class="list-group" style="border-style: none;">
                  {{#each ../answers}}
                  <li class="list-group-item" style="border-style: none;">
                    <div class="row">
                      <div class="col-md-3"><div class="input-group input-group-sm"><span class="input-group-text" onclick="alert('Contains the sequence number which determines the order in which the answers appear.')" style="cursor:help;">#</span><input type="text" class="form-control" c-model="survey.questions.[{{@../index}}].answers.[{{@index}}].sequence" placeholder="0"></div></div>
                      <div class="col-md-7"><div class="input-group input-group-sm"><span class="input-group-text" onclick="alert('Contains the answer.')" style="cursor:help;">&gt;</span><input type="text" class="form-control" c-model="survey.questions.[{{@../index}}].answers.[{{@index}}].answer"></div></div>
                      <div class="col-md-2"><div class="input-group input-group-sm"><button class="btn btn-sm btn-danger bi bi-trash" c-click="removeAnswer({{@../index}}, {{@index}})" title="Remove"></button></div></div>
                    </div>
                  </li>
                  {{/each}}
                  <li class="list-group-item" style="border-style: none;">
                    <button class="btn btn-sm btn-success bi bi-plus-circle" c-click="addAnswer({{@index}})"> Add Answer</button>
                  </li>
                </ul>
                {{else}}
                {{#ifCond ../type.name "==" "checkbox"}}
                {{#each ../answers}}
                <label class="checkbox-inline"><input type="checkbox" c-model="survey.questions.[{{@../index}}].answer" value="{{id}}"{{#if @root.survey.disabled}} disabled{{/if}}> {{answer}}</label>
                {{/each}}
                {{/ifCond}}
                {{#ifCond ../type.name "==" "radio"}}
                {{#each ../answers}}
                <label class="radio-inline"><input type="radio" c-model="survey.questions.[{{@../index}}].answer" value="{{id}}"{{#if @root.survey.disabled}} disabled{{/if}}> {{answer}}</label>
                {{/each}}
                {{/ifCond}}
                {{#ifCond ../type.name "==" "select"}}
                <select class="form-control" c-model="survey.questions.[{{@index}}].answer"{{#if @root.survey.disabled}} disabled{{/if}}>{{#each ../answers}}<option value="{{id}}">{{answer}}</option>{{/each}}</select>
                {{/ifCond}}
                {{#ifCond ../type.name "==" "text"}}
                <textarea class="form-control" c-model="survey.questions.[{{@index}}].answer" cols="60" rows="3"{{#if @root.survey.disabled}} disabled{{/if}}></textarea>
                {{/ifCond}}
                {{#ifCond @root.survey.action "==" "results"}}
                <div class="table-responsive" style="margin-top: 10px;">
                  <table class="table table-condensed table-striped">
                    <tr>
                      <td>Entry Date</td>
                      {{#ifCond @root.survey.anonymous "==" "0"}}
                      <td>Submitter</td>
                      {{/ifCond}}
                      <td>Answer</td>
                    </tr>
                    {{#each ../results}}
                    <tr>
                      <td style="white-space:nowrap;">{{entry_date}}</td>
                      {{#ifCond @root.survey.anonymous "==" "0"}}
                      <td style="white-space:nowrap;">{{#if ../contact}}{{../contact.last_name}}, {{../contact.first_name}} ({{../contact.userid}}){{else}}unknown{{/if}}</td>
                      {{/ifCond}}
                      <td>{{answer}}</td>
                    </tr>
                    {{/each}}
                  </table>
                </div>
                {{/ifCond}}
                {{/ifCond}}
              </div>
              {{/if}}
            </li>
            {{/each}}
            {{#ifCond @root.survey.action "==" "edit"}}
            <li class="list-group-item" style="border-style: none;">
              <button class="btn btn-sm btn-success bi bi-plus-circle" c-click="addQuestion()"> Add Question</button>
            </li>
            {{/ifCond}}
          </ul>
          {{#ifCond @root.survey.action "!=" "edit"}}
          <span style="color:red;">* required</span>
          {{/ifCond}}
          {{#ifCond @root.survey.action "==" "survey"}}
          <button class="float-end btn btn-success bi bi-check-circle" c-click="resultAdd()"{{#if @root.survey.disabled}} disabled{{/if}} title="Submit Feedback"></button>
          <br><br>
          {{/ifCond}}
          {{#ifCond @root.survey.action "==" "edit"}}
          <button class="float-end btn btn-success bi bi-save" c-click="surveyEdit()"{{#if @root.survey.disabled}} disabled{{/if}} title="Save Survey"></button>
          <br><br>
          {{/ifCond}}
        </div>
      </div>
    </div>
  </div>
  <!-- ]]] -->
  `
  // ]]]
}
