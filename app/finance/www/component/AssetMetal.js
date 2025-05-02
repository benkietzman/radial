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
    let f = a.d.Asset.Metal;
    let s = c.scope('AssetMetal',
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
            alert('Metal already exists.');
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
          if (!c.isNull(d.Quantity) && String(d.Quantity).length > 0)
          {
            if (!c.isNull(d.Price) && String(d.Price).length > 0)
            {
              bResult = true;
            }
            else
            {
              c.pushErrorMessage('Please provide the Price.');
            }
          }
          else
          {
            c.pushErrorMessage('Please provide the Quantity.');
          }
        }
        else
        {
          c.pushErrorMessage('Please provide the Metal.');
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
        c.update('AssetMetal');
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
          alert('Please provide a valid Metal.');
        }
      },
      // }}}
      a: a,
      d:
      {
        k: null,
        Quantity: null,
        Price: null
      },
      e:
      {
        k: null,
        Price: null,
        Quantity: null
      },
      k: null
    });
    c.setMenu('Asset', 'Metal');
    s.f = f;
  },
  // }}}
  // {{{ template
  template: `
    <h3 class="page-header">Asset - Precious Metals</h3>
    {{#if f}}
    <div class="table-responsive">
      <table class="table table-striped bg-success-subtle border border-success-subtle">
        <thead>
        <tr>
          <th style="background: inherit;">Name</th>
          <th style="background: inherit;">Quantity</th>
          <th style="background: inherit;">$/item</th>
          <th style="background: inherit;"></th>
        </tr>
        </thead>
        <tbody>
        <tr>
          <td style="background: inherit;"><input type="text" class="form-control form-control-sm bg-success-subtle" c-model="d.k"></td>
          <td style="background: inherit;"><input type="text" class="form-control form-control-sm bg-success-subtle" c-model="d.Quantity"></td>
          <td style="background: inherit;"><input type="text" class="form-control form-control-sm bg-success-subtle" c-model="d.Price"></td>
          <td style="background: inherit;"><button class="btn btn-sm btn-success bi bi-plus-circle" c-click="add()" title="Add"></button></td>
        </tr>
        {{#each f}}
        <tr>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../k}}{{else}}<input type="text" class="form-control form-control-sm bg-success-subtle" c-model="e.k">{{/ifCond}}</td>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../Quantity}}{{else}}<input type="text" class="form-control form-control-sm bg-success-subtle" c-model="e.Quantity">{{/ifCond}}</td>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../Price}}{{else}}<input type="text" class="form-control form-control-sm bg-success-subtle" c-model="e.Price">{{/ifCond}}</td>
          <td style="background: inherit; display:inline-block; white-space: nowrap;">{{#ifCond @key '!=' ../k}}<button class="btn btn-sm btn-warning bi bi-pencil" c-click="edit('{{@key}}')" title="Edit"></button><button class="btn btn-sm btn-danger bi bi-trash" c-click="remove('{{@key}}')" title="Remove"></button>{{else}}<button class="btn btn-sm btn-success bi bi-save" c-click="update('{{@key}}')" title="Save"></button>{{/ifCond}}</td>
        </tr>
        {{/each}}
        </tbody>
      </table>
    </div>
    {{/if}}
    <p>
      <small>NOTE:  An Asset named gold, palladium, platinum, or silver will automatically retrieve its $/item from kitco.com.</small>
    </p>
  `
  // }}}
}
