// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-10-13
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
    <div style="display:inline-grid;">
      {{#ifCond @key '==' 'Employment'}}
      <div class="card border border-success-subtle" style="margin-top: 10px;">
        <div class="card-header bg-success fw-bold">
          {{@key}}
        </div>
        <div class="card-body bg-success-subtle table-responsive" style="padding: 0px;">
          <table class="table table-striped">
            <thead>
            <tr>
              <th style="background: inherit;" title="Name of Employer">Employer</th>
              <th style="background: inherit; text-align:right;" title="Salary">Salary</th>
              <th style="background: inherit; text-align:right;" title="Bonus">Bonus</th>
              <th style="background: inherit; text-align:right;" title="Investment Percentage">Invest</th>
              <th style="background: inherit; text-align:right;" title="Employer Match Percentage">Match</th>
              <th style="background: inherit; text-align:right;" title="Gross Income">Gross</th>
              <th style="background: inherit; text-align:right;" title="Health Savings Account">HSA</th>
              <th style="background: inherit; text-align:right;" title="Medical Premium">Medical</th>
              <th style="background: inherit; text-align:right;" title="Withholding Percentage">Tax</th>
              <th style="background: inherit; text-align:right;" title="Net Income">Net</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td style="background: inherit;">{{@key}}</td>
              <td style="background: inherit; text-align:right;" title="{{number Salary}}">{{numberShort Salary}}</td>
              <td style="background: inherit; text-align:right;" title="{{number Bonus}}">{{numberShort Bonus}}</td>
              <td style="background: inherit; text-align:right;">{{Invest}}%</td>
              <td style="background: inherit; text-align:right;">{{Match}}%</td>
              <td style="background: inherit; text-align:right;" title="{{number (incomeEmployment @key)}}">{{numberShort (incomeEmployment @key)}}</td>
              <td style="background: inherit; text-align:right;" title="{{number Hsa}}">{{numberShort Hsa}}</td>
              <td style="background: inherit; text-align:right;" title="{{number Medical}}">{{numberShort Medical}}</td>
              <td style="background: inherit; text-align:right;">{{Tax}}%</td>
              <td style="background: inherit; text-align:right;" title="{{number (subtract (incomeEmployment @key) (incomeEmploymentWithheld @key))}}">{{numberShort (subtract (incomeEmployment @key) (incomeEmploymentWithheld @key))}}</td>
            </tr>
            {{/each}}
            <tr>
              <th colspan="6" style="background: inherit; text-align:right;" title="{{number (incomeEmploymentSum)}}">{{numberShort (incomeEmploymentSum)}}</th>
              <th colspan="4" style="background: inherit; text-align:right;" title="{{number (subtract (incomeEmploymentSum) (incomeEmploymentWithheldSum))}}">{{numberShort (subtract (incomeEmploymentSum) (incomeEmploymentWithheldSum))}}</th>
            </tr>
            </tbody>
          </table>
        </div>
      </div>
      {{else}}
      <div class="card border border-success-subtle" style="margin-top: 10px;">
        <div class="card-header bg-success fw-bold">
          {{@key}}
        </div>
        <div class="card-body bg-success-subtle table-responsive" style="padding: 0px;">
          <table class="table table-striped">
            <thead>
            <tr>
              <th style="background: inherit;">Name</th>
              <th style="background: inherit; text-align:right;">Amount</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr">
              <td style="background: inherit;">{{@key}}</td>
              <td style="background: inherit; text-align:right;" title="{{number Amount}}">{{numberShort Amount}}</td>
            </tr>
            {{/each}}
            <tr>
              <th style="background: inherit; text-align:left;">Total</th>
              <th style="background: inherit; text-align:right;" title="{{number (genericTypeSum ../../f @key)}}">{{numberShort (genericTypeSum ../../f @key)}}</th>
            </tr>
            </tbody>
          </table>
        </div>
      </div>
      {{/ifCond}}
    </div>
    {{/each}}
    <p class="fw-bold text-success" style="margin-top: 10px;">
      Total:  <span title="{{number (incomeSum)}}">{{numberShort (incomeSum)}}</span>
    </p>
  `
  // }}}
}
