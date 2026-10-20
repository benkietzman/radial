// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2026-10-20
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
    let s = c.scope('Approvals',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Approvals');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ access
    // [[[ accesses()
    s.accesses = () =>
    {
      if (c.isLocalAdmin('Maestro'))
      {
        let request = {Interface: 'maestro', 'Function': 'accesses'};
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
      }
      else
      {
        c.pushErrorMessage('You are not authorized to perform this action.');
      }
    };
    // ]]]
    // [[[ accessRequestApprove()
    s.accessRequestApprove = (strPlan, strAccess, strScope) =>
    {
      if (c.isLocalAdmin('Maestro'))
      {
        let request = {Interface: 'maestro', 'Function': 'accessRequestApprove', Request: {Plan: strPlan, Access: strAccess, Scope: strScope}};
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
    // [[[ accessRequestDeny()
    s.accessRequestDeny = (strPlan, strAccess, strScope) =>
    {
      if (c.isLocalAdmin('Maestro'))
      {
        let request = {Interface: 'maestro', 'Function': 'accessRequestDeny', Request: {Plan: strPlan, Access: strAccess, Scope: strScope}};
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
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      s.accesses();
    };
    // ]]]
    // [[[ main
    c.setMenu('Approvals');
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
      if (data.detail && data.detail.Action && (data.detail.Action == 'accessRemove' || data.detail.Action == 'accessRequest' || data.detail.Action == 'accessRequestApprove' || data.detail.Action == 'accessRequestCancel' || data.detail.Action == 'accessRequestDeny' || data.detail.Action == 'planAdd' || data.detail.Action == 'planRemove' || data.detail.Action == 'flowAdd' || data.detail.Action == 'flowRemove' || data.detail.Action == 'flowUpdate') && !s.composition)
      {
        s.init();
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  {{#isLocalAdmin "Maestro"}}
  <h5>Pending Approvals</h5>
  <div class="table-responsive">
    <table class="table table-sm table-condensed table-striped">
    <thead>
      <tr>
        <th>Plan</th>
        <th>Core</th>
      </tr>
    </thead>
    <tbody>
      {{#each ../as}}
      <tr>
        <td>
          {{@key}}
        </td>
        <td>
          <table class="table table-sm table-condensed" style="--bs-table-bg: transparent;">
          <tbody>
            {{#each .}}
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
                    <td><button class="btn btn-sm btn-danger bi bi-trash" c-click="accessRequestDeny('{{@../../key}}', '{{@../key}}', '{{.}}')" title="Deny Access Request"></button></td>
                    <td><button class="btn btn-sm btn-success bi bi-plus-circle" c-click="accessRequestApprove('{{@../../key}}', '{{@../key}}', '{{.}}')" title="Approve Access Request"></button></td>
                  </tr>
                  {{/each}}
                </tbody>
                </table>
              </td>
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
  {{/isLocalAdmin}}
  `
  // ]]]
}
