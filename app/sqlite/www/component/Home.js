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
    // [[[ addDatabase()
    s.addDatabase = () =>
    {
      if (s.node.v)
      {
        if (s.newDatabase.v)
        {
          let request = {Interface: 'sqlite', 'Function': 'create', Request: {Database: s.newDatabase.v, Node: s.node.v}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            if (c.wsResponse(response, error))
            {
              s.getDatabases();
            }
            else
            {
              a.pushMessage(error.message);
            }
          });
        }
        else
        {
          alert('Please provide the Database.')
        }
      }
      else
      {
        alert('Please provide the Node.');
      }
    };
    // ]]]
    // [[[ enter()
    s.enter = () =>
    {
      if (window.event.keyCode == 13)
      {
        s.statement.v = s.statement.v.replace(/(?:\r\n|\r|\n)/g, '');
        s.query();
      }
    };
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
        s.getNodes();
        s.getTables();
      });
    };
    // ]]]
    // [[[ getNodes()
    s.getNodes = () =>
    {

      let request = {Interface: 'link', 'Function': 'status'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          let strNode = response.Response.Node;
          let nodes = [strNode];
          for (let i = 0; i < response.Response.Links.length; i++)
          {
            nodes.push(response.Response.Links[i]);
          }
          nodes.sort();
          s.subNodes = null;
          s.subNodes = [];
          for (let i = 0; i < nodes.length; i++)
          {
            let request = {Interface: 'sqlite', 'Function': 'status', Request: {Node: nodes[i]}};
            if (nodes[i] != strNode)
            {
              request.Node = nodes[i];
            }
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              if (c.wsResponse(response, error))
              {
                s.node = null;
                s.nodes = null;
                s.nodes = [];
                s.subNodes.push(response.Request.Node);
                s.subNodes.sort();
                s.nodes = s.subNodes;
                s.node = s.nodes[0];
                s.u();
              }
              else
              {
                s.message.v = error.message;
              }
            });
          }
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ getStructure()
    s.getStructure = () =>
    {
      if (s.table.v)
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
      }
      else
      {
        s.structure = null;
        s.u();
        s.statement.v = null;
        s.query();
      }
    };
    // ]]]
    // [[[ getTables()
    s.getTables = () =>
    {
      let request = {Interface: 'sqlite', 'Function': 'query', Request: {Database: s.database.v.name, Statement: 'select name from sqlite_master where type=\'table\' order by name'}};
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
      if (s.statement.v)
      {
        s.result = null;
        let request = {Interface: 'sqlite', 'Function': 'query', Request: {Database: s.database.v.name, Statement: s.statement.v}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            if (response.Response.ResultSet)
            {
              s.result = response.Response.ResultSet;
            }
            let tokens = s.statement.v.split(' ');
            if (tokens.length > 0)
            {
              let strAction = tokens[0].toLowerCase();
              if (strAction == 'create' || strAction == 'drop')
              {
                s.getTables();
              }
              else if (strAction == 'insert' || strAction == 'delete')
              {
                s.statement.v = 'select * from ' + s.table.v.name;
                s.query();
              }
            }
          }
          else
          {
            a.pushMessage(error.message);
          }
          s.u();
        });
      }
      else
      {
        s.result = null;
        s.u();
      }
    };
    // ]]]
    // [[[ removeDatabase()
    s.removeDatabase = () =>
    {
      if (s.database.v)
      {
        if (confirm('Are you sure you want to drop the "' + s.database.v.name + '" database?'))
        {
          let request = {Interface: 'sqlite', 'Function': 'drop', Request: {Database: s.database.v.name}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            if (c.wsResponse(response, error))
            {
              s.getDatabases();
            }
            else
            {
              a.pushMessage(error.message);
            }
          });
        }
      }
      else
      {
        alert('Please choose a Database.');
      }
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
          <div class="card" style="margin-top: 10px;">
            <div class="card-header bg-primary text-white" style="font-weight: bold;">
              Databases
              {{#if database}}
              <button class="btn btn-sm btn-danger bi bi-dash-circle float-end" c-click="removeDatabase()" title="remove database"></button>
              {{/if}}
            </div>
            <div class="card-body">
              <select class="form-select form-select-sm" c-model="database" c-change="getTables()" size="2" style="background: inherit; color: inherit; font-family: monospace, monospace; height: 25vh; margin-bottom: 10px;" c-json>{{#each databases}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
            </div>
            {{#if nodes}}
            <div class="card-footer">
              <div class="row">
                <div class="col-md" style="padding-right: 0px;">
                  <select class="form-select form-select-sm" c-model="node">{{#each nodes}}<option value="{{.}}">{{.}}</option>{{/each}}</select>
                </div>
                <div class="col-md" style="padding-left: 4px; padding-right: 4px;">
                  <input type="text" class="form-control form-control-sm" c-model="newDatabase" placeholder="database">
                </div>
                <div class="col-md-2" style="padding-left: 0px;">
                  <button class="btn btn-sm btn-success bi bi-plus-circle" c-click="addDatabase()" title="add database"></button>
                </div>
              </div>
            </div>
            {{/if}}
          </div>
        </div>
      </div>
      <div class="row">
        <div class="col">
          <div class="card" style="margin-top: 10px;">
            <div class="card-header bg-primary text-white" style="font-weight: bold;">
              Tables
            </div>
            <div class="card-body">
              <select class="form-select form-select-sm" c-model="table" c-change="getStructure()" size="2" style="background: inherit; color: inherit; font-family: monospace, monospace; height: 25vh; margin-bottom: 10px;" c-json>{{#each tables}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
            </div>
          </div>
        </div>
      </div>
    </div>
    <div class="col-md-9">
      <div class="row">
        <div class="col">
          <div class="card" style="margin-top: 10px;">
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
          <div class="card" style="margin-top: 10px;">
            <div class="card-header bg-primary text-white" style="font-weight: bold;">
              Query
            </div>
            <div class="card-body">
              <textarea class="form-control" id="statement" c-model="statement" c-keyup="enter()"></textarea>
            </div>
          </div>
        </div>
      </div>
      {{#if result}}
      <div class="row">
        <div class="col">
          <div class="card" style="margin-top: 10px;">
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
