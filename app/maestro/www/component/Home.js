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
      if (data.detail && data.detail.Action && (data.detail.Action == '...' || data.detail.Action == '...'))
      {
        s.u();
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  {{#isValid}}
  {{else}}
  <p class="fw-bold text-danger">Please login to use this application.</p>
  {{/isValid}}
  `
  // ]]]
}
