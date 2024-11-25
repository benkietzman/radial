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
    let f = a.d.Liability.Simple;
    let s = c.scope('LiabilitySimple',
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
            alert('Simple already exists.');
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
                if (!c.isNull(d.Duration) && String(d.Duration).length > 0)
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
          error.message = 'Please provide the Simple.';
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
        c.update('LiabilitySimple');
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
          alert('Please provide a valid Simple.');
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
        Payment: null,
        Duration: null
      },
      e:
      {
        k: null,
        Principal: null,
        Rate: null,
        Payment: null,
        Duration: null
      },
      k: null
    });
    c.setMenu('Liability', 'Simple');
    s.f = f;
  },
  // }}}
  // {{{ template
  template: `
    <h3 class="page-header">Liability - Simple</h3>
    {{#if f}}
    <div class="table-responsive">
      <table class="table table-striped bg-danger-subtle border border-danger-subtle">
        <thead>
        <tr>
          <th style="background: inherit;">Name</th>
          <th style="background: inherit;">Principal</th>
          <th style="background: inherit;">Rate</th>
          <th style="background: inherit;">Payment</th>
          <th style="background: inherit;">Duration (yrs)</th>
          <th style="background: inherit;"></th>
        </tr>
        </thead>
        <tbody>
        <tr>
          <td style="background: inherit;"><input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="d.k"></td>
          <td style="background: inherit;"><input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="d.Principal"></td>
          <td style="background: inherit;"><div class="input-group input-group-sm"><input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="d.Rate"><span class="input-group-text bg-danger-subtle">%</span></div></td>
          <td style="background: inherit;"><input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="d.Payment"></td>
          <td style="background: inherit;"><input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="d.Duration"></td>
          <td style="background: inherit;"><button class="btn btn-sm btn-success bi bi-plus-circle" c-click="add()" title="Add"></button></td>
        </tr>
        {{#each f}}
        <tr>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../k}}{{else}}<input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="e.k">{{/ifCond}}</td>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../Principal}}{{else}}<input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="e.Principal">{{/ifCond}}</td>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../Rate}}%{{else}}<div class="input-group input-group-sm"><input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="e.Rate"><span class="input-group-text bg-danger-subtle">%</span></div>{{/ifCond}}</td>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../Payment}}{{else}}<input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="e.Payment">{{/ifCond}}</td>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../Duration}}{{else}}<input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="e.Duration">{{/ifCond}}</td>
          <td style="background: inherit; display:inline-block; white-space: nowrap;">{{#ifCond @key '!=' ../k}}<button class="btn btn-sm btn-warning bi bi-pencil" c-click="edit('{{@key}}')" title="Edit"></button><button class="btn btn-sm btn-danger bi bi-trash" c-click="remove('{{@key}}')" title="Remove"></button>{{else}}<button class="btn btn-sm btn-success bi bi-save" c-click="update('{{@key}}')" title="Save"></button>{{/ifCond}}</td>
        </tr>
        {{/each}}
        </tbody>
      </table>
    </div>
    {{/if}}
  `
  // }}}
}
