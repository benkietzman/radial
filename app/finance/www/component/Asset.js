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
    <div style="display:inline-block;">
      {{#ifCond @key '==' 'Metal'}}
      <div class="card border border-success-subtle" style="margin-top: 10px;">
        <div class="card-header bg-success fw-bold">
          {{@key}}
        </div>
        <div class="card-body bg-success-subtle table-responsive" style="padding: 0px;">
          <table class="table table-striped">
            <thead>
            <tr>
              <th style="background: inherit;">Name</th>
              <th style="background: inherit; text-align: right;">Quantity</th>
              <th style="background: inherit; text-align: right;">$/item</th>
              <th style="background: inherit; text-align: right;">Value</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td style="background: inherit;">{{@key}}</td>
              <td style="background: inherit; text-align: right;" title="{{number Quantity}}">{{numberShort Quantity}}</td>
              <td style="background: inherit; text-align: right;" title="{{number Price}}">{{numberShort Price}}</td>
              <td style="background: inherit; text-align: right;" title="{{number (multiply Quantity Price)}}">{{numberShort (multiply Quantity Price)}}</td>
            </tr>
            {{/each}}
            <tr>
              <th style="background: inherit; text-align:left;">Total</th>
              <th colspan="3" style="background: inherit; text-align: right;" title="{{number (assetMetalSum)}}">{{numberShort (assetMetalSum)}}</th>
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
        <div class="card-body bg-success-subtle table-responsive" style="padding: 0px;">
          <table id="stock" class="table table-striped">
            <thead>
            <tr>
              <th c-click="c.tableSort('stock', 0)" style="background: inherit; cursor: ns-resize; font-size: small;" title="Stock Symbol">Stk</th>
              <th c-click="c.tableSort('stock', 1)" style="background: inherit; cursor: ns-resize; font-size: small;" title="Account">Acct</th>
              <th c-click="c.tableSort('stock', 2)" style="background: inherit; cursor: ns-resize; font-size: small;" title="Position Category">Cat</th>
              <th c-click="c.tableSort('stock', 3, true)" style="background: inherit; cursor: ns-resize; font-size: small;" title="Score">Scr</th>
              <th c-click="c.tableSort('stock', 4)" style="background: inherit; cursor: ns-resize; font-size: small;" title="Sector Classification">Sector</th>
              <th c-click="c.tableSort('stock', 5, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="# of Shares of stock">Shares</th>
              <th c-click="c.tableSort('stock', 6, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Price per Share of stock">Price</th>
              <th c-click="c.tableSort('stock', 7)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right; white-space: nowrap;" title="5-Year Dividend Change">δ Div</th>
              <th c-click="c.tableSort('stock', 8, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right; white-space: nowrap;" title="1-Year Average Dividend per Share">x̄ Div</th>
              <th c-click="c.tableSort('stock', 9, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right; white-space: nowrap;" title="1-Year Average Dividend Yield">x̄ Yield</th>
              <th c-click="c.tableSort('stock', 10, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Latest Dividend per Share">Div</th>
              <th c-click="c.tableSort('stock', 11, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Dividend Yield">Yield</th>
              <th c-click="c.tableSort('stock', 12, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Current Allocation">Alloc</th>
              <th c-click="c.tableSort('stock', 13, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Current Value">Value</th>
              <th c-click="c.tableSort('stock', 14, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Target Allocation">TgtA</th>
              <th c-click="c.tableSort('stock', 15, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Target Value">TgtV</th>
              <th c-click="c.tableSort('stock', 16, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Gap between Current Value and Target Value">Gap</th>
              <th c-click="c.tableSort('stock', 17, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Annual Dividend">Div</th>
              <th c-click="c.tableSort('stock', 18, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Dividend received in Jan, Apr, Jul, and Oct">JAJO</th>
              <th c-click="c.tableSort('stock', 19, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Dividend received in Feb, May, Aug, and Nov">FMAN</th>
              <th c-click="c.tableSort('stock', 20, true)" style="background: inherit; cursor: ns-resize; font-size: small; text-align: right;" title="Dividend received in Mar, Jun, Sep, and Dev">MJSD</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td style="background: inherit; font-size: small;">{{@key}}</td>
              <td style="background: inherit; font-size: small;">{{Account}}</td>
              <td style="background: inherit; font-size: small;">{{#indexOf ../../../a.m_assetStockCategories Category}}{{/indexOf}}</td>
              <td style="background: inherit; font-size: small;">{{#indexOf ../../../a.m_assetStockScores Score}}{{/indexOf}}</td>
              <td style="background: inherit; font-size: small;">{{#indexOf ../../../a.m_assetStockSectors Sector}}{{/indexOf}}</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number Shares}}">{{numberShort Shares}}</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number Price}}">{{numberShort Price}}</td>
              <td style="background: inherit;"><div style="max-height: 20px;"><canvas id="chartChange_{{@key}}"></canvas></div></td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number Dividend}}">{{numberShort Dividend}}</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number (multiply (divide Dividend Price) 100)}}%">{{numberShort (multiply (divide Dividend Price) 100)}}%</td>
              <td class="{{#ifCond ChangeDividend "<" 0}}text-danger{{/ifCond}}{{#ifCond ChangeDividend "<=" -10}} fw-bold{{/ifCond}}{{#ifCond ChangeDividend "<=" -25}} fw-bolder{{/ifCond}}{{#ifCond ChangeDividend  "<=" -50}} fst-italic{{/ifCond}}{{#ifCond ChangeDividend ">" 0}}text-success{{/ifCond}}{{#ifCond ChangeDividend ">=" 10}} fw-bold{{/ifCond}}{{#ifCond ChangeDividend ">=" 25}} fw-bolder{{/ifCond}}{{#ifCond ChangeDividend ">=" 50}} fst-italic{{/ifCond}}" style="background: inherit; font-size: small; text-align: right;" title="{{number LatestDividend}} ({{number ChangeDividend}}%)">{{numberShort LatestDividend}}</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number (multiply (divide LatestDividend Price) 100)}}%">{{numberShort (multiply (divide LatestDividend Price) 100)}}%</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number (divide (multiply (multiply Shares Price) 100) (assetStockSum))}}%">{{numberShort (divide (multiply (multiply Shares Price) 100) (assetStockSum))}}%</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number (multiply Shares Price)}}">{{numberShort (multiply Shares Price)}}</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number (multiply (divide (divide Dividend Price) (divide (assetStockYieldSum) 100)) 100)}}%">{{numberShort (multiply (divide (divide Dividend Price) (divide (assetStockYieldSum) 100)) 100)}}%</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number (multiply (divide (divide Dividend Price) (divide (assetStockYieldSum) 100)) (assetStockSum))}}">{{numberShort (multiply (divide (divide Dividend Price) (divide (assetStockYieldSum) 100)) (assetStockSum))}}</td>
              <td class="text-{{#ifCond (subtract (multiply Shares Price) (multiply (divide (divide Dividend Price) (divide (assetStockYieldSum) 100)) (assetStockSum))) '>=' 0}}success{{else}}danger{{/ifCond}}" style="background: inherit; font-size: small; text-align: right;" title="{{number (subtract (multiply Shares Price) (multiply (divide (divide Dividend Price) (divide (assetStockYieldSum) 100)) (assetStockSum)))}}">{{numberShort (subtract (multiply Shares Price) (multiply (divide (divide Dividend Price) (divide (assetStockYieldSum) 100)) (assetStockSum)))}}</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number (multiply Dividend Shares)}}">{{numberShort (multiply Dividend Shares)}}</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number (assetStockReceive Shares Dividend LatestDividend Receive 1 @root.a.d.Assumption.DividendSpan)}}">{{numberShort (assetStockReceive Shares Dividend LatestDividend Receive 1 @root.a.d.Assumption.DividendSpan)}}</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number (assetStockReceive Shares Dividend LatestDividend Receive 2 @root.a.d.Assumption.DividendSpan)}}">{{numberShort (assetStockReceive Shares Dividend LatestDividend Receive 2 @root.a.d.Assumption.DividendSpan)}}</td>
              <td style="background: inherit; font-size: small; text-align: right;" title="{{number (assetStockReceive Shares Dividend LatestDividend Receive 3 @root.a.d.Assumption.DividendSpan)}}">{{numberShort (assetStockReceive Shares Dividend LatestDividend Receive 3 @root.a.d.Assumption.DividendSpan)}}</td>
            </tr>
            {{/each}}
            </tbody>
            <tfoot>
            <tr>
              <th style="background: inherit; font-size: small; text-align:left;">Total</th>
              <th colspan="9" style="background: inherit; font-size: small; text-align: right;" title="{{number (divide (multiply (assetStockDividendSum '1-year') 100) (assetStockSum))}}%">{{numberShort (divide (multiply (assetStockDividendSum '1-year') 100) (assetStockSum))}}%</th>
              <th colspan="2" style="background: inherit; font-size: small; text-align: right;" title="{{number (divide (multiply (assetStockDividendSum 'latest') 100) (assetStockSum))}}%">{{numberShort (divide (multiply (assetStockDividendSum 'latest') 100) (assetStockSum))}}%</th>
              <th colspan="2" style="background: inherit; font-size: small; text-align: right;" title="{{number (assetStockSum)}}">{{numberShort (assetStockSum)}}</th>
              <th colspan="4" style="background: inherit; font-size: small; text-align: right;" title="{{number (assetStockDividendSum @root.a.d.Assumption.DividendSpan)}}">{{numberShort (assetStockDividendSum @root.a.d.Assumption.DividendSpan)}}</th>
              <th style="background: inherit; font-size: small; text-align: right;" title="{{number (assetStockReceiveSum 1 @root.a.d.Assumption.DividendSpan)}}">{{numberShort (assetStockReceiveSum 1 @root.a.d.Assumption.DividendSpan)}}</th>
              <th style="background: inherit; font-size: small; text-align: right;" title="{{number (assetStockReceiveSum 2 @root.a.d.Assumption.DividendSpan)}}">{{numberShort (assetStockReceiveSum 2 @root.a.d.Assumption.DividendSpan)}}</th>
              <th style="background: inherit; font-size: small; text-align: right;" title="{{number (assetStockReceiveSum 3 @root.a.d.Assumption.DividendSpan)}}">{{numberShort (assetStockReceiveSum 3 @root.a.d.Assumption.DividendSpan)}}</th>
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
        <div class="card-body bg-success-subtle table-responsive" style="padding: 0px;">
          <table class="table table-striped">
            <thead>
            <tr>
              <th style="background: inherit;">Name</th>
              <th style="background: inherit; text-align: right;">Amount</th>
            </tr>
            </thead>
            <tbody>
            {{#each ../.}}
            <tr>
              <td style="background: inherit;">{{@key}}</td>
              <td style="background: inherit; text-align: right;" title="{{number Amount}}">{{numberShort Amount}}</td>
            </tr>
            {{/each}}
            <tr>
              <th style="background: inherit; text-align:left;">Total</th>
              <th style="background: inherit; text-align: right;" title="{{number (genericTypeSum ../../f @key)}}">{{numberShort (genericTypeSum ../../f @key)}}</th>
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
