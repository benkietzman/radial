// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-08-29
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
    let s = c.scope('ApplicationsRepositories',
    {
      // [[[ u()
      u: () =>
      {
        c.update('ApplicationsRepositories');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ loadApplications()
    s.loadApplications = () =>
    {
      s.info.v = 'Retrieving applications...';
      let request = {Interface: 'database', Database: 'central_r', Query: 'select a.id, a.name, c.repo from application a, application_repo b, repo c where a.id = b.application_id and b.repo_id = c.id and lower(b.identifier) = lower(\''+c.esc(s.identifier.v)+'\') order by a.name'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          s.applications = response.Response;
          for (let i = 0; i < s.applications.length; i++)
          {
            let request = {Interface: 'central', 'Function': 'applicationUsersByApplicationID', Request: {application_id: s.applications[i].id, 'Primary Developer': 1, 'Backup Developer': 1, i: i}};
            c.wsRequest('radial', request).then((response) =>
            {
              var error = {};
              if (c.wsResponse(response, error))
              {
                s.applications[response.Request.i].contacts = response.Response;
                s.u();
              }
              else
              {
                s.message.v = error.message;
              }
            });
          }
          s.u();
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ search()
    s.search = () =>
    {
      s.applications = null;
      s.loadApplications();
    };
    // ]]]
    // [[[ main
    c.setMenu('Applications', 'Repositories');
    s.u();
    if (a.ready())
    {
      if (c.isParam(nav, 'identifier'))
      {
        s.identifier.v = c.getParam(nav, 'identifier');
        s.loadApplications();
      }
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      if (c.isParam(nav, 'user'))
      {
        s.identifier.v = c.getParam(nav, 'identifier');
        s.loadApplications();
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
      <div class="input-group"><span class="input-group-text">Identifier</span><input type="text" c-model="identifier" class="form-control"></div>
      <button class="btn btn-primary float-end" c-click="search()">Search</button>
    </div>
    <div class="col-md-9 table-responsive">
      <h3 class="page-header">Applications</h3>
      <div class="input-group float-end"><span class="input-group-text">Narrow</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow Results"></div>
      <div c-model="info" class="text-warning"></div>
      <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
      {{#if applications}}
      <table class="table table-condensed table-striped">
        <tr>
          <th>Application</th>
          <th>Developer(s)</th>
          <th>Repository</th>
        </tr>
        {{#eachFilter applications "name" narrow}}
        <tr>
          <td valign="top"><a href="#/Applications/{{id}}">{{name}}</a></td>
          <td valign="top">
            <table class="table table-condensed" style="background: inherit;">
              <tr>
                <th class="bg-primary" style="white-space: nowrap;">Primary Developers</th>
              </tr>
              {{#each contacts}}
              {{#ifCond type.type "==" 'Primary Developer'}}
              <tr>
                <td style="white-space: nowrap;"><a href="#/Users/{{../user_id}}">{{../last_name}}, {{../first_name}}</a> <small>({{../userid}})</small></td>
              </tr>
              {{/ifCond}}
              {{/each}}
              <tr>
                <th class="bg-primary" style="white-space: nowrap;">Backup Developers</th>
              </tr>
              {{#each contacts}}
              {{#ifCond type.type "==" 'Backup Developer'}}
              <tr>
                <td style="white-space: nowrap;"><a href="#/Users/{{../user_id}}">{{../last_name}}, {{../first_name}}</a> <small>({{../userid}})</small></td>
              </tr>
              {{/ifCond}}
              {{/each}}
            </table>
          </td>
          <td valign="top">{{repo}}</td>
        </tr>
        {{/eachFilter}}
      </table>
      {{/if}}
    </div>
  </div>
  `
  // ]]]
}
