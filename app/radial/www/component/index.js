// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-10-23
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
    let s = c.scope('index',
    {
      // [[[ u()
      u: () =>
      {
        c.update('index');
      },
      // ]]]
      a: a,
      c: c,
      interfaces: null,
      nodes: null
    });
    // ]]]
    // [[[ action()
    s.action = (strAction, strInterface, strNode) =>
    {
      if (c.isLocalAdmin('Radial'))
      {
        let request = {Interface: 'status', 'Function': strAction, Request: {Interface: strInterface}};
        if (strNode != '')
        {
          request.Request.Node = strNode;
        }
        s.info.v = 'Processing request...';
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.info.v = null;
          if (c.wsResponse(response, error))
          {
            if (response.Response && response.Response.Nodes)
            {
              alert('The '+strInterface+' has been '+((strAction == 'restart')?'restarted':'stopped')+((strNode != '')?' on '+strNode:' across all nodes')+'.');
            }
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
      else
      {
        s.message.v = 'You are not authorized to perform this action.';
      }
    };
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      let request = {Interface: 'status', 'Function': 'status'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          if (response.Response && response.Response.Nodes)
          {
            s.org(response.Response);
          }
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ org()
    s.org = (stat) =>
    {
      s.interfaces = null;
      s.interfaces = {};
      s.nodes = null;
      s.nodes = stat;
      for (let n of Object.keys(stat.Nodes))
      {
        for (let i of Object.keys(stat.Nodes[n]))
        {
          if (!c.isDefined(s.interfaces[i]))
          {
            s.interfaces[i] = {};
          }
          s.interfaces[i][n] = stat.Nodes[n][i];
        }
      }
      s.u();
    };
    // ]]]
    // [[[ main
    s.u();
    if (a.ready())
    {
      s.init();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.init();
    });
    c.attachEvent('commonWsMessage_Radial', (data) =>
    {
      if (data.detail && data.detail.Action && data.detail.Action == 'status' && data.detail.Nodes)
      {
        s.org(data.detail);
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
  <div class="row">
  {{#each interfaces}}
  <div class="col">
  <div class="card" style="margin-top: 10px;">
    <div class="card-header bg-info text-white" style="font-weight: bold;">
      <button class="btn btn-sm btn-danger bi bi-x-circle float-end" style="margin-left: 10px;" c-click="action('stop', '{{@key}}', '')" title="stop"></button>
      <button class="btn btn-sm btn-success bi bi-arrow-clockwise float-end" c-click="action('restart', '{{@key}}', '')" title="restart"></button>
      {{@key}}
    </div>
    <div class="card-body">
      <table class="table table-sm table-condensed table-striped">
        <thead>
          <tr>
            <td>Node</td>
            <td>PID</td>
            {{#isLocalAdmin "Radial"}}
            <td colspan="2">Actions</td>
            {{/isLocalAdmin}}
          </tr>
        </thead>
        <tbody>
          {{#each .}}
          <tr>
            <td>{{@key}}</td>
            <td>{{PID}}</td>
            {{#isLocalAdmin "Radial"}}
            <td><button class="btn btn-sm btn-success bi bi-arrow-clockwise" c-click="action('restart', '{{@../key}}', '{{@key}}')" title="restart"></button></td>
            <td><button class="btn btn-sm btn-danger bi bi-x-circle" c-click="action('stop', '{{@../key}}', '{{@key}}')" title="stop"></button></td>
            {{/isLocalAdmin}}
          </tr>
          {{/each}}
        </tbody>
      </table>
    </div>
  </div>
  </div>
  {{/each}}
  </div>
  `
  // ]]]
}
