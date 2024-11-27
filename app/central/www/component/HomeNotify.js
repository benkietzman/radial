// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2024-07-24
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
    let s = c.scope('HomeNotify',
    {
      // [[[ u()
      u: () =>
      {
        c.update('HomeNotify');
      },
      // ]]]
      a: a,
      c: c,
      display: false,
      types: ['application', 'group', 'server', 'user']
    });
    // ]]]
    // [[[ load()
    s.load = () =>
    {
      if (c.isValid())
      {
        s.application = null;
        s.applications = null;
        s.display = false;
        s.group = null;
        s.groups = null;
        s.notified = null;
        s.server = null;
        s.servers = null;
        s.user = null;
        s.users = null;
        s.u();
        if (s.type.v == 'application')
        {
          s.loadApplications();
        }
        else if (s.type.v == 'group')
        {
          s.loadGroups();
        }
        else if (s.type.v == 'server')
        {
          s.loadServers();
        }
        else if (s.type.v == 'user')
        {
          s.loadUsers();
        }
      }
      else
      {
        s.info.v = 'Please login to send a notification.';
      }
    };
    // ]]]
    // [[[ loadApplication()
    s.loadApplication = () =>
    {
      if (c.isValid())
      {
        s.display = ((s.application.v)?true:false);
        s.u();
      }
      else
      {
        s.info.v = 'Please login to send a notification.';
      }
    };
    // ]]]
    // [[[ loadApplications()
    s.loadApplications = () =>
    {
      if (c.isValid())
      {
        s.info.v = 'Retrieving applications...';
        let request = {Interface: 'central', 'Function': 'applications', Request: {}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.info.v = null;
            s.applications = response.Response;
            s.u();
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      else
      {
        s.info.v = 'Please login to send a notification.';
      }
    };
    // ]]]
    // [[[ loadGroup()
    s.loadGroup = () =>
    {
      if (c.isValid())
      {
        s.display = ((s.group.v)?true:false);
        s.u();
      }
      else
      {
        s.info.v = 'Please login to send a notification.';
      }
    };
    // ]]]
    // [[[ loadGroups()
    s.loadGroups = () =>
    {
      if (c.isValid())
      {
        s.info.v = 'Retrieving groups...';
        let request = {Interface: 'central', 'Function': 'groups', Request: {}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.info.v = null;
            s.groups = response.Response;
            s.u();
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      else
      {
        s.info.v = 'Please login to send a notification.';
      }
    };
    // ]]]
    // [[[ loadServer()
    s.loadServer = () =>
    {
      if (c.isValid())
      {
        s.display = ((s.server.v)?true:false);
        s.u();
      }
      else
      {
        s.info.v = 'Please login to send a notification.';
      }
    };
    // ]]]
    // [[[ loadServers()
    s.loadServers = () =>
    {
      if (c.isValid())
      {
        s.info.v = 'Retrieving servers...';
        let request = {Interface: 'central', 'Function': 'servers', Request: {}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.info.v = null;
            s.servers = response.Response;
            s.u();
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      else
      {
        s.info.v = 'Please login to send a notification.';
      }
    };
    // ]]]
    // [[[ loadUser()
    s.loadUser = () =>
    {
      if (c.isValid())
      {
        s.display = ((s.user.v)?true:false);
        s.u();
      }
      else
      {
        s.info.v = 'Please login to send a notification.';
      }
    };
    // ]]]
    // [[[ loadUsers()
    s.loadUsers = () =>
    {
      if (c.isValid())
      {
        s.info.v = 'Retrieving users...';
        let request = {Interface: 'central', 'Function': 'users', Request: {}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.info.v = null;
            s.users = response.Response;
            for (let i = 0; i < s.users.length; i++)
            {
              s.users[i].name = s.users[i].last_name + ', ' + s.users[i].first_name + ' (' + s.users[i].userid + ')';
            }
            s.u();
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      else
      {
        s.info.v = 'Please login to send a notification.';
      }
    };
    // ]]]
    // [[[ send()
    s.send = () =>
    {
      if (c.isValid())
      {
        if (s.type.v)
        {
          if (s.notification.v)
          {
            s.info.v = 'Sending notification...';
            s.notified.v = null;
            let request = {Interface: 'central', 'Function': s.type.v + 'Notify', Request: {}};
            if (s.type.v == 'application')
            {
              request.Request.id = s.application.v.id;
            }
            else if (s.type.v == 'group')
            {
              request.Request.id = s.group.v.id;
            }
            else if (s.type.v == 'server')
            {
              request.Request.id = s.server.v.id;
            }
            else if (s.type.v == 'user')
            {
              request.Request.id = s.user.v.id;
            }
            request.Request.notification = s.notification.v;
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              if (c.wsResponse(response, error))
              {
                s.info.v = null;
                s.notified.v = 'Notification has been sent.';
                s.u();
              }
              else
              {
                c.pushErrorMessage(error.message);
              }
            });
          }
          else
          {
            alert('Please provide the notification.');
          }
        }
        else
        {
          alert('Please select a Type.');
        }
      }
      else
      {
        s.info.v = 'Please login to send a notification.';
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Home', 'Notify');
    s.type = s.types[0];
    s.u();
    if (a.ready())
    {
      s.load();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.load();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <h3 class="page-header">Send a Notification</h3>
  <p>
    Notifications can be sent to applications, group, servers, and users.
  </p>
  <div class="row" style="margin-bottom: 10px;">
    <div class="col-md-12">
      <div c-model="info" class="text-warning"></div>
      {{#isValid}}
      <div class="row">
        <div class="col-md-3" style="padding-top: 10px;">
          <div class="input-group"><span class="input-group-text">Type</span><select class="form-control" c-model="type" c-change="load()">{{#each ../types}}<option value="{{.}}">{{.}}</option>{{/each}}</select></div>
          {{#if ../applications}}
          <div class="input-group"><span class="input-group-text">App</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow"></div>
          <select class="form-control" c-model="application" c-change="loadApplication()" size="2" style="height: 200px;" c-json>
            {{#eachFilter ../applications 'name' ../narrow}}
            <option value="{{json .}}">{{name}}</option>
            {{/eachFilter}}
          </select>
          {{/if}}
          {{#if ../groups}}
          <div class="input-group"><span class="input-group-text">Group</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow"></div>
          <select class="form-control" id="group" c-model="group" c-change="loadGroup()" size="2" style="height: 200px;" c-json>
            {{#eachFilter ../groups 'name' ../narrow}}
            <option value="{{json .}}">{{name}}</option>
            {{/eachFilter}}
          </select>
          {{/if}}
          {{#if ../servers}}
          <div class="input-group"><span class="input-group-text">Server</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow"></div>
          <select class="form-control" id="server" c-model="server" c-change="loadServer()" size="2" style="height: 200px;" c-json>
            {{#eachFilter ../servers 'name' ../narrow}}
            <option value="{{json .}}">{{name}}</option>
            {{/eachFilter}}
          </select>
          {{/if}}
          {{#if ../users}}
          <div class="input-group"><span class="input-group-text">User</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow"></div>
          <select class="form-control" id="user" c-model="user" c-change="loadUser()" size="2" style="height: 200px;" c-json>
            {{#eachFilter ../users 'name' ../narrow}}
            <option value="{{json .}}">{{name}}</option>
            {{/eachFilter}}
          </select>
          {{/if}}
        </div>
        <div class="col-md-9">
          {{#if ../display}}
          <div class="card border border-primary-subtle">
            <div class="card-header bg-primary fw-bold">
              {{#ifCond ../type.val "==" "application"}}
              {{../application.val.name}}
              {{else ifCond ../type.val "==" "group"}}
              {{../group.val.name}}
              {{else ifCond ../type.val "==" "server"}}
              {{../server.val.name}}
              {{else ifCond ../type.val "==" "user"}}
              {{../user.val.first_name}} {{../user.val.last_name}} ({{../user.val.userid}})
              {{/ifCond}}
            </div>
            <div class="card-body bg-primary-subtle">
              <div class="row" style="margin-top: 10px;">
                <div class="col-md-12">
                  <textarea c-model="notification" class="form-control bg-primary-subtle" rows="5" style="width: 100%;" placeholder="enter notification"></textarea>
                </div>
              </div>
              <div class="row" style="margin-top: 10px;">
                <div class="col-md-12">
                  <button class="btn btn-primary bi bi-send float-end" c-click="send()" title="Send Notification"></button>
                </div>
              </div>
            </div>
            {{#if notified}}
            <div class="card-footer">
              <div c-model="notified" class="text-success"></div>
            </div>
            {{/if}}
          </div>
          {{/if}}
        </div>
      </div>
      {{/isValid}}
    </div>
  </div>
  `
  // ]]]
}
