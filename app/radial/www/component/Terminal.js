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
          let request = {Interface: 'terminal', 'Function': 'connect', Request: {Server: s.server.v, Port: s.port.v, Screen: true}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            if (c.wsResponse(response, error))
            {
              if (c.isDefined(response.Session))
              {
                s.strSession = response.Session;
              }
              s.process(response.Response);
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
      if (c.isDefined(response.Screen))
      {
        let strScreen = '';
        for (let i = 0; i < response.Screen.length; i++)
        {
          if (i > 0)
          {
            strScreen += '\n';
          }
          if (c.isDefined(response.Row) && c.isDefined(response.Col) && i == response.Row && response.Screen[i].length > response.Col)
          {
            strScreen += response.Screen[i].substr(0, response.Col) + '<span style="background: green; color: black;">' + response.Screen[i].substr(response.Col, 1) + '</span>' + response.Screen[i].substr((response.Col + 1), (response.Screen[i].length - (response.Col + 1)));
          }
          else
          {
            strScreen += response.Screen[i];
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
          strFunction = 'sendEnter';
        }
        else if (code == 37)
        {
          strFunction = 'sendLeft';
        }
        else if (code == 38)
        {
          strFunction = 'sendUp';
        }
        else if (code == 39)
        {
          strFunction = 'sendRight';
        }
        else if (code == 40)
        {
          strFunction = 'sendDown';
        }
        let request = {Interface: 'terminal', 'Function': strFunction, Session: s.strSession, Request: {Screen: true}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.process(response.Response);
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
      else if (key.length == 1)
      {
        let request = {Interface: 'terminal', 'Function': ((s.useWait.v == 'yes')?'sendWait':'send'), Session: s.strSession, Request: {Data: key, Screen: true, Wait: s.useWait.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.process(response.Response);
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
      let request = {Interface: 'terminal', 'Function': 'sendEnter', Session: s.strSession, Request: {Screen: true, Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response.Response);
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
      let request = {Interface: 'terminal', 'Function': 'sendEscape', Session: s.strSession, Request: {Screen: true, Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response.Response);
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
      let request = {Interface: 'terminal', 'Function': 'sendFunction', Session: s.strSession, Request: {Data: nValue, Screen: true, Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response.Response);
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
      let request = {Interface: 'terminal', 'Function': 'sendKeypadEnter', Session: s.strSession, Request: {Screen: true, Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response.Response);
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
      let request = {Interface: 'terminal', 'Function': 'sendShiftFunction', Session: s.strSession, Request: {Data: nValue, Screen: true, Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response.Response);
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
      let request = {Interface: 'terminal', 'Function': 'sendTab', Session: s.strSession, Request: {Screen: true, Wait: true}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response.Response);
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
      let request = {Interface: 'terminal', 'Function': 'wait', Session: s.strSession, Request: {Screen: true, Wait: false}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.process(response.Response);
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
