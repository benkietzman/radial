// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2025-10-17
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
      bLoaded: false
    });
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      s.bLoaded = true;
      if (c.isValid('Builder'))
      {
        s.servers = null;
        s.servers = [];
        s.server = null;
        let request = {Interface: 'central', 'Function': 'servers'};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.servers = response.Response;
            if (s.servers.length > 0)
            {
              s.server = s.servers[0];
            }
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
          s.config = null;
          s.config = {};
          s.packages = null;
          s.packages = [];
          s['package'] = null;
          let request = {Interface: 'builder', 'Function': 'config'};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            if (c.wsResponse(response, error))
            {
              s.config = response.Response;
              for (let key of Object.keys(s.config.packages))
              {
                let pkg = s.config.packages[key];
                pkg.name = key;
                s.packages.push(pkg);
              }
              if (s.packages.length > 0)
              {
                s['package'] = s.packages[0];
              }
            }
            else
            {
              c.pushErrorMessage(error.message);
            }
            s.u();
          });
        });
      }
      else
      {
        s.u();
      }
    };
    // ]]]
    // [[[ process()
    s.process = (strFunction) =>
    {
      if (c.isValid('Builder'))
      {
        s.results = null;
        s.results = [];
        s.u();
        let request = {Interface: 'builder', 'Function': strFunction, Request: {Server: s.server.v.name, Package: s['package'].v.name}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
          s.u();
        });
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Home');
    s.u();
    if (a.ready())
    {
      s.init();
    }
    c.attachEvent('appReady', (data) =>
    {
      s.init();
    });
    c.attachEvent('commonWsMessage_Builder', (data) =>
    {
      if (data.detail && data.detail.Action && data.detail.Action == 'terminal' && data.detail.Data)
      {
        s.results.push(data.detail.Data);
        s.u();
        window.scrollTo(0, document.body.scrollHeight);
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  {{#isValid "Builder"}}
  <div class="row">
    <div class="col-md-auto">
      <div class="input-group"><span class="input-group-text bg-primary">Server</span><select class="form-control bg-primary-subtle border border-primary-subtle" c-model="server" c-change="u()" c-json>{{#each ../servers}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
    </div>
    <div class="col-md-auto">
      <div class="input-group"><span class="input-group-text bg-warning text-dark">Package</span><select class="form-control bg-warning-subtle border border-warning-subtle" c-model="package" c-change="u()" c-json>{{#each ../packages}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
    </div>
    <div class="col-md-auto">
      <button class="btn btn-success" c-click="process('install')">Install</button>
    </div>
    <div class="col-md-auto">
      <button class="btn btn-danger" c-click="process('uninstall')">Uninstall</button>
    </div>
  </div>
  {{#if ../results}}
  <div class="card border border-secondary-subtle" style="margin-top: 10px;">
    <div class="card-header bg-secondary fw-bold">
      Console
    </div>
    <div class="card-body bg-secondary-subtle">
      {{#each ../results}}
      <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{.}}</pre>
      {{/each}}
    </div>
  </div>
  {{/if}}
  {{else}}
  {{#if ../bLoaded}}
  {{^isValid}}
  <p class="fw-bold text-danger">Please login to use this application.</p>
  {{/isValid}}
  <p class="fw-bold text-danger">You must be registered as a contact for the Builder application in Central.</p>
  {{/if}}
  {{/isValid}}
  `
  // ]]]
}
