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
    let f = a.d.Liability.Compound;
    let s = c.scope('LiabilityCompound',
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
            alert('Compound already exists.');
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
          if (!c.isNull(d.Principal) && String(d.Principal).length > 0)
          {
            if (!c.isNull(d.Rate) && String(d.Rate).length > 0)
            {
              if (!c.isNull(d.Payment) && String(d.Payment).length > 0)
              {
                if (!c.isNull(d.Duration) && String(d.Escrow).length > 0)
                {
                  bResult = true;
                }
                else
                {
                  error.message = 'Please provide the Duration.';
                }
              }
              else
              {
                error.message = 'Please provide the Payment.';
              }
            }
            else
            {
              error.message = 'Please provide the Rate.';
            }
          }
          else
          {
            error.message = 'Please provide the Principal.';
          }
        }
        else
        {
          error.message = 'Please provide the Compound.';
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
        c.update('LiabilityCompound');
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
          alert('Please provide a valid Compound.');
        }
      },
      // }}}
      a: a,
      c: c,
      d:
      {
        k: null,
        Principal: null,
        Rate: null,
        Compoundings: null,
        Payment: null
      },
      e:
      {
        k: null,
        Principal: null,
        Rate: null,
        Compoundings: null,
        Payment: null
      },
      k: null
    });
    c.setMenu('Liability', 'Compound');
    s.f = f;
  },
  // }}}
  // {{{ template
  template: `
    <h3 class="page-header">Liability - Compound</h3>
    {{#if f}}
    <div class="table-responsive">
      <table class="table table-striped">
        <thead>
        <tr>
          <th>Name</th>
          <th>Principal</th>
          <th>Rate</th>
          <th>Compoundings (#/yr)</th>
          <th>Payment</th>
          <th></th>
        </tr>
        </thead>
        <tbody>
        <tr>
          <td><input type="text" class="form-control form-control-sm" c-model="d.k"></td>
          <td><input type="text" class="form-control form-control-sm" c-model="d.Principal"></td>
          <td><div class="input-group input-group-sm"><input type="text" class="form-control form-control-sm" c-model="d.Rate"><span class="input-group-text">%</span></div></td>
          <td><input type="text" class="form-control form-control-sm" c-model="d.Compoundings"></td>
          <td><input type="text" class="form-control form-control-sm" c-model="d.Payment"></td>
          <td><button class="btn btn-sm btn-success bi bi-plus-circle" c-click="add()" title="Add"></button></td>
        </tr>
        {{#each f}}
        <tr>
          <td>{{#ifCond @key '!=' ../k}}{{../k}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.k">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Principal}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.Principal">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Rate}}%{{else}}<div class="input-group input-group-sm"><input type="text" class="form-control form-control-sm" c-model="e.Rate"><span class="input-group-text">%</span></div>{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Compoundings}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.Compoundings">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Payment}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.Payment">{{/ifCond}}</td>
          <td style="display:inline-block;">{{#ifCond @key '!=' ../k}}<button class="btn btn-sm btn-warning bi bi-pencil" c-click="edit('{{@key}}')" title="Edit"></button><button class="btn btn-sm btn-danger bi bi-trash" c-click="remove('{{@key}}')" title="Remove"></button>{{else}}<button class="btn btn-sm btn-success bi bi-save" c-click="update('{{@key}}')" title="Save"></button>{{/ifCond}}</td>
        </tr>
        {{/each}}
        </tbody>
      </table>
    </div>
    {{/if}}
  `
  // }}}
}
