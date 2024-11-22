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
      s.info.v = 'Loading databases...';
      let request = {Interface: 'sqlite', 'Function': 'databases'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
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
      s.info.v = 'Loading structure...';
      let request = {Interface: 'sqlite', 'Function': 'query', Request: {Database: s.database.v.name, Statement: 'select sql from sqlite_master where name = \'' + s.table.v.name + '\''}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
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
      s.info.v = 'Loading tables...';
      let request = {Interface: 'sqlite', 'Function': 'query', Request: {Database: s.database.v.name, Statement: 'select name from sqlite_master where type=\'table\''}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
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
      s.info.v = 'Processing query...';
      let request = {Interface: 'sqlite', 'Function': 'query', Request: {Database: s.database.v.name, Statement: s.statement.v}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
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
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.init();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div c-model="info" class="text-warning"></div>
  <div class="input-group input-group-sm"><span class="input-group-text">Database</span><select class="form-control" c-model="database" c-change="getTables()" c-json>{{#each databases}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
  <div class="input-group input-group-sm"><span class="input-group-text">Table</span><select class="form-control" c-model="table" c-change="getStructure()" c-json>{{#each tables}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
  {{structure}}
  <textarea class="form-control" c-model="statement"></textarea>
  {{#if result}}
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
  {{/if}}
  `
  // ]]]
}
