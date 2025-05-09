// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-28
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
    let s = c.scope('Users',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Users');
      },
      // ]]]
      a: a,
      c: c,
      d: {},
      bNotified: false,
    });
    // ]]]
    // [[[ addReminder()
    s.addReminder = () =>
    {
      let request = {Interface: 'central', 'Function': 'userReminderAdd', Request: c.simplify(s.reminder)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.reminder = null;
          s.user.reminders = null;
          s.showForm('Reminders');
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ addUser()
    s.addUser = () =>
    {
      let request = {Interface: 'central', 'Function': 'userAdd', Request: c.simplify(s.d.user)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          document.location.href = '#/Users/?userid=' + encodeURIComponent(s.d.user.userid.v);
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ cronUpdate()
    s.cronUpdate = () =>
    {
      s.reminder.cronPlaceholder = (((c.isDefined(s.reminder.cron.v) && s.reminder.cron.v.value == 1) || s.reminder.cron.value == 1)?'* * * * *':'YYYY-MM-DD HH:MM:SS');
      if (c.isDefined(s.user.reminders) && c.isArray(s.user.reminders))
      {
        for (let i = 0; i < s.user.reminders.length; i++)
        {
          s.user.reminders[i].cronPlaceholder = (((c.isDefined(s.user.reminders[i].cron.v) && s.user.reminders[i].cron.v.value == 1) || s.user.reminders[i].cron.value == 1)?'* * * * *':'YYYY-MM-DD HH:MM;SS');
        }
      }
      s.u();
    };
    // ]]]
    // [[[ editReminder()
    s.editReminder = (nIndex) =>
    {
      s.info.v = 'Updating reminder...';
      let request = {Interface: 'central', 'Function': 'userReminderEdit', Request: c.simplify(s.user.reminders[nIndex])};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          s.user.reminders[nIndex].bEdit = false;
          s.user.reminders = c.simplify(s.user.reminders);
          s.u();
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ editUser()
    s.editUser = () =>
    {
      let user = c.simplify(s.user);
      if (user.password == '')
      {
        delete user.password;
      }
      let request = {Interface: 'central', 'Function': 'userEdit', Request: user};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.user = null;
          s.loadUser();
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
      if (!c.isDefined(s.user.forms))
      {
        s.user.forms =
        {
          General:      {value: 'General',      icon: 'info-circle', active: null},
          Applications: {value: 'Applications', icon: 'app',         active: null},
          Groups:       {value: 'Groups',       icon: 'people',      active: null},
          Servers:      {value: 'Servers',      icon: 'server',      active: null},
        };
      }
      if (!c.isDefined(s.user.forms_order))
      {
        s.user.forms_order = ['General', 'Applications', 'Groups', 'Servers'];
      }
    };
    // ]]]
    // [[[ loadUser()
    s.loadUser = () =>
    {
      if (c.isParam(nav, 'id') || c.isParam(nav, 'userid'))
      {
        let strForm = ((c.isParam(nav, 'form'))?c.getParam(nav, 'form'):'General');
        if (c.isDefined(s.user) && s.user != null && (c.getParam(nav, 'id') == s.user.id || c.getParam(nav, 'userid') == s.user.userid))
        {
          s.showForm(strForm);
        }
        else
        {
          s.info.v = 'Retrieving user...';
          s.user = null;
          let request = {Interface: 'central', 'Function': 'user', Request: {form: strForm}};
          if (c.isParam(nav, 'id'))
          {
            request.Request.id = c.getParam(nav, 'id');
          }
          else
          {
            request.Request.userid = c.getParam(nav, 'userid');
          }
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              let strForm = response.Request.form;
              s.user = response.Response;
              s.user.bAdmin = ((c.isGlobalAdmin() || c.getUserID() == s.user.userid)?true:false);
              s.showForm(strForm);
              if (s.user.bAdmin)
              {
                s.user.forms.Reminders = {value: 'Reminders', icon: 'calendar-event', active: null};
                s.user.forms_order.splice(3, 0, 'Reminders');
                s.showForm(strForm);
              }
              if (c.isValid())
              {
                s.user.forms.Notify = {value: 'Notify', icon: 'send', active: null};
                s.user.forms_order.splice(3, 0, 'Notify');
                s.showForm(strForm);
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
    // [[[ loadUsers()
    s.loadUsers = () =>
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
        s.users = null;
        s.users = [];
        s.u();
        s.info.v = 'Retrieving users...';
        let request = {Interface: 'central', 'Function': 'users', Request: {}};
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
            s.users = response.Response;
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
        s.loadUser();
      }
    };
    // ]]]
    // [[[ preEditReminder()
    s.preEditReminder = (nIndex) =>
    {
      if (s.user.reminders[nIndex].bEdit)
      {
        s.user.reminders[nIndex].bEdit = false;
        s.user.reminders = c.simplify(s.user.reminders);
      }
      else
      {
        s.user.reminders[nIndex].bEdit = true;
      }
      s.u();
    };
    // ]]]
    // [[[ preEditUser()
    s.preEditUser = (bEdit) =>
    {
      s.user.bEdit = bEdit;
      if (!bEdit)
      {
        s.user = c.simplify(s.user);
      }
      s.u();
    };
    // ]]]
    // [[[ removeReminder()
    s.removeReminder = (nID) =>
    {
      if (confirm('Are you sure you want to remove this reminder?'))
      {
        let request = {Interface: 'central', 'Function': 'userReminderRemove', Request: {id: nID}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.user.reminders = null;
            s.showForm('Reminders');
          }
          else
          {
            c.pushErrorMessage(error.message);
          }
        });
      }
    };
    // ]]]
    // [[[ removeUser()
    s.removeUser = () =>
    {
      if (confirm('Are you sure you want to remove this user?'))
      {
        let request = {Interface: 'central', 'Function': 'userRemove', Request: {id: s.user.id}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.user = null;
            document.location.href = '#/Users';
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
      if (confirm('Are you sure you want to send this user notification?'))
      {
        let request = {Interface: 'central', 'Function': 'userNotify', Request: {id: s.user.id, notification: s.notification.v}};
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
      if (!c.isDefined(s.user.forms[strForm]))
      {
        strForm = 'General';
      }
      for (let key of Object.keys(s.user.forms))
      {
        s.user.forms[key].active = null;
      }
      s.user.forms[strForm].active = 'active';
      // [[[ General
      if (strForm == 'General')
      {
        s.user.active = a.setNoYes(s.user.active);
        s.user.admin = a.setNoYes(s.user.admin);
        s.user.alert_chat = a.setNoYes(s.user.alert_chat);
        s.user.alert_email = a.setNoYes(s.user.alert_email);
        s.user.alert_pager = a.setNoYes(s.user.alert_pager);
        s.user.alert_live_audio = a.setNoYes(s.user.alert_live_audio);
        s.user.alert_live_message = a.setNoYes(s.user.alert_live_message);
        s.user.locked = a.setNoYes(s.user.locked);
      }
      // ]]]
      // [[[ Applications
      else if (strForm == 'Applications')
      {
        if (!c.isDefined(s.user.applications) || s.user.applications == null)
        {
          s.info.v = 'Retrieving applications...';
          let request = {Interface: 'central', 'Function': 'applicationsByUserID', Request: {contact_id: s.user.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              s.user.applications = response.Response;
              for (let i = 0; i < s.user.applications.length; i++)
              {
                if (s.user.applications[i].retirement_date)
                {
                  s.user.applications[i].style = 'opacity:0.4;filter:alpha(opacity=40);';
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
      // [[[ Groups
      else if (strForm == 'Groups')
      {
        if (!c.isDefined(s.user.groups) || s.user.groups == null)
        {
          s.info.v = 'Retrieving groups...';
          let request = {Interface: 'central', 'Function': 'groupsByUserID', Request: {contact_id: s.user.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              s.user.groups = response.Response;
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
      // [[[ Reminders
      else if (strForm == 'Reminders')
      {
        if (!c.isDefined(s.reminder) || s.reminder == null)
        {
          s.reminder = {'alert': a.m_noyes[0], chat: a.m_noyes[0], cron: a.m_noyes[0], email: a.m_noyes[0], live: a.m_noyes[0], text: a.m_noyes[0]};
        }
        s.cronUpdate();
        if (!c.isDefined(s.user.reminders) || s.user.reminders == null)
        {
          s.info.v = 'Retrieving reminders...';
          let request = {Interface: 'central', 'Function': 'userReminders', Request: {person_id: s.user.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              s.user.reminders = response.Response;
              for (let i = 0; i < s.user.reminders.length; i++)
              {
                s.user.reminders[i].timestamp = s.user.reminders[i].datetime;
              }
              s.cronUpdate();
            }
            else
            {
              c.pushErrorMessage(error.message);
            }
          });
        }
      }
      // ]]]
      // [[[ Servers
      else if (strForm == 'Servers')
      {
        if (!c.isDefined(s.user.servers) || s.user.servers == null)
        {
          s.info.v = 'Retrieving serverss...';
          let request = {Interface: 'central', 'Function': 'serversByUserID', Request: {contact_id: s.user.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              s.user.servers = response.Response;
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
    c.setMenu('Users');
    s.list = true;
    if (c.isParam(nav, 'id') || c.isParam(nav, 'userid'))
    {
      s.list = false;
    }
    s.u();
    if (a.ready())
    {
      s.loadUsers();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.loadUsers();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <!-- [[[ users -->
  {{#if list}}
  <h3 class="page-header">Users</h3>
  <div class="input-group float-end"><span class="input-group-text">Narrow</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow Results"></div>
  {{#each a.m_letters}}
  <div style="display: inline-block;">
    <a href="#/Users/?letter={{urlEncode .}}">
      <button class="btn btn-sm btn-{{#ifCond . "==" @root.letter}}warning{{else}}primary{{/ifCond}}">{{.}}</button>
    </a>
  </div>
  {{/each}}
  <div c-model="info" class="text-warning"></div>
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>User</th><th>Email</th><th>Text</th><th>Active</th><th>Admin</th><th>locked</th>
      </tr>
      {{#isValid}}
      <tr>
        <td><input type="text" class="form-control" c-model="d.user.userid" placeholder="User ID"></td>
        <td><button class="btn btn-primary bi bi-plus-circle" c-click="addUser()" title="Add User"></button></td>
      </tr>
      {{/isValid}}
      {{#eachFilter users "first_name,last_name,userid" narrow}}
      <tr>
        <td><a href="#/Users/{{id}}">{{last_name}}, {{first_name}}</a> <small>({{userid}})</small></td>
        <td><a href="mailto:{{email}}">{{email}}</a></td>
        <td><a href="mailto:{{pager}}">{{pager}}</a></td>
        <td>{{#if active.value}}Yes{{else}}No{{/if}}</td>
        <td>{{#if admin.value}}Yes{{else}}No{{/if}}</td>
        <td>{{#if locked.value}}Yes{{else}}No{{/if}}</td>
      </tr>
      {{/eachFilter}}
    </table>
  </div>
  <!-- ]]] -->
  <!-- [[[ user -->
  {{else}}
  <h3 class="page-header">{{user.last_name}}, {{user.first_name}} ({{user.userid}})</h3>
  <div c-model="info" class="text-warning"></div>
  <nav class="container navbar navbar-expand-lg navbar-dark bg-dark bg-gradient">
    <div class="container-fluid">
      <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#usrnavigationbar" aria-controls="usrnavigationbar" aria-expanded="false", aria-label="Toggle navigation">
        <span class="navbar-toggler-icon"></span>
      </button>
      <div class="collapse navbar-collapse" id="usrnavigationbar">
        <ul class="navbar-nav me-auto mb-2 mb-lg-0">
          {{#each user.forms_order}}
          <li class="nav-item"><a class="nav-link {{#with (lookup @root.user.forms .)}}{{active}}{{/with}}" href="#/Users/{{@root.user.id}}/{{.}}"><i class="bi bi-{{#with (lookup @root.user.forms .)}}{{icon}}{{/with}}"></i> {{.}}</a></li>
          {{/each}}
        </ul>
      </div>
    </div>
  </nav>
  <!-- [[[ general -->
  {{#if user.forms.General.active}}
  {{#if user.bAdmin}}
  <div class="float-end">
    {{#if user.bEdit}}
    <div style="white-space: nowrap;">
      <button class="btn btn-sm btn-warning bi bi-x-circle" c-click="preEditUser(false)" title="Cancel"></button>
      <button class="btn btn-sm btn-success bi bi-save" c-click="editUser()" style="margin-left: 10px;" title="Save"></button>
    </div>
    {{else}}
    <div style="white-space: nowrap;">
      <button class="btn btn-sm btn-warning bi bi-pencil" c-click="preEditUser(true)" title="Edit"></button>
      <button class="btn btn-sm btn-danger bi bi-trash" c-click="removeUser()" style="margin-left: 10px;" class="Remove"></button>
    </div>
    {{/if}}
  </div>
  {{/if}}
  <table class="table table-condensed">
    <tr>
      <th style="white-space: nowrap;">
        User ID:
      </th>
      <td>
        {{#if user.bEdit}}
        <input type="text" class="form-control" c-model="user.userid">
        {{else}}
        {{user.userid}}
        {{/if}}
      </td>
      <th>
        Email:
      </th>
      <td>
        {{#if user.bEdit}}
        <input type="text" class="form-control" c-model="user.email">
        {{else}}
        {{user.email}}
        {{/if}}
      </td>
      <th>
        Active:
      </th>
      <td>
        {{#if user.bEdit}}
        <select class="form-control" c-model="user.active" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
        {{else}}
        {{user.active.name}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th style="white-space: nowrap;">
        First Name:
      </th>
      <td>
        {{#if user.bEdit}}
        <input type="text" class="form-control" c-model="user.first_name">
        {{else}}
        {{user.first_name}}
        {{/if}}
      </td>
      <th>
        Text:
      </th>
      <td>
        {{#if user.bEdit}}
        <input type="text" class="form-control" c-model="user.pager">
        {{else}}
        {{user.pager}}
        {{/if}}
      </td>
      <th>
        Admin:
      </th>
      <td>
        {{#if user.bEdit}}
        <select class="form-control" c-model="user.admin" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
        {{else}}
        {{user.admin.name}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th style="white-space: nowrap;">
        Last Name:
      </th>
      <td>
        {{#if user.bEdit}}
        <input type="text" class="form-control" c-model="user.last_name">
        {{else}}
        {{user.last_name}}
        {{/if}}
      </td>
      <th>
        Password:
      </th>
      <td>
        {{#if user.bEdit}}
        <input type="password" class="form-control" c-model="user.password">
        {{else if user.bAdmin}}
        {{#ifCond user.password "!=" ""}}
        ******
        {{/ifCond}}
        {{/if}}
      </td>
      <th>
        Locked:
      </th>
      <td>
        {{#if user.bEdit}}
        <select class="form-control" c-model="user.locked" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
        {{else}}
        {{user.locked.name}}
        {{/if}}
      </td>
    </tr>
  </table>
  <div class="row">
    <div class="col-md-4">
      <div class="card">
        <div class="card-header bg-info fw-bold">
          <span title="Configuration settings for receiving alert messages."><i class="bi bi-send"></i> Notify Settings</span>
        </div>
        <div class="card-body bg-info-subtle">
          <table class="table table-condensed">
            <tr>
              <th style="background: inherit;">
                <span title="Sends a private chat message to the user on IRC.">Chat:</span>
              </th>
              <td style="background: inherit;">
                {{#if user.bEdit}}
                <select class="form-control" c-model="user.alert_chat" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
                {{else}}
                {{user.alert_chat.name}}
                {{/if}}
              </td>
              <th style="background: inherit; white-space: nowrap;">
                <span title="Plays audio to the user on any website utilizing the Common framework for which the user is logged into that website.">Live Audio:</span>
              </th>
              <td style="background: inherit;">
                {{#if user.bEdit}}
                <select class="form-control" c-model="user.alert_live_audio" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
                {{else}}
                {{user.alert_live_audio.name}}
                {{/if}}
              </td>
            </tr>
            <tr>
              <th style="background: inherit;">
                <span title="Sends an email to the email address.">Email:</span>
              </th>
              <td style="background: inherit;">
                {{#if user.bEdit}}
                <select class="form-control" c-model="user.alert_email" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
                {{else}}
                {{user.alert_email.name}}
                {{/if}}
              </td>
              <th style="background: inherit; white-space: nowrap;">
                <span title="Displays a notification box to the user on any website utilizing the Common framework for which the user is logged into that website.">Live Message:</span>
              </th>
              <td style="background: inherit;">
                {{#if user.bEdit}}
                <select class="form-control" c-model="user.alert_live_message" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
                {{else}}
                {{user.alert_live_message.name}}
                {{/if}}
              </td>
            </tr>
            <tr>
              <th style="background: inherit;">
                <span title="Sends an email to the text address.">Text:</span>
              </th>
              <td style="background: inherit;">
                {{#if user.bEdit}}
                <select class="form-control" c-model="user.alert_pager" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
                {{else}}
                {{user.alert_pager.name}}
                {{/if}}
              </td>
              <td colspan="2" style="background: inherit;"></td>
            </tr>
          </table>
        </div>
      </div>
    </div>
    <div class="col-md-8">
      <div class="card">
        <div class="card-header bg-success fw-bold">
          <span title="Configuration settings for forwarding alert messages to a remote Radial instance."><i class="bi bi-hdd-network"></i> Notify - Remote Connection</span>
        </div>
        <div class="card-body bg-success-subtle">
          <table class="table table-condensed table-striped">
            <tr>
              <th style="background: inherit;">
                <span title="RESTful URL for the remote Radial instance.">URL:</span>
              </th>
              <td colspan="3" style="background: inherit;">
                {{#if user.bEdit}}
                <input type="text" class="form-control" c-model="user.alert_remote_url">
                {{else}}
                {{user.alert_remote_url}}
                {{/if}}
              </td>
            </tr>
            <tr>
              <th style="background: inherit; white-space: nowrap;">
                <span title="User for the remote Radial instance.">Auth User:</span>
              </th>
              <td style="background: inherit;">
                {{#if user.bEdit}}
                <input type="text" class="form-control" c-model="user.alert_remote_auth_user">
                {{else}}
                {{user.alert_remote_auth_user}}
                {{/if}}
              </td>
              <th style="background: inherit; white-space: nowrap;">
                <span title="Password for the remote Radial instance.">Auth Password:</span>
              </th>
              <td style="background: inherit;">
                {{#if user.bEdit}}
                <input type="text" class="form-control" c-model="user.alert_remote_auth_password">
                {{else if user.bAdmin}}
                {{#ifCond user.alert_remote_auth_password "!=" ""}}
                ******
                {{/ifCond}}
                {{/if}}
              </td>
            </tr>
            <tr>
              <th style="background: inherit;">
                <span title="Remote user to receive alert.">User:</span>
              </th>
              <td style="background: inherit;">
                {{#if user.bEdit}}
                <input type="text" class="form-control" c-model="user.alert_remote_user">
                {{else}}
                {{user.alert_remote_user}}
                {{/if}}
              </td>
              <th style="background: inherit;">
                <span title="Optional server:port for proxying the connection out of the local network.">Proxy:</span>
              </th>
              <td style="background: inherit;">
                {{#if user.bEdit}}
                <input type="text" class="form-control" c-model="user.alert_remote_proxy">
                {{else}}
                {{user.alert_remote_proxy}}
                {{/if}}
              </td>
            </tr>
            <tr>
              <th style="background: inherit; white-space: nowrap;">
                <span title="User for the proxy.">Proxy User:</span>
              </th>
              <td style="background: inherit;">
                {{#if user.bEdit}}
                <input type="text" class="form-control" c-model="user.alert_remote_proxy_user">
                {{else}}
                {{user.alert_remote_proxy_user}}
                {{/if}}
              </td>
              <th style="background: inherit; white-space: nowrap;">
                <span title="Password for the proxy.">Proxy Password:</span>
              </th>
              <td style="background: inherit;">
                {{#if user.bEdit}}
                <input type="text" class="form-control" c-model="user.alert_remote_proxy_password">
                {{else if user.bAdmin}}
                {{#ifCond user.alert_remote_proxy_password "!=" ""}}
                ******
                {{/ifCond}}
                {{/if}}
              </td>
            </tr>
          </table>
        </div>
      </div>
    </div>
  </div>
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ applications -->
  {{#if user.forms.Applications.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Application</td>
      </tr>
      {{#each user.applications}}
      <tr style="{{style}}">
        <td><a href="#/Applications/{{application_id}}">{{name}}</a></td>
      </tr>
      {{/each}}
    </table>
  </div>
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ groups -->
  {{#if user.forms.Groups.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Group</td>
      </tr>
      {{#each user.groups}}
      <tr>
        <td><a href="#/Groups/{{group_id}}">{{name}}</a></td>
      </tr>
      {{/each}}
    </table>
  </div>
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ notify -->
  {{#if user.forms.Notify.active}}
  {{#isValid}}
  <div class="row">
    <div class="col-md-12">
      <textarea c-model="notification" class="form-control border-success-subtle bg-success-subtle" placeholder="enter notification" rows="5" autofocus></textarea>
    </div>
  </div>
  <div class="row" style="margin-top: 10px;">
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
  <!-- [[[ reminders -->
  {{#if user.forms.Reminders.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Reminder</td>
        <th>Timing</th>
        <th>Notifications</th>
        <th></td>
      </tr>
      {{#if user.bAdmin}}
      <tr>
        <td>
          <table class="table table-condensed">
            <tr>
              <td>
                <div class="input-group input-group-sm"><span class="input-group-text">Title</span><input type="text" class="form-control form-control-sm" c-model="reminder.title"></div>
              </td>
            </tr>
            <tr>
              <td>
                <textarea class="form-control form-control-sm" rows="3" c-model="reminder.description" placeholder="Enter any reminder details..."></textarea>
              </td>
            </tr>
          </table>
        </td>
        <td>
          <table class="table table-condensed">
            <tr>
              <td>
                <div class="input-group input-group-sm"><span class="input-group-text">Cron</span><select class="form-control form-control-sm" c-model="reminder.cron" c-change="cronUpdate()" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
              </td>
            </tr>
            <tr>
              <td>
                <div class="input-group input-group-sm"><span class="input-group-text">Sched</span><input type="text" class="form-control form-control-sm" c-model="reminder.sched" placeholder="{{reminder.cronPlaceholder}}"></div>
              </td>
            </tr>
          </table>
        </td>
        <td>
          <table class="table table-condensed">
            <tr>
              <td>
                <div class="input-group input-group-sm"><span class="input-group-text">Chat</span><select class="form-control form-control-sm" c-model="reminder.chat" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
              </td>
              <td>
                <div class="input-group input-group-sm"><span class="input-group-text">Email</span><select class="form-control form-control-sm" c-model="reminder.email" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
              </td>
            </tr>
            <tr>
              <td>
                <div class="input-group input-group-sm"><span class="input-group-text">Live</span><select class="form-control form-control-sm" c-model="reminder.live" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
              </td>
              <td>
                <div class="input-group input-group-sm"><span class="input-group-text">Notify</span><select class="form-control form-control-sm" c-model="reminder.alert" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
              </td>
            </tr>
            <tr>
              <td>
                <div class="input-group input-group-sm"><span class="input-group-text">Text</span><select class="form-control form-control-sm" c-model="reminder.text" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
              </td>
              <td>
              </td>
            </tr>
          </table>
        </td>
        <td>
          <button class="btn btn-sm btn-success bi bi-plus-circle" c-click="addReminder()" title="Add"></button>
        </td>
      </tr>
      {{#each user.reminders}}
      <tr>
        <td>
          <table class="table table-condensed">
            <tr>
              <td>
                {{#if bEdit}}
                <div class="input-group input-group-sm"><span class="input-group-text">Title</span><input type="text" class="form-control form-control-sm" c-model="user.reminders.[{{@key}}].title"></div>
                {{else}}
                Title: {{title}}
                {{/if}}
              </td>
            </tr>
            <tr>
              <td>
                {{#if bEdit}}
                <textarea class="form-control form-control-sm" rows="3" c-model="user.reminders.[{{@key}}].description" placeholder="Enter any reminder details..."></textarea>
                {{else}}
                <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{description}}</pre>
                {{/if}}
              </td>
            </tr>
          </table>
        </td>
        <td>
          <table class="table table-condensed">
            <tr>
              <td>
                {{#if bEdit}}
                <div class="input-group input-group-sm"><span class="input-group-text">Cron</span><select class="form-control form-control-sm" c-model="user.reminders.[{{@key}}].cron" c-change="cronUpdate()" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
                {{else}}
                <span style="white-space: nowrap;">Cron: {{cron.name}}</span>
                {{/if}}
              </td>
            </tr>
            <tr>
              <td>
                {{#if bEdit}}
                <div class="input-group input-group-sm"><span class="input-group-text">Sched</span><input type="text" class="form-control form-control-sm" c-model="user.reminders.[{{@key}}].sched" placeholder="{{cronPlaceholder}}"></div>
                {{else}}
                <span style="white-space: nowrap;">Sched: {{sched}}</span>
                {{/if}}
              </td>
            </tr>
          </table>
        </td>
        <td>
          <table class="table table-condensed">
            <tr>
              <td>
                {{#if bEdit}}
                <div class="input-group input-group-sm"><span class="input-group-text">Chat</span><select class="form-control form-control-sm" c-model="user.reminders.[{{@key}}].chat" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
                {{else}}
                <span style="white-space: nowrap;">Chat: {{chat.name}}</span>
                {{/if}}
              </td>
              <td>
                {{#if bEdit}}
                <div class="input-group input-group-sm"><span class="input-group-text">Email</span><select class="form-control form-control-sm" c-model="user.reminders.[{{@key}}].email" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
                {{else}}
                <span style="white-space: nowrap;">Email: {{email.name}}</span>
                {{/if}}
              </td>
            </tr>
            <tr>
              <td>
                {{#if bEdit}}
                <div class="input-group input-group-sm"><span class="input-group-text">Live</span><select class="form-control form-control-sm" c-model="user.reminders.[{{@key}}].live" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
                {{else}}
                <span style="white-space: nowrap;">Live: {{live.name}}</span>
                {{/if}}
              </td>
              <td>
                {{#if bEdit}}
                <div class="input-group input-group-sm"><span class="input-group-text">Notify</span><select class="form-control form-control-sm" c-model="user.reminders.[{{@key}}].alert" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
                {{else}}
                <span style="white-space: nowrap;">Notify: {{alert.name}}</span>
                {{/if}}
              </td>
            </tr>
            <tr>
              <td>
                {{#if bEdit}}
                <div class="input-group input-group-sm"><span class="input-group-text">Text</span><select class="form-control form-control-sm" c-model="user.reminders.[{{@key}}].text" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
                {{else}}
                <span style="white-space: nowrap;">Text: {{text.name}}</span>
                {{/if}}
              </td>
              <td>
              </td>
            </tr>
          </table>
        </td>
        <td style="white-space: nowrap;">
          {{#if bEdit}}
          <button class="btn btn-sm btn-warning bi bi-x-circle" c-click="preEditReminder({{@key}})" title="Save"></button>
          <button class="btn btn-sm btn-success bi bi-save" c-click="editReminder({{@key}})" title="Save"></button>
          {{else}}
          <button class="btn btn-sm btn-warning bi bi-pencil" c-click="preEditReminder({{@key}})" title="Edit"></button>
          <button class="btn btn-sm btn-danger bi bi-trash" c-click="removeReminder({{id}})" title="Remove"></button>
          {{/if}}
        </td>
      </tr>
      {{/each}}
      {{/if}}
    </table>
  </div>
  {{/if}}
  <!-- ]]] -->
  <!-- [[[ servers -->
  {{#if user.forms.Servers.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Server</td>
      </tr>
      {{#each user.servers}}
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
