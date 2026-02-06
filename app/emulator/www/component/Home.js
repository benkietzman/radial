// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2026-02-05
// copyright  : Ben Kietzman
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
    let s = c.scope('Home',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Home');
      },
      // ]]]
      a: a,
      c: c,
      bLaunched: false,
      bLoaded: false
    });
    // ]]]
    // [[[ launch()
    s.launch = (strServer) =>
    {
      let request = {Interface: 'emulator', 'Function': 'launch', Request: {Command: s.command.v}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.bLaunched = true;
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ init()
    s.init = () =>
    {
    };
    // ]]]
    // [[[ main
    c.setMenu('Home');
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
      s.u();
    });
    c.attachEvent('commonWsMessage_Emulator', (data) =>
    {
      if (data.detail && data.detail.Action && data.detail.Action == 'data' && data.detail.Data)
      {
        console.log(data.detail.Data);
        //s.u();
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div c-model="info" class="text-warning"></div>
  {{#isValid "Emulator"}}
  <div class="row">
    {{#if ../bLaunched}}
    {{else}}
    <div class="input-group"><span class="input-group-text">Command</span><input type="text" class="form-control" c-model="command"></div>
    <div class="btn btn-success" c-click="launch()">Launch</button>
    {{/if}}
  </div>
  {{else}}
  {{#if ../bLoaded}}
  {{^isValid}}
  <p class="fw-bold text-danger">Please login to use this application.</p>
  {{/isValid}}
  <p class="fw-bold text-danger">You must be registered as a contact for the Emulator application in Central.</p>
  {{/if}}
  {{/isValid}}
  `
  // ]]]
}
