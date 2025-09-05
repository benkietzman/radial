// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-10-12
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
        <div style="display:inline-grid;">
          <div class="card border border-primary-subtle" style="margin-top: 10px;">
            <div class="card-header bg-primary fw-bold">
              Income &amp; Expense
            </div>
            <div class="card-body bg-primary-subtle table-responsive" style="padding: 0px;">
              <table class="table table-striped">
                <thead>
                <tr>
                  <th style="background: inherit;"></th>
                  <th class="text-end" style="background: inherit;">Amount</th>
                </tr>
                </thead>
                <tbody>
                <tr>
                  <td style="background: inherit;">Income</td>
                  <td class="text-end" style="background: inherit;" title="{{number (incomeSum)}}">{{numberShort (incomeSum)}}</td>
                </tr>
                <tr>
                  <td style="background: inherit;">Expense</td>
                  <td class="text-end" style="background: inherit;" title="{{number (expenseSum)}}">{{numberShort (expenseSum)}}</td>
                </tr>
                <tr>
                  <th style="background: inherit; text-align:left;">Cash Flow</th>
                  <th class="text-end text-{{#ifCond (cashFlow) '>=' 0}}success{{else}}danger{{/ifCond}}" style="background: inherit;" title="{{number (cashFlow)}}">{{numberShort (cashFlow)}}</th>
                </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
        <div style="display:inline-grid;">
          <div class="card border border-primary-subtle" style="margin-top: 10px;">
            <div class="card-header bg-primary fw-bold">
              Asset &amp; Liability
            </div>
            <div class="card-body bg-primary-subtle table-responsive" style="padding: 0px;">
              <table class="table table-striped">
                <thead>
                <tr>
                  <th style="background: inherit;"></th>
                  <th class="text-end" style="background: inherit;">Amount</th>
                </tr>
                </thead>
                <tbody>
                <tr>
                  <td style="background: inherit;">Asset</td>
                  <td class="text-end" style="background: inherit;" title="{{number (assetSum)}}">{{numberShort (assetSum)}}</td>
                </tr>
                <tr>
                  <td style="background: inherit;">Liability</td>
                  <td class="text-end" style="background: inherit;" title="{{number (liabilitySum)}}">{{numberShort (liabilitySum)}}</td>
                </tr>
                <tr>
                  <th style="background: inherit; text-align:left;">Net Worth</th>
                  <th class="text-end text-{{#ifCond (netWorth) '>=' 0}}success{{else}}danger{{/ifCond}}" style="background: inherit;" title="{{number (netWorth)}}">{{numberShort (netWorth)}}</th>
                </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
        <div style="display:inline-grid;">
          <div class="card border border-primary-subtle" style="margin-top: 10px;">
            <div class="card-header bg-primary fw-bold">
              Ratio
            </div>
            <div class="card-body bg-primary-subtle table-responsive" style="padding: 0px;">
              <table class="table table-striped">
                <tbody>
                <tr>
                  <td style="background: inherit;">Expense : Income</td>
                  <td class="text-end" style="background: inherit;" title="{{number (multiply (divide (expenseSum) (incomeSum)) 100)}}%">{{numberShort (multiply (divide (expenseSum) (incomeSum)) 100)}}%</td>
                </tr>
                <tr>
                  <td style="background: inherit;">Metal : Net Worth</td>
                  <td class="text-end" style="background: inherit;" title="{{number (multiply (divide (assetMetalSum) (netWorth)) 100)}}%">{{numberShort (multiply (divide (assetMetalSum) (netWorth)) 100)}}%</td>
                </tr>
                <tr>
                  <td style="background: inherit;">Stock : Net Worth</td>
                  <td class="text-end" style="background: inherit;" title="{{number (multiply (divide (assetStockSum) (netWorth)) 100)}}%">{{numberShort (multiply (divide (assetStockSum) (netWorth)) 100)}}%</td>
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
        <div class="card border border-info-subtle" style="margin-top: 10px;">
          <div class="card-header bg-info fw-bold">
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
