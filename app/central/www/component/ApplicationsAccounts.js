// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-25
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
    let s = c.scope('ApplicationsAccounts',
    {
      // [[[ u()
      u: () =>
      {
        c.update('ApplicationsAccounts');
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
      let request = {Interface: 'database', Database: 'central_r', Query: 'select a.id, a.name, b.description, b.user_id from application a, application_account b where a.id = b.application_id and lower(b.user_id) like lower(\'%'+c.esc(s.user.v)+'%\') order by a.name'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          s.applications = response.Response;
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
    c.setMenu('Applications', 'Accounts');
    s.u();
    if (a.ready())
    {
      if (c.isParam(nav, 'user'))
      {
        s.user.v = c.getParam(nav, 'user');
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
        s.user.v = c.getParam(nav, 'user');
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
      <div class="input-group"><span class="input-group-text">Account</span><input type="text" c-model="user" class="form-control" placeholder="user_id"></div>
      <button class="btn btn-primary bi bi-search float-end" c-click="search()" title="Search"></button>
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
          <th>User ID</th>
          <th>Description</th>
        </tr>
        {{#eachFilter applications "name" narrow}}
        <tr>
          <td valign="top"><a href="#/Applications/{{id}}">{{name}}</a></td>
          <td valign="top">{{user_id}}</td>
          <td valign="top">{{description}}</td>
        </tr>
        {{/eachFilter}}
      </table>
      {{/if}}
    </div>
  </div>
  `
  // ]]]
}
