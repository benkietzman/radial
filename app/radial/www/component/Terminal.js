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
      useWait: 'yes'
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
          let request = {Interface: 'terminal', 'Function': 'connect', Request: {Server: s.server.v, Port: s.port.v, Wait: true}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            if (c.wsResponse(response, error))
            {
              if (c.isDefined(response.Session))
              {
                s.strSession = response.Session;
              }
              s.process(response);
            }
            else
            {
              s.message.v = error.message;
            }
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
        if (!c.wsResponse(response, error))
        {
          s.message.v = error.message;
        }
        s.strSession = null;
        s.u();
      });
    };
    // ]]]
    // [[[ process()
    s.process = (response) =>
    {
      if (c.isDefined(response.Response) && c.isDefined(response.Response.Screen))
      {
        let strScreen = '';
        for (let i = 0; i < response.Response.Screen.length; i++)
        {
          if (i > 0)
          {
            strScreen += '\n';
          }
          if (c.isDefined(response.Response.Row) && c.isDefined(response.Response.Col) && i == response.Response.Row && response.Response.Screen[i].length > response.Response.Col)
          {
            strScreen += response.Response.Screen[i].substr(0, response.Response.Col) + '<span style="background: green; color: black;">' + response.Response.Screen[i].substr(response.Response.Col, 1) + '</span>' + response.Response.Screen[i].substr((response.Response.Col + 1), (response.Response.Screen[i].length - (response.Response.Col + 1)));
          }
          else
          {
            strScreen += response.Response.Screen[i];
          }
        }
        if (strScreen != '')
        {
          s.screen = strScreen;
        }
        s.u();
        document.getElementById('input').focus();
      }
    };
    // ]]]
    // [[[ send()
    s.send = () =>
    {
      let code = window.event.keyCode;
      let key = window.event.key;
      s.input.v = '';
      if (code == 13 || (code >= 37 && code <= 40)) // enter | left | up | right | down
      {
        let strFunction;
        if (code == 13)
        {
          strFunction = 'enter';
        }
        else if (code == 37)
        {
          strFunction = 'left';
        }
        else if (code == 38)
        {
          strFunction = 'up';
        }
        else if (code == 39)
        {
          strFunction = 'right';
        }
        else if (code == 40)
        {
          strFunction = 'down';
        }
        let request = {Interface: 'terminal', 'Function': strFunction, Session: s.strSession};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.process(response);
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
      else if (key.length == 1)
      {
        let request = {Interface: 'terminal', 'Function': 'send', Session: s.strSession, Request: {Data: key, Screen: true, Wait: s.useWait.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.process(response);
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
    };
    // ]]]
    // [[[ sendEnter()
    s.sendEnter = () =>
    {
      let request = {Interface: 'terminal', 'Function': 'enter', Session: s.strSession, Request: {Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response);
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ sendEscape()
    s.sendEscape = () =>
    {
      let request = {Interface: 'terminal', 'Function': 'escape', Session: s.strSession, Request: {Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response);
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ sendFunction()
    s.sendFunction = (nValue) =>
    {
      let request = {Interface: 'terminal', 'Function': 'function', Session: s.strSession, Request: {Data: nValue, Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response);
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ sendKeypadEnter()
    s.sendKeypadEnter = () =>
    {
      let request = {Interface: 'terminal', 'Function': 'keypadEnter', Session: s.strSession, Request: {Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response);
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ sendShiftFunction()
    s.sendShiftFunction = (nValue) =>
    {
      let request = {Interface: 'terminal', 'Function': 'shiftFunction', Session: s.strSession, Request: {Data: nValue, Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response);
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ sendTab()
    s.sendTab = () =>
    {
      let request = {Interface: 'terminal', 'Function': 'tab', Session: s.strSession, Request: {Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response);
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ wait()
    s.wait = () =>
    {
      let request = {Interface: 'terminal', 'Function': 'wait', Session: s.strSession, Request: {Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response);
        }
        else
        {
          s.message.v = error.message;
        }
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
    <div class="col-md-6">
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
    <div class="col-md-3" style="font-family: monospace;">
      <div class="input-group input-group-sm"><span class="input-group-text">Wait</span><select class="form-control" c-model="useWait"><option value="no">No</option><option value="yes">Yes</option></select></div>
      <button class="btn btn-sm btn-default" c-click="sendEnter()">Enter</button>
      <button class="btn btn-sm btn-default" c-click="sendKeypadEnter()">Enter (Keypad)</button>
      <button class="btn btn-sm btn-default" c-click="sendEscape()">Escape</button>
      <button class="btn btn-sm btn-default" c-click="sendTab()">Tab</button>
      <button class="btn btn-sm btn-default" c-click="wait()">Wait</button>
      <div class="input-group input-group-sm"><input type="text" class="form-control" id="input" c-model="input" c-keyup="send()" placeholder="type here..."></div>
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
