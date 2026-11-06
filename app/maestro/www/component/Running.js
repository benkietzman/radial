// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2026-11-06
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // [[[ controller()
  controller(id, nav)
  {
    // [[[ prep work
    let a = app;
    let c = common;
    let s = c.scope('Running',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Running');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ stop()
    s.stop = (nIndex) =>
    {
      let request = {Interface: 'maestro', 'Function': 'stop', Request: {Node: s.r[nIndex].Node, Plan: s.r[nIndex].Plan, Flow: s.r[nIndex].Flow, Thread: s.r[nIndex].Thread}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (!c.wsResponse(response, error))
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ running()
    s.running = () =>
    {
      let request = {Interface: 'maestro', 'Function': 'running'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          let r = response.Response;
          s.r = [];
          for (let n in r)
          {
            for (let p in r[n])
            {
              for (let f in r[n][p].Flows)
              {
                for (let t in r[n][p].Flows[f])
                {
                  s.r.push({Node: n, Plan: p, Flow: f, Owner: r[n][p].Owner, Thread: t, Time: r[n][p].Flows[f][t]});
                }
              }
            }
          }
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      s.running();
    };
    // ]]]
    // [[[ main
    c.setMenu('Running');
    s.u();
    if (a.ready())
    {
      s.init();
    }
    c.attachEvent('appReady', (data) =>
    {
      s.init();
    });
    c.attachEvent('commonWsMessage_Maestro', (data) =>
    {
      if (data.detail && data.detail.Action && (data.detail.Action == 'accessRemove' || data.detail.Action == 'accessRequest' || data.detail.Action == 'accessRequestApprove' || data.detail.Action == 'accessRequestCancel' || data.detail.Action == 'accessRequestDeny' || data.detail.Action == 'planAdd' || data.detail.Action == 'planRemove' || data.detail.Action == 'flowAdd' || data.detail.Action == 'flowRemove' || data.detail.Action == 'flowUpdate') && !s.composition)
      {
        s.init();
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  {{#isValid}}
  <h5>Running Flows</h5>
  <div class="table-responsive">
    <table class="table table-sm table-condensed table-striped">
    <thead>
      <tr>
        <th>Node</th>
        <th>Plan</th>
        <th>Flow</th>
        <th>Thread</th>
        <th>Started</th>
        <th></th>
      </tr>
    </thead>
    <tbody>
      {{#each ../r}}
      <tr>
        <td>
          {{Node}}
        </td>
        <td>
          <a href="#/Plans/{{Plan}}">{{Owner}}</a>
        </td>
        <td>
          <a href="#/Plans/{{Plan}}/Flow/{{Flow}}">{{Flow}}</a>
        </td>
        <td>
          {{Thread}}
        </td>
        <td>
          {{datetime Time}}
        </td>
        <td>
          <button class="btn btn-sm btn-danger btn-trash" c-click="stop({{@key}})">Stop</button>
        </td>
      </tr>
      {{/each}}
    </tbody>
    </table>
  </div>
  {{else}}
    <b class="text-warning">Please login to view this page.</b>
  {{/isValid}}
  `
  // ]]]
}
