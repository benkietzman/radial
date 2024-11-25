// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-10-12
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // {{{ controller()
  controller(id, nav)
  {
    let a = app;
    let c = common;
    let f = a.d.Assumption;
    let s = c.scope('SummaryAssumption',
    {
      // {{{ change()
      change: (bInitData) =>
      {
        for (let [k, v] of Object.entries(s.d))
        {
          if (c.isObject(v))
          {
            f[k] = v.v;
          }
          else
          {
            f[k] = v;
          }
        }
        if (bInitData)
        {
          a.dataInit({IgnoreTimestamp: true, PostFunction: s.load});
        }
      },
      // }}}
      // {{{ load()
      load: () =>
      {
        for (let [k, v] of Object.entries(f))
        {
          if (c.isObject(s.d[k]))
          {
            s.d[k].v = v;
          }
          else
          {
            s.d[k] = v;
          }
        }
      },
      // }}}
      // {{{ u()
      u: () =>
      {
        c.update('SummaryAssumption');
      },
      // }}}
      a: a,
      c: c,
      d: {}

    });
    c.setMenu('Summary', 'Assumption');
    s.f = f;
    s.load();
  },
  // }}}
  // {{{ template
  template: `
    <div class="row">
      <div class="col-md-8">
        <h3 class="page-header">Summary - Assumptions</h3>
        <div class="row">
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text">Birth Year</span><input type="text" class="form-control" c-change="change()" c-model="d.BirthYear" placeholder="YYYY"></div></div>
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text">Filing Status</span><select class="form-control" c-change="change(true)" c-model="d.FilingStatus"><option value="single">Single</option><option value="jointly">Married Filing Jointly</option><option value="separate">Married Filing Separately</option><option value="household">Head of Household</option></select></div></div>
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text">Income Raise</span><input type="text" class="form-control" c-change="change()" c-model="d.IncomeRaise"><span class="input-group-text">%</span></div></div>
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text">Inflation Duration</span><select class="form-control" c-change="change(true)" c-model="d.InflationDuration"><option value="0">None</option><option value="1">Current Year</option><option value="2">Last 2 Years</option><option value="5">Last 5 Years</option><option value="10">Last 10 Years</option><option value="20">Last 20 Years</option></select></div></div>
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text"><a href="https://data.bls.gov/registrationEngine/" target="_blank" title="register">Inflation Key</a></span><input type="text" class="form-control" c-change="change(true)" c-model="d.InflationKey" placeholder="XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" title=""></div></div>
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text">Inflation Rate</span><input type="text" class="form-control" c-change="change()" c-model="d.InflationRate"><span class="input-group-text">%</span></div></div>
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text">Pension Lump Sum</span><input type="text" class="form-control" c-change="change()" c-model="d.PensionLumpSum" placeholder="$"></div></div>
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text">Retire Year</span><input type="text" class="form-control" c-change="change()" c-model="d.RetireYear" placeholder="YYYY"></div></div>
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text">Social Security Net</span><input type="text" class="form-control" c-change="change()" c-model="d.SocialSecurityNet" placeholder="$"></div></div>
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text">Social Security Year</span><input type="text" class="form-control" c-change="change()" c-model="d.SocialSecurityYear" placeholder="YYYY"></div></div>
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text">Stock Growth</span><input type="text" class="form-control" c-change="change()" c-model="d.StockGrowth"><span class="input-group-text">%</span></div></div>
          <div class="col-md-auto"><div class="input-group input-group-sm" style="margin: 4px;"><span class="input-group-text">Tithe</span><input type="text" class="form-control" c-change="change()" c-model="d.Tithe"><span class="input-group-text">%</span></div></div>
        </div>
      </div>
      <div class="col-md-4">
        <div class="card">
          <div class="card-body bg-primary-subtle">
            <ul class="list-group list-group-flush">
              <li class="list-group-item bg-primary-subtle"><b>Inflation Key:</b> Registration key for the Bureau of Labor and Statistics (BLS).  This key allows Finance to query inflation data from BLS.</li>
              <li class="list-group-item bg-primary-subtle"><b>Social Security Net:</b> This is the annual amount you will receive once you start collecting Social Security benefits.</li>
            </ul>
          </div>
        </div>
      </div>
    </div>
  `
  // }}}
}
