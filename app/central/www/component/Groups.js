// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2024-07-16
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
    let s = c.scope('Groups',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Groups');
      },
      // ]]]
      a: a,
      c: c,
      d: {},
      bNotified: false,
      issue: {priority: '1'},
      issueList: true,
      notifyDependents: a.m_noyes[0],
      onlyOpenIssues: 1,
      serverDetail: {delay: 0, min_processes: 0, max_processes: 0, min_image: 0, max_image: 0, min_resident: 0, max_resident: 0}
    });
    // ]]]
    // [[[ addGroup()
    s.addGroup = () =>
    {
      let request = {Interface: 'central', 'Function': 'groupAdd', Request: c.simplify(s.d.group)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          document.location.href = '#/Groups/?group=' + encodeURIComponent(s.d.group.name.v);
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ addContact()
    s.addContact = () =>
    {
      s.contact.group_id = s.group.id;
      let request = {Interface: 'central', 'Function': 'groupUserAdd', Request: c.simplify(s.contact)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.group.contacts = null;
          s.showForm('Contacts');
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ editGroup()
    s.editGroup = () =>
    {
      let request = {Interface: 'central', 'Function': 'groupEdit', Request: c.simplify(s.group)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.group = null;
          s.loadGroup();
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ editContact()
    s.editContact = (nIndex) =>
    {
      let request = {Interface: 'central', 'Function': 'groupUserEdit', Request: c.simplify(s.group.contacts[nIndex])};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.group.contacts = null;
          s.showForm('Contacts');
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ initForms()
    s.initForms = () =>
    {
      if (!c.isDefined(s.group.forms))
      {
        s.group.forms =
        {
          General:      {value: 'General',      icon: 'info-circle', active: null},
          Applications: {value: 'Applications', icon: 'app',         active: null},
          Contacts:     {value: 'Contacts',     icon: 'people',      active: null},
          Servers:      {value: 'Servers',      icon: 'server',      active: null}
        };
      }
      if (!c.isDefined(s.group.forms_order))
      {
        s.group.forms_order = ['General', 'Applications', 'Contacts', 'Servers'];
      }
    };
    // ]]]
    // [[[ loadGroup()
    s.loadGroup = () =>
    {
      s.preLoad();
      if (c.isParam(nav, 'id') || c.isParam(nav, 'group'))
      {
        let strForm = ((c.isParam(nav, 'form'))?c.getParam(nav, 'form'):'General');
        if (c.isDefined(s.group) && s.group != null && (c.getParam(nav, 'id') == s.group.id || c.getParam(nav, 'group') == s.group.name))
        {
          s.showForm(strForm);
        }
        else
        {
          s.info.v = 'Retrieving group...';
          s.group = null;
          let request = {Interface: 'central', 'Function': 'group', Request: {form: strForm}};
          if (c.isParam(nav, 'id'))
          {
            request.Request.id = c.getParam(nav, 'id');
          }
          else
          {
            request.Request.name = c.getParam(nav, 'group');
          }
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              let strForm = response.Request.form;
              s.group = response.Response;
              s.group.bOwner = c.isGlobalAdmin();
              s.showForm(strForm);
              if (c.isValid())
              {
                let request = {Interface: 'central', 'Function': 'isGroupOwner', Request: {id: s.group.id, form: strForm}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    let strForm = response.Request.form;
                    s.group.bOwner = true;
                  }
                  s.group.forms.Notify = {value: 'Notify', icon: 'send', active: null};
                  s.group.forms_order.splice(3, 0, 'Notify');
                  s.showForm(strForm);
                });
              }
            }
            else
            {
              c.pushErrorMessage(error.message);
            }
          });
        }
      }
    };
    // ]]]
    // [[[ loadGroups()
    s.loadGroups = () =>
    {
      s.preLoad();
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
        s.groups = null;
        s.groups = [];
        s.u();
        s.info.v = 'Retrieving groups...';
        let request = {Interface: 'central', 'Function': 'groups', Request: {}};
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
        s.loadGroup();
      }
    };
    // ]]]
    // [[[ preEditGroup()
    s.preEditGroup = (bEdit) =>
    {
      s.group.bEdit = bEdit;
      if (!bEdit)
      {
        s.group = c.simplify(s.group);
      }
      s.u();
    };
    // ]]]
    // [[[ preEditContact()
    s.preEditContact = (nIndex, bEdit) =>
    {
      s.group.contacts[nIndex].bEdit = bEdit;
      if (!bEdit)
      {
        s.group.contacts[nIndex] = c.simplify(s.group.contacts[nIndex]);
      }
      s.u();
    };
    // ]]]
    // [[[ preLoad()
    s.preLoad = () =>
    {
      // [[[ get contact types
      if (!c.isDefined(s.contact_types))
      {
        s.contact_types = [{type: 'Primary Owner'}, {type: 'Backup Owner'}, {type: 'Primary Contact'}, {type: 'Contact'}];
        for (let i = 0; i < s.contact_types.length; i++)
        {
          let request = {Interface: 'central', 'Function': 'contactType', Request: {type: s.contact_types[i].type, i: i}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            if (c.wsResponse(response, error))
            {
              let i = response.Request.i;
              s.contact_types[i] = response.Response;
            }
            else
            {
              c.pushErrorMessage(error.message);
            }
          });
        }
      }
      // ]]]
      // [[[ get login types
      if (!c.isDefined(s.login_types))
      {
        s.login_types = [];
        let request = {Interface: 'central', 'Function': 'loginTypes', Request: {}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.login_types = response.Response;
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      // ]]]
      // [[[ get menu accesses
      if (!c.isDefined(s.menu_accesses))
      {
        s.menu_accesses = [];
        let request = {Interface: 'central', 'Function': 'menuAccesses', Request: {}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.menu_accesses = response.Response;
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      // ]]]
      // [[[ get notify priorities
      if (!c.isDefined(s.notify_priorities))
      {
        s.notify_priorities = [];
        let request = {Interface: 'central', 'Function': 'notifyPriorities', Request: {}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.notify_priorities = response.Response;
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      // ]]]
      // [[[ get package types
      if (!c.isDefined(s.package_types))
      {
        s.package_types = [];
        let request = {Interface: 'central', 'Function': 'packageTypes', Request: {}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.package_types = response.Response;
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      // ]]]
      // [[[ get repos
      if (!c.isDefined(s.repos))
      {
        s.repos = [];
        let request = {Interface: 'central', 'Function': 'repos', Request: {}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.repos = response.Response;
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
      // ]]]
    };
    // ]]]
    // [[[ removeGroup()
    s.removeGroup = () =>
    {
      if (confirm('Are you sure you want to remove this group?'))
      {
        let request = {Interface: 'central', 'Function': 'groupRemove', Request: {id: s.group.id}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.group = null;
            document.location.href = '#/Groups';
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
    };
    // ]]]
    // [[[ removeContact()
    s.removeContact = (nID) =>
    {
      if (confirm('Are you sure you want to remove this group contact?'))
      {
        let request = {Interface: 'central', 'Function': 'groupUserRemove', Request: {id: nID}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.group.contacts  = null;
            s.showForm('Contacts');
          }
          else
          {
            c.pushErrorMessage(error.message);
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
      if (confirm('Are you sure you want to send this group notification?'))
      {
        let request = {Interface: 'central', 'Function': 'groupNotify', Request: {id: s.group.id, notification: s.notification.v}};
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
            c.pushErrorMessage(error.message);
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
      if (!c.isDefined(s.group.forms[strForm]))
      {
        strForm = 'General';
      }
      for (let key of Object.keys(s.group.forms))
      {
        s.group.forms[key].active = null;
      }
      s.group.forms[strForm].active = 'active';
      // [[[ General
      if (strForm == 'General')
      {
      }
      // ]]]
      // [[[ Applications
      else if (strForm == 'Applications')
      {
        if (!c.isDefined(s.group.applications) || s.group.applications == null)
        {
          s.info.v = 'Retrieving applications...';
          let request = {Interface: 'central', 'Function': 'applicationsByGroupID', Request: {group_id: s.group.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              s.group.applications = response.Response;
              for (let i = 0; i < s.group.applications.length; i++)
              {
                if (s.group.applications[i].retirement_date)
                {
                  s.group.applications[i].style = 'opacity:0.4;filter:alpha(opacity=40);';
                }
              }
              s.u();
            }
            else
            {
              c.pushErrorMessage(error.message);
            }
          });
        }
      }
      // ]]]
      // [[[ Contacts
      else if (strForm == 'Contacts')
      {
        if (!c.isDefined(s.group.contacts) || s.group.contacts == null)
        {
          s.contact = {group_id: s.group.id, type: s.contact_types[3], notify: a.m_noyes[1]};
          s.group.contacts = null;
          s.group.contacts = [];
          s.u();
          s.info.v = 'Retrieving contacts...';
          let request = {Interface: 'central', 'Function': 'groupUsersByGroupID', Request: {group_id: s.group.id}};
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
                }
                s.group.contacts.push(response.Response[i]);
              }
              s.u();
            }
            else
            {
              c.pushErrorMessage(error.message);
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
      // [[[ Servers
      else if (strForm == 'Servers')
      {
        if (!c.isDefined(s.group.servers) || s.group.servers == null)
        {
          s.info.v = 'Retrieving serverss...';
          let request = {Interface: 'central', 'Function': 'serversByGroupID', Request: {group_id: s.group.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              s.group.servers = response.Response;
              s.u();
            }
            else
            {
              c.pushErrorMessage(error.message);
            }
          });
        }
      }
      // ]]]
      s.u();
    };
    // ]]]
    // [[[ main
    c.setMenu('Groups');
    s.list = true;
    if (c.isParam(nav, 'id') || c.isParam(nav, 'group'))
    {
      s.list = false;
    }
    s.u();
    if (a.ready())
    {
      s.loadGroups();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.loadGroups();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <!-- [[[ groups -->
  {{#if list}}
  <h3 class="page-header">Groups</h3>
  <div class="input-group float-end"><span class="input-group-text">Narrow</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow Results"></div>
  {{#each a.m_letters}}
  <div style="display: inline-block;">
    <a href="#/Groups/?letter={{urlEncode .}}">
      <button class="btn btn-sm btn-{{#ifCond . "==" @root.letter}}warning{{else}}primary{{/ifCond}}">{{.}}</button>
    </a>
  </div>
  {{/each}}
  <div c-model="info" class="text-warning"></div>
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Group</th>
      </tr>
      {{#isValid}}
      <tr>
        <td><input type="text" class="form-control" c-model="d.group.name" placeholder="Group Name"></td>
        <td><button class="btn btn-primary bi bi-plus-circle" c-click="addGroup()" title="Add Group"></button></td>
      </tr>
      {{/isValid}}
      {{#eachFilter groups "name" narrow}}
      <tr style="{{style}}">
        <td valign="top">
          <a href="#/Groups/{{id}}">{{name}}</a>
        </td>
      </tr>
      {{/eachFilter}}
    </table>
  </div>
  <!-- ]]] -->
  <!-- [[[ group -->
  {{else}}
  <h3 class="page-header">{{group.name}}</h3>
  <div c-model="info" class="text-warning"></div>
  <nav class="container navbar navbar-expand-lg navbar-dark bg-dark bg-gradient">
    <div class="container-fluid">
      <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#appnavigationbar" aria-controls="appnavigationbar" aria-expanded="false", aria-label="Toggle navigation">
        <span class="navbar-toggler-icon"></span>
      </button>
      <div class="collapse navbar-collapse" id="appnavigationbar">
        <ul class="navbar-nav me-auto mb-2 mb-lg-0">
          {{#each group.forms_order}}
          <li class="nav-item"><a class="nav-link {{#with (lookup @root.group.forms .)}}{{active}}{{/with}}" href="#/Groups/{{@root.group.id}}/{{.}}"><i class="bi bi-{{#with (lookup @root.group.forms .)}}{{icon}}{{/with}}"></i> {{.}}</a></li>
          {{/each}}
        </ul>
      </div>
    </div>
  </nav>
  <!-- [[[ general -->
  {{#if group.forms.General.active}}
  {{#if group.bOwner}}
  <div class="float-end">
    {{#if group.bEdit}}
    <div style="white-space: nowrap;">
      <button class="btn btn-sm btn-warning bi bi-x-circle" c-click="preEditGroup(false)" title="Cancel"></button>
      <button class="btn btn-sm btn-success bi bi-save" c-click="editGroup()" style="margin-left: 10px;" title="Save"></button>
    </div>
    {{else}}
    <div style="white-space: nowrap;">
      <button class="btn btn-sm btn-warning bi bi-pencil" c-click="preEditGroup(true)" title="Edit"></button>
      <button class="btn btn-sm btn-danger bi bi-trash" c-click="removeGroup()" style="margin-left: 10px;" title="Remove"></button>
    </div>
    {{/if}}
  </div>
  {{/if}}
  <table class="table table-condensed">
    <tr>
      <th style="white-space: nowrap;">
        Group ID:
      </th>
      <td>
        {{group.id}}
      </td>
    </tr>
    <tr>
      <th style="white-space: nowrap;">
        Group Name:
      </th>
      <td>
        {{#if group.bEdit}}
        <input type="text" class="form-control" c-model="group.name">
        {{else}}
        {{group.name}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th style="white-space: nowrap;">
        Creation Date:
      </th>
      <td>
        {{group.creation_date}}
      </td>
    </tr>
    <tr>
      <th>
        Description:
      </th>
      <td>
        {{#if group.bEdit}}
        <textarea class="form-control" c-model="group.description"></textarea>
        {{else}}
        <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{group.description}}</pre>
        {{/if}}
      </td>
    </tr>
  </table>
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ applications -->
  {{#if group.forms.Applications.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Application</td>
      </tr>
      {{#each group.applications}}
      <tr style="{{style}}">
        <td><a href="#/Applications/{{application_id}}">{{name}}</a></td>
      </tr>
      {{/each}}
    </table>
  </div>
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ contacts -->
  {{#if group.forms.Contacts.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>User</th>
        <th>Type</th>
        <th>Notify</th>
        <th>Description</th>
        {{#if group.bOwner}}
        <th></th>
        {{/if}}
      </tr>
      {{#if group.bOwner}}
      <tr>
        <td><input type="text" class="form-control" c-model="contact.userid" placeholder="User ID"></td>
        <td><select class="form-control" c-model="contact.type" c-json>{{#each contact_types}}<option value="{{json .}}">{{type}}</option>{{/each}}</select></td>
        <td><select class="form-control" c-model="contact.notify" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
        <td><input type="text" class="form-control" c-model="contact.description" placeholder="Description"></td>
        <td><button class="btn btn-sm btn-success bi bi-plus-circle" c-click="addContact()" title="Add"></button></td>
      </tr>
      {{/if}}
      {{#each group.contacts}}
      <tr>
        {{#if bEdit}}
          <td><input type="text" class="form-control" c-model="group.contacts.[{{@key}}].userid" placeholder="User ID"></td>
          <td><select class="form-control" c-model="group.contacts.[{{@key}}].type" c-json>{{#each @root.contact_types}}<option value="{{json .}}">{{type}}</option>{{/each}}</select></td>
          <td><select class="form-control" c-model="group.contacts.[{{@key}}].notify" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
          <td><input type="text" class="form-control" c-model="group.contacts.[{{@key}}].description" placeholder="Description"></td>
        {{else}}
          <td style="white-space:nowrap;"><a href="#/Users/{{user_id}}">{{last_name}}, {{first_name}}</a> <small>({{userid}})</small></td>
          <td style="white-space:nowrap;">{{type.type}}</td>
          <td style="white-space:nowrap;">{{notify.name}}</td>
          <td><pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{description}}</pre></td>
        {{/if}}
        {{#if @root.group.bOwner}}
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
  {{#if group.forms.Notify.active}}
  {{#isValid}}
  <div class="row">
    <div class="col-md-12">
    <textarea c-model="notification" class="form-control" placeholder="enter notification" rows="5" autofocus></textarea>
    </div>
  </div>
  <div class="row">
    <div class="col-md-12">
      <button class="btn btn-success bi bi-send float-end" c-click="sendNotification()" title="Send Notification"></button>
    </div>
  </div>
  {{#if ../bNotified}}
  <div class="row">
    <div class="col-md-12">
      <span class="text-success">Notification has been sent.</span>
    </div>
  </div>
  {{/if}}
  {{/isValid}}
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ servers -->
  {{#if group.forms.Servers.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Server</td>
      </tr>
      {{#each group.servers}}
      <tr>
        <td><a href="#/Servers/{{server_id}}">{{name}}</a></td>
      </tr>
      {{/each}}
    </table>
  </div>
  {{/if}}
  <!-- ]]] -->
  {{/if}}
  <!-- ]]] -->
  `
  // ]]]
}
