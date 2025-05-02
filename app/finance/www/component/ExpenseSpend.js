// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-10-23
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
    let f = a.d.Spend;
    let s = c.scope('ExpenseSpend',
    {
      // {{{ m()
      m: (m) =>
      {
        if (m != null)
        {
          for (let [k, v] of Object.entries(f))
          {
            s.e[k] = v[m];
          }
        }
        else
        {
          for (let [k, v] of Object.entries(s.e))
          {
            f[k][s.month] = v.v;
          }
        }
        s.month = m;
        s.u();
        a.dataInit();
        a.jsonSave();
      },
      // }}}
      // {{{ u()
      u: () =>
      {
        c.update('ExpenseSpend');
      },
      // }}}
      a: a,
      c: c,
      chart: null,
      e: {},
      month: null,
      months: ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']
    });
    c.setMenu('Expense', 'Spend');
    s.f = f;
    a.initSpend(s.u);
    c.attachEvent('render', (data) =>
    {
      if (data.detail.name == 'ExpenseSpend')
      {
        let d = {labels: [], datasets: []};
        for (let i = 0; i < s.months.length; i++)
        {
          d.labels.push(s.months[i]);
        }
        let nIndex = 0;
        for (let [k, v] of Object.entries(s.f))
        {
          d.datasets[nIndex] = {}
          d.datasets[nIndex].label = k;
          d.datasets[nIndex].data = [];
          for (let [sk, sv] of Object.entries(v))
          {
            d.datasets[nIndex].data.push(sv);
          }
          nIndex++;
        }
        const ctx = document.getElementById('chart');
        if (!c.isNull(s.chart))
        {
          s.chart.destroy();
          delete s.chart;
        }
        s.chart = new Chart(ctx, {type: 'bar', data: d, options: {x: {stacked: true}, y: {stacked: true}} });
      }
    });
  },
  // }}}
  // {{{ template
  template: `
    <h3 class="page-header">Expense - Spend</h3>
    <div style="max-height: 400px;"><canvas id="chart"></canvas></div>
    <div class="table-responsive">
      <table class="table table-sm table-striped bg-danger-subtle border border-danger-subtle">
        <thead>
          <tr>
            <th style="background: inherit;"></th>
            {{#each months}}
            <th style="background: inherit; text-align:right;">{{.}}</th>
            {{/each}}
            <th style="background: inherit; text-align:right;">Avg</th>
            <th style="background: inherit; text-align:right;">Total</th>
          </tr>
          <tr>
            <th style="background: inherit;"></th>
            {{#for 1 12 1}}
            <th style="background: inherit; text-align:right;">{{#ifCond . '!=' ../month}}<button class="btn btn-sm btn-warning bi bi-pencil" c-click="m('{{../.}}')" title="Edit"></button>{{else}}<button class="btn btn-sm btn-success bi bi-save" c-click="m(null)" title="Save"></button>{{/ifCond}}</th>
            {{/for}}
            <th colspan="2" style="background: inherit;"></th>
          </tr>
        </thead>
        <tbody>
          {{#each f}}
          <tr>
            <td style="background: inherit;">{{@key}}</td>
            {{#each .}}
            <td style="background: inherit; text-align:right;">{{#ifCond @key '!=' ../../month}}<span title="{{number ../.}}">{{numberShort ../.}}</span>{{else}}<input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="e['{{@../key}}']" style="text-align:right;">{{/ifCond}}</td>
            {{/each}}
            <th style="background: inherit; text-align:right;" title="{{number (spendItemAverage .)}}">{{numberShort (spendItemAverage .)}}</th>
            <th style="background: inherit; text-align:right;" title="{{number (spendItemTotal .)}}">{{numberShort (spendItemTotal .)}}</th>
          </tr>
          {{/each}}
          <tr>
            <th style="background: inherit;">Total</th>
            {{#for 1 12 1}}
            <th style="background: inherit; text-align:right;" title="{{number (spendMonthTotal .)}}">{{numberShort (spendMonthTotal .)}}</th>
            {{/for}}
            <th style="background: inherit; text-align:right;" title="{{number (spendAverage)}}">{{numberShort (spendAverage)}}</th>
            <th style="background: inherit; text-align:right;" title="{{number (spendTotal)}}">{{numberShort (spendTotal)}}</th>
          </tr>
        </tbody>
      </table>
    </div>
  `
  // }}}
}
