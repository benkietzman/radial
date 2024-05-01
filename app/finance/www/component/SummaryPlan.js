// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-10-29
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
    let s = c.scope('SummaryPlan',
    {
      // {{{ u()
      u: () =>
      {
        c.update('SummaryPlan');
      },
      // }}}
      a: a,
      c: c,
      d: []
    });
    c.setMenu('Summary', 'Plan');
    s.f = f;
  },
  // }}}
  // {{{ template
  template: `
    <h3 class="page-header">Summary - Plan</h3>
    <div class="table-responsive">
      <table class="table table-striped">
        <thead>
          <tr>
            <th>Goal</th>
            <th>Target</th>
            <th>Actual</th>
            <th>Met</th>
            <th>Description</th>
          </tr>
        </thead>
        <tbody>
          <tr>
            <td style="white-space: nowrap;">Cash Flow</td>
            <td>0.00</td>
            <td title="{{number (cashFlow)}}">{{numberShort (cashFlow)}}</td>
            <td>{{#ifCond (cashFlow) '>=' 0}}<span class="text-success">YES</span>{{else}}<span class="text-danger">NO</span>{{/ifCond}}</td>
            <td>The goal is to have a positive cash flow.  This technically means your income is greater than your expenses.  This is commonly known a living below your means.
          </tr>
          <tr>
            <td style="white-space: nowrap;">Emergency Fund</td>
            <td title="{{number (divide (subtract (expenseSum) (incomeEmploymentWithheldSum)) 4)}}">{{numberShort (divide (subtract (expenseSum) (incomeEmploymentWithheldSum)) 4)}}</td>
            <td title="{{number (add (genericTypeSum f.Asset 'Liquid') (assetMetalSum))}}">{{numberShort (add (genericTypeSum f.Asset 'Liquid') (assetMetalSum))}}</td>
            <td>{{#ifCond (add (genericTypeSum f.Asset 'Liquid') (assetMetalSum)) '>=' (multiply (divide (subtract (expenseSum) (incomeEmploymentWithheldSum)) 12) 3)}}<span class="text-success">YES</span>{{else}}<span class="text-danger">NO</span>{{/ifCond}}</td>
            <td>The goal is to have at least three months worth of expenses saved as an emergency fund.  As the name suggests, an emergency fund is ear marked for emergencies.</td>
          </tr>
          <tr>
            <td style="white-space: nowrap;">Debt Free</td>
            <td>0.00</td>
            <td title="{{number (liabilitySum)}}">{{numberShort (liabilitySum)}}</td>
            <td>{{#ifCond (liabilitySum) '<=' 0}}<span class="text-success">YES</span>{{else}}<span class="text-danger">NO</span>{{/ifCond}}</td>
            <td>The goal is to be completely debt free meaning you have no outstanding liabilities.  Having a mortgage can be considered an exception to the rule since it can take several years to pay off a mortgage.  However, being completely debt free, including the mortgage, is still the ultimate goal.</td>
          </tr>
          <tr>
            <td style="white-space: nowrap;">Invest</td>
            <td>15%</td>
            <td title="{{number (multiply (divide (cashFlow) (incomeSum)) 100)}}%">{{numberShort (multiply (divide (cashFlow) (incomeSum)) 100)}}%</td>
            <td>{{#ifCond (divide (cashFlow) (incomeSum)) '>=' 0.15}}<span class="text-success">YES</span>{{else}}<span class="text-danger">NO</span>{{/ifCond}}</td>
            <td>The goal is to be investing at least 15% of your gross income.  You may need to increase that percentage in order to reach your personal retirement goals.  The <a href="#/Summary/Forecast">Summary - Forecast</a> can be used to determine whether you are on track for meeting your retirement goals.</td>
          </tr>
          <tr>
            <td style="white-space: nowrap;">Retire</td>
            <td title="{{number (multiply (genericSum f.Expense) 2)}}">{{numberShort (multiply (genericSum f.Expense) 2)}}</td>
            <td title="{{number (assetStockDividendSum)}}">{{numberShort (assetStockDividendSum)}}</td>
            <td>{{#ifCond (assetStockDividendSum) '>=' (multiply (genericSum f.Expense) 2)}}<span class="text-success">YES</span>{{else}}<span class="text-danger">NO</span>{{/ifCond}}</td>
            <td>The goal is to earn twice your expenses in dividend income so that you live off half and use the other half to cover dividend taxes while reinvesting the remainder in order to overcome inflation as well as live more comfortably over the years.  The <a href="#/Summary/Forecast">Summary - Forecast</a> can be used to determine whether you are on track for meeting your retirement goals.</td>
          </tr>
        </tbody>
      </table>
    </div>
  `
  // }}}
}
