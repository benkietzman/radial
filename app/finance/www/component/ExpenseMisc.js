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
    let f = a.d.Expense.Misc;
    let s = c.scope('ExpenseMisc',
    {
      // {{{ add()
      add: () =>
      {
        a.add(f, c.simplify(s.d), s.u);
      },
      // }}}
      // {{{ edit()
      edit: (k) =>
      {
        a.edit(f, s.e, s, k, s.u);
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
        c.update('ExpenseMisc');
      },
      // }}}
      // {{{ update()
      update: (k) =>
      {
        a.update(f, c.simplify(s.e), s, k, s.u);
      },
      // }}}
      a: a,
      c: c,
      d:
      {
        k: null,
        Amount: null,
        Spend: 0
      },
      e:
      {
        k: null,
        Amount: null,
        Spend: 0
      },
      k: null
    });
    c.setMenu('Expense', 'Misc');
    s.f = f;
  },
  // }}}
  // {{{ template
  template: `
    <h3 class="page-header">Expense - Misc</h3>
    {{#if f}}
    <div class="table-responsive">
      <table class="table table-striped bg-danger-subtle border border-danger-subtle">
        <thead>
        <tr>
          <th style="background: inherit;">Name</th>
          <th style="background: inherit;">Amount</th>
          <th style="background: inherit;">Spend</th>
          <th style="background: inherit;"></th>
        </tr>
        </thead>
        <tbody>
        <tr>
          <td style="background: inherit;"><input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="d.k"></td>
          <td style="background: inherit;"><input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="d.Amount"></td>
          <td style="background: inherit;"><select class="form-control form-control-sm bg-danger-subtle" c-model="d.Spend"><option value="0">No</option><option value="1">Yes</option></select></td>
          <td style="background: inherit;"><button class="btn btn-sm btn-success bi bi-plus-circle" c-click="add()" title="Add"></button></td>
        </tr>
        {{#each f}}
        <tr>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../k}}{{else}}<input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="e.k">{{/ifCond}}</td>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../Amount}}{{else}}<input type="text" class="form-control form-control-sm bg-danger-subtle" c-model="e.Amount">{{/ifCond}}</td>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{#ifCond ../Spend '==' '1'}}Yes{{else}}No{{/ifCond}}{{else}}<select class="form-control form-control-sm bg-danger-subtle" c-model="e.Spend"><option value="0">No</option><option value="1">Yes</option></select>{{/ifCond}}</td>
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
