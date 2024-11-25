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
    let f = a.d.Income;
    let s = c.scope('Income',
    {
      // {{{ u()
      u: () =>
      {
        c.update('Income');
      },
      // }}}
      a: a,
      c: c,
      chart: null
    });
    c.setMenu('Income', null);
    s.f = f;
    c.attachEvent('render', (data) =>
    {
      if (data.detail.name == 'Income')
      {
        let d = {labels: ['Employment', 'Welfare'], datasets: [{data: []}]};
        for (let i = 0; i < d.labels.length; i++)
        {
          if (d.labels[i] == 'Employment')
          {
            d.datasets[0].data.push(a.incomeEmploymentSum());
          }
          else
          {
            d.datasets[0].data.push(a.genericTypeSum(f, d.labels[i]));
          }
        }
        const ctx = document.getElementById('chart');
        if (!c.isNull(s.chart))
        {
          s.chart.destroy();
          delete s.chart;
        }
        s.chart = new Chart(ctx, {type: 'doughnut', data: d, options: {plugins: {legend: {display: false}, title: {display: false}} } });
      }
    });
  },
  // }}}
  // {{{ template
  template: `
    <div class="float-end" style="margin: 10px; max-height: 200px;"><canvas id="chart"></canvas></div>
    <h3 class="page-header">Incomes</h3>
    {{#each f}}
    <div style="display:inline-block;">
      {{#ifCond @key '==' 'Employment'}}
      <div class="card">
        <div class="card-header bg-success">
          {{@key}}
        </div>
        <div class="card-body table-responsive">
          <table class="table table-striped">
            <thead>
            <tr>
              <th title="Name of Employer">Employer</th>
              <th style="text-align:right;" title="Salary">Salary</th>
              <th style="text-align:right;" title="Bonus">Bonus</th>
              <th style="text-align:right;" title="Investment Percentage">Invest</th>
              <th style="text-align:right;" title="Employer Match Percentage">Match</th>
              <th style="text-align:right;" title="Gross Income">Gross</th>
              <th style="text-align:right;" title="Health Savings Account">HSA</th>
              <th style="text-align:right;" title="Medical Premium">Medical</th>
              <th style="text-align:right;" title="Withholding Percentage">Tax</th>
              <th style="text-align:right;" title="Net Income">Net</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td>{{@key}}</td>
              <td style="text-align:right;" title="{{number Salary}}">{{numberShort Salary}}</td>
              <td style="text-align:right;" title="{{number Bonus}}">{{numberShort Bonus}}</td>
              <td style="text-align:right;">{{Invest}}%</td>
              <td style="text-align:right;">{{Match}}%</td>
              <td style="text-align:right;" title="{{number (incomeEmployment @key)}}">{{numberShort (incomeEmployment @key)}}</td>
              <td style="text-align:right;" title="{{number Hsa}}">{{numberShort Hsa}}</td>
              <td style="text-align:right;" title="{{number Medical}}">{{numberShort Medical}}</td>
              <td style="text-align:right;">{{Tax}}%</td>
              <td style="text-align:right;" title="{{number (subtract (incomeEmployment @key) (incomeEmploymentWithheld @key))}}">{{numberShort (subtract (incomeEmployment @key) (incomeEmploymentWithheld @key))}}</td>
            </tr>
            {{/each}}
            <tr>
              <th colspan="6" style="text-align:right;" title="{{number (incomeEmploymentSum)}}">{{numberShort (incomeEmploymentSum)}}</th>
              <th colspan="4" style="text-align:right;" title="{{number (subtract (incomeEmploymentSum) (incomeEmploymentWithheldSum))}}">{{numberShort (subtract (incomeEmploymentSum) (incomeEmploymentWithheldSum))}}</th>
            </tr>
            </tbody>
          </table>
        </div>
      </div>
      {{else}}
      <div class="card">
        <div class="card-header bg-success">
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
            {{#each ../.}}
            <tr">
              <td>{{@key}}</td>
              <td style="text-align:right;" title="{{number Amount}}">{{numberShort Amount}}</td>
            </tr>
            {{/each}}
            <tr>
              <th style="text-align:left;">Total</th>
              <th style="text-align:right;" title="{{number (genericTypeSum ../../f @key)}}">{{numberShort (genericTypeSum ../../f @key)}}</th>
            </tr>
            </tbody>
          </table>
        </div>
      </div>
      {{/ifCond}}
    </div>
    {{/each}}
    <p>
      <b>Total:  <span title="{{number (incomeSum)}}">{{numberShort (incomeSum)}}</span></b>
    </p>
  `
  // }}}
}
