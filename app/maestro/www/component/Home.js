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
    // [[[ init()
    s.init = () =>
    {
      if (c.isValid())
      {
        s.planType = 'a';
        let request = {Interface: 'central', 'Function': 'user', Request: {userid: c.getUserID()}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.userid = response.Response.id;
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
          s.u();
        });
        s.applications = null;
        s.applications = [];
        request = {Interface: 'database', Database: 'central_r', Query: 'select distinct a.id, a.name from application a, application_contact b, contact_type c, person d where a.id = b.application_id and b.type_id = c.id and b.contact_id = d.id and c.type in (\'Primary Developer\', \'Backup Developer\') and d.userid = \'' + c.getUserID() + '\' order by a.name'};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.applications = response.Response;
            s.planApplication = s.applications[0];
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
          s.u();
        });
        s.plans = null;
        s.plans = [];
        request = {Interface: 'maestro', 'Function': 'plans'};
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
    // [[[ planAdd()
    s.planAdd = () =>
    {
      if (c.isValid())
      {
        s.planName = s.planType.v + '_' + ((s.planType.v == 'a')?s.planApplication.v.id:s.userid);
        s.plan = null;
        s.plan = {};
        let request = {Interface: 'maestro', 'Function': 'planAdd', Request: {'Plan': s.planName}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            let request = {Interface: 'maestro', 'Function': 'plan', Request: {'Plan': s.planName}};
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
    // [[[ typeSelect()
    s.typeSelect = () =>
    {
      s.u();
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
  <div class="input-group">
    <span class="input-group-text bg-success-subtle border bordrer-success-subtle">Type</span>
    <select class="form-control bg-success-subtle border border-success-subtle" c-model="planType" c-change="typeSelect()"><option value="a">application</option><option value="u">user</option></select>
    {{#ifCond @root.planType.val "==" "a"}}
    <span class="input-group-text bg-success-subtle border bordrer-success-subtle">Application</span>
    <select class="form-control bg-success-subtle border border-success-subtle" c-model="planApplication" c-json>{{#each @root.applications}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
    {{/ifCond}}
    <button class="btn btn-success bi bi-plus-circle" c-click="planAdd()" title="Add Plan"></button>
  </div>
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
    <thead>
      <tr>
        <th>Plan</th>
        <th>Type</th>
        <th class="text-end text-nowrap"># Flows</th>
        </tr>
    </thead>
    <tbody>
      {{#each ../plans}}
      <tr>
        <td><a href="#/{{Name}}">{{Owner}}</a></td>
        <td>{{Type}}</td>
        <td class="text-end">{{numberShort NumFlows 0}}</td>
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
