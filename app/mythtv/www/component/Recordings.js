// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2025-04-25
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
    let s = c.scope('Recordings',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Recordings');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      s.bLoaded = true;
      if (c.isValid('MythTV'))
      {
      }
      else
      {
        s.u();
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Recordings');
    s.u();
    if (a.ready())
    {
      s.init();
    }
    c.attachEvent('appReady', (data) =>
    {
      s.init();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  {{#isValid "MythTV"}}
  {{else}}
  <p class="fw-bold text-danger">Please login to use this application.  You must be registered as a contact for the MythTV application.</p>
  {{/isValid}}
  `
  // ]]]
}
