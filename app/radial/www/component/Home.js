// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-10-27
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
    // [[[ main
    c.setMenu('Home');
    s.u();
    if (!a.ready())
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
  `
  // ]]]
}
