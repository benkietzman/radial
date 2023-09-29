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
        c.update('Servers');
      },
      // ]]]
      a: a,
      c: c,
      d: {},
      bNotified: false,
      contact_types: [{type: 'Primary Admin'}, {type: 'Backup Admin'}, {type: 'Primary Contact'}, {type: 'Contact'}],
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
    s.addServer = () =>
    {
      let request = {Interface: 'central', 'Function': 'serverAdd', Request: c.simplify(s.d.server)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          document.location.href = '#/Servers/?server=' + encodeURIComponent(s.d.server.name.v);
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
          General:      {value: 'General',      icon: 'info-circle', active: null},
          Applications: {value: 'Applications', icon: 'window',      active: null},
          Contacts:     {value: 'Contacts',     icon: 'people',      active: null},
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
                s.server.forms.Notify = {value: 'Notify', icon: 'send', active: null};
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
                    s.server.forms.Notify = {value: 'Notify', icon: 'send', active: null};
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
            s.servers = response.Response;
            for (let i = 0; i < s.servers.length; i++)
            {
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
      if (s.bNotified)
      {
        s.bNotified = false;
        s.u();
      }
      if (confirm('Are you sure you want to send this server notification?'))
      {
        let request = {Interface: 'central', 'Function': 'serverNotify', Request: {id: s.server.id, notification: s.notification.v}};
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
        if (s.server.bAdmin)
        {
          let request = {Interface: 'central', 'Function': 'servers', Request: {}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            if (c.wsResponse(response, error))
            {
              s.servers = null;
              s.servers = [{id: null, name: '-- none --'}];
              for (let i = 0; i < response.Response.length; i++)
              {
                s.servers.push(response.Response[i]);
              }
              if (s.server.parent_id != null && s.server.parent_id > 0 && !c.isDefined(s.server['parent']))
              {
                let request = {Interface: 'central', 'Function': 'server', Request: {id: s.server.parent_id}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    s.server['parent'] = response.Response;
                    for (let i = 0; i < s.servers.length; i++)
                    {
                      if (s.server['parent'].id == s.servers[i].id)
                      {
                        s.server['parent'] = s.servers[i];
                      }
                    }
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
        c.addInterval('Servers', 'sysInfoStatus', s.sysInfoStatus, 60000);
        s.sysInfoStatus();
      }
      // ]]]
      // [[[ Applications
      else if (strForm == 'Applications')
      {
        if (!c.isDefined(s.server.applications) || s.server.applications == null)
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
          s.contact = {server_id: s.server.id, type: s.contact_types[3], notify: a.m_noyes[1], physical_access: a.m_noyes[0]};
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
                  if (response.Response[i].notify.value == a.m_noyes[j].value)
                  {
                    response.Response[i].notify = a.m_noyes[j];
                  }
                  if (response.Response[i].physical_access.value == a.m_noyes[j].value)
                  {
                    response.Response[i].physical_access = a.m_noyes[j];
                  }
                }
                s.server.contacts.push(response.Response[i]);
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
        <th>Server</th><th>Parent</th><th>Children</th>
      </tr>
      {{#isValid "Central"}}
      <tr>
        <td><input type="text" class="form-control" c-model="d.server.name" placeholder="Server Name"></td>
        <td><button class="btn btn-primary bi bi-plus-circle" c-click="addServer()" title="Add Server"></button></td>
      </tr>
      {{/isValid}}
      {{#eachFilter servers "name" narrow}}
      <tr>
        <td valign="top">
          <a href="#/Servers/{{id}}">{{name}}</a>
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
          <li class="nav-item"><a class="nav-link {{#with (lookup @root.server.forms .)}}{{active}}{{/with}}" href="#/Servers/{{@root.server.id}}/{{.}}"><i class="bi bi-{{#with (lookup @root.server.forms .)}}{{icon}}{{/with}}"></i> {{.}}</a></li>
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
      <button class="btn btn-sm btn-warning bi bi-x-circle" c-click="preEditServer(false)" title="Cancel"></button>
      <button class="btn btn-sm btn-success bi bi-save" c-click="editServer()" style="margin-left: 10px;" title="Save"></button>
    </div>
    {{else}}
    <div style="white-space: nowrap;">
      <button class="btn btn-sm btn-warning bi bi-pencil" c-click="preEditServer(true)" title="Edit"></button>
      <button class="btn btn-sm btn-danger bi bi-trash" c-click="removeServer()" style="margin-left: 10px;" title="Remove"></button>
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
      <th style="white-space: nowrap;">
        IP Address:
      </th>
      <td>
        {{#each server.ips}}
        {{.}}<br>
        {{/each}}
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
      <th style="white-space: nowrap;">
        Operating System:
      </th>
      <td>
        {{server.sysInfo.OperatingSystem}}
      </td>
    </tr>
    <tr>
      <th style="white-space: nowrap;">
        Parent Server:
      </th>
      <td>
        {{#if server.bEdit}}
        <select class="form-control" c-model="server.['parent']" c-json>{{#each servers}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
        {{else}}
        {{#with (lookup server 'parent')}}<a href="#/Servers/{{id}}">{{name}}</a>{{/with}}
        {{/if}}
      </td>
      <th style="white-space: nowrap;">
        System Release:
      </th>
      <td>
        {{server.sysInfo.SystemRelease}}
      </td>
    </tr>
    <tr>
      <th style="white-space: nowrap;">
        Children Servers:
      </th>
      <td>
        {{#each server.children}}
        <a href="#/Servers/{{id}}">{{name}}</a>
        {{/each}}
      </td>
      <th>
        Processors:
      </th>
      <td>
        {{server.sysInfo.NumberOfProcessors}}@{{numberShort server.sysInfo.CpuSpeed 0}} MHz
      </td>
    </tr>
    <tr>
      <th>
        Address:
      </th>
      <td>
        {{#if server.bEdit}}
        <input type="text" class="form-control" c-model="server.address">
        {{else}}
        {{server.address}}
        {{/if}}
      </td>
      <th>
        Processes:
      </th>
      <td>
        {{#if server.bEdit}}
        <input type="text" class="form-control" c-model="server.processes">
        {{else}}
        {{server.processes}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th>
        City:
      </th>
      <td>
        {{#if server.bEdit}}
        <input type="text" class="form-control" c-model="server.city">
        {{else}}
        {{server.city}}
        {{/if}}
      </td>
      <th style="white-space: nowrap;">
        CPU Usage:
      </th>
      <td>
        {{#if server.bEdit}}
        <input type="text" class="form-control" c-model="server.cpu_usage">
        {{else}}
        {{server.cpu_usage}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th>
        State:
      </th>
      <td>
        {{#if server.bEdit}}
        <input type="text" class="form-control" c-model="server.state">
        {{else}}
        {{server.state}}
        {{/if}}
      </td>
      <th style="white-space: nowrap;">
        Main Memory:
      </th>
      <td>
        {{#if server.bEdit}}
        <input type="text" class="form-control" c-model="server.main_memory">
        {{else}}
        {{server.main_memory}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th>
        Zipcode:
      </th>
      <td>
        {{#if server.bEdit}}
        <input type="text" class="form-control" c-model="server.zipcode">
        {{else}}
        {{server.zipcode}}
        {{/if}}
      </td>
      <th style="white-space: nowrap;">
        Swap Memory:
      </th>
      <td>
        {{#if server.bEdit}}
        <input type="text" class="form-control" c-model="server.swap_memory">
        {{else}}
        {{server.swap_memory}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th>
        Location:
      </th>
      <td>
        {{#if server.bEdit}}
        <input type="text" class="form-control" c-model="server.location">
        {{else}}
        {{server.location}}
        {{/if}}
      </td>
      <th style="white-space: nowrap;">
        Disk Size:
      </th>
      <td>
        {{#if server.bEdit}}
        <input type="text" class="form-control" c-model="server.disk_size">
        {{else}}
        {{server.disk_size}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th colspan="4">
        Description:
      </th>
    </tr>
    <tr>
      <td colspan="4">
        {{#if server.bEdit}}
        <textarea class="form-control" c-model="server.description"></textarea>
        {{else}}
        <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{server.description}}</pre>
        {{/if}}
      </td>
    </tr>
  </table>
  <div class="row">
  <div class="card col-md-6">
    <div class="card-header bg-primary text-white" style="font-weight: bold;">
      Statistics
    </div>
    <div class="card-body">
      <table class="table table-condensed table-striped">
        <tr>
          <th>
            # Processes:
          </th>
          <td>
            {{server.sysInfo.NumberOfProcesses}}
          </td>
        </tr>
        <tr>
          <th>
            CPU Usage:
          </th>
          <td>
            <div class="progress" style="width: 200px;">
              <div class="progress-bar" role="progressbar" aria-valuenow="{{server.sysInfo.CpuUsage}}" aria-valuemin="0" aria-valuemax="100" style="width: {{server.sysInfo.CpuUsage}}%;">
                {{numberShort server.sysInfo.CpuUsage 0}}%
              </div>
            </div>
          </td>
        </tr>
        <tr>
          <th>
            Uptime:
          </th>
          <td>
            {{server.sysInfo.UpTime}} days
          </td>
        </tr>
      </table>
    </div>
  </div>
  <div class="card col-md-6">
    <div class="card-header bg-primary text-white" style="font-weight: bold;">
      Memory
    </div>
    <div class="card-body">
      <table class="table table-condensed table-striped">
        <tr>
          <th></th>
          <th>Used (MB)</th>
          <th>Total (MB)</th>
          <th></th>
        </tr>
        <tr>
          <th>Main</th>
          <td>{{numberShort server.sysInfo.MainUsed 0}}</td>
          <td>{{numberShort server.sysInfo.MainTotal 0}}</td>
          <td>
            <div class="progress" style="width: 200px;">
              <div class="progress-bar" role="progressbar" aria-valuenow="{{server.sysInfo.MainUsage}}" aria-valuemin="0" aria-valuemax="100" style="width: {{numberShort server.sysInfo.MainUsage 0}}%;">
                {{numberShort server.sysInfo.MainUsage 0}}%
              </div>
            </div>
          </td>
        </tr>
        <tr>
          <th>Swap</th>
          <td>{{numberShort server.sysInfo.SwapUsed 0}}</td>
          <td>{{numberShort server.sysInfo.SwapTotal 0}}</td>
          <td>
            <div class="progress" style="width: 200px;">
              <div class="progress-bar" role="progressbar" aria-valuenow="{{server.sysInfo.SwapUsage}}" aria-valuemin="0" aria-valuemax="100" style="width: {{numberShort server.sysInfo.SwapUsage 0}}%;">
                {{numberShort server.sysInfo.SwapUsage 0}}%
              </div>
            </div>
          </td>
        </tr>
      </table>
    </div>
  </div>
  </div>
  <div class="row">
  <div class="card col-md-6">
    <div class="card-header bg-primary text-white" style="font-weight: bold;">
      Partitions
    </div>
    <div class="card-body">
      <table class="table table-condensed table-striped">
        {{#each server.sysInfo.Partitions}}
        <tr>
          <th>{{@key}}</td>
          <td>
            <div class="progress" style="width: 200px;">
              <div class="progress-bar" role="progressbar" aria-valuenow="{{.}}" aria-valuemin="0" aria-valuemax="100" style="width: {{numberShort . 0}}%;">
                {{numberShort . 0}}%
              </div>
            </div>
          </td>
        </tr>
        {{/each}}
      </table>
    </div>
  </div>
  <div class="card col-md-6">
    <div class="card-header bg-primary text-white" style="font-weight: bold;">
      Alarms
    </div>
    <div class="card-body">
      <table class="table table-condensed table-striped">
        {{#each server.sysInfo.Alarms}}
        <tr>
          <td class="text-danger">{{.}}</td>
        </tr>
        {{/each}}
      </table>
    </div>
  </div>
  </div>
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ applications -->
  {{#if server.forms.Applications.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Application</td>
      </tr>
      {{#each server.applications}}
      <tr>
        <td><a href="#/Applications/{{application_id}}">{{name}}</a></td>
      </tr>
      {{/each}}
    </table>
  </div>
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ contacts -->
  {{#if server.forms.Contacts.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>User</th>
        <th>Type</th>
        <th>Notify</th>
        <th style="white-space: nowrap;">Physical Access</th>
        {{#if server.bAdmin}}
        <th></th>
        {{/if}}
      </tr>
      {{#if server.bAdmin}}
      <tr>
        <td><input type="text" class="form-control" c-model="contact.userid" placeholder="User ID"></td>
        <td><select class="form-control" c-model="contact.type" c-json>{{#each contact_types}}<option value="{{json .}}">{{type}}</option>{{/each}}</select></td>
        <td><select class="form-control" c-model="contact.notify" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
        <td><select class="form-control" c-model="contact.physical_access" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
        <td><button class="btn btn-sm btn-success bi bi-plus-circle" c-click="addContact()" title="Add"></button></td>
      </tr>
      {{/if}}
      {{#each server.contacts}}
      <tr>
        {{#if bEdit}}
          <td><input type="text" class="form-control" c-model="server.contacts.[{{@key}}].userid" placeholder="User ID"></td>
          <td><select class="form-control" c-model="server.contacts.[{{@key}}].type" c-json>{{#each @root.contact_types}}<option value="{{json .}}">{{type}}</option>{{/each}}</select></td>
          <td><select class="form-control" c-model="server.contacts.[{{@key}}].notify" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
          <td><select class="form-control" c-model="server.contacts.[{{@key}}].physical_access" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
        {{else}}
          <td style="white-space:nowrap;"><a href="#/Users/{{user_id}}">{{last_name}}, {{first_name}}</a> <small>({{userid}})</small></td>
          <td style="white-space:nowrap;">{{type.type}}</td>
          <td style="white-space:nowrap;">{{notify.name}}</td>
          <td style="white-space:nowrap;">{{physical_access.name}}</td>
        {{/if}}
        {{#if @root.server.bAdmin}}
        <td style="white-space: nowrap;">
          {{#if bEdit}}
          <button class="btn btn-sm btn-warning bi bi-x-circle" c-click="preEditContact({{@key}}, false)" title="Cancel"></button><button class="btn btn-sm btn-success bi bi-save" c-click="editContact({{@key}})" style="margin-left: 10px;" title="Save"></button>
          {{else}}
          <button class="btn btn-sm btn-warning bi bi-pencil" c-click="preEditContact({{@key}}, true)" title="Edit"></button><button class="btn btn-sm btn-danger bi bi-trash" c-click="removeContact({{id}})" style="margin-left: 10px;" title="Remove"></button>
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
  {{#if server.forms.Notify.active}}
  {{#if server.bAdmin}}
  <div class="table-responsive">
    <textarea c-model="notification" class="form-control" placeholder="enter notification" rows="5" autofocus></textarea>
    <button class="btn btn-primary bi bi-send float-end" c-click="sendNotification()" title="Send Notification"></button>
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
