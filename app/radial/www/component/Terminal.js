// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-12-27
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
    let s = c.scope('Terminal',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Terminal');
      },
      // ]]]
      a: a,
      c: c,
      nums: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10],
      strSession: null,
    });
    // ]]]
    // [[[ connect()
    s.connect = () =>
    {
      if (s.server.v)
      {
        if (s.port.v)
        {
          s.screen.v = '';
          let request = {Interface: 'terminal', 'Function': 'connect', Request: {Server: s.server.v, Port: s.port.v}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            if (!c.wsResponse(response, error))
            {
              s.message.v = error.message;
            }
            if (c.isDefined(response.Session))
            {
              s.strSession = response.Session;
            }
            s.screen();
          });
        }
        else
        {
          s.message.v = 'Please provide the Port.';
        }
      }
      else
      {
        s.message.v = 'Please provide the Server.';
      }
    };
    // ]]]
    // [[[ disconnect()
    s.disconnect = () =>
    {
      s.screen.v = '';
      let request = {Interface: 'terminal', 'Function': 'disconnect', Session: s.strSession};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.strSession = null;
          s.u();
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ screen()
    s.screen = () =>
    {
      let request = {Interface: 'terminal', 'Function': 'screen', Session: s.strSession};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (!c.wsResponse(response, error))
        {
          s.message.v = error.message;
        }
        if (c.isDefined(response.Response))
        {
          if (c.isDefined(response.Response.Screen))
          {
            s.screen.v = response.Response.Screen;
          }
        }
        s.u();
        var div = document.getElementById('screen');
        div.scrollTop = div.scrollHeight;
      });
    };
    // ]]]
    // [[[ send()
    s.send = () =>
    {
      let request = {Interface: 'terminal', 'Function': 'send', Session: s.strSession, Request: s.command.v};
      s.command.v = null;
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (!c.wsResponse(response, error))
        {
          s.message.v = error.message;
        }
        if (!c.isDefined(response.Session))
        {
          s.strSession = null;
        }
        s.screen();
      });
    };
    // ]]]
    // [[[ main
    c.setMenu('Terminal');
    s.u();
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger"></div>
  <div class="row">
    <div class="col-md-9">
      <div id="screen" style="background: black; color: green; display: inline-block; font-family: monospace; font-size: 11px; margin: 0px; padding: 0px; white-space: pre;" c-model="screen"></div>
      <div style="display: inline; font-family: monospace;">
        <div class="row">
          {{#each nums}}
          <div class="col-md-1">
            <button class="btn btn-sm btn-default" c-click="sendFunction({{.}})">F{{.}}{{#ifCond . "<" "10"}}&nbsp;{{/ifCond}}</button>
          </div>
          {{/each}}
        </div>
        <div class="row">
          {{#each nums}}
          <div class="col-md-1">
            <button class="btn btn-sm btn-default" c-click="sendShiftFunction(({{.}}))">F{{add . 10}}</button>
          </div>
          {{/each}}
        </div>
      </div>
    </div>
    <div class="col-md-3">
      <h4 class="page-header">Terminal Emulation</h4>
      {{#if strSession}}
      <div class="row">
        <div class="col">
        <button class="btn btn-danger" c-click="disconnect()">Disconnect</button>
        </div>
      </div>
        {{else}}
      <div class="row" style="margin-bottom: 10px;">
        <div class="col">
        <div class="input-group"><span class="input-group-text">Server</span><input type="text" class="form-control" c-model="server"></div>
        </div>
      </div>
      <div class="row" style="margin-bottom: 10px;">
        <div class="col">
        <div class="input-group"><span class="input-group-text">Port</span><input type="text" class="form-control" c-model="port"></div>
        </div>
      </div>
      <div class="row">
        <div class="col">
        <button class="btn btn-success" c-click="connect()">Connect</button>
        </div>
      </div>
      {{/if}}
    </div>
  </div>
  `
  // ]]]
}
