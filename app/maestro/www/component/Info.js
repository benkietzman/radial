// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2026-10-16
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
    let s = c.scope('Info',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Info');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'cores'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.cores = response.Response;
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ main
    c.setMenu('Info');
    s.u();
    if (a.ready())
    {
      s.init();
    }
    c.attachEvent('appReady', (data) =>
    {
      s.init();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div class="row">
    <div class="col-4">
      <div class="card card-body bg-primary-subtle border border-primary-subtle">
        Maestro is used by developers to build work flows for their applications.  Maestro provides functional hooks into the application eco-system of Radial.  Developers can create a plan either under an application for which they are registered as a primary/backup developer or they can create a plan under their own account.  Plans created for applications are prefixed with "a_" followed by the application ID as registered in Central.  Plans created for users are prefixed with "u_" followed by the user ID as registered in Central.
      </div>
    </div>
    <div class="col-4">
      <div class="card card-body bg-success-subtle border border-success-subtle">
        A developer can register flows against a plan.  In its essence, a flow is JavaScript.  However, Maestro provides functional hooks into Radial via a core set of custom JavaScript functions.  Some of these core functions can be used without requiring access, others require access due to the sensitive nature of their functionality.  Beyond core functions, the JavaScript logic within a flow can call another flow.  There is no access restriction when one flow calls another flow from within the same plan.  However, access is required when the logic in one flow wants to call a flow from a different plan.
      </div>
    </div>
    <div class="col-4">
      <div class="card card-body bg-warning-subtle border border-warning-subtle">
        Access requests can be submitted as needed.  Access requests for a core function get approved by a local administrator of the Maestro application.  Access requests for a flow whose plan belongs to an application get approved by a primary/backup developer of the application owning the plan to which the requsted flow belongs.  Access requests for a flow whose plan belongs to a user get approved by that user owning the plan to which the requsted flow belongs.
      </div>
    </div>
  </div>
  <h3>Core Functions</h3>
  <div class="card card-body bg-secondary-subtle border border-secondary-subtle">
    NOTE:  Functions containing scopes require an access request.
  </div>
  <div class="table-responsive">
    <table class="table table-sm table-condensed table-striped">
    <thead>
      <tr>
        <th>Function</th>
        <th>Description</th>
        <th>Scopes</th>
      </tr>
    </thead>
    <tbody>
      {{#each cores}}
      <tr>
        <td>{{Function}}</td>
        <td>{{Description}}</td>
        <td>
          <ul style="list-style-type: none;">
            {{#each Scopes}}
            <li>{{.}}</li>
            {{/each}}
          </ul>
        </td>
      </td>
      {{/each}}
    </tbody>
    </table>
  </div>
  `
  // ]]]
}
