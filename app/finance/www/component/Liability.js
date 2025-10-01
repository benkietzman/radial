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
    <div style="display:inline-grid;">
      {{#ifCond @key '==' 'Compound'}}
      <div class="card border border-danger-subtle" style="margin-top: 10px;">
        <div class="card-header bg-danger fw-bold">
          {{@key}}
        </div>
        <div class="card-body bg-danger-subtle table-responsive" style="max-height: 200px; padding: 0px;">
          <table class="table table-striped">
            <thead style="position: sticky; top: 0;">
            <tr>
              <th class="bg-danger-subtle">Name</th>
              <th class="bg-danger-subtle text-end">Principal</th>
              <th class="bg-danger-subtle text-end">Rate</th>
              <th class="bg-danger-subtle text-end">Compoundings (#/yr)</th>
              <th class="bg-danger-subtle text-end">Payment</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td style="background: inherit;">{{@key}}</td>
              <td class="text-end" style="background: inherit;" title="{{number Principal}}">{{numberShort Principal}}</td>
              <td class="text-end" style="background: inherit;">{{Rate}}%</td>
              <td class="text-end" style="background: inherit;">{{Compoundings}}</td>
              <td class="text-end" style="background: inherit;" title="{{number Payment}}">{{numberShort Payment}}</td>
            </tr>
            {{/each}}
            </tbody>
            <tfoot>
            <tr style="position: sticky; bottom: 0;">
              <th class="bg-danger-subtle" style="background: inherit; text-align:left;">Total</th>
              <th class="bg-danger-subtle text-end" colspan="4" style="background: inherit;" title="{{number (liabilityCompoundSum)}}">{{numberShort (liabilityCompoundSum)}}</th>
            </tr>
            </tfoot>
          </table>
        </div>
      </div>
      {{else ifCond @key '==' 'Simple'}}
      <div class="card border border-danger-subtle" style="margin-top: 10px;">
        <div class="card-header bg-danger fw-bold">
          {{@key}}
        </div>
        <div class="card-body bg-danger-subtle table-responsive" style="max-height: 200px; padding: 0px;">
          <table class="table table-striped">
            <thead style="position: sticky; top: 0;">
            <tr>
              <th class="bg-danger-subtle">Name</th>
              <th class="bg-danger-subtle text-end">Principal</th>
              <th class="bg-danger-subtle text-end">Rate</th>
              <th class="bg-danger-subtle text-end">Payment</th>
              <th class="bg-danger-subtle text-end">Duration (yrs)</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td style="background: inherit;">{{@key}}</td>
              <td class="text-end" style="background: inherit;" title="{{number Principal}}">{{numberShort Principal}}</td>
              <td class="text-end" style="background: inherit;">{{Rate}}%</td>
              <td class="text-end" style="background: inherit;" title="{{number Payment}}">{{numberShort Payment}}</td>
              <td class="text-end" style="background: inherit;">{{Duration}}</td>
            </tr>
            {{/each}}
            <tr>
              <th style="background: inherit; text-align:left;">Total</th>
              <th class="text-end" colspan="4" style="background: inherit;" title="{{number (liabilitySimpleSum)}}">{{numberShort (liabilitySimpleSum)}}</th>
            </tr>
            </tbody>
          </table>
        </div>
      </div>
      {{/ifCond}}
    </div>
    {{/each}}
    <p class="fw-bold text-danger" style="margin-top: 10px;">
      Total:  <span title="{{number (liabilitySum)}}">{{numberShort (liabilitySum)}}</span>
    </p>
  `
  // }}}
}
