// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-13
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // {{{ controller()
  controller(id, nav)
  {
    let a = app;
    let c = common;
    let s = c.scope('HomeFrontDoor',
    {
      // {{{ u()
      u: () =>
      {
        c.render(id, 'HomeFrontDoor', this);
      },
      // }}}
      a: a,
      c: c
    });
    c.setMenu('Home', 'FrontDoor');
    // {{{ addIssue()
    s.addIssue = () =>
    {
      s.info = 'Adding issue...';
      s.u();
      let request = {Interface: 'central', 'Function': 'applicationIssueAdd'};
      request.Request = {application_id: s.application.id, application_name: s.application.name, summary: s.issue.summary, due_date: s.issue.due_date, priority: s.issue.priority, assigned_userid: s.issue.assigned_userid, comments: s.issue.comments, server: 'kietzman.org'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.info = 'Redirecting...';
          document.location.href = '#/Applications/' + s.application.id + '/Issues/' + response.Response.id;
        }
        else
        {
          s.info = null;
          s.message = error.message;
        }
        s.u();
      });
    };
    // }}}
    // {{{ loadApplication()
    s.loadApplication = () =>
    {
      if (c.isValid() && s.selectApplication)
      {
        s.info = 'Retrieving application...';
        s.u();
        let request = {Interface: 'central', 'Function': 'application'};
        request.Request = {id: s.selectApplication.v};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.application = null;
          s.issue = null;
          s.info = null;
          if (c.wsResponse(response, error))
          {
            s.application = response.Response;
            s.issue = {priority: '1'};
          }
          else
          {
            s.message = error.message;
          }
          s.u();
        });
      }
      else
      {
        s.application = null;
        s.issue = null;
      }
      s.u();
    };
    // }}}
    // {{{ loadApplications()
    s.loadApplications = () =>
    {
      if (c.isValid())
      {
        s.info = 'Retrieving applications...';
        let request = {Interface: 'central', 'Function': 'applications', Request: {}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.info = null;
            s.applications = response.Response;
            for (let i = 0; i < s.applications.length; i++)
            {
              s.applications[i].display = true;
            }
            s.narrowApplications();
            if (c.isObject(nav.data) && c.isDefined(nav.data.id))
            {
              let bFound = false; 
              for (let i = 0; !bFound && i < s.applications.length; i++)
              { 
                if (s.applications[i].id == nav.data.id)
                { 
                  bFound = true;
                  s.selectApplication = s.applications[i].id;
                }
              }
              if (bFound)
              {
                s.loadApplication();
              }
            }
            else if (c.isObject(nav.data) && c.isDefined(nav.data.application))
            {
              s.info = 'Retrieving application...';
              let request = {Interface: 'central', 'Function': 'application'};
              request.Request = {name: $routeParams.application};
              c.wsRequest('radial', request).then((response) =>
              {
                let error = {};
                s.info = null;
                if (c.wsResponse(response, error))
                {
                  let bFound = false;
                  for (let i = 0; !bFound && i < s.applications.length; i++)
                  {
                    if (s.applications[i].id == response.Response.id)
                    {
                      bFound = true;
                      s.selectApplication = s.applications[i].id;
                    }
                  }
                  if (bFound)
                  {
                    s.loadApplication();
                  }
                }
                else
                {
                  s.message = error.message;
                }
                s.u();
              });
            }
            else
            {
              s.info = 'Please select the appropriate Application.';
            }
          }
          else
          {
            s.message = error.message;
          }
          s.u()
        });
      }
      else
      {
        s.info = 'Please login to create an application issue.';
      }
      s.u();
    };
    // }}}
    // {{{ narrowApplications()
    s.narrowApplications = () =>
    {
      if (c.isObject(s.narrow))
      {
        let selectApplication = document.getElementById("selectApplication");
        while (selectApplication.length > 0)
        {
          selectApplication.remove(0);
        }
        for (let i = 0; i < s.applications.length; i++)
        {
          if (s.narrow.v == '' || s.applications[i].name.search(new RegExp(s.narrow.v, 'i')) != -1)
          {
            let option = document.createElement("option");
            option.text = s.applications[i].name;
            option.value = s.applications[i].id;
            selectApplication.add(option);
          }
        }
      }
    }
    // }}}
    if (a.ready())
    {
      s.loadApplications();
    }
    else
    {
      s.info = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info = null;
      s.u();
      s.loadApplications();
    });
    c.attachEvent('render', (data) =>
    {
      s.narrowApplications();
    });
  },
  // }}}
  // {{{ template
  template: `
  <h3 class="page-header">Front Door - Create an Application Issue</h3>
  {{#if message}}
  <div class="text-danger" style="font-weight:bold;"><br><br>{{message}}<br><br></div>
  {{/if}}
  <p>
    The Front Door provides a comprehensive list of applications from which to choose.  You can use the App field to narrow the list of applications.  The primary/backup developers of the given application will be notified of the newly created issue and will receive a weekly <a href="/central/#/Applications/Workload">Workload</a> reminder of all of their outstanding open issues.
  </p>
  <div class="row" style="margin-bottom: 10px;">
    <div class="col-md-8">
      {{#isValid}}
      <div class="row">
        <div class="col-md-3" style="padding-top: 10px;">
          {{#if ../applications}}
          <div class="input-group"><span class="input-group-text">App</span><input type="text" class="form-control" c-model="narrow" c-keyup="narrowApplications()" placeholder="Narrow"></div>
          <select class="form-control" id="selectApplication" c-model="selectApplication" c-change="loadApplication()" size="2" style="height: 200px;">
            {{#each ../applications}}
            <option value="{{./id}}">{{./name}}</option>
            {{/each}}
          </select>
          {{/if}}
        </div>
        <div class="col-md-9">
          {{#if ../info}}
          <div class="text-warning"><br>{{../info}}<br></div>
          {{/if}}
          {{#if ../application}}
          {{#if ../issue}}
          <div class="card">
            <div class="card-header bg-success text-white" style="font-weight:bold;">
              {{../application.name}}
            </div>
            <div class="card-body">
              <div class="row">
                <div class="col-md-4">
                  <div class="input-group"><span class="input-group-text">Due</span><input type="text" class="form-control" c-model="issue.due_date" placeholder="YYYY-MM-DD"></div>
                </div>
                <div class="col-md-4">
                  <div class="input-group"><span class="input-group-text">Priority</span><select c-model="issue.priority" class="form-control"><option value="1">Low</option><option style="color: orange;" value="2">Medium</option><option style="color: red;" value="3">High</option><option style="background: red; color: white;" value="4">Critical</option></select></div>
                </div>
                <div class="col-md-4">
                  <div class="input-group"><div class="input-group-text">Assigned</div><input type="text" class="form-control" c-model="issue.assigned_userid" placeholder="User ID"></div>
                </div>
              </div>
              <div class="row" style="margin-top: 10px;">
                <div class="col-md-12">
                  <input type="text" class="form-control" c-model="issue.summary" placeholder="enter summary" style="width: 100%;">
                </div>
              </div>
              <div class="row" style="margin-top: 10px;">
                <div class="col-md-12">
                  <textarea c-model="issue.comments" class="form-control" rows="5" style="width: 100%;" placeholder="enter comments"></textarea>
                </div>
              </div>
              <div class="row" style="margin-top: 10px;">
                <div class="col-md-12">
                  <button class="btn btn-success float-end" c-click="addIssue()">Add Issue</button>
                </div>
              </div>
            </div>
          </div>
          {{/if}}
          {{/if}}
        </div>
      </div>
      {{/isValid}}
    </div>
    <div class="col-md-4" style="margin-top: 10px;">
      {{#if application.description}}
      <div class="card card-body card-inverse">
        <small>{{application.description}}</small>
      </div>
      {{/if}}
      <div class="card card-body card-inverse">
        <small class="text-info"><b>Due</b> date is used to set an appropriate date by which the issue should be completed.  This field is optional.</small>
        <br>
        <small class="text-warning"><b>Priority</b> is used to rank the importance of the issue.  This field helps determine the workload of the developer.</small>
      </div>
    </div>
  </div>
  `
  // }}}
}
