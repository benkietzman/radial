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
    let f = a.d.Asset.Liquid;
    let s = c.scope('AssetLiquid',
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
        c.update('AssetLiquid');
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
        Amount: null
      },
      e:
      {
        k: null,
        Amount: null
      },
      k: null
    });
    c.setMenu('Asset', 'Liquid');
    s.f = f;
  },
  // }}}
  // {{{ template
  template: `
    <h3 class="page-header">Asset - Liquid</h3>
    {{#if f}}
    <div class="table-responsive">
      <table class="table table-striped bg-success-subtle border border-success-subtle">
        <thead>
        <tr>
          <th style="background: inherit;">Name</th>
          <th style="background: inherit;">Amount</th>
          <th style="background: inherit;"></th>
        </tr>
        </thead>
        <tbody>
        <tr>
          <td style="background: inherit;"><input type="text" class="form-control form-control-sm bg-success-subtle" c-model="d.k"></td>
          <td style="background: inherit;"><input type="text" class="form-control form-control-sm bg-success-subtle" c-model="d.Amount"></td>
          <td style="background: inherit;"><button class="btn btn-sm btn-success bi bi-plus-circle" c-click="add()" title="Add"></button></td>
        </tr>
        {{#each f}}
        <tr>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../k}}{{else}}<input type="text" class="form-control form-control-sm bg-success-subtle" c-model="e.k">{{/ifCond}}</td>
          <td style="background: inherit;">{{#ifCond @key '!=' ../k}}{{../Amount}}{{else}}<input type="text" class="form-control form-control-sm bg-success-subtle" c-model="e.Amount">{{/ifCond}}</td>
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
