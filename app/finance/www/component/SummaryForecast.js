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
    let f = a.d;
    let s = c.scope('SummaryForecast',
    {
      // {{{ u()
      u: () =>
      {
        c.update('SummaryForecast');
      },
      // }}}
      a: a,
      c: c,
      chart0: null,
      chart1: null,
      d: []
    });
    c.setMenu('Summary', 'Forecast');
    s.f = f;
    let bRetired = false;
    let date = new Date;
    let divTax =
    [
      {
        single: 459751,
        jointly: 517201,
        separate: 258601,
        household: 488501
      },
      {
        single: 41676,
        jointly: 83351,
        separate: 41676,
        household: 55801
      }
    ];
    let fDivTaxInc = 0.0099;
    let fSocialSecurity = 0;
    let fYield = 0;
    let i = 0;
    let nYear = date.getFullYear();
    let nEndYear = Number(f.Assumption.BirthYear) + 120;
    let fDividendTaxable = 0;
    let fDividendTaxableTotal = 0;
    let fDividendTotal = 0;
    for (let [key, value] of Object.entries(f.Asset.Stock))
    {
      let fDividend = Number(value.Shares) * Number(value.Dividend);
      fDividendTotal += fDividend;
      if (value.Taxable == 1)
      {
        fDividendTaxableTotal += fDividend;
      }
    }
    fDividendTaxable = fDividendTaxableTotal / fDividendTotal;
    for (let y = nYear; y <= nEndYear; y++)
    {
      s.d[i] = {Year: y, Age: (y - Number(f.Assumption.BirthYear))};
      if (y >= Number(f.Assumption.RetireYear))
      {
        bRetired = true;
      }
      if (y == nYear)
      {
        s.d[i].Gross = 0;
        s.d[i].Invest = 0;
        s.d[i].Match = 0;
        s.d[i].Net = 0;
        if (!bRetired)
        {
          s.d[i].Gross = Number(a.incomeSum());
          for (let [k, v] of Object.entries(a.d.Income.Employment))
          {
            let fMatch = (Number(v.Salary) + Number(v.Bonus)) * (Number((Number(v.Invest) >= Number(v.Match))?v.Match:v.Invest) / 100);
            s.d[i].Invest += ((Number(v.Salary) + Number(v.Bonus)) * (v.Invest / 100) + fMatch) * (1 - (Number(v.Tax) / 100));
            s.d[i].Match += fMatch;
          }
          s.d[i].Net = Number(s.d[i].Gross) - Number(a.incomeEmploymentWithheldSum());
        }
        s.d[i].Stock = Number(a.assetStockSum());
        let nDividend = 0;
        for (let [key, value] of Object.entries(f.Asset.Stock))
        {
          nDividend += Number(value.Shares) * Number(value.Dividend);
        }
        s.d[i].Dividend = nDividend;
        fYield = Number(s.d[i].Dividend) / Number(s.d[i].Stock);
        s.d[i].Metal = a.assetMetalSum();
        s.d[i].Asset = Number(s.d[i].Stock) + Number(s.d[i].Dividend) + Number(s.d[i].Metal);
        s.d[i].DivTax = 0;
        if ((s.d[i].Gross + s.d[i].Dividend) >= divTax[0][f.Assumption.FilingStatus])
        {
          s.d[i].DivTax = s.d[i].Dividend * fDividendTaxable * 0.2;
        }
        else if ((s.d[i].Gross + s.d[i].Dividend) >= divTax[1][f.Assumption.FilingStatus])
        {
          s.d[i].DivTax = s.d[i].Dividend * fDividendTaxable * 0.15;
        }
        s.d[i].Tithe = 0;
        if (!bRetired)
        {
          s.d[i].Tithe = (Number(s.d[i].Gross) - Number(s.d[i].Match)) * (Number(f.Assumption.Tithe) / 100);
        }
        s.d[i].Income = (Number(s.d[i].Invest) + Number(s.d[i].Net) + Number(s.d[i].Dividend)) - (Number(s.d[i].DivTax) + Number(s.d[i].Tithe));
        if (y >= Number(f.Assumption.SocialSecurityYear))
        {
          if (Number(fSocialSecurity) == 0)
          {
            fSocialSecurity = Number(f.Assumption.SocialSecurityNet);
          }
          s.d[i].Income = Number(s.d[i].Income) + fSocialSecurity;
          fSocialSecurity = fSocialSecurity * (1 + (Number(f.Assumption.InflationRate) / 100));
        }
        s.d[i].Expense = Number(a.genericSum(f.Expense));
        s.d[i].Flow = Number(s.d[i].Income) - Number(s.d[i].Expense);
        s.d[i].Rate = 0;
      }
      else
      {
        s.d[i].Gross = 0;
        s.d[i].Invest = 0;
        s.d[i].Net = 0;
        if (!bRetired)
        {
          s.d[i].Gross = Number(s.d[i-1].Gross) * (1 + (Number(f.Assumption.IncomeRaise) / 100));
          s.d[i].Invest = Number(s.d[i-1].Invest) * (1 + (Number(f.Assumption.IncomeRaise) / 100));
          s.d[i].Match = Number(s.d[i-1].Match) * (1 + (Number(f.Assumption.IncomeRaise) / 100));
          s.d[i].Net = Number(s.d[i-1].Net) * (1 + (Number(f.Assumption.IncomeRaise) / 100));
        }
        s.d[i].Stock = Number(s.d[i-1].Stock) * (1 + (Number(f.Assumption.StockGrowth) / 100));
        if (y == Number(f.Assumption.RetireYear))
        {
          s.d[i].Stock = Number(s.d[i].Stock) + Number(f.Assumption.PensionLumpSum);
        }
        s.d[i].Metal = Number(s.d[i-1].Metal) * (1 + (Number(f.Assumption.InflationRate) / 100));
        if (Number(s.d[i-1].Flow) > 0)
        {
          s.d[i].Stock = Number(s.d[i].Stock) + Number(s.d[i-1].Flow);
        }
        else
        {
          let nFlow = Number(s.d[i-1].Flow) * -1;
          if (Number(s.d[i].Stock) > 0)
          {
            if (Number(s.d[i].Stock) > nFlow)
            {
              s.d[i].Stock = Number(s.d[i].Stock) - nFlow;
              nFlow = 0;
            }
            else
            {
              s.d[i].Stock = 0;
              nFlow = nFlow - Number(s.d[i].Stock);
            }
          }
          if (nFlow > 0)
          {
            if (Number(s.d[i].Metal) > nFlow)
            {
              s.d[i].Metal = Number(s.d[i].Metal) - nFlow;
            }
            else
            {
              s.d[i].Metal = 0;
            }
          }
        }
        s.d[i].Dividend = Number(s.d[i].Stock) * fYield;
        s.d[i].Asset = Number(s.d[i].Stock) + Number(s.d[i].Dividend) + Number(s.d[i].Metal);
        s.d[i].DivTax = 0;
        if ((s.d[i].Gross + s.d[i].Dividend) >= divTax[0][f.Assumption.FilingStatus])
        {
          s.d[i].DivTax = s.d[i].Dividend * fDividendTaxable * 0.2;
        }
        else if ((s.d[i].Gross + s.d[i].Dividend) >= divTax[1][f.Assumption.FilingStatus])
        {
          s.d[i].DivTax = s.d[i].Dividend * fDividendTaxable * 0.15;
        }
        s.d[i].Tithe = 0;
        if (!bRetired)
        {
          s.d[i].Tithe = (Number(s.d[i].Gross) - Number(s.d[i].Match)) * (Number(f.Assumption.Tithe) / 100);
        }
        s.d[i].Income = (Number(s.d[i].Invest) + Number(s.d[i].Net) + Number(s.d[i].Dividend)) - (Number(s.d[i].DivTax) + Number(s.d[i].Tithe));
        if (y >= Number(f.Assumption.SocialSecurityYear))
        {
          if (Number(fSocialSecurity) == 0)
          {
            fSocialSecurity = Number(f.Assumption.SocialSecurityNet);
          }
          s.d[i].Income = Number(s.d[i].Income) + fSocialSecurity;
          fSocialSecurity = Number(fSocialSecurity) * (1 + (Number(f.Assumption.InflationRate) / 100));
        }
        s.d[i].Expense = Number(s.d[i-1].Expense) * (1 + (Number(f.Assumption.InflationRate) / 100));
        s.d[i].Flow = Number(s.d[i].Income) - Number(s.d[i].Expense);
        s.d[i].Rate = (Number(s.d[i].Flow) - Number(s.d[i-1].Flow))/Number(s.d[i-1].Flow) * 100;
      }
      for (let j = 0; j < divTax.length; j++)
      {
        for (let [k, v] of Object.entries(divTax[j]))
        {
          divTax[j][k] *= (1 + fDivTaxInc);
        }
      }
      i++;
    }
    c.attachEvent('render', (data) =>
    {
      if (data.detail.name == 'SummaryForecast')
      {
        let d0 = {labels: [], datasets: []};
        d0.datasets.push({label: 'Income', data: []});
        d0.datasets.push({label: 'Expense', data: []});
        for (let i = 0; i < s.d.length; i++)
        {
          d0.labels.push(s.d[i].Year);
          d0.datasets[0].data.push(s.d[i].Income);
          d0.datasets[1].data.push(s.d[i].Expense);
        }
        const ctx0 = document.getElementById('chart0');
        if (!c.isNull(s.chart0))
        {
          s.chart0.destroy();
          delete s.chart0;
        }
        s.chart0 = new Chart(ctx0, {type: 'line', data: d0});
        let d1 = {labels: [], datasets: []};
        d1.datasets.push({label: 'Stock', data: []});
        d1.datasets.push({label: 'Metal', data: []});
        for (let i = 0; i < s.d.length; i++)
        {
          d1.labels.push(s.d[i].Year);
          d1.datasets[0].data.push(s.d[i].Stock);
          d1.datasets[1].data.push(s.d[i].Metal);
        }
        const ctx1 = document.getElementById('chart1');
        if (!c.isNull(s.chart1))
        {
          s.chart1.destroy();
          delete s.chart1;
        }
        s.chart1 = new Chart(ctx1, {type: 'line', data: d1});
      }
    });
  },
  // }}}
  // {{{ template
  template: `
    <h3 class="page-header">Summary - Forecast</h3>
    <div class="row">
      <div class="col-md-6">
        <div style="max-height: 400px"><canvas id="chart0"></canvas></div>
      </div>
      <div class="col-md-6">
        <div style="max-height: 400px"><canvas id="chart1"></canvas></div>
      </div>
    </div>
    <div class="table-responsive">
      <table class="table table-sm table-striped">
        <thead>
        <tr>
          <th class="text-end">Year</th>
          <th class="text-end">Age</th>
          <th class="text-end" title="Gross Income including Employer Match">Gross</th>
          <th class="text-end" title="Invest including Employer Match">Invest</th>
          <th class="text-end" title="Net Income">Net</th>
          <th class="text-end" title="Tithe">Tithe</th>
          <th class="text-end" title="Value of Stocks">Stock</th>
          <th class="text-end" title="Annual Dividend Yield">Dividend</th>
          <th class="text-end" title="Dividend Tax">Div Tax</th>
          <th class="text-end" title="Value of Precious Metals">Metal</th>
          <th class="text-end" title="Value of Assets">Asset</th>
          <th class="text-end" title="Net Income plus Dividend Income plus Social Security minus Dividend Tax minus Tithe">Income</th>
          <th class="text-end" title="Value of Expenses">Expense</th>
          <th class="text-end" title="Incomes minus Expenses">Flow</th>
          <th class="text-end" title="Rate of change between current Flow and previous Flow">Rate</th>
        </tr>
        </thead>
        <tbody>
        {{#each d}}
        <tr>
          <td class="text-end">{{Year}}</td>
          <td class="text-end">{{Age}}</td>
          <td class="text-end" title="{{number Gross}}">{{numberShort Gross}}</td>
          <td class="text-end" title="{{number Invest}}">{{numberShort Invest}}</td>
          <td class="text-end" title="{{number Net}}">{{numberShort Net}}</td>
          <td class="text-end" title="{{number Tithe}}">{{numberShort Tithe}}</td>
          <td class="text-end" title="{{number Stock}}">{{numberShort Stock}}</td>
          <td class="text-end" title="{{number Dividend}}">{{numberShort Dividend}}</td>
          <td class="text-end" title="{{number DivTax}}">{{numberShort DivTax}}</td>
          <td class="text-end" title="{{number Metal}}">{{numberShort Metal}}</td>
          <td class="text-end" title="{{number Asset}}">{{numberShort Asset}}</td>
          <td class="text-end" title="{{number Income}}">{{numberShort Income}}</td>
          <td class="text-end" title="{{number Expense}}">{{numberShort Expense}}</td>
          <td class="text-end text-{{#ifCond Flow '>=' 0}}success{{else}}danger{{/ifCond}}" title="{{number Flow}}">{{numberShort Flow}}</td>
          <td class="text-end text-{{#ifCond Rate '>=' 0}}success{{else}}danger{{/ifCond}}" title="{{number Rate}}%">{{numberShort Rate}}%</td>
        </tr>
        {{/each}}
        </tbody>
      </table>
    </div>
  `
  // }}}
}
