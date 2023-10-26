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
          if (c.isDefined(stat.Nodes[n][i].Throughput))
          {
            let nThroughput = 0;
            for (let t of Object.keys(stat.Nodes[n][i].Throughput))
            {
              nThroughput += stat.Nodes[n][i].Throughput[t];
            }
            s.interfaces[i][n].Throughput = nThroughput;
          }
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
      <button class="btn btn-sm btn-danger bi bi-x-circle float-end" c-click="action('stop', '{{@key}}', '')" style="margin-left: 10px;" title="stop"></button>
      <button class="btn btn-sm btn-warning bi bi-arrow-clockwise float-end" c-click="action('restart', '{{@key}}', '')" style="margin-left: 10px;" title="restart"></button>
      <button class="btn btn-sm btn-success bi bi-power float-end" c-click="action('start', '{{@key}}', '')" title="start"></button>
      {{/if}}
      {{@key}}
    </div>
    <div class="card-body">
      <table class="table table-sm table-condensed table-striped">
        <thead>
          <tr>
            <td title="Node"><i class="bi bi-node-plus"></i></td>
            <td style="text-align: right;" title="Process ID"><i class="bi bi-robot"></i></td>
            <td style="text-align: right;" title="Resident Memory"><i class="bi bi-memory"></i></td>
            <td style="text-align: right;" title="Threads"><i class="bi bi-threads"></i></td>
            <td style="text-align: right;" title="Throughput (#/min)"><i class="bi bi-speedometer"></i></td>
            {{#if @root.bDeveloper}}
            <td colspan="2"></td>
            {{/if}}
          </tr>
        </thead>
        <tbody>
          {{#each .}}
          <tr>
            <td {{#ifCond Master.Node "==" @key}} style="font-weight: bold;" title="Master"{{/ifCond}}>{{@key}}</td>
            <td style="text-align: right;">{{PID}}</td>
            <td style="text-align: right;">{{byteShort (multiply Memory.Resident 1024) 0}}</td>
            <td style="text-align: right;">{{#if Threads}}{{numberShort Threads 0}}{{else}}1{{/if}}</td>
            <td style="text-align: right;">{{#if Throughput}}{{numberShort Throughput 0}}{{else}}0{{/if}}</td>
            {{#if @root.bDeveloper}}
            <td>{{#if PID}}<button class="btn btn-sm btn-warning bi bi-arrow-clockwise float-end" c-click="action('restart', '{{@../key}}', '{{@key}}')" title="restart"></button>{{/if}}</td>
            <td>{{#if PID}}<button class="btn btn-sm btn-danger bi bi-x-circle" c-click="action('stop', '{{@../key}}', '{{@key}}')" title="stop"></button>{{else}}<button class="btn btn-sm btn-success bi bi-power" c-click="action('start', '{{@../key}}', '{{@key}}')" title="start"></button>{{/if}}</td>
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
