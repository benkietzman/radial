// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-13
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
    let s = c.scope('HomeIssues',
    {
      // [[[ u()
      u: () =>
      {
        c.update('HomeIssues');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ addIssue()
    s.addIssue = () =>
    {
      s.info.v = 'Adding issue...';
      let request = {Interface: 'central', 'Function': 'applicationIssueAdd'};
      request.Request = c.simplify(s.issue);
      request.Request.application_id = s.application.id;
      request.Request.application_name = s.application.name;
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.info.v = 'Redirecting...';
          document.location.href = '#/Applications/' + s.application.id + '/Issues/' + response.Response.id;
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ loadApplication()
    s.loadApplication = () =>
    {
      if (c.isValid() && s.selectApplication)
      {
        s.info.v = 'Retrieving application...';
        let request = {Interface: 'central', 'Function': 'application'};
        request.Request = {id: s.selectApplication.v};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.application = null;
          s.issue = null;
          s.info.v = null;
          if (c.wsResponse(response, error))
          {
            s.application = response.Response;
            s.issue = {priority: '1'};
            s.u();
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      else
      {
        s.application = null;
        s.issue = null;
      }
    };
    // ]]]
    // [[[ loadApplications()
    s.loadApplications = () =>
    {
      if (c.isValid())
      {
        s.info.v = 'Retrieving applications...';
        let request = {Interface: 'central', 'Function': 'applications', Request: {retired: false}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.info.v = null;
            s.applications = response.Response;
            for (let i = 0; i < s.applications.length; i++)
            {
              s.applications[i].display = true;
            }
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
              s.info.v = 'Retrieving application...';
              let request = {Interface: 'central', 'Function': 'application'};
              request.Request = {name: $routeParams.application};
              c.wsRequest('radial', request).then((response) =>
              {
                let error = {};
                s.info.v = null;
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
                  c.pushErrorMessage(error.message);
                }
              });
            }
            else
            {
              s.info.v = 'Please select the appropriate Application.';
            }
            s.u();
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      else
      {
        s.info.v = 'Please login to create an application issue.';
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Home', 'Issues');
    s.u();
    if (c.isParam(nav, 'id'))
    {
      document.location.href = '#/Applications/Issues/' + c.getParam(nav, 'id');
    }
    if (a.ready())
    {
      s.loadApplications();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.loadApplications();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <h3 class="page-header">Create an Application Issue</h3>
  <p>
    This provides a comprehensive list of applications from which to choose.  You can use the App field to narrow the list of applications.  The primary/backup developers of the given application will be notified of the newly created issue and will receive a weekly <a href="/central/#/Applications/Workload">Workload</a> reminder of all of their outstanding open issues.
  </p>
  <div class="row" style="margin-bottom: 10px;">
    <div class="col-md-8">
      <div c-model="info" class="text-warning"></div>
      {{#isValid}}
      <div class="row">
        <div class="col-md-3" style="padding-top: 10px;">
          {{#if ../applications}}
          <div class="input-group"><span class="input-group-text">App</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow"></div>
          <select class="form-control" id="selectApplication" c-model="selectApplication" c-change="loadApplication()" size="2" style="height: 200px;">
            {{#eachFilter ../applications 'name' ../narrow}}
            <option value="{{./id}}"{{#ifCond ../selectApplication '==' ./id}} selected{{/ifCond}}>{{./name}}</option>
            {{/eachFilter}}
          </select>
          {{/if}}
        </div>
        <div class="col-md-9">
          {{#if ../application}}
          {{#if ../issue}}
          <div class="card border border-danger-subtle">
            <div class="card-header bg-danger fw-bold"">
              {{../application.name}}
            </div>
            <div class="card-body bg-danger-subtle">
              <div class="row">
                <div class="col-md-4">
                  <div class="input-group"><span class="input-group-text bg-danger">Due</span><input type="text" class="form-control bg-danger-subtle" c-model="issue.due_date" placeholder="YYYY-MM-DD"></div>
                </div>
                <div class="col-md-4">
                  <div class="input-group"><span class="input-group-text bg-danger">Priority</span><select c-model="issue.priority" class="form-control bg-danger-subtle"><option value="1">Low</option><option style="color: orange;" value="2">Medium</option><option style="color: red;" value="3">High</option><option style="background: red; color: white;" value="4">Critical</option></select></div>
                </div>
                <div class="col-md-4">
                  <div class="input-group"><span class="input-group-text bg-danger">Assigned</span><input type="text" class="form-control bg-danger-subtle" c-model="issue.assigned_userid" placeholder="User ID"></div>
                </div>
              </div>
              <div class="row" style="margin-top: 10px;">
                <div class="col-md-12">
                  <input type="text" class="form-control bg-danger-subtle" c-model="issue.summary" placeholder="enter summary" style="width: 100%;">
                </div>
              </div>
              <div class="row" style="margin-top: 10px;">
                <div class="col-md-12">
                  <textarea c-model="issue.comments" class="form-control bg-danger-subtle" rows="5" style="width: 100%;" placeholder="enter comments"></textarea>
                </div>
              </div>
              <div class="row" style="margin-top: 10px;">
                <div class="col-md-12">
                  <button class="btn btn-danger bi bi-check-circle float-end" c-click="addIssue()" title="Create Issue"></button>
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
  // ]]]
}
