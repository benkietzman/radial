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
    <div class="table-responsive" style="max-height: 500px;">
      <table class="table table-sm table-striped bg-danger-subtle border border-danger-subtle">
        <thead style="position: sticky; top: 0;">
          <tr>
            <th class="bg-danger-subtle"></th>
            {{#each months}}
            <th class="bg-danger-subtle text-end">{{.}}</th>
            {{/each}}
            <th class="bg-danger-subtle text-end">Avg</th>
            <th class="bg-danger-subtle text-end">Total</th>
          </tr>
          <tr>
            <th class="bg-danger-subtle" style="left: 0; position: sticky;"></th>
            {{#for 1 12 1}}
            <th class="bg-danger-subtle text-end">{{#ifCond . '!=' ../month}}<button class="btn btn-sm btn-warning bi bi-pencil" c-click="m('{{../.}}')" title="Edit"></button>{{else}}<button class="btn btn-sm btn-success bi bi-save" c-click="m(null)" title="Save"></button>{{/ifCond}}</th>
            {{/for}}
            <th class="bg-danger-subtle" colspan="2"></th>
          </tr>
        </thead>
        <tbody>
          {{#each f}}
          <tr>
            <td class="bg-danger-subtle" style="left: 0; position: sticky;">{{@key}}</td>
            {{#each .}}
            <td class="text-end" style="background: inherit;">{{#ifCond @key '!=' ../../month}}<span title="{{number ../.}}">{{numberShort ../.}}</span>{{else}}<input type="text" class="form-control form-control-sm bg-danger-subtle text-end" c-model="e['{{@../key}}']">{{/ifCond}}</td>
            {{/each}}
            <th class="text-end" style="background: inherit;" title="{{number (spendItemAverage .)}}">{{numberShort (spendItemAverage .)}}</th>
            <th class="text-end" style="background: inherit;" title="{{number (spendItemTotal .)}}">{{numberShort (spendItemTotal .)}}</th>
          </tr>
          {{/each}}
        </tbody>
        <tfoot>
          <tr style="position: sticky; bottom: 0;">
            <th class="bg-danger-subtle" style="left: 0; position: sticky;">Total</th>
            {{#for 1 12 1}}
            <th class="bg-danger-subtle text-end" style="background: inherit;" title="{{number (spendMonthTotal .)}}">{{numberShort (spendMonthTotal .)}}</th>
            {{/for}}
            <th class="bg-danger-subtle text-end" style="background: inherit;" title="{{number (spendAverage)}}">{{numberShort (spendAverage)}}</th>
            <th class="bg-danger-subtle text-end" style="background: inherit;" title="{{number (spendTotal)}}">{{numberShort (spendTotal)}}</th>
          </tr>
        </tfoot>
      </table>
    </div>
  `
  // }}}
}
