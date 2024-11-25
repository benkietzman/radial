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
    let f = a.d.Liability;
    let s = c.scope('Liability',
    {
      // {{{ u()
      u: () =>
      {
        c.update('Liability');
      },
      // }}}
      a: a,
      c: c,
      chart: null
    });
    c.setMenu('Liability', null);
    s.f = f;
    c.attachEvent('render', (data) =>
    {
      if (data.detail.name == 'Liability')
      {
        let d = {labels: ['Compound', 'Simple'], datasets: [{data: []}]};
        d.datasets[0].data.push(a.liabilityCompoundSum());
        d.datasets[0].data.push(a.liabilitySimpleSum());
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
    <h3 class="page-header">Liabilities</h3>
    {{#each f}}
    <div style="display:inline-block;">
      {{#ifCond @key '==' 'Compound'}}
      <div class="card border border-danger-subtle">
        <div class="card-header bg-danger fw-bold">
          {{@key}}
        </div>
        <div class="card-body bg-danger-subtle table-responsive" style="padding: 0px;">
          <table class="table table-striped">
            <thead>
            <tr>
              <th style="background: inherit;">Name</th>
              <th style="background: inherit; text-align:right;">Principal</th>
              <th style="background: inherit; text-align:right;">Rate</th>
              <th style="background: inherit; text-align:right;">Compoundings (#/yr)</th>
              <th style="background: inherit; text-align:right;">Payment</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td style="background: inherit;">{{@key}}</td>
              <td style="background: inherit; text-align:right;" title="{{number Principal}}">{{numberShort Principal}}</td>
              <td style="background: inherit; text-align:right;">{{Rate}}%</td>
              <td style="background: inherit; text-align:right;">{{Compoundings}}</td>
              <td style="background: inherit; text-align:right;" title="{{number Payment}}">{{numberShort Payment}}</td>
            </tr>
            {{/each}}
            <tr>
              <th style="background: inherit; text-align:left;">Total</th>
              <th colspan="4" style="background: inherit; text-align:right;" title="{{number (liabilityCompoundSum)}}">{{numberShort (liabilityCompoundSum)}}</th>
            </tr>
            </tbody>
          </table>
        </div>
      </div>
      {{else ifCond @key '==' 'Simple'}}
      <div class="card border border-danger-subtle">
        <div class="card-header bg-danger fw-bold">
          {{@key}}
        </div>
        <div class="card-body bg-danger-subtle table-responsive" style="padding: 0px;">
          <table class="table table-striped">
            <thead>
            <tr>
              <th style="background: inherit;">Name</th>
              <th style="background: inherit; text-align:right;">Principal</th>
              <th style="background: inherit; text-align:right;">Rate</th>
              <th style="background: inherit; text-align:right;">Payment</th>
              <th style="background: inherit; text-align:right;">Duration (yrs)</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td style="background: inherit;">{{@key}}</td>
              <td style="background: inherit; text-align:right;" title="{{number Principal}}">{{numberShort Principal}}</td>
              <td style="background: inherit; text-align:right;">{{Rate}}%</td>
              <td style="background: inherit; text-align:right;" title="{{number Payment}}">{{numberShort Payment}}</td>
              <td style="background: inherit; text-align:right;">{{Duration}}</td>
            </tr>
            {{/each}}
            <tr>
              <th style="background: inherit; text-align:left;">Total</th>
              <th colspan="4" style="background: inherit; text-align:right;" title="{{number (liabilitySimpleSum)}}">{{numberShort (liabilitySimpleSum)}}</th>
            </tr>
            </tbody>
          </table>
        </div>
      </div>
      {{/ifCond}}
    </div>
    {{/each}}
    <p class="fw-bold text-danger">
      Total:  <span title="{{number (liabilitySum)}}">{{numberShort (liabilitySum)}}</span>
    </p>
  `
  // }}}
}
