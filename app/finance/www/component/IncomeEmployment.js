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
    let f = a.d.Income.Employment;
    let s = c.scope('IncomeEmployment',
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
            alert('Employer already exists.');
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
          if (!c.isNull(d.Salary) && String(d.Salary).length > 0)
          {
            if (!c.isNull(d.Medical) && String(d.Medical).length > 0)
            {
              if (!c.isNull(d.Invest) && String(d.Invest).length > 0)
              {
                if (!c.isNull(d.Match) && String(d.Match).length > 0)
                {
                  if (!c.isNull(d.Tax) && String(d.Tax).length > 0)
                  {
                    bResult = true;
                  }
                  else
                  {
                    error.message = 'Please provide the Tax.';
                  }
                }
                else
                {
                  error.message = 'Please provide the Match.';
                }
              }
              else
              {
                error.message = 'Please provide the Invest.';
              }
            }
            else
            {
              error.message = 'Please provide the Medical.';
            }
          }
          else
          {
            error.message = 'Please provide the Salary.';
          }
        }
        else
        {
          error.message = 'Please provide the Employer.';
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
        c.update('IncomeEmployment');
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
          alert('Please provide a valid Employer.');
        }
      },
      // }}}
      a: a,
      c: c,
      d:
      {
        k: null,
        Salary: null,
        Bonus: null,
        Medical: null,
        Invest: null,
        Match: null,
        Tax: null
      },
      e:
      {
        k: null,
        Salary: null,
        Bonus: null,
        Medical: null,
        Invest: null,
        Match: null,
        Tax: null
      },
      k: null
    });
    c.setMenu('Income', 'Employment');
    s.f = f;
  },
  // }}}
  // {{{ template
  template: `
    <h3 class="page-header">Income - Employments</h3>
    {{#if f}}
    <div class="table-responsive">
      <table class="table table-striped">
        <thead>
        <tr>
          <th title="Name of Employer">Employer</th>
          <th title="Gross Salary">Salary</th>
          <th title="Bonus Amount">Bonus</th>
          <th title="Health Savings Account">HSA</th>
          <th title="Medical Premium">Medical</th>
          <th title="Investment Percentage">Invest</th>
          <th title="Employer Match Percentage">Match</th>
          <th title="Withholding Percentage">Tax</th>
          <th></th>
        </tr>
        </thead>
        <tbody>
        <tr>
          <td><input type="text" class="form-control form-control-sm" c-model="d.k"></td>
          <td><input type="text" class="form-control form-control-sm" c-model="d.Salary"></td>
          <td><input type="text" class="form-control form-control-sm" c-model="d.Bonus"></td>
          <td><input type="text" class="form-control form-control-sm" c-model="d.Hsa"></td>
          <td><input type="text" class="form-control form-control-sm" c-model="d.Medical"></td>
          <td><div class="input-group input-group-sm"><input type="text" class="form-control form-control-sm" c-model="d.Invest"><span class="input-group-text">%</span></div></td>
          <td><div class="input-group input-group-sm"><input type="text" class="form-control form-control-sm" c-model="d.Match"><span class="input-group-text">%</span></div></td>
          <td><div class="input-group input-group-sm"><input type="text" class="form-control form-control-sm" c-model="d.Tax"><span class="input-group-text">%</span></div></td>
          <td><button class="btn btn-sm btn-success bi bi-plus-circle" c-click="add()" title="Add"></button></td>
        </tr>
        {{#each f}}
        <tr">
          <td>{{#ifCond @key '!=' ../k}}{{../k}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.k">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Salary}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.Salary">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Bonus}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.Bonus">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Hsa}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.Hsa">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Medical}}{{else}}<input type="text" class="form-control form-control-sm" c-model="e.Medical">{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Invest}}%{{else}}<div class="input-group input-group-sm"><input type="text" class="form-control form-control-sm" c-model="e.Invest"><span class="input-group-text">%</span></div>{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Match}}%{{else}}<div class="input-group input-group-sm"><input type="text" class="form-control form-control-sm" c-model="e.Match"><span class="input-group-text">%</span></div>{{/ifCond}}</td>
          <td>{{#ifCond @key '!=' ../k}}{{../Tax}}%{{else}}<div class="input-group input-group-sm"><input type="text" class="form-control form-control-sm" c-model="e.Tax"><span class="input-group-text">%</span></div>{{/ifCond}}</td>
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
