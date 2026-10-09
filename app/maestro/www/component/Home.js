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
      if (s.list)
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
        }
        s.plans = null;
        s.plans = [];
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
        s.plan = null;
        s.plan = {};
        let request = {Interface: 'maestro', 'Function': 'plan', Request: {Plan: c.getParam(nav, 'plan')}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.plan = response.Response;
            s.flows = null;
            s.flows = {};
            let request = {Interface: 'maestro', 'Function': 'flows', Request: {Plan: c.getParam(nav, 'plan')}};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              if (c.wsResponse(response, error))
              {
                s.flows = response.Response;
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
          s.u();
        });
      }
    };
    // ]]]
    // [[[ planAdd()
    s.planAdd = () =>
    {
      if (c.isValid())
      {
        s.planName = s.planType.v + '_' + ((s.planType.v == 'a')?s.planApplication.v.id:((s.planType.v == 'p')?s.planName.v:s.userid));
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
                document.location.href = '#/Home/' + response.Response.Plan;
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
    // [[[ planRemove()
    s.planRemove = () =>
    {
      if (c.isValid())
      {
        let request = {Interface: 'maestro', 'Function': 'planRemove', Request: {'Plan': s.plan.Plan}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            document.location.href = '#/Home';
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
    s.list = true;
    if (c.isParam(nav, 'plan') && c.getParam(nav, 'plan') != '')
    {
      s.list = false;
    }
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
  {{#if list}}
  {{#isValid}}
  <div class="input-group">
    <span class="input-group-text bg-success-subtle border bordrer-success-subtle">Type</span>
    <select class="form-control bg-success-subtle border border-success-subtle" c-model="planType" c-change="typeSelect()"><option value="a">application</option>{{#isValid "Maestro"}}<option value="p">public</option>{{/isValid}}<option value="u">user</option></select>
    {{#ifCond @root.planType.val "==" "a"}}
    <span class="input-group-text bg-success-subtle border bordrer-success-subtle">Application</span>
    <select class="form-control bg-success-subtle border border-success-subtle" c-model="planApplication" c-json>{{#each @root.applications}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
    {{/ifCond}}
    {{#ifCond @root.planType.val "==" "p"}}
    <span class="input-group-text bg-success-subtle border bordrer-success-subtle">Name</span>
    <input type="text" class="form-control bg-success-subtle border border-success-subtle" c-model="planName">
    {{/ifCond}}
    <button class="btn btn-success bi bi-plus-circle" c-click="planAdd()" title="Add Plan"></button>
  </div>
  {{/isValid}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
    <thead>
      <tr>
        <th>Plan</th>
        <th>Type</th>
        <th>Owner</th>
        <th class="text-end text-nowrap"># Flows</th>
        </tr>
    </thead>
    <tbody>
      {{#each plans}}
      <tr>
        <td><a href="#/Home/{{Plan}}">{{Plan}}</a></td>
        <td>{{#ifCond Type "==" "a"}}application{{else}}{{#ifCond ../Type "==" "p"}}public{{else}}user{{/ifCond}}{{/ifCond}}</td>
        <td>{{#ifCond Type "!=" "p"}}<a href="/central/#/{{#ifCond ../Type "==" "a"}}Applications{{else}}Users{{/ifCond}}/{{../ID}}" target="_blank">{{../Owner}}</a>{{/ifCond}}</td>
        <td class="text-end">{{numberShort NumFlows 0}}</td>
      </tr>
      {{/each}}
    </tbody>
    </table>
  </div>
  {{else}}
  <div class="table-responsive">
  <table class="table table-condensed table-striped">
  <tbody>
    {{#if plan.edit}}
    <tr><th class="text-end" colspan="3"><button class="btn btn-danger bi bi-trash" c-click="planRemove()" title="Remove Plan"></button></tr>
    {{/if}}
    <tr><th>Plan</th><td>{{plan.Plan}}</td></tr>
    <tr><th>Type</th><td>{{#ifCond plan.Type "==" "a"}}application{{else}}{{#ifCond ../plan.Type "==" "p"}}public{{else}}user{{/ifCond}}{{/ifCond}}</td></tr>
    <tr><th>Owner</th><td>{{#ifCond plan.Type "!=" "p"}}<a href="/central/#/{{#ifCond ../plan.Type "==" "a"}}Applications{{else}}Users{{/ifCond}}/{{../plan.ID}}" target="_blank">{{../plan.Owner}}</a>{{/ifCond}}</td>
  </tbody>
  </table>
  </div>
  {{#if plan.edit}}
  <div class="input-group">
    <span class="input-group-text bg-success-subtle border bordrer-success-subtle">Flow</span>
    <input type="text" class="form-control bg-success-subtle border border-success-subtle" c-model="flowName">
    <button class="btn btn-success bi bi-plus-circle" c-click="flowAdd()" title="Add Flow"></button>
  </div>
  {{/if}}
  <div class="row">
    {{#each flows}}
    <div class="col-auto">
      <div class="card border border-secondary-subtle mb-2 mt-3">
        <div class="card-header bg-secondary fw-bold text-center text-nowrap">
          {{@key}}
        </div>
      </div>
    </div>
    {{/each}}
  </div>
  {{/if}}
  `
  // ]]]
}
