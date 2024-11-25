// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-10-12
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
    let f = a.d;
    let s = c.scope('Summary',
    {
      // {{{ jsonExport()
      jsonExport: () =>
      {
        a.jsonExport();
      },
      // }}}
      // {{{ jsonImport()
      jsonImport: () =>
      {
        a.jsonImport(document.getElementById('jsonimport'));
      },
      // }}}
      // {{{ u()
      u: () =>
      {
        c.update('Summary');
      },
      // }}}
      a: a,
      c: c
    });
    c.setMenu('Summary', null);
    s.f = f;
  },
  // }}}
  // {{{ template
  template: `
    <div class="row">
      <div class="col-md-8">
        <h3 class="page-header">Summary</h3>
        <div style="display:inline-block;">
          <div class="card">
            <div class="card-header bg-primary">
              Income &amp; Expense
            </div>
            <div class="card-body table-responsive">
              <table class="table table-striped">
                <thead>
                <tr>
                  <th></th>
                  <th style="text-align:right;">Amount</th>
                </tr>
                </thead>
                <tbody>
                <tr>
                  <td>Income</td>
                  <td style="text-align:right;" title="{{number (incomeSum)}}">{{numberShort (incomeSum)}}</td>
                </tr>
                <tr>
                  <td>Expense</td>
                  <td style="text-align:right;" title="{{number (expenseSum)}}">{{numberShort (expenseSum)}}</td>
                </tr>
                <tr>
                  <th style="text-align:left;">Cash Flow</th>
                  <th class="text-{{#ifCond (cashFlow) '>=' 0}}success{{else}}danger{{/ifCond}}" style="text-align:right;" title="{{number (cashFlow)}}">{{numberShort (cashFlow)}}</th>
                </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
        <div style="display:inline-block;">
          <div class="card">
            <div class="card-header bg-primary">
              Asset &amp; Liability
            </div>
            <div class="card-body table-responsive">
              <table class="table table-striped">
                <thead>
                <tr>
                  <th></th>
                  <th style="text-align:right;">Amount</th>
                </tr>
                </thead>
                <tbody>
                <tr>
                  <td>Asset</td>
                  <td style="text-align:right;" title="{{number (assetSum)}}">{{numberShort (assetSum)}}</td>
                </tr>
                <tr>
                  <td>Liability</td>
                  <td style="text-align:right;" title="{{number (liabilitySum)}}">{{numberShort (liabilitySum)}}</td>
                </tr>
                <tr>
                  <th style="text-align:left;">Net Worth</th>
                  <th class="text-{{#ifCond (netWorth) '>=' 0}}success{{else}}danger{{/ifCond}}" style="text-align:right;" title="{{number (netWorth)}}">{{numberShort (netWorth)}}</th>
                </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
        <div style="display:inline-block;">
          <div class="card">
            <div class="card-header bg-primary">
              Ratio
            </div>
            <div class="card-body table-responsive">
              <table class="table table-striped">
                <tbody>
                <tr>
                  <td>Expense : Income</td>
                  <td style="text-align:right;" title="{{number (multiply (divide (expenseSum) (incomeSum)) 100)}}%">{{numberShort (multiply (divide (expenseSum) (incomeSum)) 100)}}%</td>
                </tr>
                <tr>
                  <td>Metal : Net Worth</td>
                  <td style="text-align:right;" title="{{number (multiply (divide (assetMetalSum) (netWorth)) 100)}}%">{{numberShort (multiply (divide (assetMetalSum) (netWorth)) 100)}}%</td>
                </tr>
                <tr>
                  <td>Stock : Net Worth</td>
                  <td style="text-align:right;" title="{{number (multiply (divide (assetStockSum) (netWorth)) 100)}}%">{{numberShort (multiply (divide (assetStockSum) (netWorth)) 100)}}%</td>
                </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
      </div>
      <div class="col-md-4">
        <input class="btn btn-sm btn-success" id="jsonimport" type="file" accept=".json" c-change="jsonImport()" value="Import">
        <button class="btn btn-sm btn-warning bi bi-save" c-click="jsonExport()" style="margin-left: 10px;" title="Save"></button>
        <div class="card" style="margin-top: 10px;">
          <div class="card-header bg-info">
            Track Your Finances
          </div>
          <div class="card-body bg-info-subtle">
            Modify your data on this website and then use this page to Save your data into a finance.json file.  When you come back later, you can load the file into the website.  The website itself does not store your data, so saving the finance.json file to your computer/device is critical.
          </div>
          <div class="card-footer">
            All data figures are considered annual with the exception of the figures contained on the Expense Spend page which are monthly.
          </div>
        </div>
      </div>
    </div>
  `
  // }}}
}
