// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2026-10-07
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
      c: c
    });
    // ]]]
    // [[[ planAdd()
    s.planAdd = () =>
    {
      if (c.isValid())
      {
        s.plan = null;
        s.plan = {};
        let request = {Interface: 'maestro', 'Function': 'planAdd', Request: {'Plan': s.planName.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            let request = {Interface: 'maestro', 'Function': 'plan', Request: {'Plan': s.planName.v}};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              if (c.wsResponse(response, error))
              {
                s.plan = response.Response;
              }
              else
              {
                c.pushErrorMessage(error.message);
              }
              s.u();
            });
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      else
      {
        c.pushErrorMessage('You are not authorized to perform this action.');
      }
    };
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      if (c.isValid())
      {
        s.plans = null;
        s.plans = {};
        let request = {Interface: 'maestro', 'Function': 'plans'};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.plans = response.Response;
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
          s.u();
        });
      }
      else
      {
        s.u();
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
    c.attachEvent('appReady', (data) =>
    {
      s.init();
    });
    c.attachEvent('commonWsMessage_Maestro', (data) =>
    {
      if (data.detail && data.detail.Action && (data.detail.Action == 'planAdd' || data.detail.Action == 'planRemove') && !s.composition)
      {
        s.init();
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  {{#isValid}}
  {{#if ../plan}}
  {{json ../plan}}
  {{else}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
    <thead>
      <tr><th>Plan</th><th>Type</th><th>Owner</th></tr>
    </thead>
    <tbody>
      <tr>
        <td><input type="text" class="form-conrtol bg-primary-subtle border border-primary-subtle" c-model="planName"></td>
        <td><select class="form-control bg-primary-subtle border border-primary-subtle" c-model="planType"><option value="a">application</option><option value="u">user</option></td>
        <td><button class="btn btn-primary bi bi-plus-circle" c-click="planAdd()" title="Add Plan"></button></td>
      </tr>
      {{#each ../plans}}
      <tr>
        <td>{{@key}}</td>
        <td>{{Type}}</td>
        <td>{{Owner}}</td>
      </tr>
      {{/each}}
    </tbody>
    </table>
  </div>
  {{/if}}
  {{else}}
  <p class="fw-bold text-danger">Please login to use this application.</p>
  {{/isValid}}
  `
  // ]]]
}
