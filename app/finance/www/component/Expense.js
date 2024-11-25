// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-10-13
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
    let f = a.d.Expense;
    let s = c.scope('Expense',
    {
      // {{{ u()
      u: () =>
      {
        c.update('Expense');
      },
      // }}}
      a: a,
      c: c,
      chart: null
    });
    c.setMenu('Expense', null);
    s.f = f;
    c.attachEvent('render', (data) =>
    {
      if (data.detail.name == 'Expense')
      {
        let d = {labels: ['Auto', 'Charity', 'Food', 'Home', 'Misc', 'Utility'], datasets: [{data: []}]};
        for (let i = 0; i < d.labels.length; i++)
        {
          d.datasets[0].data.push(a.genericTypeSum(f, d.labels[i]));
        }
        const ctx = document.getElementById('chart');
        if (!c.isNull(s.chart))
        {
          s.chart.destroy();
          delete s.chart;
        }
        s.chart = new Chart(ctx, {type: 'doughnut', data: d, options: {plugins: {legend: {display: false}, title: {display: false}} }});
      }
    });
  },
  // }}}
  // {{{ template
  template: `
    <div class="float-end" style="margin: 10px; max-height: 200px;"><canvas id="chart"></canvas></div>
    <h3 class="page-header">Expenses</h3>
    {{#each f}}
    <div style="display:inline-block;">
      <div class="card">
        <div class="card-header bg-danger">
          {{@key}}
        </div>
        <div class="card-body table-responsive">
          <table class="table table-striped">
            <thead>
            <tr>
              <th>Name</th>
              <th style="text-align:right;">Amount</th>
            </tr>
            </thead>
            <tbody>
            {{#each .}}
            <tr>
              <td>{{@key}}</td>
              <td style="text-align:right;" title="{{number Amount}}">{{numberShort Amount}}</td>
            </tr>
            {{/each}}
            <tr>
              <th style="text-align:left;">Total</th>
              <th style="text-align:right;" title="{{number (genericTypeSum ../f @key)}}">{{numberShort (genericTypeSum ../f @key)}}</th>
            </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
    {{/each}}
    <div style="display:inline-block;">
      <div class="card">
        <div class="card-header bg-danger">
          Withheld
        </div>
        <div class="card-body table-responsive">
          <table class="table table-striped">
            <thead>
            <tr>
              <th>Name</th>
              <th style="text-align:right;">Amount</th>
            </tr>
            </thead>
            <tbody>
            {{#each a.d.Income.Employment}}
            <tr>
              <td>{{@key}}</td>
              <td style="text-align:right;" title="{{number (incomeEmploymentWithheld @key)}}">{{numberShort (incomeEmploymentWithheld @key)}}</td>
            </tr>
            {{/each}}
            {{#if a.d.Assumption.Tithe}}
            <tr>
              <td>Tithe</td>
              <td style="text-align:right;" title="{{number (multiply (incomeEmploymentSum) (divide a.d.Assumption.Tithe 100))}}">{{numberShort (multiply (incomeEmploymentSum) (divide a.d.Assumption.Tithe 100))}}</td>
            </tr>
            {{/if}}
            <tr>
              <th style="text-align:left;">Total</th>
              <th style="text-align:right;" title="{{number (add (incomeEmploymentWithheldSum) (multiply (incomeEmploymentSum) (divide a.d.Assumption.Tithe 100)))}}">{{numberShort (add (incomeEmploymentWithheldSum) (multiply (incomeEmploymentSum) (divide a.d.Assumption.Tithe 100)))}}</th>
            </tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>
    <p>
      <b>Total:  <span title="{{number (expenseSum)}}">{{numberShort (expenseSum)}}</span></b>
    </p>
  `
  // }}}
}
