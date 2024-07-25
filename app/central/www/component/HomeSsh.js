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
    let s = c.scope('HomeSsh',
    {
      // [[[ u()
      u: () =>
      {
        c.update('HomeSsh');
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
                s.screen.v = s.convert(response.Response);
              }
              s.u();
              var div = document.getElementById('screen');
              div.scrollTop = div.scrollHeight;
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
    // [[[ convert()
    s.convert = (strData) =>
    {
      let strConv = '';
      let lines = strData.split('\n');

      for (let i = 0; i < lines.length; i++)
      {
        strConv += s.convertLine(lines[i]) + '\n';
      }

      return strConv;
    };
    // ]]]
    // [[[ convertLine()
    s.convertLine = (strData) =>
    {
      let strConv = '';
      let unPosition;

      while (strData.length > 0)
      {
        if (strData[0] == '\u001b' && (unPosition = strData.search('m')) != -1)
        {
          strData = strData.substr((unPosition + 1), (strData.length - (unPosition + 1)));
        }
        else
        {
          strConv += strData[0];
          strData = strData.substr(1, (strData.length - 1));
        }
      }

      return strConv;
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
    // [[[ enter()
    s.enter = () =>
    {
      if (window.event.keyCode == 13)
      {
        s.send();
      }
    };
    // ]]]
    // [[[ send()
    s.send = () =>
    {
      let request = {Interface: 'ssh', 'Function': 'send', Session: s.strSession, Request: s.command.v + '\n'};
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
        if (c.isDefined(response.Response))
        {
          s.screen.v += s.convert(response.Response);
        }
        s.u();
        var div = document.getElementById('screen');
        div.scrollTop = div.scrollHeight;
        document.getElementById('command').focus();
      });
    };
    // ]]]
    // [[[ main
    c.setMenu('Home', 'SSH');
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
      <div id="screen" style="background: black; color: green; display: inline-block; font-family: monospace; font-size: 11px; height: 400px; margin: 0px; max-height: 400px; max-width: 800px; overflow: auto; padding: 10px; white-space: pre; width: 800px;" c-model="screen"></div>
      <div class="row">
        <div class="col">
          <div class="input-group"><span class="input-group-text">Send</span><input type="text" class="form-control" id="command" c-model="command" c-keyup="enter()"{{^if strSession}} disabled{{/if}}></div>
        </div>
        <div class="col">
          <button class="btn btn-warning" c-click="send()"{{^if strSession}} disabled{{/if}}>Send</button>
        </div>
      </div>
    </div>
    <div class="col-md-3">
      <h4 class="page-header">Secure Shell</h4>
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
  `
  // ]]]
}
