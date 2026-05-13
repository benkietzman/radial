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
    <div class="col-auto fw-bold">
      <b>Search Options</b>
    </div>
    <div class="col">
      <div class="input-group"><span class="input-group-text">Display</span><select c-model="display" c-change="disp()" class="form-control"><option value="all">All Issues</option><option value="open">Open Issues Only</option><option value="closed">Closed Issues Only</option></select></div>
    </div>
  </div>
  <div class="row" style="margin-top: 10px;">
    <div class="col-auto fw-bold text-end">
      Opened
    </div>
    <div class="col">
      <div class="input-group"><span class="input-group-text">Start</span><input type="text" c-model="open_date_start" class="form-control" placeholder="YYYY-MM-DD"></div>
    </div>
    <div class="col">
      <div class="input-group"><span class="input-group-text">End</span><input type="text" c-model="open_date_end" class="form-control" placeholder="YYYY-MM-DD"></div>
    </div>
    {{#if close}}
    <div class="col-auto fw-bold text-end">
      Closed
    </div>
    <div class="col">
      <div class="input-group"><span class="input-group-text">Start</span><input type="text" c-model="close_date_start" class="form-control" placeholder="YYYY-MM-DD"></div>
    </div>
    <div class="col">
      <div class="input-group"><span class="input-group-text">End</span><input type="text" c-model="close_date_end" class="form-control" placeholder="YYYY-MM-DD"></div>
    </div>
    {{/if}}
    <div class="col-auto">
      <button class="btn btn-primary bi bi-search float-end" c-click="search()" title="Search"></button>
    </div>
  </div>
  <div class="row" style="margin-top: 10px;">
    <div class="col">
      <div c-model="info" class="text-warning"></div>
      {{#if issues}}
      {{#each issues}}
      <div class="row">
        <div class="col-md-4" style="margin: 10px 0px;">
          <div class="card-footer border border-secondary-subtle bg-secondary-subtle" style="padding: 10px;">
            <div class="row"><div class="col-md-12 fw-bold text-nowrap"><a href="#/Applications/{{application.id}}">{{application.name}}</a></div></div>
            <div class="row"><div class="col-md-4">Issue #</div><div class="col-md-8 text-nowrap">{{^if application.id}}<a href="#/Applications/Issues/{{id}}">{{id}}</a>{{else}}<a href="#/Applications/{{application.id}}/Issues/{{id}}">{{id}}</a>{{/if}}{{#ifCond hold "==" 1}}<span class="bg-success" style="margin-left: 20px;">HOLD</span>{{/ifCond}}</div></div>
            <div class="row"><div class="col-md-4">Open</div><div class="col-md-8 text-nowrap">{{open_date}}</div></div>
            {{#if due_date}}
            <div class="row"><div class="col-md-4">Due</div><div class="col-md-8 text-nowrap">{{due_date}}</div></div>
            {{/if}}
            {{#if release_date}}
            <div class="row"><div class="col-md-4">Release</div><div class="col-md-8 text-nowrap">{{release_date}}</div></div>
            {{/if}}
            {{#if close_date}}
            <div class="row"><div class="col-md-4">Close</div><div class="col-md-8 text-nowrap">{{close_date}}</div></div>
            {{/if}}
            {{#ifCond priority ">=" 1}}
            <div class="row"><div class="col-md-4">Priority</div>{{#ifCond ../priority "==" 1}}<div class="col-md-8">Low</div>{{else ifCond ../priority "==" 2}}<div class="col-md-8 text-warning">Medium</div>{{else ifCond ../priority "==" 3}}<div class="col-md-8 text-danger">High</div>{{else}}<div class="col-md-8"><span class="bg-danger">Critical</span></div>{{/ifCond}}</div>
            {{/ifCond}}
            {{#if comments}}
            <div class="row"><div class="col-md-4">Requester</div><div class="col-md-8 text-nowrap"><a href="#/Users/{{comments.[0].user_id}}">{{comments.[0].last_name}}, {{comments.[0].first_name}}</a></div></div>
            {{/if}}
            {{#if assigned}}
            <div class="row"><div class="col-md-4">Assigned</div><div class="col-md-8 text-nowrap"><a href="#/Users/{{assigned.id}}">{{assigned.last_name}}, {{assigned.first_name}}</a></div></div>
            {{/if}}
          </div>
        </div>
        <div class="col-md-8">
          <div class="card border border-{{#ifCond ../priority "==" 4}}danger{{else ifCond ../priority "==" 3}}danger{{else ifCond ../priority "==" 2}}warning{{else}}secondary{{/ifCond}}-subtle" style="margin: 10px 0px;">
            <div class="card-header bg-{{#ifCond ../priority "==" 4}}danger{{else}}secondary{{/ifCond}} fw-bold text-{{#ifCond ../priority "==" 3}}danger{{else ifCond ../priority "==" 2}}warning{{else}}light{{/ifCond}}">
              {{#if summary}}
              {{summary}}
              {{/if}}
            </div>
            <div class="card-body bg-{{#ifCond ../priority "==" 4}}danger{{else}}secondary{{/ifCond}}-subtle">
              <pre style="background: inherit; color: inherit; white-space: pre-wrap; word-break: break-word;">{{{comments.[0].comments}}}</pre>
            </div>
          </div>
        </div>
      </div>


      {{/each}}
      {{/if}}
    </div>
  </div>
  `
  // ]]]
}
