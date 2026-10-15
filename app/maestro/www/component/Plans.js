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
    let s = c.scope('Plans',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Plans');
      },
      // ]]]
      a: a,
      c: c,
      expand: {general: true, access: true, flows: true, flow: true}
    });
    // ]]]
    // [[[ access
    // [[[ accessAdd()
    s.accessAdd = () =>
    {
      if (c.isLocalAdmin('Maestro'))
      {
        let request = {Interface: 'maestro', 'Function': 'accessAdd', Request: {Plan: s.strPlan, Access: s.accessName.v, Scope: s.scopeName.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
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
    // [[[ accesses()
    s.accesses = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'accesses', Request: {Plan: s.strPlan}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s['as'] = response.Response;
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ accessRemove()
    s.accessRemove = (strAccess, strScope) =>
    {
      if (c.isValid())
      {
        let request = {Interface: 'maestro', 'Function': 'accessRemove', Request: {Plan: s.strPlan, Access: strAccess, Scope: strScope}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            document.location.href = '#/Plans/' + s.strPlan + '/' + s.strFlow;
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
    // ]]]
    // [[[ applications()
    s.applications = () =>
    {
      let request = {Interface: 'database', Database: 'central_r', Query: 'select distinct a.id, a.name from application a, application_contact b, contact_type c, person d where a.id = b.application_id and b.type_id = c.id and b.contact_id = d.id and c.type in (\'Primary Developer\', \'Backup Developer\') and d.userid = \'' + c.getUserID() + '\' order by a.name'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.apps = response.Response;
          s.planApplication = s.apps[0];
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ collapseExpand()
    s.collapseExpand = (strKey) =>
    {
      s.expand[strKey] = ((s.expand[strKey])?false:true);
      s.u();
    };
    // ]]]
    // [[[ flow
    // [[[ flow()
    s.flow = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'flow', Request: {Plan: s.strPlan, Flow: s.strFlow}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s['status'] = null;
          s.f = response.Response;
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ flowAdd()
    s.flowAdd = () =>
    {
      if (c.isValid())
      {
        let request = {Interface: 'maestro', 'Function': 'flowAdd', Request: {Plan: s.strPlan, Flow: s.flowName.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            document.location.href = '#/Plans/' + s.strPlan + '/' + s.flowName.v;
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
    // [[[ flowExec()
    s.flowExec = () =>
    {
      if (c.isValid())
      {
        s.response.v = '';
        let request = {Interface: 'maestro', 'Function': 'flowExec', Request: {Plan: s.strPlan, Flow: s.strFlow}};
        if (s.request.v)
        {
          request.Request.Request = JSON.parse(s.request.v);
        }
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s['status'].v = 'executed flow';
            if (response.Response && response.Response)
            {
              s.response.v = JSON.stringify(response.Response);
            }
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
    // [[[ flowRemove()
    s.flowRemove = () =>
    {
      if (c.isValid())
      {
        let request = {Interface: 'maestro', 'Function': 'flowRemove', Request: {Plan: s.strPlan, Flow: s.strFlow}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            document.location.href = '#/Plans/' + s.strPlan;
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
    // [[[ flows()
    s.flows = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'flows', Request: {Plan: s.strPlan}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.fs = response.Response;
          if (s.strFlow != '')
          {
            s.flow();
          }
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ flowUpdate()
    s.flowUpdate = () =>
    {
      if (c.isValid())
      {
        let request = {Interface: 'maestro', 'Function': 'flowUpdate', Request: {Plan: s.strPlan, Flow: s.strFlow, Script: s.f.Script.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s['status'].v = 'updated flow';
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
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      s.response = '';
      if (s.strPlan != '')
      {
        s.plan();
      }
      else
      {
        if (c.isValid())
        {
          s.planType = 'a';
          s.user();
          s.applications();
        }
        s.plans();
      }
    };
    // ]]]
    // [[[ plan
    // [[[ plan()
    s.plan = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'plan', Request: {Plan: s.strPlan}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.p = response.Response;
          s.accesses();
          s.flows();
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ planAdd()
    s.planAdd = () =>
    {
      if (c.isValid())
      {
        s.planName = s.planType.v + '_' + ((s.planType.v == 'a')?s.planApplication.v.id:((s.planType.v == 'p')?s.planName.v:s.userid));
        let request = {Interface: 'maestro', 'Function': 'planAdd', Request: {Plan: s.planName}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            let request = {Interface: 'maestro', 'Function': 'plan', Request: {Plan: s.planName}};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              if (c.wsResponse(response, error))
              {
                document.location.href = '#/Plans/' + response.Response.Plan;
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
        let request = {Interface: 'maestro', 'Function': 'planRemove', Request: {Plan: s.strPlan}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            document.location.href = '#/Plans';
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
    // [[[ plans()
    s.plans = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'plans'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.ps = response.Response;
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // ]]]
    // [[[ typeSelect()
    s.typeSelect = () =>
    {
      s.u();
    };
    // ]]]
    // [[[ user()
    s.user = () =>
    {
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
    };
    // ]]]
    // [[[ main
    c.setMenu('Plans');
    s.strPlan = s.strFlow = '';
    if (c.isParam(nav, 'plan') && c.getParam(nav, 'plan') != '')
    {
      s.strPlan = c.getParam(nav, 'plan');
      if (c.isParam(nav, 'flow') && c.getParam(nav, 'flow') != '')
      {
        s.strFlow = c.getParam(nav, 'flow');
      }
    }
    s.apps = null;
    s.p = null;
    s.ps = null;
    s.f = null;
    s.fs = null;
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
      if (data.detail && data.detail.Action && (data.detail.Action == 'accessAdd' || data.detail.Action == 'accessRemove' || data.detail.Action == 'planAdd' || data.detail.Action == 'planRemove' || data.detail.Action == 'flowAdd' || data.detail.Action == 'flowRemove' || data.detail.Action == 'flowUpdate') && !s.composition)
      {
        s.init();
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <!-- [[[ plan -->
  {{#if p}}
  <div class="row">
    <!-- [[[ general -->
    <div class="col-auto">
      <div class="card border border-secondary mb-2 mt-3">
        <div class="card-header bg bg-secondary fw-bold text-center text-nowrap" c-click="collapseExpand('general')" style="cursor: row-resize;">
          Plan - {{strPlan}}
        </div>
        {{#if expand.general}}
        <div class="card-body bg bg-secondary-subtle">
          <div class="input-group">
            {{#if p.edit}}
            <button class="btn btn-danger bi bi-trash" c-click="planRemove()" title="Remove Plan"></button>
            {{/if}}
            <span class="input-group-text bg-secondary border border-secondary">Plan</span>
            <span class="input-group-text bg-secondary-subtle border border-secondary">{{p.Plan}}</span>
          </div>
          <div class="table-responsive mt-2">
            <table class="table table-sm table-condensed table-striped" style="--bs-table-bg: transparent;">
            <tbody>
              <tr><th>Type</th><td colspan="2">{{#ifCond p.Type "==" "a"}}application{{else}}user{{/ifCond}}</td></tr>
              <tr><th>Owner</th><td colspan="2"><a href="/central/#/{{#ifCond p.Type "==" "a"}}Applications{{else}}Users{{/ifCond}}/{{p.ID}}" target="_blank">{{p.Owner}}</a></td>
            </tbody>
            </table>
          </div>
        </div>
        {{/if}}
      </div>
    </div>
    <!-- ]]] -->
    <!-- [[[ access -->
    <div class="col-auto">
      <div class="card border border-warning mb-2 mt-3">
        <div class="card-header bg bg-warning fw-bold text-dark text-center text-nowrap" c-click="collapseExpand('access')" style="cursor: row-resize;">
          Access
        </div>
        {{#if expand.access}}
        <div class="card-body bg bg-warning-subtle">
          {{#isLocalAdmin "Maestro"}}
          <div class="input-group mt-2">
            <span class="input-group-text bg-secondary border bordrer-secondary">Access</span>
            <input type="text" class="form-control bg-secondary-subtle border border-warning-subtle" c-model="accessName" style="flex-grow: 0; flex-basis: 100px;" placeholder="access">
            <input type="text" class="form-control bg-secondary-subtle border border-warning-subtle" c-model="scopeName" style="flex-grow: 0; flex-basis: 100px;" placeholder="scope">
            <button class="btn btn-success bi bi-plus-circle" c-click="accessAdd()" title="Add Access"></button>
          </div>
          {{/isLocalAdmin}}
          <div class="table-responsive mt-2">
            <table class="table table-sm table-condensed table-striped" style="--bs-table-bg: transparent;">
            <thead>
              <tr>
                <th>Access</th>
                <th>Scope</th>
              </tr>
            </thead>
            <tbody>
              {{#each as}}
              <tr>
                <td>
                  {{@key}}
                </td>
                <td>
                  <table class="table table-sm table-condensed" style="--bs-table-bg: transparent;">
                  <tbody>
                    {{#each .}}
                    <tr>
                      <td class="w-100">{{.}}</td>
                      {{#if @root.p.edit}}
                      <td><button class="btn btn-sm btn-danger bi bi-trash" c-click="accessRemove('{{@../key}}', '{{.}}')" title="Remove Access"></button></td>
                      {{/if}}
                    </tr>
                    {{/each}}
                  </tbody>
                  </table>
                </td>
              </tr>
              {{/each}}
            </tbody>
            </table>
          </div>
        </div>
        {{/if}}
      </div>
    </div>
    <!-- ]]] -->
    <!-- [[[ flows -->
    <div class="col-auto">
      <div class="card border border-success mb-2 mt-3">
        <div class="card-header bg bg-success fw-bold text-center text-nowrap" c-click="collapseExpand('flows')" style="cursor: row-resize;">
          Flows
        </div>
        {{#if expand.flows}}
        <div class="card-body bg bg-success-subtle">
          {{#if p.edit}}
          <div class="input-group mt-2">
            <span class="input-group-text bg-secondary border bordrer-secondary">Flow</span>
            <input type="text" class="form-control bg-secondary-subtle border border-success-subtle" c-model="flowName" style="flex-grow: 0; flex-basis: 100px;">
            <button class="btn btn-success bi bi-plus-circle" c-click="flowAdd()" title="Add Flow"></button>
          </div>
          {{/if}}
          <div class="table-responsive mt-2">
            <table class="table table-sm table-condensed table-striped" style="--bs-table-bg: transparent;">
            <tbody>
              {{#each fs}}
              <tr>
                <td><a href="#/Plans/{{Plan}}/{{Flow}}">{{Flow}}</a></td>
              </tr>
              {{/each}}
            </tbody>
            </table>
          </div>
        </div>
        {{/if}}
      </div>
    </div>
    <!-- ]]] -->
    <!-- [[[ flow -->
    {{#if f}}
    <div class="col-*">
      <div class="card border border-primary mb-2 mt-3">
        <div class="card-header bg bg-primary fw-bold text-center text-nowrap" c-click="collapseExpand('flow')" style="cursor: row-resize;">
          Flow - {{strFlow}}
        </div>
        {{#if expand.flow}}
        <div class="card-body bg bg-primary-subtle">
          <div class="input-group">
            {{#if p.edit}}
            <button class="btn btn-danger bi bi-trash" c-click="flowRemove()" title="Remove Flow"></button>
            {{/if}}
            <span class="input-group-text bg-secondary border border-secondary">Flow</span>
            <span class="input-group-text bg-primary-subtle border border-primary">{{f.Flow}}</span>
            <button class="btn btn-success bi bi-save" c-click="flowUpdate()" title="Save Script"></button> 
            <button class="btn btn-warning bi bi-box-arrow-right" c-click="flowExec()" title="Execute flow"></button>
            <span class="input-group-text bg-secondary-subtle border border-secondary text-warning" c-model="status"></span>
          </div>
          <textarea class="form-control bg-secondary-subtle border border-secondary mt-2" c-model="request" style="font-family: monospace;" rows="4" placeholder="JSON request"></textarea>
          <textarea class="form-control mt-2" c-model="f\.Script" style="font-family: monospace;" rows="14" placeholder="javascript"></textarea>
          <pre class="mt-2" c-model="response" style="background: inherit; color: inherit; white-space: pre-wrap;"></pre>
        </div>
        {{/if}}
      </div>
    </div>
    {{/if}}
    <!-- ]]] -->
  </div>
  <!-- ]]] -->
  <!-- [[[ plans -->
  {{else}}
  {{#isValid}}
  <div class="input-group">
    <span class="input-group-text bg-secondary border bordrer-secondary">Type</span>
    <select class="form-control bg-success-subtle border border-success-subtle" c-model="planType" c-change="typeSelect()" style="flex-grow: 0; flex-basis: 120px;"><option value="a">application</option><option value="u">user</option></select>
    {{#ifCond @root.planType.val "==" "a"}}
    <span class="input-group-text bg-secondary border bordrer-secondary">Application</span>
    <select class="form-control bg-success-subtle border border-success-subtle" c-model="planApplication" style="flex-grow: 0; flex-basis: 200px;" c-json>{{#each @root.apps}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
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
        <th class="text-end text-nowrap"># Accesses</th>
        <th class="text-end text-nowrap"># Flows</th>
      </tr>
    </thead>
    <tbody>
      {{#each ps}}
      <tr>
        <td><a href="#/Plans/{{Plan}}">{{Plan}}</a></td>
        <td>{{#ifCond Type "==" "a"}}application{{else}}{{#ifCond ../Type "==" "p"}}public{{else}}user{{/ifCond}}{{/ifCond}}</td>
        <td>{{#ifCond Type "!=" "p"}}<a href="/central/#/{{#ifCond ../Type "==" "a"}}Applications{{else}}Users{{/ifCond}}/{{../ID}}" target="_blank">{{../Owner}}</a>{{/ifCond}}</td>
        <td class="text-end">{{numberShort NumAccesses 0}}</td>
        <td class="text-end">{{numberShort NumFlows 0}}</td>
      </tr>
      {{/each}}
    </tbody>
    </table>
  </div>
  {{/if}}
  <!-- ]]] -->
  `
  // ]]]
}
