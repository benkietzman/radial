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
      stat: null
    });
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
      s.stat = stat;
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
  {{#each stat.Nodes}}
  <div class="card">
    <div class="card-header bg-info text-white" style="font-weight: bold;">
      {{@key}}
    </div>
    <div class="card-body">
      <table class="table table-condensed table-striped">
        <thead>
          <tr>
            <th>Interface</th>
            <th>Command</th>
            <th style="white-space: nowrap;">Access Function</th>
            <th>PID</th>
            <th>Respawn</th>
            <th>Restricted</th>
          </tr>
        </thead>
        <tbody>
          {{#each .}}
          <tr>
            <td>{{@key}}</td>
            <td>{{Command}}</td>
            <td>{{AccessFunction}}</td>
            <td>{{PID}}</td>
            <td>{{#if Respawn}}yes{{else}}no{{/if}}</td>
            <td>{{#if Restricted}}yes{{else}}no{{/if}}</td>
          </tr>
          {{/each}}
        </tbody>
      </table>
    </div>
  </div>
  {{/each}}
  `
  // ]]]
}
