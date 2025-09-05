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
    let f = a.d.Asset;
    let s = c.scope('Asset',
    {
      // {{{ u()
      u: () =>
      {
        c.update('Asset');
      },
      // }}}
      a: a,
      c: c,
      chart: null,
      chartAccounts: null,
      chartCategories: null,
      chartChange: {},
      chartPrices: {},
      chartScores: null,
      chartSectors: null
    });
    c.setMenu('Asset', null);
    s.f = f;
    c.attachEvent('render', (data) =>
    {
      if (data.detail.name == 'Asset')
      {
        // {{{ chart
        let d = {labels: ['Liquid', 'Metal', 'Property', 'Stock'], datasets: [{data: []}]};
        for (let i = 0; i < d.labels.length; i++)
        {
          if (d.labels[i] == 'Metal')
          {
            d.datasets[0].data.push(a.assetMetalSum());
          }
          else if (d.labels[i] == 'Stock')
          {
            d.datasets[0].data.push(a.assetStockSum());
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
        s.chart = new Chart(ctx, {type: 'doughnut', data: d, options: {plugins: {legend: {display: false}, title: {display: true, text: 'Assets'} } } });
        // }}}
        // {{{ accounts
        let accounts = {};
        for (let key in f.Stock)
        {
          let strAccount = 'NONE';
          if (c.isDefined(f.Stock[key]['Account']) && f.Stock[key]['Account'] != '')
          {
            strAccount = f.Stock[key]['Account'];
          }
          if (!c.isDefined(accounts[strAccount]))
          {
            accounts[strAccount] = 0;
          }
          if (c.isDefined(f.Stock[key]['Price']) && f.Stock[key]['Price'] != '' && c.isDefined(f.Stock[key]['Shares']) && f.Stock[key]['Shares'] != '')
          {
            accounts[strAccount] += f.Stock[key]['Price'] * f.Stock[key]['Shares'];
          }
        }
        let dAccounts = {labels: [], datasets: [{data: []}]};
        for (let key in accounts)
        {
          dAccounts.labels.push(key);
          dAccounts.datasets[0].data.push(accounts[key]);
        }
        const ctxAccounts = document.getElementById('chartAccounts');
        if (!c.isNull(s.chartAccounts))
        {
          s.chartAccounts.destroy();
          delete s.chartAccounts;
        }
        s.chartAccounts = new Chart(ctxAccounts, {type: 'doughnut', data: dAccounts, options: {plugins: {legend: {display: false}, title: {display: true, text: 'Stock - Accounts'} } } });
        // }}}
        // {{{ categories
        let categories = {};
        for (let key in f.Stock)
        {
          let strCategory = 'NONE';
          if (c.isDefined(f.Stock[key]['Category']) && f.Stock[key]['Category'] != '' && c.isDefined(a.m_assetStockCategories[f.Stock[key]['Category']]))
          {
            strCategory = a.m_assetStockCategories[f.Stock[key]['Category']];
          }
          if (!c.isDefined(categories[strCategory]))
          {
            categories[strCategory] = 0;
          }
          if (c.isDefined(f.Stock[key]['Price']) && f.Stock[key]['Price'] != '' && c.isDefined(f.Stock[key]['Shares']) && f.Stock[key]['Shares'] != '')
          {
            categories[strCategory] += f.Stock[key]['Price'] * f.Stock[key]['Shares'];
          }
        }
        let dCategories = {labels: [], datasets: [{data: []}]};
        for (let key in categories)
        {
          dCategories.labels.push(key);
          dCategories.datasets[0].data.push(categories[key]);
        }
        const ctxCategories = document.getElementById('chartCategories');
        if (!c.isNull(s.chartCategories))
        {
          s.chartCategories.destroy();
          delete s.chartCategories;
        }
        s.chartCategories = new Chart(ctxCategories, {type: 'doughnut', data: dCategories, options: {plugins: {legend: {display: false}, title: {display: true, text: 'Stock - Categories'} } } });
        // }}}
        // {{{ change
        for (let key in f.Stock)
        {
          let dChange = {labels: [], datasets: [{data: []}]};
          for (let subkey in f.Stock[key]['Change'])
          {
            dChange.labels.push(subkey);
            dChange.datasets[0].data.push(f.Stock[key]['Change'][subkey]);
          }
          const ctxChange = document.getElementById('chartChange_'+key);
          if (c.isDefined(s.chartChange[key]) && !c.isNull(s.chartChange[key]))
          {
            s.chartChange[key].destroy();
            delete s.chartChange[key];
          }
          s.chartChange[key] = new Chart(ctxChange, {type: 'line', data: dChange, options: {elements: {point: {radius: 0}}, plugins: {legend: {display: false}, title: {display: false} }, scales: {x: {display: false}, y: {display: false}} } });
          let dPrices = {labels: [], datasets: [{data: []}]};
          for (let subkey in f.Stock[key]['Prices'])
          {
            dPrices.labels.push(subkey);
            dPrices.datasets[0].data.push(f.Stock[key]['Prices'][subkey]);
          }
          const ctxPrices = document.getElementById('chartPrices_'+key);
          if (c.isDefined(s.chartPrices[key]) && !c.isNull(s.chartPrices[key]))
          {
            s.chartPrices[key].destroy();
            delete s.chartPrices[key];
          }
          s.chartPrices[key] = new Chart(ctxPrices, {type: 'line', data: dPrices, options: {elements: {point: {radius: 0}}, plugins: {legend: {display: false}, title: {display: false} }, scales: {x: {display: false}, y: {display: false}} } });
        }
        // }}}
        // {{{ score
        let scores = {};
        for (let key in f.Stock)
        {
          let strScore = 'NONE';
          if (c.isDefined(f.Stock[key]['Score']) && f.Stock[key]['Score'] != '' && c.isDefined(a.m_assetStockScores[f.Stock[key]['Score']]))
          {
            strScore = a.m_assetStockScores[f.Stock[key]['Score']];
          }
          if (!c.isDefined(scores[strScore]))
          {
            scores[strScore] = 0;
          }
          if (c.isDefined(f.Stock[key]['Price']) && f.Stock[key]['Price'] != '' && c.isDefined(f.Stock[key]['Shares']) && f.Stock[key]['Shares'] != '')
          {
            scores[strScore] += f.Stock[key]['Price'] * f.Stock[key]['Shares'];
          }
        }
        let dScores = {labels: [], datasets: [{data: []}]};
        for (let key in scores)
        {
          dScores.labels.push(key);
          dScores.datasets[0].data.push(scores[key]);
        };
        const ctxScores = document.getElementById('chartScores');
        if (!c.isNull(s.chartScores))
        {
          s.chartScores.destroy();
          delete s.chartScores;
        }
        s.chartScores = new Chart(ctxScores, {type: 'doughnut', data: dScores, options: {plugins: {legend: {display: false}, title: {display: true, text: 'Stock - Scores'} } } });
        // }}}
        // {{{ sectors
        let sectors = {};
        for (let key in f.Stock)
        {
          let strSector = 'NONE';
          if (c.isDefined(f.Stock[key]['Sector']) && f.Stock[key]['Sector'] != '' && c.isDefined(a.m_assetStockSectors[f.Stock[key]['Sector']]))
          {
            strSector = a.m_assetStockSectors[f.Stock[key]['Sector']];
          }
          if (!c.isDefined(sectors[strSector]))
          {
            sectors[strSector] = 0;
          }
          if (c.isDefined(f.Stock[key]['Price']) && f.Stock[key]['Price'] != '' && c.isDefined(f.Stock[key]['Shares']) && f.Stock[key]['Shares'] != '')
          {
            sectors[strSector] += f.Stock[key]['Price'] * f.Stock[key]['Shares'];
          }
        }
        let dSectors = {labels: [], datasets: [{data: []}]};
        for (let key in sectors)
        {
          dSectors.labels.push(key);
          dSectors.datasets[0].data.push(sectors[key]);
        };
        const ctxSectors = document.getElementById('chartSectors');
        if (!c.isNull(s.chartSectors))
        {
          s.chartSectors.destroy();
          delete s.chartSectors;
        }
        s.chartSectors = new Chart(ctxSectors, {type: 'doughnut', data: dSectors, options: {plugins: {legend: {display: false}, title: {display: true, text: 'Stock - Sectors'} } } });
        // }}}
      }
    });
  },
  // }}}
  // {{{ template
  template: `
    <div class="float-end" style="margin: 10px; max-height: 200px;"><canvas id="chartSectors"></canvas></div>
    <div class="float-end" style="margin: 10px; max-height: 200px;"><canvas id="chartScores"></canvas></div>
    <div class="float-end" style="margin: 10px; max-height: 200px;"><canvas id="chartCategories"></canvas></div>
    <div class="float-end" style="margin: 10px; max-height: 200px;"><canvas id="chartAccounts"></canvas></div>
    <div class="float-end" style="margin: 10px; max-height: 200px;"><canvas id="chart"></canvas></div>
    <h3 class="page-header">Assets</h3>
    {{#each f}}
    <div style="display:inline-grid;">
      {{#ifCond @key '==' 'Metal'}}
      <div class="card border border-success-subtle" style="margin-top: 10px;">
        <div class="card-header bg-success fw-bold">
          {{@key}}
        </div>
        <div class="card-body bg-success-subtle table-responsive" style="max-height: 200px; padding: 0px;">
          <table class="table table-striped">
            <thead style="position: sticky; top: 0;">
            <tr>
              <th class="bg-success-subtle">Name</th>
              <th class="bg-success-subtle text-end">Quantity</th>
              <th class="bg-success-subtle text-end">$/item</th>
              <th class="bg-success-subtle text-end">Value</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td style="background: inherit;">{{@key}}</td>
              <td class="text-end" style="background: inherit;" title="{{number Quantity}}">{{numberShort Quantity}}</td>
              <td class="text-end" style="background: inherit;" title="{{number Price}}">{{numberShort Price}}</td>
              <td class="text-end" style="background: inherit;" title="{{number (multiply Quantity Price)}}">{{numberShort (multiply Quantity Price)}}</td>
            </tr>
            {{/each}}
            <tr>
              <th class="text-end" style="background: inherit;">Total</th>
              <th class="text-end" colspan="3" style="background: inherit;" title="{{number (assetMetalSum)}}">{{numberShort (assetMetalSum)}}</th>
            </tr>
            </tbody>
          </table>
        </div>
      </div>
      {{else ifCond @key '==' 'Stock'}}
      <div class="card border border-success-subtle" style="margin-top: 10px;">
        <div class="card-header bg-success fw-bold">
          {{@key}}
        </div>
        <div class="card-body bg-success-subtle table-responsive" style="max-height: 500px; padding: 0px;">
          <table id="stock" class="table table-striped">
            <thead style="position: sticky; top: 0;">
            <tr>
              <th c-click="c.tableSort('stock', 0)" class="bg-success-subtle" style="cursor: ns-resize; left: 0; position: sticky;" title="Stock Symbol">Stk</th>
              <th c-click="c.tableSort('stock', 1)" class="bg-success-subtle" style="cursor: ns-resize;" title="Account">Acct</th>
              <th c-click="c.tableSort('stock', 2)" class="bg-success-subtle" style="cursor: ns-resize;" title="Position Category">Cat</th>
              <th c-click="c.tableSort('stock', 3, true)" class="bg-success-subtle" style="cursor: ns-resize;" title="Score">Scr</th>
              <th c-click="c.tableSort('stock', 4)" class="bg-success-subtle" style="cursor: ns-resize;" title="Sector Classification">Sector</th>
              <th c-click="c.tableSort('stock', 5)" class="bg-success-subtle" style="cursor: ns-resize; white-space: nowrap;" title="Birth Year">Birth</th>
              <th c-click="c.tableSort('stock', 6, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="# of Shares of stock">Shares</th>
              <th c-click="c.tableSort('stock', 7, true)" class="bg-success-subtle text-end" style="cursor: ns-resize; white-space: nowrap;" title="1-Year Average Price per Share of stock">x̄ Price</th>
              <th c-click="c.tableSort('stock', 8, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Price per Share of stock">Price</th>
              <th c-click="c.tableSort('stock', 9)" class="bg-success-subtle text-end" style="cursor: ns-resize; white-space: nowrap;" title="5-Year Price Change">δ Price</th>
              <th c-click="c.tableSort('stock', 10)" class="bg-success-subtle text-end" style="cursor: ns-resize; white-space: nowrap;" title="5-Year Dividend Change">δ Div</th>
              <th c-click="c.tableSort('stock', 11, true)" class="bg-success-subtle text-end" style="cursor: ns-resize; white-space: nowrap;" title="1-Year Average Dividend per Share">x̄ Div</th>
              <th c-click="c.tableSort('stock', 12, true)" class="bg-success-subtle text-end" style="cursor: ns-resize; white-space: nowrap;" title="1-Year Average Dividend Yield">x̄ Yield</th>
              <th c-click="c.tableSort('stock', 13, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Latest Dividend per Share">Div</th>
              <th c-click="c.tableSort('stock', 14, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Dividend Yield">Yield</th>
              <th c-click="c.tableSort('stock', 15, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Adjusted Dividend Yield">AdjYld</th>
              <th c-click="c.tableSort('stock', 16, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Current Allocation">Alloc</th>
              <th c-click="c.tableSort('stock', 17, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Current Value">Value</th>
              <th c-click="c.tableSort('stock', 18, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Target Allocation">TgtA</th>
              <th c-click="c.tableSort('stock', 19, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Target Value">TgtV</th>
              <th c-click="c.tableSort('stock', 20, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Gap between Current Value and Target Value">Gap</th>
              <th c-click="c.tableSort('stock', 21, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Annual Dividend">Div</th>
              <th c-click="c.tableSort('stock', 22, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Dividend received in Jan, Apr, Jul, and Oct">JAJO</th>
              <th c-click="c.tableSort('stock', 23, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Dividend received in Feb, May, Aug, and Nov">FMAN</th>
              <th c-click="c.tableSort('stock', 24, true)" class="bg-success-subtle text-end" style="cursor: ns-resize;" title="Dividend received in Mar, Jun, Sep, and Dev">MJSD</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td class="bg-success-subtle" style="left: 0; position: sticky;">{{@key}}</td>
              <td style="background: inherit;">{{Account}}</td>
              <td style="background: inherit;">{{indexOf ../../../a.m_assetStockCategories Category}}</td>
              <td style="background: inherit;">{{indexOf ../../../a.m_assetStockScores Score}}</td>
              <td style="background: inherit;" title="{{indexOf ../../../a.m_assetStockSectors Sector}}">{{truncate (indexOf ../../../a.m_assetStockSectors Sector) 4 true}}</td>
              <td style="background: inherit;"">{{dateyear FirstTrade}}</td>
              <td class="text-end" style="background: inherit;" title="{{number Shares}}">{{numberShort Shares}}</td>
              <td class="text-end" style="background: inherit;" title="{{number AveragePrice}}">{{numberShort AveragePrice}}</td>
              <td class="text-end {{#ifCond ChangePrice "<" 0}}text-danger{{/ifCond}}{{#ifCond ChangePrice "<=" -10}} fw-bold{{/ifCond}}{{#ifCond ChangePrice "<=" -25}} fw-bolder{{/ifCond}}{{#ifCond ChangePrice  "<=" -50}} fst-italic{{/ifCond}}{{#ifCond ChangePrice ">" 0}}text-success{{/ifCond}}{{#ifCond ChangePrice ">=" 10}} fw-bold{{/ifCond}}{{#ifCond ChangePrice ">=" 25}} fw-bolder{{/ifCond}}{{#ifCond ChangePrice ">=" 50}} fst-italic{{/ifCond}}" style="background: inherit;" title="{{number Price}} ({{number ChangePrice}}%)">{{numberShort Price}}</td>
              <td style="background: inherit;"><div style="max-height: 20px; max-width: 40px;"><canvas id="chartPrices_{{@key}}"></canvas></div></td>
              <td style="background: inherit;"><div style="max-height: 20px; max-width: 40px;"><canvas id="chartChange_{{@key}}"></canvas></div></td>
              <td class="text-end" style="background: inherit;" title="{{number Dividend}}">{{numberShort Dividend}}</td>
              <td class="text-end" style="background: inherit;" title="{{number (multiply (divide Dividend Price) 100)}}%">{{numberShort (multiply (divide Dividend Price) 100)}}%</td>
              <td class="text-end {{#ifCond ChangeDividend "<" 0}}text-danger{{/ifCond}}{{#ifCond ChangeDividend "<=" -10}} fw-bold{{/ifCond}}{{#ifCond ChangeDividend "<=" -25}} fw-bolder{{/ifCond}}{{#ifCond ChangeDividend  "<=" -50}} fst-italic{{/ifCond}}{{#ifCond ChangeDividend ">" 0}}text-success{{/ifCond}}{{#ifCond ChangeDividend ">=" 10}} fw-bold{{/ifCond}}{{#ifCond ChangeDividend ">=" 25}} fw-bolder{{/ifCond}}{{#ifCond ChangeDividend ">=" 50}} fst-italic{{/ifCond}}" style="background: inherit;" title="{{number DividendLatest}} ({{number ChangeDividend}}%)">{{numberShort DividendLatest}}</td>
              <td class="text-end" style="background: inherit;" title="{{number (multiply (divide DividendLatest Price) 100)}}%">{{numberShort (multiply (divide DividendLatest Price) 100)}}%</td>
              <td class="text-end" style="background: inherit;" title="{{number (assetStockYieldAdjust @key)}}%">{{numberShort (assetStockYieldAdjust @key)}}%</td>
              <td class="text-end" style="background: inherit;" title="{{number (divide (multiply (multiply Shares Price) 100) (assetStockSum))}}%">{{numberShort (divide (multiply (multiply Shares Price) 100) (assetStockSum))}}%</td>
              <td class="text-end" style="background: inherit;" title="{{number (multiply Shares Price)}}">{{numberShort (multiply Shares Price)}}</td>
              <td class="text-end" style="background: inherit;" title="{{number (multiply (divide (divide (assetStockYieldAdjust @key) 100) (divide (assetStockYieldAdjustSum) 100)) 100)}}%">{{numberShort (multiply (divide (divide (assetStockYieldAdjust @key) 100) (divide (assetStockYieldAdjustSum) 100)) 100)}}%</td>
              <td class="text-end" style="background: inherit;" title="{{number (multiply (divide (divide (assetStockYieldAdjust @key) 100) (divide (assetStockYieldAdjustSum) 100)) (assetStockSum))}}">{{numberShort (multiply (divide (divide (assetStockYieldAdjust @key) 100) (divide (assetStockYieldAdjustSum) 100)) (assetStockSum))}}</td>
              <td class="text-end text-{{#ifCond (subtract (multiply Shares Price) (multiply (divide (divide (assetStockYieldAdjust @key) 100) (divide (assetStockYieldAdjustSum) 100)) (assetStockSum))) '>=' 0}}success{{else}}danger{{/ifCond}}" style="background: inherit;" title="{{number (subtract (multiply Shares Price) (multiply (divide (divide (assetStockYieldAdjust @key) 100) (divide (assetStockYieldAdjustSum) 100)) (assetStockSum)))}}">{{numberShort (subtract (multiply Shares Price) (multiply (divide (divide (assetStockYieldAdjust @key) 100) (divide (assetStockYieldAdjustSum) 100)) (assetStockSum)))}}</td>
              <td class="text-end" style="background: inherit;" title="{{number (assetStockDividend @key @root.a.d.Assumption.DividendSpan)}}">{{numberShort (assetStockDividend @key @root.a.d.Assumption.DividendSpan)}}</td>
              <td class="text-end" style="background: inherit;" title="{{number (assetStockReceive @key 1)}}">{{numberShort (assetStockReceive @key 1)}}</td>
              <td class="text-end" style="background: inherit;" title="{{number (assetStockReceive @key 2)}}">{{numberShort (assetStockReceive @key 2)}}</td>
              <td class="text-end" style="background: inherit;" title="{{number (assetStockReceive @key 3)}}">{{numberShort (assetStockReceive @key 3)}}</td>
            </tr>
            {{/each}}
            </tbody>
            <tfoot>
            <tr>
              <th class="bg-success-subtle" style="left: 0; position: sticky; text-align:left;">Total</th>
              <th class="text-end" colspan="12" style="background: inherit;" title="{{number (divide (multiply (assetStockDividendSum '1-year') 100) (assetStockSum))}}%">{{numberShort (divide (multiply (assetStockDividendSum '1-year') 100) (assetStockSum))}}%</th>
              <th class="text-end" colspan="2" style="background: inherit;" title="{{number (divide (multiply (assetStockDividendSum 'latest') 100) (assetStockSum))}}%">{{numberShort (divide (multiply (assetStockDividendSum 'latest') 100) (assetStockSum))}}%</th>
              <th class="text-end" colspan="3" style="background: inherit;" title="{{number (assetStockSum)}}">{{numberShort (assetStockSum)}}</th>
              <th class="text-end" colspan="4" style="background: inherit;" title="{{number (assetStockDividendSum @root.a.d.Assumption.DividendSpan)}}">{{numberShort (assetStockDividendSum @root.a.d.Assumption.DividendSpan)}}</th>
              <th class="text-end" style="background: inherit;" title="{{number (assetStockReceiveSum 1)}}">{{numberShort (assetStockReceiveSum 1)}}</th>
              <th class="text-end" style="background: inherit;" title="{{number (assetStockReceiveSum 2)}}">{{numberShort (assetStockReceiveSum 2)}}</th>
              <th class="text-end" style="background: inherit;" title="{{number (assetStockReceiveSum 3)}}">{{numberShort (assetStockReceiveSum 3)}}</th>
            </tr>
            </tfoot>
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
              <th style="background: inherit;text-align: left;">Total</th>
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
      Total:  <span title="{{number (assetSum)}}">{{numberShort (assetSum)}}</span>
    </p>
  `
  // }}}
}
