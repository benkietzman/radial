// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2026-10-07
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
      c: c
    });
    // ]]]
    // [[[ compositionAdd()
    s.compositionAdd = () =>
    {
      if (c.isValid())
      {
        s.composition = null;
        s.composition = {};
        let request = {Interface: 'maestro', 'Function': 'compositionAdd', Request: {'Name': s.compositionName.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            let request = {Interface: 'maestro', 'Function': 'composition', Request: {'Name': s.compositionName.v}};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              if (c.wsResponse(response, error))
              {
                s.composition = response.Response;
              }
              else
              {
                c.pushErrorMessage(error.message);
              }
              s.u();
            });
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      else
      {
        c.pushErrorMessage('You are not authorized to perform this action.');
      }
    };
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      if (c.isValid())
      {
        s.compositions = null;
        s.compositions = {};
        let request = {Interface: 'maestro', 'Function': 'compositions'};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.compositions = response.Response;
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
          s.u();
        });
      }
      else
      {
        s.u();
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
    c.attachEvent('commonWsMessage_Maestro', (data) =>
    {
      if (data.detail && data.detail.Action && (data.detail.Action == 'compositionAdd' || data.detail.Action == 'compositionRemove') && !s.composition)
      {
        s.init();
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  {{#isValid}}
  {{#if ../composition}}
  {{json ../composition}}
  {{else}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
    <thead>
      <tr><th>Composition</th><th>Owners</th></tr>
    </thead>
    <tbody>
      <tr>
        <td><input type="text" class="form-conrtol" c-model="compositionName" placeholder="Composition"></td>
        <td><button class="btn btn-primary bi bi-plus-circle" c-click="compositionAdd()" title="Add Composition"></button></td>
      </tr>
      {{#each ../compositions}}
      <tr>
        <td>{{@key}}</td>
        <td>{{json Owners}}</td>
      </tr>
      {{/each}}
    </tbody>
    </table>
  </div>
  {{/if}}
  {{else}}
  <p class="fw-bold text-danger">Please login to use this application.</p>
  {{/isValid}}
  `
  // ]]]
}
