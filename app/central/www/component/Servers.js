// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-27
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
    let s = c.scope('Servers',
    {
      // [[[ u()
      u: () =>
      {
        c.render(id, 'Servers', this);
      },
      // ]]]
      a: a,
      c: c,
      d: {},
      contact_types: [{type: 'Primary Developer'}, {type: 'Backup Developer'}, {type: 'Primary Contact'}, {type: 'Contact'}],
      issue: {priority: '1'},
      issueList: true,
      login_types: [],
      menu_accesses: [],
      notify_priorities: [],
      onlyOpenIssues: 1,
      package_types: [],
      serverDetail: {delay: 0, min_processes: 0, max_processes: 0, min_image: 0, max_image: 0, min_resident: 0, max_resident: 0}
    });
    // ]]]
    // [[[ addContact()
    s.addContact = () =>
    {
      s.contact.server_id = s.server.id;
      let request = {Interface: 'central', 'Function': 'serverUserAdd', Request: c.simplify(s.contact)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.server.contacts = null;
          s.showForm('Contacts');
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ addServer()
    s.addApplication = () =>
    {
      let request = {Interface: 'central', 'Function': 'serverAdd', Request: c.simplify(s.d.server)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          document.location.href = '#/Servers/?server=' + encodeURIComponent(s.d.server.name);
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ editContact()
    s.editContact = (nIndex) =>
    {
      let request = {Interface: 'central', 'Function': 'serverUserEdit', Request: c.simplify(s.server.contacts[nIndex])};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.server.contacts = null;
          s.showForm('Contacts');
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ editServer()
    s.editServer = () =>
    {
      let request = {Interface: 'central', 'Function': 'serverEdit', Request: c.simplify(s.server)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.server = null;
          s.loadServer();
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ initForms()
    s.initForms = () =>
    {
      if (!c.isDefined(s.server.forms))
      {
        s.server.forms =
        {
          General:      {value: 'General',      active: null},
          Applications: {value: 'Applications', active: null},
          Contacts:     {value: 'Contacts',     active: null},
        };
      }
      if (!c.isDefined(s.server.forms_order))
      {
        s.server.forms_order = ['General', 'Applications', 'Contacts'];
      }
    };
    // ]]]
    // [[[ loadServer()
    s.loadServer = () =>
    {
      if (c.isParam(nav, 'id') || c.isParam(nav, 'server'))
      {
        let strForm = ((c.isParam(nav, 'form'))?c.getParam(nav, 'form'):'General');
        if (c.isDefined(s.server) && s.server != null && (c.getParam(nav, 'id') == s.server.id || c.getParam(nav, 'server') == s.server.name))
        {
          s.showForm(strForm);
        }
        else
        {
          s.info.v = 'Retrieving server...';
          s.server = null;
          let request = {Interface: 'central', 'Function': 'server', Request: {form: strForm}};
          if (c.isParam(nav, 'id'))
          {
            request.Request.id = c.getParam(nav, 'id');
          }
          else
          {
            request.Request.name = c.getParam(nav, 'server');
          }
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              let strForm = response.Request.form;
              s.server = response.Response;
              s.server.bAdmin = c.isGlobalAdmin();
              s.initForms();
              s.showForm(strForm);
              if (s.server.bAdmin)
              {
                s.server.forms.Notify = {value: 'Notify', active: null};
                s.server.forms_order.splice(5, 0, 'Notify');
                s.showForm(strForm);
              }
              else if (c.isValid())
              {
                let request = {Interface: 'central', 'Function': 'isServerAdmin', Request: {id: s.server.id, form: strForm}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    let strForm = response.Request.form;
                    s.server.bAdmin = true;
                    s.server.forms.Notify = {value: 'Notify', active: null};
                    s.server.forms_order.splice(5, 0, 'Notify');
                    s.showForm(strForm);
                  }
                  else
                  {
                    s.message.v = error.message;
                  }
                });
              }
              let request = {Interface: 'junction', Request: [{Service: 'addrInfo', Server: s.server.name}]};
              c.wsRequest('radial', request).then((response) =>
              {
                let error = {};
                if (c.wsResponse(response, error))
                {
                  s.server.ips = [];
                  if (response.Response[0].IPv6)
                  {
                    for (let i = 0; i < response.Response[0].IPv6.length; i++)
                    {
                      s.server.ips.push(response.Response[0].IPv6[i]);
                    }
                  }
                  else if (response.Response[0].IPv4)
                  {
                    for (let i = 0; i < response.Response[0].IPv4.length; i++)
                    {
                      s.server.ips.push(response.Response[0].IPv4[i]);
                    }
                  }
                }
                else
                {
                  s.message.v = error.message;
                }
              });
              if (s.server.parent_id)
              {
                request = null;
                request = {Interface: 'central', 'Function': 'server', Request: {id: s.server.parent_id}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    s.server['parent'] = response.Response;
                  }
                  else
                  {
                    s.message.v = error.message;
                  }
                });
              }
              request = null;
              request = {Interface: 'central', 'Function': 'serversByParentID', Request: {parent_id: s.server.id}};
              c.wsRequest('radial', request).then((response) =>
              {
                var error = {};
                if (c.wsResponse(response, error))
                {
                  s.server['children'] = response.Response;
                }
                else
                {
                  s.message.v = error.message;
                }
              });
              s.sysInfoStatus();
            }
            else
            {
              s.message.v = error.message;
            }
          });
        }
      }
    };
    // ]]]
    // [[[ loadServers()
    s.loadServers = () =>
    {
      if (s.list)
      {
        if (c.isParam(nav, 'letter'))
        {
          s.letter = c.getParam(nav, 'letter');
        }
        else if (!c.isDefined(s.letter))
        {
          s.letter = '#';
        }
        s.servers = null;
        s.servers = [];
        s.u();
        s.info.v = 'Retrieving servers...';
        let request = {Interface: 'central', 'Function': 'servers', Request: {}};
        if (s.letter != 'ALL')
        {
          request.Request.letter = s.letter;
        }
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.info.v = null;
          if (c.wsResponse(response, error))
          {
            let appList = [];
            for (let i = 0; i < response.Response.length; i++)
            {
              s.servers.push(response.Response[i]);
            }
            for (let i = 0; i < s.servers.length; i++)
            {
              let request = {Interface: 'central', 'Function': 'serverUsersByServerID', Request: {server_id: s.servers[i].id, 'Primary Admin': 1, i: i}};
              c.wsRequest('radial', request).then((response) =>
              {
                let error = {};
                if (c.wsResponse(response, error))
                {
                  let i = response.Request.i;
                  s.servers[i].contacts = response.Response;
                  s.u();
                }
                else
                {
                  s.message.v = error.message;
                }
              });
              if (s.servers[i].parent_id)
              {
                request = {Interface: 'central', 'Function': 'server', Request: {id: s.servers[i].parent_id, i: i}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    let i = response.Request.i;
                    s.servers[i]['parent'] = response.Response;
                    s.u();
                  }
                  else
                  {
                    s.message.v = error.message;
                  }
                });
              }
              request = {Interface: 'central', 'Function': 'serversByParentID', Request: {parent_id: s.servers[i].id, i: i}};
              c.wsRequest('radial', request).then((response) =>
              {
                let error = {};
                if (c.wsResponse(response, error))
                {
                  let i = response.Request.i;
                  s.servers[i].children = response.Response;
                  s.u();
                }
                else
                {
                  s.message.v = error.message;
                }
              });
            }
            s.u();
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
      else
      {
        s.loadServer();
      }
    };
    // ]]]
    // [[[ preEditContact()
    s.preEditContact = (nIndex, bEdit) =>
    {
      s.server.contacts[nIndex].bEdit = bEdit;
      if (!bEdit)
      {
        s.server.contacts[nIndex] = c.simplify(s.server.contacts[nIndex]);
      }
      s.u();
    };
    // ]]]
    // [[[ preEditServer()
    s.preEditServer = (bEdit) =>
    {
      s.server.bEdit = bEdit;
      if (!bEdit)
      {
        s.server = c.simplify(s.server);
      }
      s.u();
    };
    // ]]]
    // [[[ removeContact()
    s.removeContact = (nID) =>
    {
      if (confirm('Are you sure you want to remove this server contact?'))
      {
        let request = {Interface: 'central', 'Function': 'serverUserRemove', Request: {id: nID}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.server.contacts  = null;
            s.showForm('Contacts');
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
    };
    // ]]]
    // [[[ removeServer()
    s.removeServer = () =>
    {
      if (confirm('Are you sure you want to remove this server?'))
      {
        let request = {Interface: 'central', 'Function': 'serverRemove', Request: {id: s.server.id}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.server = null;
            document.location.href = '#/Servers';
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
    };
    // ]]]
    // [[[ sendNotification()
    s.sendNotification = () =>
    {
      if (confirm('Are you sure you want to send this server notification?'))
      {
        let request = {Interface: 'central', 'Function': 'serverNotify', Request: {id: s.server.id, notification: s.notification.v, server: location.host}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.bNotified = true;
            s.u();
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
    };
    // ]]]
    // [[[ showForm()
    s.showForm = (strForm) =>
    {
      s.info.v = null;
      s.initForms();
      if (!c.isDefined(s.server.forms[strForm]))
      {
        strForm = 'General';
      }
      for (let key of Object.keys(s.server.forms))
      {
        s.server.forms[key].active = null;
      }
      s.server.forms[strForm].active = 'active';
      // [[[ General
      if (strForm == 'General')
      {
      }
      // ]]]
      // [[[ Applications
      else if (strForm == 'Applications')
      {
        if (!c.isDefined(s.application.applications) || s.application.applications == null)
        {
          s.info.v = 'Retrieving applications...';
          let request = {Interface: 'central', 'Function': 'applicationsByServerID', Request: {server_id: s.server.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              s.server.applications = response.Response;
              s.u();
            }
            else
            {
              s.message.v = error.message;
            }
          });
        }
      }
      // ]]]
      // [[[ Contacts
      else if (strForm == 'Contacts')
      {
        if (!c.isDefined(s.server.contacts) || s.server.contacts == null)
        {
          s.contact = {server_id: s.server.id, type: s.contact_types[3], admin: a.m_noyes[0], locked: a.m_noyes[0], notify: a.m_noyes[1]};
          s.server.contacts = null;
          s.server.contacts = [];
          s.u();
          s.info.v = 'Retrieving contacts...';
          let request = {Interface: 'central', 'Function': 'serverUsersByServerID', Request: {server_id: s.server.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              for (let i = 0; i < response.Response.length; i++)
              {
                for (let j = 0; j < s.contact_types.length; j++)
                {
                  if (response.Response[i].type.type == s.contact_types[j].type)
                  {
                    response.Response[i].type = s.contact_types[j];
                  }
                }
                for (let j = 0; j < a.m_noyes.length; j++)
                {
                  if (response.Response[i].admin.value == a.m_noyes[j].value)
                  {
                    response.Response[i].admin = a.m_noyes[j];
                  }
                  if (response.Response[i].notify.value == a.m_noyes[j].value)
                  {
                    response.Response[i].notify = a.m_noyes[j];
                  }
                  if (response.Response[i].physical_access.value == a.m_noyes[j].value)
                  {
                    response.Response[i].physical_access = a.m_noyes[j];
                  }
                }
                s.application.contacts.push(response.Response[i]);
              }
              s.u();
            }
            else
            {
              s.message.v = error.message;
            }
          });
        }
      }
      // ]]]
      // [[[ Notify
      else if (strForm == 'Notify')
      {
      }
      // ]]]
      s.u();
    };
    // ]]]
    // [[[ sysInfoStatus()
    s.sysInfoStatus = () =>
    {
      if (s.server && s.server.name)
      {
        let request = {Interface: 'junction', Request: [{Service: 'sysInfo', Action: 'system', Server: s.server.name}]};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.server.sysInfo = response.Response[1];
            s.server.sysInfo.MainUsage = s.server.sysInfo.MainUsed * 100 / s.server.sysInfo.MainTotal;
            s.server.sysInfo.SwapUsage = s.server.sysInfo.SwapUsed * 100 / s.server.sysInfo.SwapTotal;
            s.server.sysInfo.Alarms = s.server.sysInfo.Alarms.split(',');
            s.u();
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
    };
    // ]]]
    // [[[ sysInfoUpdate()
    s.sysInfoUpdate = () =>
    {
      c.wsRequest('radial', {Interface: 'junction', Request: [{Service: 'sysInfo', Action: 'update'}]}).then((response) =>
      {
        let error = {};
        if (!c.wsResponse(response, error))
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ toggleClosedIssues()
    s.toggleClosedIssues = () =>
    {
      s.application.issues = null;
      s.onlyOpenIssues = ((s.onlyOpenIssues == 1)?0:1);
      s.showForm('Issues');
    };
    // ]]]
    // [[[ main
    c.setMenu('Servers');
    s.list = true;
    if (c.isParam(nav, 'id') || c.isParam(nav, 'server'))
    {
      s.list = false;
    }
    s.u();
    if (a.ready())
    {
      s.loadServers();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.loadServers();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <!-- [[[ servers -->
  {{#if list}}
  <h3 class="page-header">Servers</h3>
  <div class="input-group float-end"><span class="input-group-text">Narrow</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow Results"></div>
  {{#each a.m_letters}}
  <div style="display: inline-block;">
    <a href="#/Servers/?letter={{urlEncode .}}">
      <button class="btn btn-sm btn-{{#ifCond . "==" @root.letter}}warning{{else}}primary{{/ifCond}}">{{.}}</button>
    </a>
  </div>
  {{/each}}
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Server</th><th>Primary Admins</th><th>Parent</th><th>Children</th>
      </tr>
      {{#isValid "Central"}}
      <tr>
        <td><input type="text" class="form-control" c-model="d.server.name" placeholder="Server Name"></td>
        <td><button class="btn btn-primary" c-click="addServer()">Add Server</button></td>
      </tr>
      {{/isValid}}
      {{#eachFilter servers "name" narrow}}
      <tr style="{{application.style}}">
        <td valign="top">
          <a href="#/Servers/{{id}}">{{name}}</a>
        </td>
        <td valign="top">
          <table class="table table-condensed" style="background: inherit;">
            {{#each contacts}}
            <tr>
              <td><a href="#/Users/{{user_id}}">{{last_name}}, {{first_name}}</a> <small>({{userid}})</small></td>
            </tr>
            {{/each}}
          </table>
        </td>
        <td valign="top">
          <table class="table table-condensed" style="background: inherit;">
            {{#each parents}}
            <tr>
              <td><a href="#/Servers/{{server_id}}">{{name}}</a></td>
            </tr>
            {{/each}}
          </table>
        </td>
        <td valign="top">
          <table class="table table-condensed" style="background: inherit;">
            {{#each children}}
            <tr>
              <td><a href="#/Servers/{{server_id}}">{{name}}</a></td>
            </tr>
            {{/each}}
          </table>
        </td>
      </tr>
      {{/eachFilter}}
    </table>
  </div>
  <!-- ]]] -->
  <!-- [[[ server -->
  {{else}}
  <h3 class="page-header">{{server.name}}</h3>
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
  <nav class="container navbar navbar-expand-lg navbar-dark bg-dark bg-gradient">
    <div class="container-fluid">
      <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#srvnavigationbar" aria-controls="srvnavigationbar" aria-expanded="false", aria-label="Toggle navigation">
        <span class="navbar-toggler-icon"></span>
      </button>
      <div class="collapse navbar-collapse" id="srvnavigationbar">
        <ul class="navbar-nav me-auto mb-2 mb-lg-0">
          {{#each server.forms_order}}
          <li class="nav-item"><a class="nav-link {{#with (lookup @root.server.forms .)}}{{active}}{{/with}}" href="#/Applications/{{@root.server.id}}/{{.}}">{{.}}</a></li>
          {{/each}}
        </ul>
      </div>
    </div>
  </nav>
  <!-- [[[ general -->
  {{#if server.forms.General.active}}
  {{#if server.bAdmin}}
  <div class="float-end">
    {{#if server.bEdit}}
    <div style="white-space: nowrap;">
      <button class="btn btn-xs btn-warning" c-click="preEditServer(false)">Cancel</button>
      <button class="btn btn-xs btn-success" c-click="editServer()" style="margin-left: 10px;">Save</button>
    </div>
    {{else}}
    <div style="white-space: nowrap;">
      <button class="btn btn-xs btn-warning" c-click="preEditServer(true)">Edit</button>
      <button class="btn btn-xs btn-danger" c-click="removeServer()" style="margin-left: 10px;">Remove</button>
    </div>
    {{/if}}
  </div>
  {{/if}}
  <table class="table table-condensed">
    <tr>
      <th style="white-space: nowrap;">
        Server ID:
      </th>
      <td>
        {{server.id}}
      </td>
    </tr>
    <tr>
      <th style="white-space: nowrap;">
        Server Name:
      </th>
      <td>
        {{#if server.bEdit}}
        <input type="text" class="form-control" c-model="server.name">
        {{else}}
        {{server.name}}
        {{/if}}
      </td>
    </tr>
  </table>
  {{#if application.sysInfo}}
  <table class="table table-condensed table-striped">
    <tr>
      <th>Server</th>
      <th>Daemon</th>
      <th>Start Time</th>
      <th>Owner</th>
      <th>Processes</th>
      <th>Image (KB)</th>
      <th>Resident (KB)</th>
      <th>Current Alarms</th>
    </tr>
    {{#each application.sysInfo}}
    <tr>
      <td><a href="#/Servers/{{ServerID}}">{{Server}}</a></td>
      <td>{{Daemon}}</td>
      <td>{{data.StartTime}}</td>
      <td>{{json data.Owners}}</td>
      <td>{{numberShort data.NumberOfProcesses}}</td>
      <td>{{numberShort data.ImageSize}}</td>
      <td>{{numberShort data.ResidentSize}}</td>
      <td class="text-danger">{{data.Alarms}}</td>
    </tr>
    {{/each}}
  </table>
  {{/if}}
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ applications -->
  {{#if application.forms.Servers.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th style="width: 100%;">Server</th>
        {{#if application.bDeveloper}}
        <th colspan="2"></th>
        {{/if}}
      </tr>
      {{#if application.bDeveloper}}
      <tr>
        <td><select class="form-control" c-model="server" c-json>{{#each servers}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
        <td><button class="btn btn-xs btn-success" c-click="addServer()">Add</button></td>
      </tr>
      {{/if}}
      {{#each application.servers}}
      <tr>
        <td><a href="#/Servers/{{server_id}}">{{name}}</a></td>
        {{#if @root.application.bDeveloper}}
        <td style="white-space: nowrap;"><button class="btn btn-xs btn-warning" data-bs-toggle="modal" data-bs-target="#serverModal" c-click="serverDetails({{id}})">Edit</button><button class="btn btn-xs btn-danger" c-click="removeServer({{id}})">Remove</button></td>
        {{/if}}
      </tr>
      {{/each}}
    </table>
    <div id="serverModal" class="modal modal-xl">
      <div class="modal-dialog">
        <div class="modal-content">
          <div class="modal-header">
            <h4 class="modal-title">Edit Monitoring Details - {{modalServer.name}}</h4>
            <button type="button" class="btn-close" data-bs-dismiss="modal"></button>
          </div>
          <div class="modal-body table-responsive">
            <div c-model="modalServerInfo" class="text-warning"></div>
            <div c-model="modalServermessage" class="text-danger" style="font-weight:bold;"></div>
            <table class="table table-condensed table-striped">
              <tr>
                <th colspan="5"></th>
                <th colspan="2">Processes</th>
                <th colspan="2">Image</th>
                <th colspan="2">Resident</th>
                {{#if application.bDeveloper}}
                <th colspan="2"></th>
                {{/if}}
              </tr>
              <tr>
                <th>Daemon</th>
                <th>Version</th>
                <th>Owner</th>
                <th>Script</th>
                <th>Delay</th>
                <th>Min</th>
                <th>Max</th>
                <th>Min</th>
                <th>Max</th>
                <th>Min</th>
                <th>Max</th>
                {{#if application.bDeveloper}}
                <th colspan="2"></th>
                {{/if}}
              </tr>
              {{#if application.bDeveloper}}
              <tr>
                <td><input type="text" class="form-control" c-model="serverDetail.daemon"></td>
                <td><input type="text" class="form-control" c-model="serverDetail.version"></td>
                <td><input type="text" class="form-control" c-model="serverDetail.owner"></td>
                <td><input type="text" class="form-control" c-model="serverDetail.script"></td>
                <td><input type="text" class="form-control" c-model="serverDetail.delay"></td>
                <td><input type="text" class="form-control" c-model="serverDetail.min_processes"></td>
                <td><input type="text" class="form-control" c-model="serverDetail.max_processes"></td>
                <td><input type="text" class="form-control" c-model="serverDetail.min_image"></td>
                <td><input type="text" class="form-control" c-model="serverDetail.max_image"></td>
                <td><input type="text" class="form-control" c-model="serverDetail.min_resident"></td>
                <td><input type="text" class="form-control" c-model="serverDetail.max_resident"></td>
                <td colspan="2">{{^if bEdit}}<button class="btn btn-xs btn-success" c-click="addServerDetail()">Add</button>{{/if}}</td>
              </tr>
              {{/if}}
              {{#each modalServer.details}}
              <tr>
                {{#if bEdit}}
                <td><input type="text" class="form-control" c-model="modalServer.details.[{{@key}}].daemon"></td>
                <td><input type="text" class="form-control" c-model="modalServer.details.[{{@key}}].version"></td>
                <td><input type="text" class="form-control" c-model="modalServer.details.[{{@key}}].owner"></td>
                <td><input type="text" class="form-control" c-model="modalServer.details.[{{@key}}].script"></td>
                <td><input type="text" class="form-control" c-model="modalServer.details.[{{@key}}].delay"></td>
                <td><input type="text" class="form-control" c-model="modalServer.details.[{{@key}}].min_processes"></td>
                <td><input type="text" class="form-control" c-model="modalServer.details.[{{@key}}].max_processes"></td>
                <td><input type="text" class="form-control" c-model="modalServer.details.[{{@key}}].min_image"></td>
                <td><input type="text" class="form-control" c-model="modalServer.details.[{{@key}}].max_image"></td>
                <td><input type="text" class="form-control" c-model="modalServer.details.[{{@key}}].min_resident"></td>
                <td><input type="text" class="form-control" c-model="modalServer.details.[{{@key}}].max_resident"></td>
                <td><button class="btn btn-xs btn-warning" c-click="preEditServerDetail({{@key}}, false)">Cancel</button></td>
                <td><button class="btn btn-xs btn-success" c-click="editServerDetail({{@key}})">Save</button></td>
                {{else}}
                <td>{{daemon}}</td>
                <td>{{version}}</td>
                <td>{{owner}}</td>
                <td>{{script}}</td>
                <td>{{delay}}</td>
                <td>{{min_processes}}</td>
                <td>{{max_processes}}</td>
                <td>{{min_image}}</td>
                <td>{{max_image}}</td>
                <td>{{min_resident}}</td>
                <td>{{max_resident}}</td>
                <td><button class="btn btn-xs btn-warning" c-click="preEditServerDetail({{@key}}, true)">Edit</button></td>
                <td><button class="btn btn-xs btn-danger" c-click="removeServerDetail({{id}})">Remove</button></td>
                {{/if}}
              </tr>
              {{/each}}
            </table>
          </div>
          <div class="modal-footer">
            <button type="button" class="btn btn-danger" data-bs-dismiss="modal">Close</button>
          </div>
        </div>
      </div>
    </div>
  </div>
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ contacts -->
  {{#if application.forms.Contacts.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>User</th>
        <th>Type</th>
        <th>Admin</th>
        <th>Locked</th>
        <th>Notify</th>
        <th>Description</th>
        {{#if application.bLocalAdmin}}
        <th></th>
        {{/if}}
      </tr>
      {{#if application.bLocalAdmin}}
      <tr>
        <td><input type="text" class="form-control" c-model="contact.userid" placeholder="User ID"></td>
        <td><select class="form-control" c-model="contact.type" c-json>{{#each contact_types}}<option value="{{json .}}">{{type}}</option>{{/each}}</select></td>
        <td><select class="form-control" c-model="contact.admin" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
        <td><select class="form-control" c-model="contact.locked" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
        <td><select class="form-control" c-model="contact.notify" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
        <td><input type="text" class="form-control" c-model="contact.description" placeholder="Description"></td>
        <td><button class="btn btn-xs btn-success" c-click="addContact()">Add</button></td>
      </tr>
      {{/if}}
      {{#each application.contacts}}
      <tr>
        {{#if bEdit}}
          <td><input type="text" class="form-control" c-model="application.contacts.[{{@key}}].userid" placeholder="User ID"></td>
          <td><select class="form-control" c-model="application.contacts.[{{@key}}].type" c-json>{{#each @root.contact_types}}<option value="{{json .}}">{{type}}</option>{{/each}}</select></td>
          <td><select class="form-control" c-model="application.contacts.[{{@key}}].admin" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
          <td><select class="form-control" c-model="application.contacts.[{{@key}}].locked" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
          <td><select class="form-control" c-model="application.contacts.[{{@key}}].notify" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
          <td><input type="text" class="form-control" c-model="application.contacts.[{{@key}}].description" placeholder="Description"></td>
        {{else}}
          <td style="white-space:nowrap;"><a href="#/Users/{{user_id}}">{{last_name}}, {{first_name}}</a> <small>({{userid}})</small></td>
          <td style="white-space:nowrap;">{{type.type}}</td>
          <td style="white-space:nowrap;">{{admin.name}}</td>
          <td style="white-space:nowrap;">{{locked.name}}</td>
          <td style="white-space:nowrap;">{{notify.name}}</td>
          <td><pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{description}}</pre></td>
        {{/if}}
        {{#if @root.application.bLocalAdmin}}
        <td style="white-space: nowrap;">
          {{#if bEdit}}
          <button class="btn btn-xs btn-warning" c-click="preEditContact({{@key}}, false)">Cancel</button><button class="btn btn-xs btn-success" c-click="editContact({{@key}})" style="margin-left: 10px;">Save</button>
          {{else}}
          <button class="btn btn-xs btn-warning" c-click="preEditContact({{@key}}, true)">Edit</button><button class="btn btn-xs btn-danger" c-click="removeContact({{id}})" style="margin-left: 10px;">Remove</button>
          {{/if}}
        </td>
        {{/if}}
      </tr>
      {{/each}}
    </table>
  </div>
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ notify -->
  {{#if application.forms.Notify.active}}
  {{#if application.bDeveloper}}
  <div class="table-responsive">
    <textarea c-model="notification" class="form-control" placeholder="enter notification" rows="5" autofocus></textarea>
    <button class="btn btn-primary float-end" c-click="sendNotification()">Send Notification</button>
    {{#if bNotified}}
    <span class="text-success">Notification has been sent.</span>
    {{/if}}
  </div>
  {{/if}}
  {{/if}}
  <!-- ]]] -->
  {{/if}}
  <!-- ]]] -->
  `
  // ]]]
}
