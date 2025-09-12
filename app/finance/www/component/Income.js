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
        <div class="card-body bg-success-subtle table-responsive" style="max-height: 200px; padding: 0px;">
          <table class="table table-striped">
            <thead style="position: sticky; top: 0;">
            <tr>
              <th class="bg-success-subtle" title="Name of Employer">Employer</th>
              <th class="bg-success-subtle text-end" title="Salary">Salary</th>
              <th class="bg-success-subtle text-end" title="Bonus">Bonus</th>
              <th class="bg-success-subtle text-end" title="Investment Percentage">Invest</th>
              <th class="bg-success-subtle text-end" title="Employer Match Percentage">Match</th>
              <th class="bg-success-subtle text-end" title="Gross Income">Gross</th>
              <th class="bg-success-subtle text-end" title="Health Savings Account">HSA</th>
              <th class="bg-success-subtle text-end" title="Medical Premium">Medical</th>
              <th class="bg-success-subtle text-end" title="Withholding Percentage">Tax</th>
              <th class="bg-success-subtle text-end" title="Net Income">Net</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td style="background: inherit;">{{@key}}</td>
              <td class="text-end" style="background: inherit;" title="{{number Salary}}">{{numberShort Salary}}</td>
              <td class="text-end" style="background: inherit;" title="{{number Bonus}}">{{numberShort Bonus}}</td>
              <td class="text-end" style="background: inherit;">{{Invest}}%</td>
              <td class="text-end" style="background: inherit;">{{Match}}%</td>
              <td class="text-end" style="background: inherit;" title="{{number (incomeEmployment @key)}}">{{numberShort (incomeEmployment @key)}}</td>
              <td class="text-end" style="background: inherit;" title="{{number Hsa}}">{{numberShort Hsa}}</td>
              <td class="text-end" style="background: inherit;" title="{{number Medical}}">{{numberShort Medical}}</td>
              <td class="text-end" style="background: inherit;">{{Tax}}%</td>
              <td class="text-end" style="background: inherit;" title="{{number (subtract (incomeEmployment @key) (incomeEmploymentWithheld @key))}}">{{numberShort (subtract (incomeEmployment @key) (incomeEmploymentWithheld @key))}}</td>
            </tr>
            {{/each}}
            <tr>
              <th style="background: inherit; text-align:left;">Total</th>
              <th class="text-end" colspan="5" style="background: inherit;" title="{{number (incomeEmploymentSum)}}">{{numberShort (incomeEmploymentSum)}}</th>
              <th class="text-end" colspan="4" style="background: inherit;" title="{{number (subtract (incomeEmploymentSum) (incomeEmploymentWithheldSum))}}">{{numberShort (subtract (incomeEmploymentSum) (incomeEmploymentWithheldSum))}}</th>
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
        <div class="card-body bg-success-subtle table-responsive" style="max-height: 200px; padding: 0px;">
          <table class="table table-striped">
            <thead style="position: sticky; top: 0;">
            <tr>
              <th class="bg-success-subtle">Name</th>
              <th class="bg-success-subtle text-end">Amount</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td style="background: inherit;">{{@key}}</td>
              <td class="text-end" style="background: inherit;" title="{{number Amount}}">{{numberShort Amount}}</td>
            </tr>
            {{/each}}
            <tr>
              <th style="background: inherit; text-align:left;">Total</th>
              <th class="text-end" style="background: inherit;" title="{{number (genericTypeSum ../../f @key)}}">{{numberShort (genericTypeSum ../../f @key)}}</th>
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
