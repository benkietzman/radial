// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-12-08
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // {{{ controller()
  controller(id, nav)
  {
    let a = app;
    let c = common;
    let s = c.scope('progress',
    {
      a: a,
      c: c,
      display: false,
      handle: null,
      progress: 0
    });
    c.attachEvent('financeProgress', (data) =>
    {
      let nDuration = 120000;
      s.display = true;
      s.progress = data.detail;
      if (s.progress == 100)
      {
        nDuration = 15000;
      }
      c.render('progress');
      if (s.handle != null)
      {
        clearTimeout(s.handle);
      }
      s.handle = setTimeout(() => {s.display = false; c.render('progress');}, nDuration);
    });
  },
  // }}}
  // {{{ template
  template: `
    <div style="position: relative; z-index: 100;">
      <div style="position: fixed; bottom: 10px; right: 10px;">
      {{#display}}
      <div class="progress" style="width: 100px;">
        <div class="progress-bar bg-warning text-dark" role="progressbar" style="width: {{progress}}%" aria-valuenow="{{progress}}" aria-valuemin="0" aria-valuemax="{{progress}}">{{progress}}%</div>
        <div class="progress-bar bg-secondary" role="progressbar" style="width: {{subtract 100 progress}}%" aria-valuenow="{{subtract 100 progress}}" aria-valuemin="0" aria-valuemax="{{subtract 100 progress}}"></div>
      </div>
      {{/display}}
      </div>
    </div>
  `
  // }}}
}
