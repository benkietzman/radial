// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-12-08
// copyright  : AT&T
// email      : bk6471@att.com
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
      nodes: {}
    });
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      s.info.v = 'Retrieving nodes...';
      let request = {Interface: 'link', 'Function': 'status'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          let strNode = response.Response.Node;
          let nodes = [strNode];
          for (let i = 0; i < response.Response.Links.length; i++)
          {
            nodes.push(response.Response.Links[i]);
          }
          nodes.sort();
          s.nodes = null;
          s.nodes = {};
          for (let i = 0; i < nodes.length; i++)
          {
            s.nodes[nodes[i]] = {};
            let request = {Interface: 'central', 'Function': 'status', Request: {Node: nodes[i]}};
            if (nodes[i] != strNode)
            {
              request.Node = nodes[i];
            }
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              if (c.wsResponse(response, error))
              {
                s.nodes[response.Request.Node] = response.Response;
                s.u();
              }
              else
              {
                s.message.v = error.message;
              }
            });
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
      if (data.detail && data.detail.Action && data.detail.Action == 'status' && data.detail.Source)
      {
        s.nodes[data.detail.Source] = data.detail;
        s.u();
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
  {{#each nodes}}
  <div class="col-auto">
  <div class="card" style="margin-top: 10px;">
    <div class="card-header bg-info text-white">
      {{#ifCond Master.Node "==" @key}}<b>{{@key}}</b>{{else}}{{@key}}{{/ifCond}}
    </div>
    <div class="card-body">
      <b>Status</b>
      <table class="table table-sm table-condensed table-striped">
        <tr><td>Process ID</td><td style="text-align: right;">{{PID}}</td></tr>
        <tr><td>Resident</td><td style="text-align: right;">{{#if Memory.Resident}}{{byteShort (multiply Memory.Resident 1024) 0}}{{/if}}</td></tr>
        <tr><td>Threads</td><td style="text-align: right;">{{#if Threads}}{{numberShort Threads 0}}{{/if}}</td></tr>
      </table>
      {{#if Throughput}}
      <b>Throughput</b>
      <table class="table table-sm table-condensed table-striped">
      {{#each Throughput}}
        <tr><td>{{@key}}</td><td style="text-align: right;">{{numberShort . 0}}</td></tr>
      {{/each}}
      </table>
      {{/if}}
    </div>
  </div>
  </div>
  {{/each}}
  </div>
  `
  // ]]]
}
