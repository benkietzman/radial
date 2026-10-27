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
      expand: {general: true, access: true, flows: true, flow: true, schedules: true, schedule: true, services: true, service: true}
    });
    // ]]]
    // [[[ access
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
    // [[[ accessRequest()
    s.accessRequest = () =>
    {
      if (c.isValid())
      {
        s['accessStatus'].v = '';
        let strAccess = ((s.typeName.v == 'application')?'a_':((s.typeName.v == 'user')?'u_':'')) + s.accessName.v;
        let request = {Interface: 'maestro', 'Function': 'accessRequest', Request: {Plan: s.strPlan, Access: strAccess, Scope: s.scopeName.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s['accessStatus'].v = 'requested access';
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
    // [[[ accessRequestApprove()
    s.accessRequestApprove = (strAccess, strScope) =>
    {
      if (c.isValid())
      {
        s['accessStatus'].v = '';
        let request = {Interface: 'maestro', 'Function': 'accessRequestApprove', Request: {Plan: strAccess, Access: s.strPlan, Scope: strScope}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s['accessStatus'].v = 'approved requested access';
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
    // [[[ accessRequestCancel()
    s.accessRequestCancel = (strAccess, strScope) =>
    {
      if (c.isValid())
      {
        s['accessStatus'].v = '';
        let request = {Interface: 'maestro', 'Function': 'accessRequestCancel', Request: {Plan: s.strPlan, Access: strAccess, Scope: strScope}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s['accessStatus'].v = 'canceled requested access';
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
    // [[[ accessRequestDeny()
    s.accessRequestDeny = (strAccess, strScope) =>
    {
      if (c.isValid())
      {
        s['accessStatus'].v = '';
        let request = {Interface: 'maestro', 'Function': 'accessRequestDeny', Request: {Plan: strAccess, Access: s.strPlan, Scope: strScope}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s['accessStatus'].v = 'deny requested access';
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
            document.location.href = '#/Plans/' + s.strPlan + '/Flow/' + s.flowName.v;
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
        s['error'].v = '';
        s['flowStatus'].v = '';
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
            s['flowStatus'].v = 'executed flow';
            if (response.Response && response.Response)
            {
              s.response.v = JSON.stringify(response.Response);
            }
          }
          else
          {
            s.error.v = error.message;
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
        s['flowStatus'].v = '';
        let request = {Interface: 'maestro', 'Function': 'flowUpdate', Request: {Plan: s.strPlan, Flow: s.strFlow, Description: s.f.Description.v, Script: s.f.Script.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s['flowStatus'].v = 'updated flow';
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
    // [[[ getAccesses()
    s.getAccesses = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'accesses', Request: {Type: c.simplify(s.typeName)}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.access = response.Response;
          if (s.access.length > 0)
          {
            s.accessName = s.access[0];
            s.getScopes();
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
    // [[[ getScopes()
    s.getScopes = () =>
    {
      if (s.access.length > 0)
      {
        let request = {Interface: 'maestro', 'Function': 'scopes', Request: {Type: c.simplify(s.typeName), Access: c.simplify(s.accessName)}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.scopes = response.Response;
            if (s.scopes.length > 0)
            {
              s.scopeName = s.scopes[0];
            }
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
          s.typeName = 'application';
          s.getAccesses();
          s.p = response.Response;
          s.accesses();
          s.flows();
          s.schedules();
          s.services();
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
    // [[[ schedule
    // [[[ schedule()
    s.schedule = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'schedule', Request: {Plan: s.strPlan, Schedule: s.strSchedule}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.s = response.Response;
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ scheduleAdd()
    s.scheduleAdd = () =>
    {
      if (c.isValid())
      {
        let request = {Interface: 'maestro', 'Function': 'scheduleAdd', Request: {Plan: s.strPlan, Schedule: s.scheduleName.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            document.location.href = '#/Plans/' + s.strPlan + '/Schedule/' + s.scheduleName.v;
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
    // [[[ scheduleRemove()
    s.scheduleRemove = () =>
    {
      if (c.isValid())
      {
        let request = {Interface: 'maestro', 'Function': 'scheduleRemove', Request: {Plan: s.strPlan, Schedule: s.strSchedule}};
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
    // [[[ schedules()
    s.schedules = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'schedules', Request: {Plan: s.strPlan}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.ss = response.Response;
          if (s.strSchedule != '')
          {
            s.schedule();
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
    // [[[ scheduleUpdate()
    s.scheduleUpdate = () =>
    {
      if (c.isValid())
      {
        s['scheduleStatus'].v = '';
        let request = {Interface: 'maestro', 'Function': 'scheduleUpdate', Request: {Plan: s.strPlan, Schedule: s.strSchedule, Description: s.s.Description.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s['scheduleStatus'].v = 'updated schedule';
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
    // [[[ service
    // [[[ service()
    s.service = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'service', Request: {Plan: s.strPlan, Service: s.strService}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.d = response.Response;
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ serviceAdd()
    s.serviceAdd = () =>
    {
      if (c.isValid())
      {
        let request = {Interface: 'maestro', 'Function': 'serviceAdd', Request: {Plan: s.strPlan, Service: s.serviceName.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            document.location.href = '#/Plans/' + s.strPlan + '/Service/' + s.serviceName.v;
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
    // [[[ serviceRemove()
    s.serviceRemove = () =>
    {
      if (c.isValid())
      {
        let request = {Interface: 'maestro', 'Function': 'serviceRemove', Request: {Plan: s.strPlan, Service: s.strService}};
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
    // [[[ services()
    s.services = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'services', Request: {Plan: s.strPlan}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.ds = response.Response;
          if (s.strService != '')
          {
            s.service();
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
    // [[[ serviceUpdate()
    s.serviceUpdate = () =>
    {
      if (c.isValid())
      {
        s['serviceStatus'].v = '';
        let request = {Interface: 'maestro', 'Function': 'serviceUpdate', Request: {Plan: s.strPlan, Service: s.strService, Description: s.d.Description.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s['serviceStatus'].v = 'updated service';
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
    s.strPlan = s.strFlow = s.strSchedule = s.strService = '';
    if (c.isParam(nav, 'plan') && c.getParam(nav, 'plan') != '')
    {
      s.strPlan = c.getParam(nav, 'plan');
      if (c.isParam(nav, 'section') && c.getParam(nav, 'section') != '')
      {
        let strSection = c.getParam(nav, 'section');
        if (c.isParam(nav, 'id') && c.getParam(nav, 'id') != '')
        {
          let strID = c.getParam(nav, 'id');
          if (strSection == 'Flow')
          {
            s.strFlow = strID;
          }
          else if (strSection == 'Schedule')
          {
            s.strSchedule = strID;
          }
          else if (strSection == 'Service')
          {
            s.strService = strID;
          }
        }
      }
    }
    s.apps = null;
    s.d = null;
    s.ds = null;
    s.p = null;
    s.ps = null;
    s.f = null;
    s.fs = null;
    s.s = null;
    s.ss = null;
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
      if (data.detail && data.detail.Action && (data.detail.Action == 'accessRemove' || data.detail.Action == 'accessRequest' || data.detail.Action == 'accessRequestApprove' || data.detail.Action == 'accessRequestCancel' || data.detail.Action == 'accessRequestDeny' || data.detail.Action == 'planAdd' || data.detail.Action == 'planRemove' || data.detail.Action == 'flowAdd' || data.detail.Action == 'flowRemove' || data.detail.Action == 'flowUpdate' || data.detail.Action == 'scheduleAdd' || data.detail.Action == 'scheduleRemove' || data.detail.Action == 'scheduleUpdate' || data.detail.Action == 'serviceAdd' || data.detail.Action == 'serviceRemove' || data.detail.Action == 'serviceUpdate') && !s.composition)
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
        <div class="card-header bg bg-secondary fw-bold text-center text-nowrap">
          <div class="row flex-nowrap">
            <div class="col-auto">
              {{#if p.edit}}
              <button class="btn btn-sm btn-danger bi bi-trash" c-click="planRemove()" title="Remove Plan"></button>
              {{/if}}
            </div>
            <div class="col" c-click="collapseExpand('general')" style="cursor: row-resize;">
              Plan - {{strPlan}}
            </div>
            <div class="col-auto" c-click="collapseExpand('general')" style="cursor: row-resize;">
            </div>
          </div>
        </div>
        {{#if expand.general}}
        <div class="card-body bg bg-secondary-subtle">
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
        <div class="card-footer">
        </div>
      </div>
    </div>
    <!-- ]]] -->
    <!-- [[[ access -->
    <div class="col-auto">
      <div class="card border border-danger mb-2 mt-3">
        <div class="card-header bg bg-danger fw-bold text-center text-nowrap" c-click="collapseExpand('access')" style="cursor: row-resize;">
          Access ({{numberShort p.NumAccesses 0}})
        </div>
        {{#if expand.access}}
        <div class="card-body bg bg-danger-subtle">
          {{#isLocalAdmin "Maestro"}}
          <div class="input-group flex-nowrap mt-2">
            <span class="input-group-text bg-secondary border bordrer-secondary">Access</span>
            <select class="form-select bg bg-secondary-subtle border border-danger-subtle" c-model="typeName" c-click="getAccesses()" style="flex-grow: 0; flex-basis: 100px;"><option value="application">application</option><option value="core">core</option><option value="user">user</option></select>
            <select class="form-select bg bg-secondary-subtle border border-danger-subtle" c-model="accessName" c-click="getScopes()" style="flex-grow: 0; flex-basis: 100px;">{{#each ../access}}<option value="{{.}}">{{.}}</option>{{/each}}</select>
            <select class="form-select bg bg-secondary-subtle border border-danger-subtle" c-model="scopeName" style="flex-grow: 0; flex-basis: 100px;">{{#each ../scopes}}<option value="{{.}}">{{.}}</option>{{/each}}</select>
            <button class="btn btn-danger bi bi-send" c-click="accessRequest()" title="Request Access"></button>
          </div>
          {{/isLocalAdmin}}
          {{#if as.a}}
          <div class="table-responsive mt-2">
            <table class="table table-sm table-condensed table-striped" style="--bs-table-bg: transparent;">
            <thead>
              <tr>
                <th>Access</th>
                <th>Scope</th>
              </tr>
            </thead>
            <tbody>
              {{#each as.a}}
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
          {{/if}}
          {{#if as.r}}
          <h5>Pending Requests</h5>
          <div class="table-responsive">
            <table class="table table-sm table-condensed table-striped" style="--bs-table-bg: transparent;">
            <thead>
              <tr>
                <th>Access</th>
                <th>Scope</th>
              </tr>
            </thead>
            <tbody>
              {{#each as.r}}
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
                      <td><button class="btn btn-sm btn-danger bi bi-trash" c-click="accessRequestCancel('{{@../key}}', '{{.}}')" title="Cancel Access Request"></button></td>
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
          {{/if}}
          {{#if as.p}}
          <h5>Pending Approvals</h5>
          <div class="table-responsive">
            <table class="table table-sm table-condensed table-striped" style="--bs-table-bg: transparent;">
            <thead>
              <tr>
                <th>Plan</th>
                <th>Scope</th>
              </tr>
            </thead>
            <tbody>
              {{#each as.p}}
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
                      <td><button class="btn btn-sm btn-danger bi bi-trash" c-click="accessRequestDeny('{{@../key}}', '{{.}}')" title="Deny Access Request"></button></td>
                      <td><button class="btn btn-sm btn-success bi bi-plus-circle" c-click="accessRequestApprove('{{@../key}}', '{{.}}')" title="Approve Access Request"></button></td>
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
          {{/if}}
        </div>
        {{/if}}
        <div class="card-footer">
          <span class="float-end text-warning" c-model="accessStatus"></span>
        </div>
      </div>
    </div>
    <!-- ]]] -->
    <!-- [[[ flows -->
    <div class="col-auto">
      <div class="card border border-success mb-2 mt-3">
        <div class="card-header bg bg-success fw-bold text-center text-nowrap" c-click="collapseExpand('flows')" style="cursor: row-resize;">
          Flows ({{numberShort p.NumFlows 0}})
        </div>
        {{#if expand.flows}}
        <div class="card-body bg bg-success-subtle">
          {{#if p.edit}}
          <div class="input-group flex-nowrap mt-2">
            <span class="input-group-text bg bg-secondary border bordrer-secondary">Flow</span>
            <input type="text" class="form-control bg bg-secondary-subtle border border-success-subtle" c-model="flowName" style="flex-grow: 0; flex-basis: 100px;">
            <button class="btn btn-success bi bi-plus-circle" c-click="flowAdd()" title="Add Flow"></button>
          </div>
          {{/if}}
          <div class="table-responsive mt-2">
            <table class="table table-sm table-condensed table-striped" style="--bs-table-bg: transparent;">
            <tbody>
              {{#each fs}}
              <tr>
                <td><a href="#/Plans/{{@root.strPlan}}/Flow/{{.}}">{{.}}</a></td>
              </tr>
              {{/each}}
            </tbody>
            </table>
          </div>
        </div>
        {{/if}}
        <div class="card-footer">
        </div>
      </div>
    </div>
    <!-- ]]] -->
    <!-- [[[ schedules -->
    <div class="col-auto">
      <div class="card border border-warning mb-2 mt-3">
        <div class="card-header bg bg-warning fw-bold text-dark text-center text-nowrap" c-click="collapseExpand('schedules')" style="cursor: row-resize;">
          Schedules ({{numberShort p.NumSchedules 0}})
        </div>
        {{#if expand.schedules}}
        <div class="card-body bg bg-warning-subtle">
          {{#if p.edit}}
          <div class="input-group flex-nowrap mt-2">
            <span class="input-group-text bg bg-secondary border bordrer-secondary">Schedule</span>
            <input type="text" class="form-control bg bg-secondary-subtle border border-warning-subtle" c-model="scheduleName" style="flex-grow: 0; flex-basis: 100px;">
            <button class="btn btn-warning bi bi-plus-circle" c-click="scheduleAdd()" title="Add Schedule"></button>
          </div>
          {{/if}}
          <div class="table-responsive mt-2">
            <table class="table table-sm table-condensed table-striped" style="--bs-table-bg: transparent;">
            <tbody>
              {{#each ss}}
              <tr>
                <td><a href="#/Plans/{{@root.strPlan}}/Schedule/{{.}}">{{.}}</a></td>
              </tr>
              {{/each}}
            </tbody>
            </table>
          </div>
        </div>
        {{/if}}
        <div class="card-footer">
        </div>
      </div>
    </div>
    <!-- ]]] -->
    <!-- [[[ services -->
    <div class="col-auto">
      <div class="card border border-info mb-2 mt-3">
        <div class="card-header bg bg-info fw-bold text-dark text-center text-nowrap" c-click="collapseExpand('services')" style="cursor: row-resize;">
          Services ({{numberShort p.NumServices 0}})
        </div>
        {{#if expand.services}}
        <div class="card-body bg bg-info-subtle">
          {{#if p.edit}}
          <div class="input-group flex-nowrap mt-2">
            <span class="input-group-text bg bg-secondary border bordrer-secondary">Service</span>
            <input type="text" class="form-control bg bg-secondary-subtle border border-info-subtle" c-model="serviceName" style="flex-grow: 0; flex-basis: 100px;">
            <button class="btn btn-info bi bi-plus-circle" c-click="serviceAdd()" title="Add Servicee"></button>
          </div>
          {{/if}}
          <div class="table-responsive mt-2">
            <table class="table table-sm table-condensed table-striped" style="--bs-table-bg: transparent;">
            <tbody>
              {{#each ds}}
              <tr>
                <td><a href="#/Plans/{{@root.strPlan}}/Service/{{.}}">{{.}}</a></td>
              </tr>
              {{/each}}
            </tbody>
            </table>
          </div>
        </div>
        {{/if}}
        <div class="card-footer">
        </div>
      </div>
    </div>
    <!-- ]]] -->
    <!-- [[[ flow -->
    {{#if f}}
    <div class="col-{{#if expand.flow}}*{{else}}auto{{/if}}">
      <div class="card border border-primary mb-2 mt-3">
        <div class="card-header bg bg-primary fw-bold text-center text-nowrap">
          <div class="row flex-nowrap">
            <div class="col-auto">
              {{#if p.edit}}
              <button class="btn btn-sm btn-danger bi bi-trash" c-click="flowRemove()" title="Remove Flow"></button>
              {{/if}}
            </div>
            <div class="col" c-click="collapseExpand('flow')" style="cursor: row-resize;">
              Flow - {{strFlow}}
            </div>
            <div class="col-auto">
              {{#if p.edit}}
              <button class="btn btn-sm btn-warning bi bi-save" c-click="flowUpdate()" title="Save Flow"></button> 
              {{/if}}
            </div>
          </div>
        </div>
        {{#if expand.flow}}
        <div class="card-body bg bg-primary-subtle">
          {{#if p.edit}}
          <textarea class="form-control bg bg-success-subtle border border-success mt-2" c-model="f\.Description" rows="4" placeholder="description"></textarea>
          <textarea class="form-control mt-2" c-model="f\.Script" style="font-family: monospace;" rows="14" placeholder="javascript"></textarea>
          <textarea class="form-control bg bg-secondary-subtle border border-secondary mt-2" c-model="request" style="font-family: monospace;" rows="4" placeholder="JSON request"></textarea>
          <pre class="mt-2 text-danger" c-model="error" style="white-space: pre-wrap;"></pre>
          <pre class="mt-2" c-model="response" style="white-space: pre-wrap;"></pre>
          {{else}}
          <b>Description<b>
          <pre class="bg bg-success-subtle border border-success mt-2 p-2 rounded" style="white-space: pre-wrap;">{{{f.Description}}}</pre>
          <b>Script</b>
          <pre class="bg bg-secondary-subtle border border-secondary mt-2 p-2 rounded" style="white-space: pre-wrap;">{{f.Script}}</pre>
          {{/if}}
        </div>
        {{/if}}
        <div class="card-footer">
          {{#if p.edit}}
          <button class="btn btn-sm btn-primary bi bi-box-arrow-right" c-click="flowExec()" title="Execute flow"></button>
          <span class="float-end text-warning" c-model="flowStatus"></span>
          {{/if}}
        </div>
      </div>
    </div>
    {{/if}}
    <!-- ]]] -->
    <!-- [[[ schedule -->
    {{#if s}}
    <div class="col-{{#if expand.schedule}}*{{else}}auto{{/if}}">
      <div class="card border border-primary mb-2 mt-3">
        <div class="card-header bg bg-primary fw-bold text-center text-nowrap">
          <div class="row flex-nowrap">
            <div class="col-auto">
              {{#if p.edit}}
              <button class="btn btn-sm btn-danger bi bi-trash" c-click="scheduleRemove()" title="Remove Schedule"></button>
              {{/if}}
            </div>
            <div class="col" c-click="collapseExpand('schedule')" style="cursor: row-resize;">
              Schedule - {{strSchedule}}
            </div>
            <div class="col-auto">
              {{#if p.edit}}
              <button class="btn btn-sm btn-warning bi bi-save" c-click="scheduleUpdate()" title="Save Schedule"></button> 
              {{/if}}
            </div>
          </div>
        </div>
        {{#if expand.schedule}}
        <div class="card-body bg bg-primary-subtle">
          {{#if p.edit}}
          <textarea class="form-control bg bg-success-subtle border border-success mt-2" c-model="s\.Description" rows="4" placeholder="description"></textarea>
          <textarea class="form-control bg bg-secondary-subtle border border-secondary mt-2" c-model="request" style="font-family: monospace;" rows="4" placeholder="JSON request"></textarea>
          <pre class="mt-2 text-danger" c-model="error" style="white-space: pre-wrap;"></pre>
          {{else}}
          <b>Description<b>
          <pre class="bg bg-success-subtle border border-success mt-2 p-2 rounded" style="white-space: pre-wrap;">{{{s.Description}}}</pre>
          {{/if}}
        </div>
        {{/if}}
        <div class="card-footer">
          {{#if p.edit}}
          <span class="float-end text-warning" c-model="scheduleStatus"></span>
          {{/if}}
        </div>
      </div>
    </div>
    {{/if}}
    <!-- ]]] -->
    <!-- [[[ service -->
    {{#if d}}
    <div class="col-{{#if expand.servicee}}*{{else}}auto{{/if}}">
      <div class="card border border-primary mb-2 mt-3">
        <div class="card-header bg bg-primary fw-bold text-center text-nowrap">
          <div class="row flex-nowrap">
            <div class="col-auto">
              {{#if p.edit}}
              <button class="btn btn-sm btn-danger bi bi-trash" c-click="serviceeRemove()" title="Remove Service"></button>
              {{/if}}
            </div>
            <div class="col" c-click="collapseExpand('service')" style="cursor: row-resize;">
              Service - {{strService}}
            </div>
            <div class="col-auto">
              {{#if p.edit}}
              <button class="btn btn-sm btn-warning bi bi-save" c-click="serviceUpdate()" title="Save Service"></button> 
              {{/if}}
            </div>
          </div>
        </div>
        {{#if expand.service}}
        <div class="card-body bg bg-primary-subtle">
          {{#if p.edit}}
          <textarea class="form-control bg bg-success-subtle border border-success mt-2" c-model="d\.Description" rows="4" placeholder="description"></textarea>
          <textarea class="form-control bg bg-secondary-subtle border border-secondary mt-2" c-model="request" style="font-family: monospace;" rows="4" placeholder="JSON request"></textarea>
          <pre class="mt-2 text-danger" c-model="error" style="white-space: pre-wrap;"></pre>
          {{else}}
          <b>Description<b>
          <pre class="bg bg-success-subtle border border-success mt-2 p-2 rounded" style="white-space: pre-wrap;">{{{d.Description}}}</pre>
          {{/if}}
        </div>
        {{/if}}
        <div class="card-footer">
          {{#if p.edit}}
          <span class="float-end text-warning" c-model="serviceStatus"></span>
          {{/if}}
        </div>
      </div>
    </div>
    {{/if}}
    <!-- ]]] -->
  </div>
  <!-- ]]] -->
  <!-- [[[ plans -->
  {{else}}
  {{#isValid}}
  <div class="input-group flex-nowrap">
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
