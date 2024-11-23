// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-12
// copyright  : kietzman.org
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
      c: c
    });
    // ]]]
    // [[[ load()
    let load = () =>
    {
      let request = {Interface: 'central', 'Function': 'applications', Request: {count: 1}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.numApplications = response.Response[0].num;
          s.u()
        }
      });
      request = {Interface: 'central', 'Function': 'servers', Request: {count: 1}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.numServers = response.Response[0].num;
          s.u();
        }
      });
      request = {Interface: 'central', 'Function': 'users', Request: {count: 1}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.numUsers = response.Response[0].num;
          s.u();
        }
      });
      request = {Interface: 'central', 'Function': 'application', Request: {name: 'Central Monitor'}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.centralMonitor = response.Response;
          s.u();
        }
        else if (error.message == 'No results returned.');
        {
          let request = {Interface: 'central', 'Function': 'application', Request: {name: 'System Information'}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            if (c.wsResponse(response, error))
            {
              s.centralMonitor = response.Response;
              s.u();
            }
          });
        }
      });
      request = {Interface: 'central', 'Function': 'homeUsefulTools'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.usefulTools = response.Response;
          s.u();
        }
      });
    };
    // ]]]
    // [[[ main
    c.setMenu('Home');
    s.u();
    if (a.ready())
    {
      load();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      load();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div class="page-header row" style="margin-top: 0px;">
    <div class="col-md-6">
      <h4 style="margin-top: 0px;">Welcome to Central</h4>
    </div>
    <div class="col-md-6">
      <div class="row">
        <div class="col-md-4">
          {{#if numApplications}}
          <div class="card card-body card-inverse" style="text-align: center;">
            <a class="text-warning" href="#/Applications"><b>Applications:</b> {{numberShort numApplications 0}}</a>
          </div>
          {{/if}}
        </div>
        <div class="col-md-4">
          {{#if numServers}}
          <div class="card card-body card-inverse" style="text-align: center;">
            <a class="text-success" href="#/Servers"><b>Servers:</b> {{numberShort numServers 0}}</a>
          </div>
          {{/if}}
        </div>
        <div class="col-md-4">
          {{#if numUsers}}
          <div class="card card-body card-inverse" style="text-align: center;">
            <a class="text-info" href="#/Users"><b>Users:</b> {{numberShort numUsers 0}}</a>
          </div>
          {{/if}}
        </div>
      </div>
    </div>
  </div>
  <div class="row">
    <div class="col-md-7">
      <p>
        This website organizes and tracks <a href="/central/#/Applications">application</a>, <a href="/central/#/Servers">server</a>, and <a href="/central/#/Users">user</a> information.  This website offers many different areas of value for automation teams.  It keeps software engineers and server administrators more efficient by providing a central location to store associated information.
      </p>
      <p>
        Central provides many useful and vital capabilities at a more detailed level.  For instance, Central provides the ability to manage application issues which allows developers the ability to organize and prioritize their workload.  Central provides a front-end to the centralized web-based security modules allowing applications to easily switch between various authentication mechanisms for their websites.  Central has hooks into <a href="/central/#/Applications/{{centralMonitor.id}}">Central Monitor</a> which actively monitors the health of servers as well as the daemonized services of applications.
      </p>
    </div>
    <div class="col-md-5">
      <div class="card">
        <div class="card-header bg-info text-white" style="font-weight: bold;">
          <i class="bi bi-info-circle"></i> Useful Tools
        </div>
        <div class="card-body bg-info-subtle">
          <ul class="list-group">
            {{#each usefulTools}}
            <li class="list-group-item">{{{.}}}</li>
            {{/each}}
          </ul>
        </div>
      </div>
    </div>
  </div>
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
  {{#isValid}}
  <div class="row">
    <div class="col-md-5">
      <div class="card"">
        <div class="card-header bg-success text-white" style="font-weight: bold;">
          <i class="bi bi-ticket"></i> Issues
        </div>
        <div class="card-body bg-success-subtle">
          <p>
            Please use <a href="/central/#/Home/Issues">Issues</a> when creating a new issue for an application.  Issues provides a comprehensive list of applications from which to choose.
          </p>
          <p>
            The primary/backup developers of the given application will be notified of the newly created issue.  The developers will also receive a weekly <a href="/central/#/Applications/Workload">Workload</a> reminder of all outstanding open issues for which they are the primary/backup developers.
          </p>
        </div>
      </div>
    </div>
    <div class="col-md-7">
      <div class="card">
        <div class="card-header bg-warning text-white" style="font-weight: bold;">
          <i class="bi bi-person-workspace"></i> Workload
        </div>
        <div class="card-body bg-warning-subtle">
          <p>
            Developers may access the <a href="/central/#/Applications/Workload">Workload</a> section of Central in order to view their list of open application issues for which they are registered as a primary/backup developer.  The Workload displays issues sorted by priority, due date, and open date.  This helps to keep developers organized in how they tackle their daily issues.
          </p>
          <p>
            Central sends out a weekly Workload email to all developers who have open issues.  The email is sent out every Monday morning and contains a list of open application issues organized similarly to the Workload section of Central.  This helps to keep issues on the radar for developers who may not visit the Workload section of Central regularly.
          </p>
        </div>
      </div>
    </div>
  </div>
  {{/isValid}}
  `
  // ]]]
}
