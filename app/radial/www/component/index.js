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
      bDeveloper: false,
      interfaces: null,
      nodes: null
    });
    // ]]]
    // [[[ action()
    s.action = (strAction, strInterface, strNode) =>
    {
      if (s.bDeveloper)
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
            s.stat();
            alert('The '+strInterface+' has been '+((strAction == 'restart')?'restarted':((strAction == 'start')?'started':'stopped'))+((strNode != '')?' on '+strNode:' across all nodes')+'.');
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
      s.bDeveloper = c.isGlobalAdmin();
      if (s.bDeveloper)
      {
        s.u();
      }
      else if (c.isValid())
      {
        let request = {Interface: 'central', 'Function': 'application', Request: {name: 'Radial'}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            let request = {Interface: 'central', 'Function': 'isApplicationDeveloper', Request: {id: response.Response.id}};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              if (c.wsResponse(response, error))
              {
                s.application.bDeveloper = true;
                s.u();
              }
            });
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
      s.stat();
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
    // [[[ stat()
    s.stat = () =>
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
      {{#if @root.bDeveloper}}
      <button class="btn btn-sm btn-success bi bi-arrow-clockwise float-end" c-click="action('restart', '{{@key}}', '')" title="restart"></button>
      {{/if}}
      {{@key}}
    </div>
    <div class="card-body">
      <table class="table table-sm table-condensed table-striped">
        <thead>
          <tr>
            <td>Node</td>
            <td>PID</td>
            <td>Mem</td>
            {{#if @root.bDeveloper}}
            <td></td>
            {{/if}}
          </tr>
        </thead>
        <tbody>
          {{#each .}}
          <tr>
            <td>{{@key}}</td>
            <td>{{PID}}</td>
            <td>{{byteShort (multiply Memory.Resident 1024) 0}}</td>
            {{#if @root.bDeveloper}}
            <td><button class="btn btn-sm btn-success bi bi-arrow-clockwise" c-click="action('restart', '{{@../key}}', '{{@key}}')" title="restart"></button></td>
            {{/if}}
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
