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
    let f = a.d.Asset.Stock;
    let s = c.scope('AssetStock',
    {
      // {{{ add()
      add: () =>
      {
        let d = c.simplify(s.d);
        let error = {message: null};

        if (s.isValid(d, error))
        {
          if (!c.isDefined(f[d.k]))
          {
            f[d.k] = JSON.parse(JSON.stringify(d));
            s.u();
            a.jsonSave();
          }
          else
          {
            alert('Stock already exists.');
          }
        }
        else
        {
          alert(error.message);
        }
      },
      // }}}
      // {{{ edit()
      edit: (k) =>
      {
        a.edit(f, s.e, s, k, s.u);
      },
      // }}}
      // {{{ isValid()
      isValid: (d, error) =>
      {
        let bResult = false;

        if (!c.isNull(d.k) && d.k.length > 0)
        {
          if (!c.isNull(d.Account) && String(d.Account).length > 0)
          {
            if (!c.isNull(d.Category) && String(d.Category).length > 0)
            {
              if (!c.isNull(d.Score) && String(d.Score).length > 0)
              {
                if (!c.isNull(d.Sector) && String(d.Sector).length > 0)
                {
                  if (!c.isNull(d.Shares) && String(d.Shares).length > 0)
                  {
                    if (!c.isNull(d.Price) && String(d.Price).length > 0)
                    {
                      if (!c.isNull(d.Dividend) && String(d.Dividend).length > 0)
                      {
                        if (!c.isNull(d.Receive) && String(d.Receive).length > 0)
                        {
                          bResult = true;
                        }
                        else
                        {
                          error.message = 'Please provide the Receive.';
                        }
                      }
                      else
                      {
                        error.message = 'Please provide the Dividend.';
                      }
                    }
                    else
                    {
                      error.message = 'Please provide the Price.';
                    }
                  }
                  else
                  {
                    error.message = 'Please provide the Shares.';
                  }
                }
                else
                {
                  error.message = 'Please provide the Sector.';
                }
              }
              else
              {
                error.message = 'Please provide the Score.';
              }
            }
            else
            {
              error.message = 'Please provide the Category.';
            }
          }
          else
          {
            error.message = 'Please provide the Account.';
          }
        }
        else
        {
          error.message = 'Please provide the Stock.';
        }

        return bResult;
      },
      // }}}
      // {{{ remove()
      remove: (k) =>
      {
        a.remove(f, s, k, s.u);
      },
      // }}}
      // {{{ u()
      u: () =>
      {
        c.update('AssetStock');
      },
      // }}}
      // {{{ update()
      update: (k) =>
      {
        let d = c.simplify(s.e);
        let error = {message: null};

        if (c.isDefined(f[k]))
        {
          if (s.isValid(d, error))
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
            s.u();
            a.jsonSave();
          }
          else
          {
            alert(error.message);
          }
        }
        else
        {
          alert('Please provide a valid Stock.');
        }
      },
      // }}}
      a: a,
      d:
      {
        k: null,
        Account: null,
        Category: null,
        Score: null,
        Sector: null,
        Shares: null,
        Price: null,
        Dividend: null,
        Receive: null,
        Taxable: null
      },
      e:
      {
        k: null,
        Account: null,
        Category: null,
        Score: null,
        Sector: null,
        Shares: null,
        Price: null,
        Dividend: null,
        Receive: null,
        Taxable: null
      },
      k: null
    });
    c.setMenu('Asset', 'Stock');
    s.f = f;
  },
  // }}}
  // {{{ template
  template: `
    <h3 class="page-header">Asset - Stocks</h3>
    {{#if f}}
    <div class="table-responsive">
      <table class="table table-striped">
        <thead>
        <tr>
          <th title="Stock Symbol">Stock</th>
          <th title="Source Account">Account</th>
          <th title="Position Category">Category</th>
          <th title="Score">Score</th>
          <th title="Sector Classification">Sector</th>
          <th title="# of Shares of stock">Shares</th>
          <th title="Price per Share of stock">$/Share</th>
          <th title="Dividend per Share">Dividend</th>
          <th title="When Dividend is Received">Receive</th>
          <th title="Are the dividends taxable?">Tax</th>
          <th></th>
        </tr>
        </thead>
        <tbody>
        <tr>
          <td><input type="text" class="form-control form-control-sm" c-model="d.k"></td>
          <td><input type="text" class="form-control form-control-sm" c-model="d.Account"></td>
          <td><select class="form-control form-control-sm" c-model="d.Category">{{#each a.m_assetStockCategories}}<option value="{{@key}}">{{.}}</option>{{/each}}</select></td>
          <td><select class="form-control form-control-sm" c-model="d.Score">{{#each a.m_assetStockScores}}<option value="{{@key}}">{{.}}</option>{{/each}}</select></td>
          <td><select class="form-control form-control-sm" c-model="d.Sector">{{#each a.m_assetStockSectors}}<option value="{{@key}}">{{.}}</option>{{/each}}</select></td>
          <td><input type="text" class="form-control form-control-sm" c-model="d.Shares"></td>
          <td><input type="text" class="form-control form-control-sm" c-model="d.Price"></td>
          <td><input type="text" class="form-control form-control-sm" c-model="d.Dividend"></td>
          <td><select class="form-control form-control-sm" c-model="d.Receive">{{#each a.m_assetStockReceives}}<option value="{{@key}}">{{.}}</option>{{/each}}</select></td>
          <td><select class="form-control form-control-sm" c-model="d.Taxable">{{#each a.m_noYes}}<option value="{{@key}}">{{.}}</option>{{/each}}</select></td>
          <td><button class="btn btn-sm btn-success bi bi-plus-circle" c-click="add()" title="Add"></button></td>
        </tr>
        {{#each f}}
        <tr>
          <td>{{#ifCond @key '!=' ../k}}{{../k}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.k">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Account}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.Account">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{#indexOf ../../a.m_assetStockCategories ../Category}}{{/indexOf}}{{else}}<select class="form-control form-control-sm" c-model="e.Category">{{#each ../../a.m_assetStockCategories}}<option value="{{@key}}">{{.}}</option>{{/each}}</select>{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{#indexOf ../../a.m_assetStockScores ../Score}}{{/indexOf}}{{else}}<select class="form-control form-control-sm" c-model="e.Score">{{#each ../../a.m_assetStockScores}}<option value="{{@key}}">{{.}}</option>{{/each}}</select>{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{#indexOf ../../a.m_assetStockSectors ../Sector}}{{/indexOf}}{{else}}<select class="form-control form-control-sm" c-model="e.Sector">{{#each ../../a.m_assetStockSectors}}<option value="{{@key}}">{{.}}</option>{{/each}}</select>{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Shares}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.Shares">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Price}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.Price">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Dividend}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.Dividend">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{#indexOf ../../a.m_assetStockReceives ../Receive}}{{/indexOf}}{{else}}<select class="form-control form-control-sm" c-model="e.Receive">{{#each ../../a.m_assetStockReceives}}<option value="{{@key}}">{{.}}</option>{{/each}}</select>{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{#indexOf ../../a.m_noYes ../Taxable}}{{/indexOf}}{{else}}<select class="form-control form-control-sm" c-model="e.Taxable">{{#each ../../a.m_noYes}}<option value="{{@key}}">{{.}}</option>{{/each}}</select>{{/ifCond}}</td>
          <td style="display:inline-block;">{{#ifCond @key '!=' ../k}}<button class="btn btn-sm btn-warning bi bi-pencil" c-click="edit('{{@key}}')" class="Edit"></button><button class="btn btn-sm btn-danger bi bi-trash" c-click="remove('{{@key}}')" title="Remove"></button>{{else}}<button class="btn btn-sm btn-success bi bi-save" c-click="update('{{@key}}')" title="Save"></button>{{/ifCond}}</td>
        </tr>
        {{/each}}
        </tbody>
      </table>
    </div>
    {{/if}}
  `
  // }}}
}
