// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-10-12
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
class App
{
  // {{{ constructor()
  constructor(options)
  {
    this.c = options.common;
    this.j = options.junction;
    this.m_bReady = false;
    this.m_bCommonAuthReady = false;
    this.m_bCommonFooterReady = false;
    this.m_bCommonWsReady = false;
    this.m_assetStockCategories =
    [
      'BDC',
      'CEF',
      'CORP',
      'eREIT',
      'ETF',
      'MLP',
      'mREIT'
    ];
    this.m_assetStockReceives =
    [
      'Monthly',
      '1st Mth/Qtr',
      '2nd Mth/Qtr',
      '3rd Mth/Qtr'
    ],
    this.m_assetStockScores =
    [
      '1.0',
      '1.5',
      '2.0',
      '2.5',
      '3.0'
    ];
    this.m_assetStockSectors =
    [
      'Finance',
      'Bonds',
      'Business',
      'Sinful',
      'Commodity',
      'Income',
      'Entertain',
      'Healthcare',
      'Healthcare Fac',
      'Insurance',
      'Manufac',
      'Mtg Com',
      'Mtg Res',
      'Energy',
      'Safe, Secure, Store',
      'Transport',
      'Tech',
      'Telecom',
      'Utility',
      'Misc',
      'Cash, Margin'
    ];
    this.m_noYes =
    [
      'No',
      'Yes'
    ];
    this.m_nExpired = 14400000; // 4 hours
    this.m_nProgress = [0, 0, 0];
    this.m_strFile = 'finance.json';
    this.d = null;
    let data = JSON.parse(sessionStorage.getItem('finance'));
    this.dataInit({Data: data});
    this.c.attachEvent('commonAuthReady', (data) =>
    {
      this.m_bCommonAuthReady = true;
      this.ready();
    });
    this.c.attachEvent('commonFooterReady', (data) =>
    {
      this.m_bCommonFooterReady = true;
      this.ready();
    });
    this.c.attachEvent('commonWsReady_Finance', (data) =>
    {
      this.m_bCommonWsReady = true;
      this.ready();
    });
    this.resetMenu();
    this.c.attachEvent('resetMenu', (data) =>
    {
      this.resetMenu();
    });
    Handlebars.registerHelper('assetMetalSum', () =>
    {
      return this.assetMetalSum();
    });
    Handlebars.registerHelper('assetStockDividendSum', () =>
    {
      return this.assetStockDividendSum();
    });
    Handlebars.registerHelper('assetStockReceive', (nDividend, nReceive, nMonth) =>
    {
      return this.assetStockReceive(nDividend, nReceive, nMonth);
    });
    Handlebars.registerHelper('assetStockReceiveSum', (nMonth) =>
    {
      return this.assetStockReceiveSum(nMonth);
    });
    Handlebars.registerHelper('assetStockSum', () =>
    {
      return this.assetStockSum();
    });
    Handlebars.registerHelper('assetStockYieldSum', () =>
    {
      return this.assetStockYieldSum();
    });
    Handlebars.registerHelper('assetSum', () =>
    {
      return this.assetSum();
    });
    Handlebars.registerHelper('cashFlow', () =>
    {
      return this.cashFlow();
    });
    Handlebars.registerHelper('expenseSum', () =>
    {
      return this.expenseSum();
    });
    Handlebars.registerHelper('genericSum', (f) =>
    {
      return this.genericSum(f);
    });
    Handlebars.registerHelper('genericTypeSum', (f, t) =>
    {
      return this.genericTypeSum(f, t);
    });
    Handlebars.registerHelper('incomeEmployment', (k) =>
    {
      return this.incomeEmployment(k);
    });
    Handlebars.registerHelper('incomeEmploymentSum', () =>
    {
      return this.incomeEmploymentSum();
    });
    Handlebars.registerHelper('incomeEmploymentWithheld', (k) =>
    {
      return this.incomeEmploymentWithheld(k);
    });
    Handlebars.registerHelper('incomeEmploymentWithheldSum', () =>
    {
      return this.incomeEmploymentWithheldSum();
    });
    Handlebars.registerHelper('incomeSum', () =>
    {
      return this.incomeSum();
    });
    Handlebars.registerHelper('liabilityCompoundSum', () =>
    {
      return this.liabilityCompoundSum();
    });
    Handlebars.registerHelper('liabilitySimpleSum', () =>
    {
      return this.liabilitySimpleSum();
    });
    Handlebars.registerHelper('liabilitySum', () =>
    {
      return this.liabilitySum();
    });
    Handlebars.registerHelper('netWorth', () =>
    {
      return this.netWorth();
    });
    Handlebars.registerHelper('spendAverage', () =>
    {
      return this.spendAverage();
    });
    Handlebars.registerHelper('spendItemAverage', (v) =>
    {
      return this.spendItemAverage(v);
    });
    Handlebars.registerHelper('spendItemTotal', (v) =>
    {
      return this.spendItemTotal(v);
    });
    Handlebars.registerHelper('spendMonthTotal', (nMonth) =>
    {
      return this.spendMonthTotal(nMonth);
    });
    Handlebars.registerHelper('spendTotal', () =>
    {
      return this.spendTotal();
    });
  }
  // }}}
  // {{{ add()
  add(f, d, u)
  {
    let error = {message: null};

    if (this.isValid(d, error))
    {
      if (!this.c.isDefined(f[d.k]))
      {
        f[d.k] = JSON.parse(JSON.stringify(d));
        u();
        this.jsonSave();
      }
      else
      {
        alert('Already exists.');
      }
    }
    else
    {
      alert(error.message);
    }
  }
  // }}}
  // {{{ assetMetalSum()
  assetMetalSum()
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Asset.Metal))
    {
      nSum += Number(v.Quantity) * Number(v.Price);
    }

    return nSum;
  }
  // }}}
  // {{{ assetStockReceive()
  assetStockReceive(nDividend, nReceive, nMonth)
  {
    let nValue = 0;

    if (Number(nReceive) == 0)
    {
      nValue = nDividend / 12;
    }
    else if ((nMonth == 1 && Number(nReceive) == 1) || (nMonth == 2 && Number(nReceive) == 2) || (nMonth == 3 && Number(nReceive) == 3))
    {
      nValue = nDividend / 4;
    }

    return nValue;
  }
  // }}}
  // {{{ assetStockReceiveSum()
  assetStockReceiveSum(nMonth)
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Asset.Stock))
    {
      nSum += this.assetStockReceive((Number(v.Dividend) * Number(v.Shares)), Number(v.Receive), nMonth);
    }

    return nSum;
  }
  // }}}
  // {{{ assetStockSum()
  assetStockSum()
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Asset.Stock))
    {
      nSum += Number(v.Shares) * Number(v.Price);
    }

    return nSum;
  }
  // }}}
  // {{{ assetStockDividendSum()
  assetStockDividendSum()
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Asset.Stock))
    {
      nSum += Number(v.Shares) * Number(v.Dividend);
    }

    return nSum;
  }
  // }}}
  // {{{ assetStockYieldSum()
  assetStockYieldSum()
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Asset.Stock))
    {
      nSum += (Number(v.Dividend) / Number(v.Price)) * 100;
    }

    return nSum;
  }
  // }}}
  // {{{ assetSum()
  assetSum()
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Asset))
    {
      if (k == 'Metal')
      {
        nSum += this.assetMetalSum();
      }
      else if (k == 'Stock')
      {
        nSum += this.assetStockSum();
      }
      else
      {
        nSum += this.genericTypeSum(this.d.Asset, k);
      }
    }

    return nSum;
  }
  // }}}
  // {{{ cashFlow()
  cashFlow()
  {
    return this.incomeSum() - this.expenseSum();
  }
  // }}}
  // {{{ dataInit()
  dataInit(options)
  {
    let bIgnoreTimestamp = false;
    let data = null;
    let postFunction = null;
    if (this.c.isObject(options.Data))
    {
      data = options.Data;
    }
    if (this.c.isDefined(options.IgnoreTimestamp) && options.IgnoreTimestamp)
    {
      bIgnoreTimestamp = true;
    }
    if (this.c.isDefined(options.PostFunction))
    {
      postFunction = options.PostFunction;
    }
    if (!this.c.isObject(this.d))
    {
      this.d = {};
    }
    if (this.c.isObject(data))
    {
      let sections = ['Asset', 'Assumption', 'Expense', 'Income', 'Liability', 'Spend'];
      for (let i = 0; i < sections.length; i++)
      {
        if (this.c.isObject(this.d[sections[i]]))
        {
          let removals = [];
          for (let [k, v] of Object.entries(this.d[sections[i]]))
          {
            removals.push(k);
          }
          for (let j = 0; j < removals.length; j++)
          {
            delete this.d[sections[i]][removals[j]];
          }
        }
        if (this.c.isObject(data[sections[i]]))
        {
          if (!this.c.isObject(this.d[sections[i]]))
          {
            this.d[sections[i]] = {};
          }
          for (let [k, v] of Object.entries(data[sections[i]]))
          {
            this.d[sections[i]][k] = data[sections[i]][k];
          }
        }
      }
    }
    this.progress();
    this.jsonSave();
    if (!this.c.isObject(this.d.Assumption))
    {
      this.d.Assumption = {};
    }
    if (!this.c.isDefined(this.d.Assumption.FilingStatus))
    {
      this.d.Assumption.FilingStatus = 'single';
    }
    if (!this.c.isDefined(this.d.Assumption.BirthYear))
    {
      this.d.Assumption.BirthYear = null;
    }
    if (!this.c.isDefined(this.d.Assumption.IncomeRaise))
    {
      this.d.Assumption.IncomeRaise = null;
    }
    if (!this.c.isDefined(this.d.Assumption.InflationDuration))
    {
      this.d.Assumption.InflationDuration = 0;
    }
    if (!this.c.isDefined(this.d.Assumption.InflationKey))
    {
      this.d.Assumption.InflationKey = null;
    }
    if (!this.c.isDefined(this.d.Assumption.InflationRate))
    {
      this.d.Assumption.InflationRate = null;
    }
    if (!this.c.isDefined(this.d.Assumption.inflationTimestamp))
    {
      this.d.Assumption.inflationTimestamp = 0;
    }
    if (this.d.Assumption.InflationDuration > 0 && this.c.isDefined(this.d.Assumption.InflationKey))
    {
      let nTimestamp = Date.now();
      if (bIgnoreTimestamp || (nTimestamp - this.d.Assumption.inflationTimestamp) > this.m_nExpired)
      {
        let nYear = new Date().getFullYear();
        let request = [{Service: 'bls', 'Resource': 'timeseries/data/', reqApp: 'Finance'}, {Post: {registrationkey: this.d.Assumption.InflationKey, seriesid: ['CUUR0000SA0'], startyear: ((nYear + 1) - Number(this.d.Assumption.InflationDuration)), endyear: nYear, calculations: true}}];
        this.j.request(request, true, (response) =>
        {
          let error = {};
          if (this.j.response(response, error))
          {
            if (this.c.isDefined(response.Response) && this.c.isDefined(response.Response[1]) && this.c.isDefined(response.Response[1].Results) && this.c.isDefined(response.Response[1].Results.series) && this.c.isDefined(response.Response[1].Results.series[0]) && this.c.isDefined(response.Response[1].Results.series[0].data))
            {
              let infm = {};
              let infy = [];
              let nSum = 0;
              for (let i = 0; i < response.Response[1].Results.series[0].data.length; i++)
              {
                if (this.c.isDefined(response.Response[1].Results.series[0].data[i].calculations) && this.c.isDefined(response.Response[1].Results.series[0].data[i].calculations.pct_changes) && this.c.isDefined(response.Response[1].Results.series[0].data[i].calculations.pct_changes['12']) && this.c.isDefined(response.Response[1].Results.series[0].data[i].year) && this.c.isDefined(response.Response[1].Results.series[0].data[i].period))
                {
                  if (!this.c.isDefined(infm[response.Response[1].Results.series[0].data[i].year]))
                  {
                    infm[response.Response[1].Results.series[0].data[i].year] = [];
                  }
                  infm[response.Response[1].Results.series[0].data[i].year].push(Number(response.Response[1].Results.series[0].data[i].calculations.pct_changes['12']));
                }
              }
              for (let [k, v] of Object.entries(infm))
              {
                let nSubSum = 0;
                for (let i = 0; i < v.length; i++)
                {
                  nSubSum += v[i];
                }
                infy.push(nSubSum / v.length);
              }
              for (let i = 0; i < infy.length; i++)
              {
                nSum += infy[i];
              }
              this.d.Assumption.InflationRate = Number(nSum / infy.length).toFixed(2);
              this.d.Assumption.inflationTimestamp = nTimestamp;
            }
          }
          else
          {
            console.log(response);
          }
          if (!this.c.isNull(postFunction))
          {
            postFunction();
          }
          if (this.c.isDefined(this.c.component))
          {
            this.c.render(this.c.id, this.c.name, this.c.component);
          }
          this.progress();
          this.jsonSave();
        });
      }
    }
    if (!this.c.isDefined(this.d.Assumption.PensionLumpSum))
    {
      this.d.Assumption.PensionLumpSum = null;
    }
    if (!this.c.isDefined(this.d.Assumption.RetireYear))
    {
      this.d.Assumption.RetireYear = null;
    }
    if (!this.c.isDefined(this.d.Assumption.SocialSecurityNet))
    {
      this.d.Assumption.SocialSecurityNet = null;
    }
    if (!this.c.isDefined(this.d.Assumption.SocialSecurityYear))
    {
      this.d.Assumption.SocialSecurityYear = null;
    }
    if (!this.c.isDefined(this.d.Assumption.StockGrowth))
    {
      this.d.Assumption.StockGrowth = null;
    }
    if (!this.c.isDefined(this.d.Assumption.Tithe))
    {
      this.d.Assumption.Tithe = null;
    }
    if (!this.c.isObject(this.d.Asset))
    {
      this.d.Asset = {};
    }
    if (!this.c.isObject(this.d.Asset.Liquid))
    {
      this.d.Asset.Liquid = {};
    }
    if (this.c.isObject(this.d.Asset.Metal))
    {
      let nTimestamp = Date.now();
      for (let [k, v] of Object.entries(this.d.Asset.Metal))
      {
        if (k == 'gold' || k == 'palladium' || k == 'platinum' || k == 'silver')
        {
          if (!this.c.isDefined(v.timestamp) || bIgnoreTimestamp || (nTimestamp - v.timestamp) > this.m_nExpired)
          {
            let request = [{Service: 'finance', 'Function': 'commodity', Symbol: k, reqApp: 'Finance'}];
            this.j.request(request, true, (response) =>
            {
              let error = {};
              if (this.j.response(response, error))
              {
                if (this.c.isDefined(response.Response) && this.c.isDefined(response.Response[1]) && this.c.isDefined(response.Response[1].Bid) && response.Response[1].Bid)
                {
                  this.d.Asset.Metal[k].Price = response.Response[1].Bid;
                  this.d.Asset.Metal[k].timestamp = nTimestamp;
                }
              }
              else
              {
                console.log(response);
              }
              if (!this.c.isNull(postFunction))
              {
                postFunction();
              }
              if (this.c.isDefined(this.c.component))
              {
                this.c.render(this.c.id, this.c.name, this.c.component);
              }
              this.progress();
              this.jsonSave();
            });
          }
        }
      }
    }
    else
    {
      this.d.Asset.Metal = {};
    }
    if (!this.c.isObject(this.d.Asset.Property))
    {
      this.d.Asset.Property = {};
    }
    if (this.c.isObject(this.d.Asset.Stock))
    {
      let nTimestamp = Date.now();
      for (let [k, v] of Object.entries(this.d.Asset.Stock))
      {
        if ((!this.c.isDefined(v.timestamp) || bIgnoreTimestamp || (nTimestamp - v.timestamp) > this.m_nExpired))
        {
          let request = [{Service: 'finance', 'Function': 'stock', Symbol: k, reqApp: 'Finance'}];
          this.j.request(request, true, (response) =>
          {
            let bPrice = false;
            let error = {};
            if (this.j.response(response, error))
            {
              if (this.c.isDefined(response.Response) && this.c.isDefined(response.Response[1]) && this.c.isDefined(response.Response[1].Last) && response.Response[1].Last)
              {
                bPrice = true;
                this.d.Asset.Stock[k].Price = response.Response[1].Last;
              }
            }
            else
            {
              console.log(response);
            }
            let strBase = 'https://query2.finance.yahoo.com/v8/finance/chart/';
            let strSymbol = k;
            let date = new Date();
            let CStart = Math.floor((date.getTime() - (5 * 365 * 24 * 60 * 60 * 1000)) / 1000);
            let CEnd = Math.floor(date.getTime() / 1000);
            let strInterval = '1d';
            let strEvents = 'div';
            let strURL = strBase + strSymbol + '?';
            let args = {period1: CStart, period2: CEnd, interval: strInterval, events: strEvents};
            let out = [];
            for (let key in args)
            {
              out.push(key + '=' + encodeURIComponent(args[key]));
            }
            strURL += out.join('&');
            let request = [{Service: 'curl'},{URL: strURL, Display: 'Content'}]; 
            this.j.request(request, true, (response) =>
            {
              let error = {};
              if (this.j.response(response, error))
              {
                if (response.Response[1].Status == 'okay')
                {
                  let lines = response.Response[1].Content.split('\n');
                  let dividends = {};
                  if (bPrice)
                  {
                    this.d.Asset.Stock[k].timestamp = nTimestamp;
                  }
                  for (let [key, value] of Object.entries(response.Response[1].Content.chart.result[0].events.dividends))
                  {
                    if (!this.c.isDefined(dividends[key]))
                    {
                      dividends[key] = 0;
                    }
                    dividends[key] += Number(value.amount);
                  }
                  this.d.Asset.Stock[k].Change = dividends;
                }
                else
                {
                  console.log(response.Response[1].Error);
                }
              }
              else
              {
                console.log(response);
              }
              if (!this.c.isNull(postFunction))
              {
                postFunction();
              }
              if (this.c.isDefined(this.c.component))
              {
                this.c.render(this.c.id, this.c.name, this.c.component);
              }
              this.progress();
              this.jsonSave();
            });
          });
        }
      }
    }
    else
    {
      this.d.Asset.Stock = {};
    }
    if (!this.c.isObject(this.d.Expense))
    {
      this.d.Expense = {};
    }
    if (!this.c.isObject(this.d.Expense.Auto))
    {
      this.d.Expense.Auto = {};
    }
    if (!this.c.isObject(this.d.Expense.Charity))
    {
      this.d.Expense.Charity = {};
    }
    if (!this.c.isObject(this.d.Expense.Food))
    {
      this.d.Expense.Food = {};
    }
    if (!this.c.isObject(this.d.Expense.Home))
    {
      this.d.Expense.Home = {};
    }
    if (!this.c.isObject(this.d.Expense.Medical))
    {
      this.d.Expense.Medical = {};
    }
    if (!this.c.isObject(this.d.Expense.Misc))
    {
      this.d.Expense.Misc = {};
    }
    if (!this.c.isObject(this.d.Expense.Utility))
    {
      this.d.Expense.Utility = {};
    }
    if (!this.c.isObject(this.d.Income))
    {
      this.d.Income = {};
    }
    if (!this.c.isObject(this.d.Income.Employment))
    {
      this.d.Income.Employment = {};
    }
    if (!this.c.isObject(this.d.Income.Welfare))
    {
      this.d.Income.Welfare = {};
    }
    if (!this.c.isObject(this.d.Liability))
    {
      this.d.Liability = {};
    }
    if (!this.c.isObject(this.d.Liability.Compound))
    {
      this.d.Liability.Compound = {};
    }
    if (!this.c.isObject(this.d.Liability.Simple))
    {
      this.d.Liability.Simple = {};
    }
    if (!this.c.isObject(this.d.Spend))
    {
      this.d.Spend = {};
    }
    for (let [k, v] of Object.entries(this.d.Expense))
    {
      for (let [sk, sv] of Object.entries(v))
      {
        if (sv.Spend == 1 && this.c.isDefined(this.d.Spend[k+' | '+sk]))
        {
          sv.Amount = this.spendItemTotal(this.d.Spend[k+' | '+sk]);
        }
      }
    }
    if (!this.c.isNull(postFunction))
    {
      postFunction();
    }
    if (this.c.isDefined(this.c.component))
    {
      this.c.render(this.c.id, this.c.name, this.c.component);
    }
  }
  // }}}
  // {{{ edit()
  edit(f, d, s, k, u)
  {
    if (this.c.isDefined(f[k]))
    {
      for (let [key, value] of Object.entries(f[k]))
      {
        if (this.c.isDefined(d[key]))
        {
          d[key] = value;
        }
      }
      s.k = k;
      u();
      this.jsonSave();
    }
    else
    {
      alert('Please provide a valid key.');
    }
  }
  // }}}
  // {{{ expenseSum()
  expenseSum()
  {
    return this.incomeEmploymentWithheldSum() + (this.incomeEmploymentSum() * (Number(this.d.Assumption.Tithe) / 100)) + this.genericSum(this.d.Expense);
  }
  // }}}
  // {{{ formatNumber()
  formatNumber(value)
  {
    let bNegative = false;
    let nNumber = Number(value);
    let strSuffix = '';

    if (nNumber < 0)
    {
      bNegative = true;
      nNumber *= -1;
    }
    if (nNumber > 1000)
    {
      strSuffix = 'K';
      nNumber /= 1000;
      if (nNumber > 1000)
      {
        strSuffix = 'M';
        nNumber /= 1000;
        if (nNumber > 1000)
        {
          strSuffix = 'B';
          nNumber /= 1000;
          if (nNumber > 1000)
          {
            strSuffix = 'T';
            nNumber /= 1000;
          }
        }
      }
    }

    return ((bNegative)?'-':'') + Number(nNumber).toFixed(2) + strSuffix;
  }
  // }}}
  // {{{ genericSum()
  genericSum(f)
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(f))
    {
      nSum += this.genericTypeSum(f, k);
    }

    return nSum;
  }
  // }}}
  // {{{ genericTypeSum()
  genericTypeSum(f, t)
  {
    let nSum = 0;

    if (this.c.isDefined(f[t]))
    {
      for (let [k, v] of Object.entries(f[t]))
      {
        nSum += Number(v.Amount);
      }
    }

    return nSum;
  }
  // }}}
  // {{{ incomeEmployment()
  incomeEmployment(k)
  {
    return Number(this.d.Income.Employment[k].Salary) + Number(this.d.Income.Employment[k].Bonus) + (Number(this.d.Income.Employment[k].Salary) + Number(this.d.Income.Employment[k].Bonus)) * (Number((Number(this.d.Income.Employment[k].Invest) >= Number(this.d.Income.Employment[k].Match))?this.d.Income.Employment[k].Match:this.d.Income.Employment[k].Invest) / 100);
  }
  // }}}
  // {{{ incomeEmploymentSum()
  incomeEmploymentSum()
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Income.Employment))
    {
      nSum += this.incomeEmployment(k);
    }

    return nSum;
  }
  // }}}
  // {{{ incomeEmploymentWithheld()
  incomeEmploymentWithheld(k)
  {
    return ((Number(this.d.Income.Employment[k].Salary) + Number(this.d.Income.Employment[k].Bonus)) * (Number(this.d.Income.Employment[k].Invest) / 100)) + ((Number(this.d.Income.Employment[k].Salary) + Number(this.d.Income.Employment[k].Bonus) + ((Number(this.d.Income.Employment[k].Salary) + Number(this.d.Income.Employment[k].Bonus)) * Number(this.d.Income.Employment[k].Invest / 100))) * (Number(this.d.Income.Employment[k].Tax) / 100)) + Number(this.d.Income.Employment[k].Hsa) + Number(this.d.Income.Employment[k].Medical);
  }
  // }}}
  // {{{ incomeEmploymentWithheldSum()
  incomeEmploymentWithheldSum()
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Income.Employment))
    {
      nSum += this.incomeEmploymentWithheld(k);
    }

    return nSum;
  }
  // }}}
  // {{{ incomeSum()
  incomeSum()
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Income))
    {
      if (k == 'Employment')
      {
        nSum += this.incomeEmploymentSum();
      }
      else
      {
        nSum += this.genericTypeSum(this.d.Income, k);
      }
    }

    return nSum;
  }
  // }}}
  // {{{ initSpend()
  initSpend(u)
  {
    let removals = [];
    for (let [k, v] of Object.entries(this.d.Expense))
    {
      for (let [sk, sv] of Object.entries(v))
      {
        if (sv.Spend == '1' && !this.c.isObject(this.d.Spend[k + ' | ' + sk]))
        {
          this.d.Spend[k + ' | ' + sk] = {};
        }
      }
    }
    for (let [k, v] of Object.entries(this.d.Spend))
    {
      let tokens = k.split(' | ');
      if (tokens.length == 2)
      {
        if (!this.c.isObject(this.d.Expense[tokens[0]]) || !this.c.isObject(this.d.Expense[tokens[0]][tokens[1]]) || this.d.Expense[tokens[0]][tokens[1]].Spend != '1')
        {
          removals.push(k);
        }
      }
      else
      {
        removals.push(k);
      }
    }
    for (let i = 0; i < removals.length; i++)
    {
      delete this.d.Spend[removals[i]];
    }
    for (let [k, v] of Object.entries(this.d.Spend))
    {
      for (let i = 1; i <= 12; i++)
      {
        if (!this.c.isDefined(v[i]))
        {
          this.d.Spend[k][i] = 0;
        }
      }
    }
    u();
    this.jsonSave();
  }
  // }}}
  // {{{ isValid()
  isValid(d, error)
  {
    let bResult = false;

    if (!this.c.isNull(d.k) && d.k.length > 0)
    {
      if (!this.c.isNull(d.Amount) && String(d.Amount).length > 0)
      {
        bResult = true;
      }
      else
      {
        error.message = 'Please provide the Amount.';
      }
    }
    else
    {
      error.message = 'Please provide the key.';
    }

    return bResult;
  }
  // }}}
  // {{{ jsonExport()
  jsonExport()
  {
    let request = [{Service: 'format', 'Function': 'convert', Format:{In: 'json', Out: 'json'}, reqApp: 'Finance'}, this.d];
    this.j.request(request, true, (response) =>
    {
      let error = {};
      if (this.j.response(response, error) && this.c.isDefined(response.Response) && this.c.isDefined(response.Response[0]) && this.c.isDefined(response.Response[0].Status) && response.Response[0].Status == 'okay' && this.c.isDefined(response.Response[1]))
      {
        this.d = null;
        this.d = response.Response[1];
      } 
      let data = JSON.stringify(this.d);
      sessionStorage.setItem('finance', data);
      const a = document.createElement('a');
      const file = new Blob([data], {type: 'application/json'});
      a.href = URL.createObjectURL(file);
      a.download = this.m_strFile;
      a.click();
      URL.revokeObjectURL(a.href);
    }); 
  }
  // }}}
  // {{{ jsonImport()
  jsonImport(inp)
  {
    let file = inp.files[0];
    this.m_strFile = file.name;
    const reader = new FileReader()
    reader.addEventListener('load', (e) =>
    {
      this.dataInit({Data: JSON.parse(e.target.result)});
    });
    reader.readAsText(file);
  }
  // }}}
  // {{{ jsonSave()
  jsonSave()
  {
    sessionStorage.setItem('finance', JSON.stringify(this.d));
  }
  // }}}
  // {{{ liabilityCompoundSum()
  liabilityCompoundSum()
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Liability.Compound))
    {
      nSum += Number(v.Principal);
    }

    return nSum;
  }
  // }}}
  // {{{ liabilitySimpleSum()
  liabilitySimpleSum()
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Liability.Simple))
    {
      nSum += Number(v.Principal);
    }

    return nSum;
  }
  // }}}
  // {{{ liabilitySum()
  liabilitySum()
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Liability))
    {
      if (k == 'Compound')
      {
        nSum += this.liabilityCompoundSum();
      }
      else
      {
        nSum += this.liabilitySimpleSum();
      }
    }

    return nSum;
  }
  // }}}
  // {{{ netWorth()
  netWorth()
  {
    return this.assetSum() - this.liabilitySum();
  }
  // }}}
  // {{{ progress()
  progress()
  {
    let nTimestamp = Date.now();
    this.m_nProgress[0] = 0;
    this.m_nProgress[1] = 0;
    this.m_nProgress[2] = 0;
    if (this.c.isDefined(this.d.Assumption) && this.c.isDefined(this.d.Assumption.InflationKey))
    {
      this.m_nProgress[0]++;
      if (this.c.isDefined(this.d.Assumption.inflationTimestamp) && (nTimestamp - this.d.Assumption.inflationTimestamp) <= this.m_nExpired)
      {
        this.m_nProgress[1]++;
      }
    }
    if (this.c.isObject(this.d.Asset))
    {
      if (this.c.isObject(this.d.Asset.Metal))
      {
        for (let [k, v] of Object.entries(this.d.Asset.Metal))
        {
          if (k == 'gold' || k == 'palladium' || k == 'platinum' || k == 'silver')
          {
            this.m_nProgress[0]++;
            if (this.c.isDefined(v.timestamp) && (nTimestamp - v.timestamp) <= this.m_nExpired)
            {
              this.m_nProgress[1]++;
            }
          }
        }
      }
      if (this.c.isObject(this.d.Asset.Stock))
      {
        for (let [k, v] of Object.entries(this.d.Asset.Stock))
        {
          this.m_nProgress[0]++;
          if (this.c.isDefined(v.timestamp) && (nTimestamp - v.timestamp) <= this.m_nExpired)
          {
            this.m_nProgress[1]++;
          }
        }
      }
    }
    if (this.m_nProgress[0] > 0)
    {
      this.c.dispatchEvent('financeProgress', Math.floor(this.m_nProgress[1] * 100 / this.m_nProgress[0]));
    }
  }
  // }}}
  // {{{ ready()
  ready(response, error)
  {
    //if (!this.m_bReady && this.m_bCommonAuthReady && this.m_bCommonFooterReady && this.m_bCommonWsReady)
    if (!this.m_bReady && this.m_bCommonFooterReady)
    {
      this.m_bReady = true;
      this.c.dispatchEvent('financeReady', null);
    }

    return this.m_bReady;
  }
  // }}}
  // {{{ remove()
  remove(f, s, k, u)
  {
    delete f[k];
    s.k = null;
    u();
    this.jsonSave();
  }
  // }}}
  // {{{ resetMenu()
  resetMenu()
  {
    let unIndex, unSubIndex;
    this.c.clearMenu();
    this.c.menu = {left: [], right: []};
    unIndex = 0;
    this.c.menu.left[unIndex] = {value: 'Summary', href: '/Summary', icon: 'receipt', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    unSubIndex = 0;
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Assumption', href: '/Summary/Assumption', icon: 'question-circle', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Forecast', href: '/Summary/Forecast', icon: 'graph-up', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Plan', href: '/Summary/Plan', icon: 'person-workspace', active: null};
    unIndex++;
    this.c.menu.left[unIndex] = {value: 'Income', href: '/Income', icon: 'download', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    unSubIndex = 0;
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Employment', href: '/Income/Employment', icon: 'person-walking', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Welfare', href: '/Income/Welfare', icon: 'person-raised-hand', active: null};
    unIndex++;
    this.c.menu.left[unIndex] = {value: 'Expense', href: '/Expense', icon: 'upload', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    unSubIndex = 0;
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Auto', href: '/Expense/Auto', icon: 'car-front', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Charity', href: '/Expense/Charity', icon: 'bag-plus', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Food', href: '/Expense/Food', icon: 'cart3', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Home', href: '/Expense/Home', icon: 'house', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Medical', href: '/Expense/Medical', icon: 'bandaid', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Misc', href: '/Expense/Misc', icon: 'cloud', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Utility', href: '/Expense/Utility', icon: 'lightning', active: null};
    unSubIndex = 0;
    this.c.menu.left[unIndex].submenu.right[unSubIndex++] = {value: 'Spend', href: '/Expense/Spend', icon: 'currency-dollar', active: null};
    unIndex++;
    this.c.menu.left[unIndex] = {value: 'Asset', href: '/Asset', icon: 'journal-plus', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    unSubIndex = 0;
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Liquid', href: '/Asset/Liquid', icon: 'water', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Metal', href: '/Asset/Metal', icon: 'magnet', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Property', href: '/Asset/Property', icon: 'house', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Stock', href: '/Asset/Stock', icon: 'ticket', active: null};
    unIndex++;
    this.c.menu.left[unIndex] = {value: 'Liability', href: '/Liability', icon: 'journal-minus', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    unSubIndex = 0;
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Compound', href: '/Liability/Compound', icon: 'reception-4', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Simple', href: '/Liability/Simple', icon: 'reception-2', active: null};
    this.c.setMenu(this.c.strMenu, this.c.strSubMenu);
  }
  // }}}
  // {{{ spendAverage()
  spendAverage()
  {
    return this.spendTotal() / 12;
  }
  // }}}
  // {{{ spendItemAverage()
  spendItemAverage(v)
  {
    return this.spendItemTotal(v) / 12;
  }
  // }}}
  // {{{ spendItemTotal()
  spendItemTotal(v)
  {
    let nSum = 0;

    for (let i = 1; i <= 12; i++)
    {
      nSum += Number(v[i]);
    }

    return nSum;
  }
  // }}}
  // {{{ spendMonthTotal()
  spendMonthTotal(nMonth)
  {
    let nSum = 0;

    for (let [k, v] of Object.entries(this.d.Spend))
    {
      nSum += Number(v[nMonth]);
    }

    return nSum;
  }
  // }}}
  // {{{ spendTotal()
  spendTotal()
  {
    let nSum = 0;

    for (let i = 1; i <= 12; i++)
    {
      nSum += this.spendMonthTotal(i);
    }

    return nSum;
  }
  // }}}
  // {{{ spendUpdate()
  spendUpdate()
  {
    for (let [k, v] of Object.entries(this.d.Spend))
    {
      let tokens = k.split(' | ');
      this.d.Expense[tokens[0]][tokens[1]].Amount = Number(this.spendItemTotal(v)).toFixed(2);
    }
    this.jsonSave();
  }
  // }}}
  // {{{ update()
  update(f, d, s, k, u)
  {
    let error = {message: null};

    if (this.c.isDefined(f[k]))
    {
      if (this.isValid(d, error))
      {
        if (d.k != k)
        {
          f[d.k] = f[k];
          delete f[k];
        }
        for (let [key, value] of Object.entries(d))
        {
          f[d.k][key] = value;
        }
        s.k = null;
        u();
        this.jsonSave();
      }
      else
      {
        alert(error.message);
      }
    }
    else
    {
      alert('Please provide a valid key.');
    }
  }
  // }}}
}
