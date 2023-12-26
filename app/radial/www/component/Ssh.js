// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-12-26
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
    let s = c.scope('Ssh',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Ssh');
      },
      // ]]]
      a: a,
      c: c,
      strSession: null,
    });
    // ]]]
    // [[[ connect()
    s.connect = () =>
    {
      if (s.server.v)
      {
        if (s.user.v)
        {
          if (s.password.v)
          {
            s.screen.v = '';
            let request = {Interface: 'ssh', 'Function': 'connect', Request: {Server: s.server.v, User: s.user.v, Password: s.password.v}};
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
              if (c.isDefined(response.Response))
              {
                s.screen.v = response.Response;
              }
              s.u();
            });
          }
          else
          {
            s.message.v = 'Please provide the Password.';
          }
        }
        else
        {
          s.message.v = 'Please provide the User.';
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
      let request = {Interface: 'ssh', 'Function': 'disconnect', Session: s.strSession};
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
    // [[[ send()
    s.send = () =>
    {
      let request = {Interface: 'ssh', 'Function': 'send', Session: s.strSession, Request: s.command.v + '\n'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.command.v = null;
        }
        else
        {
          s.message.v = error.message;
        }
        if (!c.isDefined(response.Session))
        {
          s.strSession = null;
        }
        if (c.isDefined(response.Response))
        {
          s.screen.v += response.Response;
        }
        s.u();
      });
    };
    // ]]]
    // [[[ main
    c.setMenu('Ssh');
    s.u();
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <h4 class="page-header">Secure Shell (SSH)</h4>
    <div c-model="info" class="text-warning"></div>
    <div c-model="message" class="text-danger"></div>
    <div class="row">
      <div class="col-md-8">
        <div style="background: black; color: green; display: inline-block; font-family: monospace; font-size: 11px; height: 400px; margin: 0px; max-height: 400px; max-width: 600px; overflow: auto; padding: 10px; white-space: pre; width: 600px;" c-model="screen"></div>
        <div class="row">
          <div class="col">
            <div class="input-group"><span class="input-group-text">Send</span><input type="text" class="form-control" c-model="command"{{^if strSession}} disabled{{/if}}></div>
          </div>
          <div class="col">
            <button class="btn btn-warning" c-click="send()"{{^if strSession}} disabled{{/if}}>Send</button>
          </div>
        </div>
      </div>
      <div class="col-md-4">
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
          <div class="input-group"><span class="input-group-text">User</span><input type="text" class="form-control" c-model="user"></div>
          </div>
        </div>
        <div class="row" style="margin-bottom: 10px;">
          <div class="col">
          <div class="input-group"><span class="input-group-text">Password</span><input type="password" class="form-control" c-model="password"></div>
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
  </div>
  `
  // ]]]
}
