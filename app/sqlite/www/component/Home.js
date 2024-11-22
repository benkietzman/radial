// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2024-11-21
// copyright  : kietzman.org
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
    let s = c.scope('Home',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Home');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ getDatabases()
    s.getDatabases = () =>
    {
      let request = {Interface: 'sqlite', 'Function': 'databases'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.databases = null;
        s.databases = [];
        s.database = null;
        if (c.wsResponse(response, error))
        {
          for (let key of Object.keys(response.Response))
          {
            s.databases.push({name: key});
          }
          if (s.databases.length > 0)
          {
            s.database = s.databases[0];
          }
        }
        else
        {
          a.pushMessage(error.message);
        }
        s.u();
        s.getTables();
      });
    };
    // ]]]
    // [[[ getStructure()
    s.getStructure = () =>
    {
      let request = {Interface: 'sqlite', 'Function': 'query', Request: {Database: s.database.v.name, Statement: 'select sql from sqlite_master where name = \'' + s.table.v.name + '\''}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.structure = null;
        if (c.wsResponse(response, error))
        {
          s.structure = response.Response.ResultSet[0].sql;
        }
        else
        {
          a.pushMessage(error.message);
        }
        s.u();
        s.statement.v = 'select * from ' + s.table.v.name;
        s.query();
      });
    };
    // ]]]
    // [[[ getTables()
    s.getTables = () =>
    {
      let request = {Interface: 'sqlite', 'Function': 'query', Request: {Database: s.database.v.name, Statement: 'select name from sqlite_master where type=\'table\''}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.tables = null;
        s.tables = [];
        s.table = null;
        if (c.wsResponse(response, error))
        {
          s.tables = response.Response.ResultSet;
          if (s.tables.length > 0)
          {
            s.table = s.tables[0];
          }
        }
        else
        {
          a.pushMessage(error.message);
        }
        s.u();
        s.getStructure();
      });
    };
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      s.getDatabases();
    };
    // ]]]
    // [[[ query()
    s.query = () =>
    {
      let request = {Interface: 'sqlite', 'Function': 'query', Request: {Database: s.database.v.name, Statement: s.statement.v}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.result = null;
        if (c.wsResponse(response, error))
        {
          s.result = response.Response.ResultSet;
        }
        else
        {
          a.pushMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ main
    c.setMenu('Home');
    s.u();
    if (a.ready())
    {
      s.init();
    }
    c.attachEvent('appReady', (data) =>
    {
      s.init();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div class="row">
    <div class="col-md-3">
      <div class="row">
        <div class="col">
          <div class="card">
            <div class="card-header bg-primary text-white" style="font-weight: bold;">
              Databases
            </div>
            <div class="card-body">
              <select class="form-select form-select-sm" c-model="database" c-change="getTables()" size="2" style="background: inherit; color: inherit; font-family: monospace, monospace; height: 30vh; margin-bottom: 10px;" c-json>{{#each databases}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
            </div>
          </div>
        </div>
      </div>
      <div class="row">
        <div class="col">
          <div class="card">
            <div class="card-header bg-primary text-white" style="font-weight: bold;">
              Tables
            </div>
            <div class="card-body">
              <select class="form-select form-select-sm" c-model="table" c-change="getStructure()" size="2" style="background: inherit; color: inherit; font-family: monospace, monospace; height: 30vh; margin-bottom: 10px;" c-json>{{#each tables}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
            </div>
          </div>
        </div>
      </div>
    </div>
    <div class="col-md-9">
      <div class="row">
        <div class="col">
          <div class="card">
            <div class="card-header bg-primary text-white" style="font-weight: bold;">
              Structure
            </div>
            <div class="card-body">
              <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{structure}}</pre>
            </div>
          </div>
        </div>
      </div>
      <div class="row">
        <div class="col">
          <div class="card">
            <div class="card-header bg-primary text-white" style="font-weight: bold;">
              Query
            </div>
            <div class="card-body">
              <textarea class="form-control" c-model="statement"></textarea>
            </div>
            <div class="card-footer">
              <button class="btn btn-sm btn-success float-end" c-click="query()">Submit</button>
            </div>
          </div>
        </div>
      </div>
      {{#if result}}
      <div class="row">
        <div class="col">
          <div class="card">
            <div class="card-header bg-primary text-white" style="font-weight: bold;">
              Results
            </div>
            <div class="card-body">
              <div class="table-responsive">
                <table class="table table-condensed table-striped">
                  <thead>
                    <tr>
                      {{#each result.[0]}}
                      <th>{{@key}}</th>
                      {{/each}}
                    </tr>
                  </thead>
                  <tbody>
                    {{#each result}}
                    <tr>
                      {{#each .}}
                      <td>{{.}}</td>
                      {{/each}}
                    </tr>
                    {{/each}}
                  </tbody>
                </table>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
  {{/if}}
  `
  // ]]]
}
