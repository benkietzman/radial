// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-25
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
    let s = c.scope('ApplicationsIssues',
    {
      // [[[ u()
      u: () =>
      {
        c.update('ApplicationsIssues');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ loadIssue()
    s.loadIssue = (nID) =>
    {
      s.info.v = 'Retrieving issue...';
      let request = {Interface: 'central', 'Function': 'applicationIssue', Request: {id: nID}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          document.location.href = '#/Applications/' + response.Response.application_id + '/Issues/' + nID;
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ loadIssues()
    s.loadIssues = () =>
    {
      s.info.v = 'Retrieving issues...';
      let request = {Interface: 'central', 'Function': 'applicationIssues', Request: {display: s.display.v, open: s.open, open_date_start: s.open_date_start.v, open_date_end: s.open_date_end.v, close_date_start: s.close_date_start.v, close_date_end: s.close_date_end.v}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          s.issues = response.Response;
          for (let i = 0; i < s.issues.length; i++)
          {
            let request = {Interface: 'central', 'Function': 'application', Request: {id: s.issues[i].application_id, i: i}};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              s.info.v = null;
              if (c.wsResponse(response, error))
              {
                let i = response.Request.i;
                s.issues[i].application = response.Response;
                let request = {Interface: 'central', 'Function': 'applicationUsersByApplicationID', Request: {application_id: s.issues[i].application_id, 'Primary Developer': 1, 'Backup Developer': 1, 'Primary Contact': 1, i: i}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    let i = response.Request.i;
                    s.issues[i].application.contacts = response.Response;
                    s.u();
                  }
                  else
                  {
                    c.pushErrorMessage(error.message);
                  }
                });
                s.u();
              }
              else
              {
                c.pushErrorMessage(error.message);
              }
            });
            request = null;
            request = {Interface: 'central', 'Function': 'applicationIssueComments', Request: {issue_id: s.issues[i].id, limit: 1, i: i}};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              if (c.wsResponse(response, error))
              {
                let i = response.Request.i;
                s.issues[i].comments = response.Response;
                s.u();
              }
              else
              {
                c.pushErrorMessage(error.message);
              }
            });
          }
          s.u();
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ disp()
    s.disp = () =>
    {
      if (s.display.v == "all")
      {
        s.open = 1;
        s.close = 1;
      }
      else if (s.display.v == "closed")
      {
        s.open = 0;
        s.close = 1;
      }
      else if (s.display.v == "open")
      {
        s.open = 1;
        s.close = 0;
        s.close_date_start = new Observable;
        s.close_date_end = new Observable;
      }
      s.u();
    };
    // ]]]
    // [[[ search()
    s.search = () =>
    {
      s.issues = null;
      s.loadIssues();
    };
    // ]]]
    // [[[ main
    c.setMenu('Applications', 'Issues');
    s.u();
    s.display.v = 'open';
    s.disp();
    if (a.ready())
    {
      if (c.isParam(nav, 'id'))
      {
        s.loadIssue(c.getParam(nav, 'id'));
      }
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      if (c.isParam(nav, 'id'))
      {
        s.loadIssue(c.getParam(nav, 'id'));
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div class="row">
    <div class="col-md-3">
      <h5 class="page-header">Search Options</h5>
      <div class="input-group"><span class="input-group-text">Display</span><select c-model="display" c-change="disp()" class="form-control"><option value="all">All Issues</option><option value="open">Open Issues Only</option><option value="closed">Closed Issues Only</option></select></div>
      <div>
        <b>Open Date</b>
        <div class="input-group"><span class="input-group-text">Start</span><input type="text" c-model="open_date_start" class="form-control" placeholder="YYYY-MM-DD"></div>
        <div class="input-group"><span class="input-group-text">End</span><input type="text" c-model="open_date_end" class="form-control" placeholder="YYYY-MM-DD"></div>
      </div>
      {{#if close}}
      <div>
        <b>Close Date</b>
        <div class="input-group"><span class="input-group-text">Start</span><input type="text" c-model="close_date_start" class="form-control" placeholder="YYYY-MM-DD"></div>
        <div class="input-group"><span class="input-group-text">End</span><input type="text" c-model="close_date_end" class="form-control" placeholder="YYYY-MM-DD"></div>
      </div>
      {{/if}}
      <button class="btn btn-primary bi bi-search float-end" c-click="search()" title="Search"></button>
    </div>
    <div class="col-md-9 table-responsive">
      <h3 class="page-header">Issues</h3>
      <div c-model="info" class="text-warning"></div>
      {{#if issues}}
      <table class="table table-condensed table-striped">
        {{#each issues}}
        <tr>
          <td valign="top">
            <table class="table table-condensed" style="background: inherit;">
              <tr><td colspan="2" style="font-weight: bold; white-space: nowrap;"><a href="#/Applications/{{application.id}}">{{application.name}}</a></td></tr>
              <tr><td>Issue #</td><td style="white-space: nowrap;">{{^if application.id}}<a href="#/Applications/Issues/{{id}}">{{id}}</a>{{else}}<a href="#/Applications/{{application.id}}/Issues/{{id}}">{{id}}</a>{{/if}}{{#ifCond hold "==" 1}}<span style="margin-left: 20px; padding: 0px 2px; background: green; color: white;">HOLD</span>{{/ifCond}}</td></tr>
              <tr><td>Open</td><td style="white-space: nowrap;">{{open_date}}</td></tr>
              {{#if due_date}}
              <tr><td>Due</td><td style="white-space: nowrap;">{{due_date}}</td></tr>
              {{/if}}
              {{#if release_date}}
              <tr><td>Release</td><td style="white-space: nowrap;">{{release_date}}</td></tr>
              {{/if}}
              {{#if close_date}}
              <tr><td>Close</td><td style="white-space: nowrap;">{{close_date}}</td></tr>
              {{/if}}
              {{#ifCond priority ">=" 1}}
              <tr><td>Priority</td>{{#ifCond ../priority "==" 1}}<td>Low</td>{{else ifCond ../priority "==" 2}}<td style="color: orange;">Medium</td>{{else ifCond ../priority "==" 3}}<td style="color: red;">High</td>{{else}}<td style="color: white;"><span style="padding: 0px 2px; background: red;">Critical</span></td>{{/ifCond}}</tr>
              {{/ifCond}}
              {{#if comments}}
              <tr><td>Requester</td><td><a href="#/Users/{{comments.[0].user_id}}">{{comments.[0].last_name}}, {{comments.[0].first_name}}</a> <small>({{comments.[0].userid}})</small></td></tr>
              {{/if}}
              {{#if assigned}}
              <tr><td>Assigned</td><td><a href="#/Users/{{assigned.id}}">{{assigned.last_name}}, {{assigned.first_name}}</a> <small>({{assigned.userid}})</small></td></tr>
              {{/if}}
            </table>
          </td>
          <td valign="top">
            {{#if summary}}
            <p style="font-weight: bold;">{{summary}}</p>
            {{/if}}
            <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{comments.[0].comments}}</pre>
          </td>
        </tr>
        {{/each}}
      </table>
      {{/if}}
    </div>
  </div>
  `
  // ]]]
}
