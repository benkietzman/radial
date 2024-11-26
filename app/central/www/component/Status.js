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
    let s = c.scope('Status',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Status');
      },
      // ]]]
      a: a,
      c: c,
      bDeveloper: false,
      bDisabled: false,
      interfaces: null,
      nodes: null,
      strAction: null
    });
    // ]]]
    // [[[ action()
    s.action = (strAction, strInterface, strNode) =>
    {
      if (s.bDeveloper)
      {
        s.bDisabled = true;
        s.strAction = strInterface;
        s.u();
        let request = {Interface: 'status', 'Function': 'action', Request: {Action: strAction, Interface: strInterface}};
        if (strNode != '')
        {
          request.Request.Node = strNode;
        }
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.bDisabled = false;
          s.strAction = null;
          if (c.wsResponse(response, error))
          {
            s.stat();
          }
          else
          {
            s.message.v = error.message;
            s.u();
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
      s.info.v = 'Initializing...';
      s.bDeveloper = c.isGlobalAdmin();
      if (s.bDeveloper)
      {
        s.u();
      }
      else if (c.isValid())
      {
        let request = {Interface: 'central', 'Function': 'application', Request: {name: c.application}};
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
                s.bDeveloper = true;
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
        s.info.v = null;
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
    c.setMenu('Status');
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
    c.attachEvent('commonWsMessage_Central', (data) =>
    {
      if (!s.bDisabled && data.detail && data.detail.Action && data.detail.Action == 'status' && data.detail.Nodes)
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
  <div class="card border border-info-subtle" style="margin-top: 10px;">
    <div class="card-header bg-info-subtle fw-bold"">
      {{#if @root.bDeveloper}}
      {{#statusShowStop .}}
      <button class="btn btn-sm btn-danger bi bi-x-circle float-end" c-click="action('stop', '{{@key}}', '')" style="margin-left: 10px;" title="stop"{{#if @root.bDisabled}} disabled{{/if}}></button>
      {{/statusShowStop}}
      {{#statusShowRestart .}}
      <button class="btn btn-sm btn-warning bi bi-arrow-clockwise float-end" c-click="action('restart', '{{@key}}', '')" style="margin-left: 10px;" title="restart"{{#if @root.bDisabled}} disabled{{/if}}></button>
      {{/statusShowRestart}}
      {{#statusShowStart .}}
      <button class="btn btn-sm btn-success bi bi-power float-end" c-click="action('start', '{{@key}}', '')" title="start"{{#if @root.bDisabled}} disabled{{/if}}></button>
      {{/statusShowStart}}
      {{/if}}
      {{@key}}
    </div>
    <div class="card-body bg-info-subtle">
      <table class="table table-sm table-condensed table-striped">
        <thead>
          <tr>
            <td style="background: inherit;" title="Node"><i class="bi bi-node-plus"></i></td>
            <td style="background: inherit; text-align: right;" title="Process ID"><i class="bi bi-robot"></i></td>
            <td style="background: inherit; text-align: right;" title="Resident Memory"><i class="bi bi-memory"></i></td>
            <td style="background: inherit; text-align: right;" title="Threads"><i class="bi bi-threads"></i></td>
            <td style="background: inherit; text-align: right;" title="Throughput (#/min)"><i class="bi bi-speedometer"></i></td>
            {{#if @root.bDeveloper}}
            <td colspan="2" style="background: inherit;"></td>
            {{/if}}
          </tr>
        </thead>
        <tbody>
          {{#each .}}
          <tr{{#ifCond Master.Node "==" @key}} class="bg-info" title="Master"{{/ifCond}}>
            <td style="background: inherit;">{{@key}}</td>
            <td style="background: inherit; text-align: right;">{{PID}}</td>
            <td style="background: inherit; text-align: right;">{{#if Memory.Resident}}{{byteShort (multiply Memory.Resident 1024) 2}}{{/if}}</td>
            <td style="background: inherit; text-align: right;">{{#if Threads}}{{numberShort Threads 0}}{{/if}}</td>
            <td style="background: inherit; text-align: right;">{{#if Throughput}}{{numberShort Throughput 0}}{{/if}}</td>
            {{#if @root.bDeveloper}}
            <td style="background: inherit;">{{#if PID}}<button class="btn btn-sm btn-warning bi bi-arrow-clockwise float-end" c-click="action('restart', '{{@../key}}', '{{@key}}')" title="restart"{{#if @root.bDisabled}} disabled{{/if}}></button>{{/if}}</td>
            <td style="background: inherit;">{{#if PID}}<button class="btn btn-sm btn-danger bi bi-x-circle" c-click="action('stop', '{{@../key}}', '{{@key}}')" title="stop"{{#if @root.bDisabled}} disabled{{/if}}></button>{{else}}<button class="btn btn-sm btn-success bi bi-power" c-click="action('start', '{{@../key}}', '{{@key}}')" title="start"{{#if @root.bDisabled}} disabled{{/if}}></button>{{/if}}</td>
            {{/if}}
          </tr>
          {{/each}}
        </tbody>
      </table>
    </div>
    {{#ifCond @root.strAction "==" @key}}
    <div class="card-footer">
      <span class="text-warning">processing...</span>
    </div>
    {{/ifCond}}
  </div>
  </div>
  {{/each}}
  </div>
  `
  // ]]]
}
