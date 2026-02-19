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
    let s = c.scope('ApplicationsWorkload',
    {
      // [[[ u()
      u: () =>
      {
        c.update('ApplicationsWorkload');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ loadIssues()
    s.loadIssues = () =>
    {
      s.info.v = 'Retrieving applications...';
      let request = {Interface: 'database', Database: 'central_r', Query: 'select distinct a.id, a.name from application a, application_contact b, contact_type c, person d where a.id=b.application_id and b.type_id=c.id and b.contact_id=d.id and c.type in (\'Primary Developer\', \'Backup Developer\') and d.userid = \'' + c.esc(c.getUserID()) + '\' order by a.name'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          s.issues = null;
          s.issues = [];
          let ids = '';
          for (let i = 0; i < response.Response.length; i++)
          {
            if (ids != '')
            {
              ids += ',';
            }
            ids += response.Response[i].id;
          }
          s.info.v = 'Retrieving issues...';
          let request = {Interface: 'central', 'Function': 'applicationIssuesByApplicationID', Request: {application_id: ids, open: 1}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              for (let i = 0; i < response.Response.length; i++)
              {
                s.issues.push(response.Response[i]);
                let j = s.issues.length - 1;
                let request = {Interface: 'central', 'Function': 'application', Request: {id: s.issues[j].application_id, i: j}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    let i = response.Request.i;
                    s.issues[i].application = response.Response;
                    s.u();
                  }
                  else
                  {
                    c.pushErrorMessage(error.message);
                  }
                });
                request = null;
                request = {Interface: 'central', 'Function': 'applicationIssueComments', Request: {issue_id: s.issues[j].id, limit: 1, i: j}};
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
          s.u();
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ main
    c.setMenu('Applications', 'Workload');
    s.u();
    if (a.ready())
    {
      s.loadIssues();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.loadIssues();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div class="row table-responsive">
    <h5 class="page-header">Open Application Issues</h5>
    <p>This page provides your personalized workload of open application issues.  The open issues listed below are being pulled from applications for which you are registered as either a primary or backup developer.  The issues are sorted according to priority, due date, and open date.</p>
    <div c-model="info" class="text-warning"></div>
    {{#if issues}}
    <table class="table table-condensed table-striped">
      {{#each issues}}
      <tr>
        <td valign="top">
          <div class="row"><div class="col fw-bold text-nowrap"><a href="#/Applications/{{application.id}}">{{application.name}}</a></div></div>
          <div class="row"><div class="col">Issue #</div><div class="col text-nowrap">{{^if application.id}}<a href="#/Applications/Issues/{{id}}">{{id}}</a>{{else}}<a href="#/Applications/{{application.id}}/Issues/{{id}}">{{id}}</a>{{/if}}{{#ifCond hold "==" 1}}<span style="margin-left: 20px; padding: 0px 2px; background: green; color: white;">HOLD</span>{{/ifCond}}</div></div>
          <div class="row"><div class="col">Open</div><div class="col text-nowrap">{{open_date}}</div></div>
          {{#if due_date}}
          <div class="row"><div class="col">Due</div><div class="col text-nowrap">{{due_date}}</div></div>
          {{/if}}
          {{#if release_date}}
          <div class="row"><div class="col">Release</div><div class="col text-nowrap">{{release_date}}</div></div>
          {{/if}}
          {{#if close_date}}
          <div class="row"><div class="col">Close</div><div class="col text-nowrap">{{close_date}}</div></div>
          {{/if}}
          {{#ifCond priority ">=" 1}}
          <div class="row"><div class="col">Priority</div>{{#ifCond ../priority "==" 1}}<div class="col">Low</div>{{else ifCond ../priority "==" 2}}<div class="col" style="color: orange;">Medium</div>{{else ifCond ../priority "==" 3}}<div class="col" style="color: red;">High</div>{{else}}<div class="col" style="color: white;"><span style="padding: 0px 2px; background: red;">Critical</span></div>{{/ifCond}}</div>
          {{/ifCond}}
          {{#if comments}}
          <div class="row"><div class="col">Requester</div><div class="col text-nowrap"><a href="#/Users/{{comments.[0].user_id}}">{{comments.[0].last_name}}, {{comments.[0].first_name}}</a></div></div>
          {{/if}}
          {{#if assigned}}
          <div class="row"><div class="col">Assigned</div><div class="col text-nowrap"><a href="#/Users/{{assigned.id}}">{{assigned.last_name}}, {{assigned.first_name}}</a></div></div>
          {{/if}}
        </td>
        <td valign="top">
          {{#if summary}}
          <p style="font-weight: bold;">{{summary}}</p>
          {{/if}}
          <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{{comments.[0].comments}}}</pre>
        </td>
      </tr>
      {{/each}}
    </table>
    {{/if}}
  </div>
  `
  // ]]]
}
