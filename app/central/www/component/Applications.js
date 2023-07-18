// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-17
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // {{{ controller()
  controller(id, nav)
  {
    // {{{ prep work
    let a = app;
    let c = common;
    let s = c.scope('Applications',
    {
      // {{{ u()
      u: () =>
      {
        c.render(id, 'Applications', this);
      },
      // }}}
      a: a,
      c: c,
      list: true,
      onlyOpenIssues: 1
    });
    // }}}
    // {{{ addAccount()
    s.addAccount = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationAccountAdd', Request: s.account};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.application.accounts = null;
          s.showForm('Accounts');
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ addApplication()
    s.addApplication = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationAdd', Request: s.application};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          document.location.href = '#/Applications/?application=' + encodeURIComponent(s.application.name);
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ addContact()
    s.addContact = (strType) =>
    {
      s.contact[strType].application_id = s.application.id;
      let request = {Interface: 'central', 'Function': 'applicationUserAdd', Request: s.contact[strType]};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.application.contacts = null;
          s.showForm('Contacts');
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ addDepend()
    s.addDepend = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationDependAdd', Request: {application_id: s.application.id, dependant_id: s.depend.id}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.application.depends = null;
          s.application.dependents = null;
          s.showForm('Depend');
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ addIssue()
    s.addIssue = () =>
    {
      s.info.v = 'Adding issue...';
      let request = {Interface: 'central', 'Function': 'applicationIssueAdd', Request: {application_id: s.application.id, application_name: s.application.name, summary: s.issue.summary, due_date: s.issue.due_date, priority: s.issue.priority, assigned_userid: s.issue.assigned_userid, comments: s.issue.comments, server: location.host}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.application.issues = null;
          s.info.v = 'Redirecting...';
          s.issue.comments = null;
          s.application.issue = null;
          document.location.href = '#/Applications/' + s.application.id + '/Issues/' + response.Response.id;
          s.showForm('Issues');
        }
        else
        {
          s.info.v = null;
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ addIssueComment()
    s.addIssueComment = (strAction, nIssueID, nApplicationID) =>
    {
      s.info.v = 'Adding comment...';
      let request = {Interface: 'central', 'Function': 'applicationIssueCommentAdd', Request: {issue_id: nIssueID, comments: s.issue.comments, action: strAction, application_id: nApplicationID, server: location.host}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.issue.comments = null;
          s.info.v = 'Redirecting...';
          s.application.issue = null;
          if (strAction == 'close')
          {
            s.application.issue = null;
          }
          document.location.href = '#/Applications/' + s.application.id + '/Issues/' + nIssueID;
          s.showForm('Issues');
        }
        else
        {
          s.info.v = null;
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ addServer()
    s.addServer = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationServerAdd', Request: {application_id: s.application.id, server_id: s.server_id}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.application.servers = null;
          s.showForm('Servers');
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ addServer(Detail)
    s.addServerDetail = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationServerDetailAdd', Request: s.serverDetail};
      request.Request.application_server_id = s.modalServer.id;
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.serverDetails(s.modalServer);
          s.sysInfoUpdate();
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ editAccount()
    s.editAccount = (account) =>
    {
      let request = {Interface: 'central', 'Function': 'applicationAccountEdit', Request: account};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.application.accounts = null;
          s.showForm('Accounts');
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ editApplication()
    s.editApplication = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationEdit', Request: s.application};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.application = null;
          s.loadApplication()
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ editContact()
    s.editContact = (contact) =>
    {
      let request = {Interface: 'central', 'Function': 'applicationUserEdit', Request: contact};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.application.contacts = null;
          s.showForm('Contacts');
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ editIssue()
    s.editIssue = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationIssueEdit', Request: s.application.issue};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          if (c.isDefined(s.application.issue.transfer) && c.isDefined(s.application.issue.transfer.id) && s.application.issue.transfer.id != s.application.id)
          {
            s.info.v = 'Emailing issue...';
            let request = {Interface: 'central', 'Function':  'applicationIssueEmail', Request: {id: s.application.issue.id, action: 'transfer', application_id: s.application.id, server: location.host}};
            c.wsRequest('radial', request).then((response) =>
            {
              s.info.v = null;
              let error = {};
              if (c.wsResponse(response, error))
              {
                s.info.v = 'Transfering issue...';
                document.location.href = '/central/#/Applications/?application=' + encodeURIComponent(s.application.issue.transfer.name) + '&form=Issues&issue_id=' + s.application.issue.id;
              }
              else
              {
                s.message.v = error.message;
              }
            });
          }
          else
          {
            s.info.v = null;
            s.application.issue = null;
            s.showForm('Issues');
          }
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ editIssueComment()
    s.editIssueComment = (comment) =>
    {
      s.info.v = 'Updating issue...';
      let request = {Interface: 'central', 'Function': 'applicationIssueCommentEdit', Request: {id: comment.id, comments: comment.comments}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          comment.bEdit = false;
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ editServerDetail()
    s.editServerDetail = (detail) =>
    {
      let request = {Interface: 'central', 'Function': 'applicationServerDetailEdit', Request: detail};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.serverDetails(s.modalServer);
          s.sysInfoUpdate();
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ initForms()
    s.initForms = () =>
    {
      if (!c.isDefined(s.application.forms))
      {
        s.application.forms =
        {
          General:  {value: 'General',  active: null},
          Contacts: {value: 'Contacts', active: null},
          Depend:   {value: 'Depend',   active: null},
          Issues:   {value: 'Issues',   active: null},
          Servers:  {value: 'Servers',  active: null}
        };
      }
      if (!c.isDefined(s.application.forms_order))
      {
        s.application.forms_order = ['General', 'Contacts', 'Depend', 'Issues', 'Servers'];
      }
    };
    // }}}
    // {{{ loadApplication()
    s.loadApplication = () =>
    {
      if (c.isObject(nav.data) && (c.isDefined(nav.data.id) || c.isDefined(nav.data.application)))
      {
        let strForm = ((c.isDefined(nav.data.form))?nav.data.form:'General');
        if (c.isDefined(s.application) && s.application != null && (nav.data.id == s.application.id || nav.data.application == s.application.name))
        {
          s.showForm(strForm);
        }
        else
        {
          s.info.v = 'Retrieving application...';
          s.application.v = null;
          let request = {Interface: 'central', 'Function': 'application', Request: {form: strForm}};
          if (c.isDefined(nav.data.id))
          {
            request.Request.id = nav.data.id;
          }
          else
          {
            request.Request.name = nav.data.application;
          }
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              let strForm = response.Request.form;
              s.application = response.Response;
              s.application.bDeveloper = c.isGlobalAdmin();
              s.initForms();
              s.showForm(strForm);
              if (s.application.bDeveloper)
              {
                s.application.forms.Accounts = {value: 'Accounts', active: null};
                s.application.forms_order.splice(1, 0, 'Accounts');
                s.application.forms.Notify = {value: 'Notify', active: null};
                s.application.forms_order.splice(6, 0, 'Notify');
                s.showForm(strForm);
              }
              else if (c.isValid())
              {
                let request = {Interface: 'central', 'Function': 'isApplicationDeveloper', Request: {id: s.application.id, form: strForm}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    let strForm = response.Request.form;
                    s.application.bDeveloper = true;
                    s.application.forms.Accounts = {value: 'Accounts', active: null};
                    s.application.forms_order.splice(1, 0, 'Accounts');
                    s.application.forms.Notify = {value: 'Notify', active: null};
                    s.application.forms_order.splice(6, 0, 'Notify');
                    s.showForm(strForm);
                  }
                  else
                  {
                    s.message.v = error.message;
                  }
                });
              }
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
    // }}}
    // {{{ loadApplications()
    s.loadApplications = () =>
    {
      // {{{ get contact types
      if (!c.isDefined(s.contactTypeOrder))
      {
        s.contactTypeOrder = [{type: 'Primary Developer'}, {type: 'Backup Developer'}, {type: 'Primary Contact'}, {type: 'Contact'}];
        for (let i = 0; i < s.contactTypeOrder.length; i++)
        {
          let request = {Interface: 'central', 'Function': 'contactType', Request: {type: s.contactTypeOrder[i].type, i: i}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            if (c.wsResponse(response, error))
            {
              let i = response.Request.i;
              s.contactTypeOrder[i] = response.Response;
            }
            else
            {
              s.message.v = error.message;
            }
          });
        }
      }
      // }}}
      // {{{ get login types
      if (!c.isDefined(s.login_types))
      {
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
            s.message.v = error.message;
          }
        });
      }
      // }}}
      // {{{ get menu accesses
      if (!c.isDefined(s.menu_accesses))
      {
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
            s.message = error.message;
          }
        });
      }
      // }}}
      // {{{ get notify priorities
      if (!c.isDefined(s.notify_priorities))
      {
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
            s.message.v = error.message;
          }
        });
      }
      // }}}
      // {{{ get package types
      if (!c.isDefined(s.package_types))
      {
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
            s.message.v = error.message;
          }
        });
      }
      // }}}
      if (s.list)
      {
        if (c.isObject(nav.data) && c.isDefined(nav.data.letter))
        {
          s.letter = nav.data.letter;
        }
        else if (!c.isDefined(s.letter))
        {
          s.letter = '#';
        }
        s.applications = null;
        s.applications = [];
        s.info.v = 'Retrieving applications...';
        let request = {Interface: 'central', 'Function': 'applications', Request: {}};
        if (s.letter != 'ALL')
        {
          request.Request.letter = s.letter;
        }
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.info = null;
          if (c.wsResponse(response, error))
          {
            let appList = [];
            for (let i = 0; i < response.Response.length; i++)
            {
              s.applications.push(response.Response[i]);
            }
            for (let i = 0; i < s.applications.length; i++)
            {
              if (s.applications[i].retirement_date)
              {
                s.applications[i].style = 'opacity:0.4;filter:alpha(opacity=40);';
              }
              let request = {Interface: 'central', 'Function': 'applicationUsersByApplicationID', Request: {application_id: s.applications[i].id, 'Primary Developer': 1, i: i}};
              c.wsRequest('radial', request).then((response) =>
              {
                let error = {};
                if (c.wsResponse(response, error))
                {
                  let i = response.Request.i;
                  s.applications[i].contacts = response.Response;
                }
                else
                {
                  s.message.v = error.message;
                }
              });
              request = {Interface: 'central', 'Function': 'serversByApplicationID', Request: {application_id: s.applications[i].id, i: i}};
              c.wsRequest('radial', request).then((response) =>
              {
                let error = {};
                if (c.wsResponse(response, error))
                {
                  let i = response.Request.i;
                  s.applications[i].servers = response.Response;
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
      }
      else
      {
        s.loadApplication();
      }
    };
    // }}}
    // {{{ main
    c.setMenu('Applications');
    if (c.isObject(nav.data) && (c.isDefined(nav.data.id) || c.isDefined(nav.data.application)))
    {
      s.list = false;
    }
    s.u();
    if (a.ready())
    {
      s.loadApplications();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.loadApplications();
    });
    // }}}
  },
  // }}}
  // {{{ template
  template: `
  {{#if list}}
  <div class="input-group float-end"><span class="input-group-text">Narrow</div><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow Results"></div>
  <h3 class="page-header">Applications</h3>
  {{#each c.m_letters}}
  <div style="display: inline-block;">
    <a href="#/Applications/?letter={{urlEncode .}}">
      <button class="btn btn-{{#ifCond . '==' letter}}warning{{else}}default{{/ifCond}}" style="margin: 2px;">{{../.}}</button>
    </a>
  </div>
  {{/each}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Application</th><th>Primary Developers</th><th>Servers</th>
      </tr>
      {{#isValid 'Central'}}
      <tr>
        <td><input type="text" class="form-control" c-model="application.name" placeholder="Application Name"></td>
        <td><button class="btn btn-default" c-click="addApplication()">Add Application</button></td>
      </tr>
      {{/isValid}}
      {{#eachFilter applications 'name' narrow}}
      <tr style="{{application.style}}">
        <td valign="top">
          <a href="#/Applications/{{./id}}">{{./name}}</a>
        </td>
        <td valign="top">
          <table class="table table-condensed" style="background: inherit;">
            {{#each ./application.contacts}}
            <tr>
              <td><a href="#/Users/{{./user_id}}">{{./last_name}}, {{./first_name}}</a> <small>({{./userid}})</small></td>
            </tr>
            {{/each}}
          </table>
        </td>
        <td valign="top">
          <table class="table table-condensed" style="background: inherit;">
            {{#each ./servers}}
            <tr>
              <td><a href="#/Servers/{{./server_id}}">{{./name}}</a></td>
            </tr>
            {{/each}}
          </table>
        </td>
      </tr>
    </table>
  </div>
  {{else}}
  <h3 class="page-header">{{store.application.name}}<small ng-show="store.application.retirement_date" class="text-danger"> --- RETIRED</small></h3>
  <div c-model="info" class="text-warning">{{info}}</div>
  <div c-model="message" class="text-danger" style="font-weight:bold;">{{store.message}}</div>
  <div class="navbar navbar-default" role="navigation">
    <div class="container-fluid" id="navfluid">
      <div class="navbar-header">
        <button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#nmanavigationbar">
          <span class="sr-only">Toggle navigation</span>
          <span class="icon-bar"></span>
          <span class="icon-bar"></span>
          <span class="icon-bar"></span>
        </button>
      </div>
      <div class="collapse navbar-collapse" id="nmanavigationbar">
        <ul class="nav navbar-nav">
          <li ng-repeat="form in store.application.forms_order" class="{{store.application.forms[form].active}}" style="font-weight:bold;"><a href="#/Applications/{{store.application.id}}/{{form}}">{{form}}</a></li>
        </ul>
      </div>
    </div>
  </div>
  <div ng-show="store.application.forms.General.active && store.application">
    <div ng-show="common.isGlobalAdmin() || store.application.bDeveloper" class="pull-right">
      <div ng-show="!store.application.bEdit" style="white-space: nowrap;">
        <button class="btn btn-xs btn-warning glyphicon glyphicon-pencil" ng-click="store.application.bEdit = true"></button>
        <button class="btn btn-xs btn-danger glyphicon glyphicon-remove" ng-click="removeApplication(store.application.id)" style="margin-left: 10px;"></button>
      </div>
      <div ng-show="store.application.bEdit" style="white-space: nowrap;">
        <button class="btn btn-xs btn-warning glyphicon glyphicon-arrow-left" ng-click="store.application.bEdit = false"></button>
        <button class="btn btn-xs btn-success glyphicon glyphicon-ok" ng-click="editApplication()" style="margin-left: 10px;"></button>
      </div>
    </div>
    <table class="table table-condensed">
      <tr>
        <th style="white-space: nowrap;">
          Application ID:
        </th>
        <td>
          {{store.application.id}}
        </td>
      </tr>
      <tr>
        <th style="white-space: nowrap;">
          Application Name:
        </th>
        <td>
          <span ng-show="!store.application.bEdit">{{store.application.name}}</span>
          <input ng-show="store.application.bEdit" type="text" class="form-control" ng-model="store.application.name">
        </td>
      </tr>
      <tr>
        <th style="white-space: nowrap;">
          Creation Date:
        </th>
        <td>
          {{store.application.creation_date}}
        </td>
      </tr>
      <tr ng-show="store.application.bEdit || store.application.retirement_date">
        <th style="white-space: nowrap;">
          Retirement Date:
        </th>
        <td>
          <span ng-show="!store.application.bEdit">{{store.application.retirement_date}}</span>
          <input ng-show="store.application.bEdit" type="text" class="form-control" ng-model="store.application.retirement_date" placeholder="YYYY-MM-DD">
        </td>
      </tr>
      <tr ng-show="store.application.bEdit || store.application.notify_priority_id">
        <th style="white-space: nowrap;">
          Notify Priority:
        </th>
        <td>
          <span ng-show="!store.application.bEdit">{{store.application.notify_priority.priority}}</span>
          <select ng-show="store.application.bEdit" class="form-control" ng-model="store.application.notify_priority" ng-options="notify_priority.priority for notify_priority in store.notify_priorities"></select>
        </td>
      </tr>
      <tr ng-show="store.application.bEdit || store.application.website">
        <th>
          Website:
        </th>
        <td>
          <a ng-show="!store.application.bEdit" href="{{store.application.website}}" target="_blank">{{store.application.website}}</a>
          <input ng-show="store.application.bEdit" type="text" class="form-control" ng-model="store.application.website">
        </td>
      </tr>
      <tr ng-show="store.application.bEdit || store.application.login_type_id">
        <th style="white-space: nowrap;">
          Login Type:
        </th>
        <td>
          <div ng-show="!store.application.bEdit" style="white-space: nowrap;">
            {{store.application.login_type.type}}
            <br>
            Secure: {{store.application.secure_port.name}}
            <br>
            Auto-Register: {{store.application.auto_register.name}}
            <br>
            Account Check: {{store.application.account_check.name}}
          </div>
          <div ng-show="store.application.bEdit">
            <select class="form-control" ng-model="store.application.login_type" ng-options="login_type.type for login_type in store.login_types"></select>
            <div class="form-inline">
              <div class="form-group"><div class="input-group"><div class="input-group-addon">Secure</div><select class="form-control" ng-model="store.application.secure_port" ng-options="noyes.name for noyes in central.m_noyes"></select></div></div>
              <div class="form-group"><div class="input-group"><div class="input-group-addon">Auto-Register</div><select class="form-control" ng-model="store.application.auto_register" ng-options="noyes.name for noyes in central.m_noyes"></select></div></div>
              <div class="form-group"><div class="input-group"><div class="input-group-addon">Account Check</div><select class="form-control" ng-model="store.application.account_check" ng-options="noyes.name for noyes in central.m_noyes"></select></div></div>
            </div>
          </div>
        </td>
      </tr>
      <tr ng-show="store.application.bEdit || store.application.package_type_id">
        <th style="white-space: nowrap;">
          Package Type:
        </th>
        <td>
          <span ng-show="!store.application.bEdit">{{store.application.package_type.type}}</span>
          <select ng-show="store.application.bEdit" class="form-control" ng-model="store.application.package_type" ng-options="package_type.type for package_type in store.package_types"></select>
        </td>
      </tr>
      <tr>
        <th>
          Dependable:
        </th>
        <td>
          <span ng-show="!store.application.bEdit">{{store.application.dependable.name}}</span>
          <select ng-show="store.application.bEdit" class="form-control" ng-model="store.application.dependable" ng-options="noyes.name for noyes in central.m_noyes"></select>
        </td>
      </tr>
      <tr ng-show="store.application.bEdit || store.application.menu_id">
        <th style="white-space: nowrap;">
          Menu Availability:
        </th>
        <td>
          <span ng-show="!store.application.bEdit">{{store.application.menu_access.type}}</span>
          <select ng-show="store.application.bEdit" class="form-control" ng-model="store.application.menu_access" ng-options="menu_access.type for menu_access in store.menu_accesses"></select>
        </td>
      </tr>
      <tr ng-if="store.application.bEdit || store.application.wiki.value == 1">
        <th>
          WIKI:
        </th>
        <td>
          <a ng-show="!store.application.bEdit" href="/wiki/index.php/{{store.application.name | commonURLEncode}}" target="_blank">/wiki/index.php/{{store.application.name}}</a>
          <select ng-show="store.application.bEdit" class="form-control" ng-model="store.application.wiki" ng-options="noyes.name for noyes in central.m_noyes"></select>
        </td>
      </tr>
      <tr>
        <th>
          Description:
        </th>
        <td>
          <pre ng-show="!store.application.bEdit" style="background: inherit; color: inherit; white-space: pre-wrap;">{{store.application.description}}</pre>
          <textarea ng-show="store.application.bEdit" class="form-control" ng-model="store.application.description"></textarea>
        </td>
      </tr>
    </table>
    <table ng-show="store.application.sysInfo" class="table table-condensed table-striped">
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
      <tr ng-repeat="info in store.application.sysInfo | orderBy:info.Server">
        <td><a href="#/Servers/{{info.ServerID}}">{{info.Server}}</a></td>
        <td>{{info.Daemon}}</td>
        <td>{{info.data.StartTime}}</td>
        <td>{{info.data.Owners}}</td>
        <td>{{info.data.NumberOfProcesses | number}}</td>
        <td>{{info.data.ImageSize | number}}</td>
        <td>{{info.data.ResidentSize | number}}</td>
        <td class="text-danger">{{info.data.Alarms}}</td>
      </tr>
    </table>
  </div>
  <div ng-show="store.application.forms.Accounts.active && (common.isGlobalAdmin() || store.application.bDeveloper)" class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>User ID</th>
        <th>Encrypt</th>
        <th>Password</th>
        <th>Type</th>
        <th>Description</th>
        <th ng-show="common.isGlobalAdmin() || store.application.bDeveloper"></th>
      </tr>
      <tr ng-show="common.isGlobalAdmin() || store.application.bDeveloper">
        <td><input type="text" class="form-control" ng-model="store.account.user_id" placeholder="User ID"></td>
        <td><select class="form-control" ng-model="store.account.encrypt" ng-options="noyes.name for noyes in central.m_noyes"></select></td>
        <td><input type="password" class="form-control" ng-model="store.account.password" placeholder="Password"></td>
        <td><select class="form-control" ng-model="store.account.type" ng-options="type.type for type in store.account.types"></select></td>
        <td><input type="text" class="form-control" ng-model="store.account.description" placeholder="Description"></td>
        <td><button class="btn btn-xs btn-success glyphicon glyphicon-plus" ng-click="addAccount()"></button></td>
      </tr>
      <tr ng-repeat="account in store.application.accounts">
        <td>
          <span ng-show="!account.bEdit">{{account.user_id}}</span>
          <input ng-show="account.bEdit" type="text" class="form-control" ng-model="account.user_id" placeholder="User ID">
        </td>
        <td>
          <span ng-show="!account.bEdit">{{account.encrypt.name}}</span>
          <select ng-show="account.bEdit" class="form-control" ng-model="account.encrypt" ng-options="noyes.name for noyes in central.m_noyes"></select>
        </td>
        <td>
          <span ng-show="!account.bEdit">{{account.password}}</span>
          <input ng-show="account.bEdit" type="text" class="form-control" ng-model="account.password" placeholder="Password">
        </td>
        <td>
          <span ng-show="!account.bEdit">{{account.type.type}}</span>
          <select ng-show="account.bEdit" class="form-control" ng-model="account.type" ng-options="type.type for type in store.account.types"></select>
        </td>
        <td>
          <pre ng-show="!account.bEdit && account.description" style="background: inherit; color: inherit; white-space: pre-wrap;">{{account.description}}</pre>
          <textarea ng-show="account.bEdit" class="form-control" ng-model="account.description" placeholder="Description"></textarea>
        </td>
        <td ng-show="common.isGlobalAdmin() || store.application.bDeveloper" style="white-space: nowrap;">
          <div ng-show="!account.bEdit"><button class="btn btn-xs btn-warning glyphicon glyphicon-pencil" ng-click="account.bEdit = true"></button><button class="btn btn-xs btn-danger glyphicon glyphicon-remove" ng-click="removeAccount(account.id)" style="margin-left: 10px;"></button></div>
          <div ng-show="account.bEdit"><button class="btn btn-xs btn-warning glyphicon glyphicon-arrow-left" ng-click="account.bEdit = false"></button><button class="btn btn-xs btn-success glyphicon glyphicon-ok" ng-click="editAccount(account)" style="margin-left: 10px;"></button></div>
        </td>
      </tr>
    </table>
  </div>
  <div ng-show="store.application.forms.Contacts.active" class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>User</th>
        <th>Type</th>
        <th>Admin</th>
        <th>Locked</th>
        <th>Notify</th>
        <th>Description</th>
        <th ng-show="common.isLocalAdmin(store.application.name) || store.application.bDeveloper"></th>
      </tr>
      <tr ng-show="common.isLocalAdmin(store.application.name) || store.application.bDeveloper">
        <td><input type="text" class="form-control" ng-model="store.contact[contactType.type].userid" placeholder="User ID"></td>
        <td><select class="form-control" ng-model="store.contact[contactType.type].type" ng-options="type.type for type in store.contactTypeOrder"></select></td>
        <td><select class="form-control" ng-model="store.contact[contactType.type].admin" ng-options="noyes.name for noyes in central.m_noyes"></select></td>
        <td><select class="form-control" ng-model="store.contact[contactType.type].locked" ng-options="noyes.name for noyes in central.m_noyes"></select></td>
        <td><select class="form-control" ng-model="store.contact[contactType.type].notify" ng-options="noyes.name for noyes in central.m_noyes"></select></td>
        <td><input type="text" class="form-control" ng-model="store.contact[contactType.type].description" placeholder="Description"></td>
        <td><button class="btn btn-xs btn-success glyphicon glyphicon-plus" ng-click="addContact(contactType.type)"></button></td>
      </tr>
      <tr ng-repeat="contact in store.application.contacts">
        <td style="white-space:nowrap;">
          <a ng-show="!contact.bEdit" href="#/Users/{{contact.user_id}}">{{contact.last_name}}, {{contact.first_name}}</a> <small>({{contact.userid}})</small>
          <div ng-show="contact.bEdit"><input type="text" class="form-control" ng-model="contact.userid" placeholder="User ID"></div>
        </td>
        <td style="white-space:nowrap;">
          <span ng-show="!contact.bEdit">{{contact.type.type}}</span>
          <select ng-show="contact.bEdit" class="form-control" ng-model="contact.type" ng-options="type.type for type in store.contactTypeOrder"></select>
        </td>
        <td>
          <span ng-show="!contact.bEdit">{{contact.admin.name}}</span>
          <select ng-show="contact.bEdit" class="form-control" ng-model="contact.admin" ng-options="noyes.name for noyes in central.m_noyes"></select>
        </td>
        <td>
          <span ng-show="!contact.bEdit">{{contact.locked.name}}</span>
          <select ng-show="contact.bEdit" class="form-control" ng-model="contact.locked" ng-options="noyes.name for noyes in central.m_noyes"></select>
        </td>
        <td>
          <span ng-show="!contact.bEdit">{{contact.notify.name}}</span>
          <select ng-show="contact.bEdit" class="form-control" ng-model="contact.notify" ng-options="noyes.name for noyes in central.m_noyes"></select>
        </td>
        <td>
          <pre ng-show="!contact.bEdit && contact.description" style="background: inherit; color: inherit; white-space: pre-wrap;">{{contact.description}}</pre>
          <input ng-show="contact.bEdit" type="text" class="form-control" ng-model="contact.description" placeholder="Description">
        </td>
        <td ng-show="common.isLocalAdmin() || store.application.bDeveloper" style="white-space: nowrap;">
          <div ng-show="!contact.bEdit"><button class="btn btn-xs btn-warning glyphicon glyphicon-pencil" ng-click="contact.bEdit = true"></button><button class="btn btn-xs btn-danger glyphicon glyphicon-remove" ng-click="removeContact(contact.id)" style="margin-left: 10px;"></button></div>
          <div ng-show="contact.bEdit"><button class="btn btn-xs btn-warning glyphicon glyphicon-arrow-left" ng-click="contact.bEdit = false"></button><button class="btn btn-xs btn-success glyphicon glyphicon-ok" ng-click="editContact(contact)" style="margin-left: 10px;"></button></div>
        </td>
      </tr>
    </table>
  </div>
  <div ng-show="store.application.forms.Depend.active" class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Application</th>
        <th ng-show="common.isGlobalAdmin() || store.application.bDeveloper"></th>
      </tr>
      <tr ng-show="common.isGlobalAdmin() || store.application.bDeveloper">
        <td><select class="form-control" ng-model="store.depend" ng-options="depend.name for depend in store.dependApplications"></select></td>
        <td><button class="btn btn-xs btn-success glyphicon glyphicon-plus" ng-click="addDepend()"></button></td>
      </tr>
      <tr ng-repeat="depend in store.application.depends">
        <td><a href="#/Applications/{{depend.application_id}}">{{depend.name}}</a></td>
        <td ng-show="common.isGlobalAdmin() || store.application.bDeveloper"><button class="btn btn-xs btn-danger glyphicon glyphicon-remove" ng-click="removeDepend(depend.id)"></button></td>
      </tr>
    </table>
    <h4 class="page-header">Depend Upon {{store.application.name}}</h4>
    <table class="table table-condensed table-striped">
      <tr>
        <th>Application</th>
      </tr>
      <tr ng-repeat="dependent in store.application.dependents">
        <td><a href="#/Applications/{{dependent.application_id}}">{{dependent.name}}</a></td>
      </tr>
    </table>
  </div>
  <div ng-show="store.application.forms.Issues.active" class="table-responsive">
    <div ng-show="!store.issue">
      <table class="table table-condensed table-striped">
        <tr ng-show="common.isValid(null)">
          <td style="width:25%;">
            <table class="table table-condensed" style="background: inherit;">
              <tr><th>Due</th><td><input type="text" ng-model="issue.due_date" placeholder="YYYY-MM-DD"></td></tr>
              <tr><th>Priority</th><td><select ng-model="issue.priority" class="form-control"><option value="1">Low</option><option style="color: orange;" value="2">Medium</option><option style="color: red;" value="3">High</option><option style="background: red; color: white;" value="4">Critical</option></select></td></tr>
              <tr><th>Assigned</th><td><input type="text" ng-model="issue.assigned_userid" placeholder="User ID"></td></tr>
            </table>
          </td>
          <td style="width:75%;">
            <input type="text" class="form-control" ng-model="issue.summary" placeholder="enter summary" style="width: 100%;">
            <br>
            <textarea ng-model="issue.comments" class="form-control" rows="5" style="width: 100%;" placeholder="enter comments"></textarea>
            <button class="btn btn-sm btn-default pull-right" ng-click="addIssue()" style="margin-top: 10px;">Add Issue</button>
          </td>
        </tr>
      </table>
      <button class="btn btn-sm btn-default pull-right" ng-click="toggleClosedIssues()"><span ng-if="store.onlyOpenIssues == 0">Hide</span><span ng-if="store.onlyOpenIssues == 1">Show</span> Closed Issues</button>
      <table class="table table-condensed table-striped">
        <tr ng-repeat="issue in store.application.issues">
          <td>
            <table class="table table-condensed" style="background: inherit;">
              <tr><th style="white-space: nowrap;">Issue #</th><td><a href="#/Applications/{{store.application.id}}/Issues/{{issue.id}}">{{issue.id}}</a><span ng-if="issue.hold == 1" style="margin-left: 20px; padding: 0px 2px; background: green; color: white;">HOLD</span></td></tr>
              <tr ng-show="issue.open_date"><th>Open</th><td style="white-space: nowrap;">{{issue.open_date}}</td></tr>
              <tr ng-show="issue.due_date"><th>Due</th><td style="white-space: nowrap;">{{issue.due_date}}</td></tr>
              <tr ng-show="issue.release_date"><th>Release</th><td style="white-space: nowrap;">{{issue.release_date}}</td></tr>
              <tr ng-show="issue.close_date"><th>Close</th><td style="white-space: nowrap;">{{issue.close_date}}</td></tr>
              <tr ng-if="issue.priority >= 1"><th>Priority</th><td ng-if="issue.priority == 1">Low</td><td ng-if="issue.priority == 2" style="color: orange;">Medium</td><td ng-if="issue.priority == 3" style="color: red;">High</td><td ng-if="issue.priority > 3" style="color: white;"><span style="padding: 0px 2px; background: red;">Critical</span></td></tr>
              <tr ng-show="issue.comments"><td colspan="2"><a href="#/Users/{{issue.comments[0].user_id}}">{{issue.comments[0].last_name}}, {{issue.comments[0].first_name}}</a> <small>({{issue.comments[0].userid}})</small></td></tr>
              <tr ng-show="issue.assigned"><td colspan="2"><a href="#/Users/{{issue.assigned.id}}">{{issue.assigned.last_name}}, {{issue.assigned.first_name}}</a> <small>({{issue.assigned.userid}})</small></td></tr>
            </table>
          </td>
          <td>
            <p ng-show="issue.summary" style="font-weight: bold;">{{issue.summary}}</p>
            <pre ng-show="issue.comments" style="background: inherit; color: inherit; white-space: pre-wrap;">{{issue.comments[0].comments}}</pre>
          </td>
        </tr>
      </table>
    </div>
    <div ng-show="store.issue && store.application.issue">
      <button ng-show="common.isValid(null) && store.application.issue.close_date" class="btn btn-sm btn-success glyphicon glyphicon-refresh pull-right" ng-click="store.application.issue.close_date = null; editIssue(store.application.issue.id)"></button>
      <div class="row">
        <div class="col-md-3">
          <table class="table table-condensed well">
            <tr><th style="white-space: nowrap;">Issue #</th><td style="white-space: nowrap;">{{store.application.issue.id}}</td></tr>
            <tr ng-show="store.application.issue.hold == 1 && !common.isGlobalAdmin() && !store.application.bDeveloper"><th></th><td style="margin-left: 10px; background: green; color: white; white-space: nowrap;">HOLD</td></tr>
            <tr ng-show="(common.isGlobalAdmin() || store.application.bDeveloper) && !store.application.issue.close_date"><th style="white-space: nowrap;">On Hold</th><td><select ng-model="store.application.issue.hold" class="form-control"><option value="0">No</option><option style="background: green; color: white;" value="1">Yes</option></select></td></tr>
            <tr ng-show="(store.application.issue.priority >= 1 && !common.isGlobalAdmin() && !store.application.bDeveloper) || store.application.issue.close_date"><th>Priority</th><td><span ng-if="store.application.issue.priority == 1">Low</span><span ng-if="store.application.issue.priority == 2" style="color: orange;">Medium</span><span ng-if="store.application.issue.priority == 3" style="color: red;">High</span><span ng-if="store.application.issue.priority > 3" style="padding: 0px 2px; background: red; color: white;">Critical</span></td></tr>
            <tr ng-show="(common.isGlobalAdmin() || store.application.bDeveloper) && !store.application.issue.close_date"><th>Priority</th><td><select ng-model="store.application.issue.priority" class="form-control"><option value="1">Low</option><option style="color: orange;" value="2">Medium</option><option style="color: red;" value="3">High</option><option style="background: red; color: white;" value="4">Critical</option></select></td></tr>
            <tr ng-show="store.application.issue.open_date"><th>Open</th><td style="white-space: nowrap;">{{store.application.issue.open_date}}</td></tr>
            <tr ng-show="(store.application.issue.due_date && !common.isGlobalAdmin() && !store.application.bDeveloper) || store.application.issue.close_date"><th>Due</th><td style="white-space: nowrap;">{{store.application.issue.due_date}}</td></tr>
            <tr ng-show="(common.isGlobalAdmin() || store.application.bDeveloper) && !store.application.issue.close_date"><th>Due</th><td><input type="text" class="form-control" ng-model="store.application.issue.due_date" placeholder="YYYY-MM-DD"></td></tr>
            <tr ng-show="(store.application.issue.release_date && !common.isGlobalAdmin() && !store.application.bDeveloper) || store.application.issue.close_date"><th>Release</th><td style="white-space: nowrap;">{{store.application.issue.release_date}}</td></tr>
            <tr ng-show="(common.isGlobalAdmin() || store.application.bDeveloper) && !store.application.issue.close_date"><th>Release</th><td><input type="text" class="form-control" ng-model="store.application.issue.release_date" placeholder="YYYY-MM-DD"></td></tr>
            <tr ng-show="store.application.issue.close_date"><th>Close</th><td style="white-space: nowrap;">{{store.application.issue.close_date}}</td></tr>
            <tr ng-show="(store.application.issue.assigned && !common.isGlobalAdmin() && !store.application.bDeveloper) || store.application.issue.close_date"><th>Assigned</th><td style="white-space: nowrap;"><a href="#/Users/{{store.application.issue.assigned.id}}">{{store.application.issue.assigned.last_name}}, {{store.application.issue.assigned.first_name}}</a> <small>({{store.application.issue.assigned.userid}})</small></td></tr>
            <tr ng-show="(common.isGlobalAdmin() || store.application.bDeveloper) && !store.application.issue.close_date"><th>Assigned</th><td><input type="text" class="form-control" ng-model="store.application.issue.assigned_userid" placeholder="User ID"></td></tr>
            <tr ng-show="(common.isGlobalAdmin() || store.application.bDeveloper) && !store.application.issue.close_date"><th>Transfer</th><td><select class="form-control" ng-model="store.application.issue.transfer" ng-options="application.name for application in store.applications"></select></td></tr>
          </table>
          <button ng-show="(common.isGlobalAdmin() || store.application.bDeveloper) && !store.application.issue.close_date" class="btn btn-warning pull-right" ng-click="editIssue()">Save</button>
        </div>
        <div class="col-md-9">
          <p ng-show="store.application.issue.summary && ((!common.isGlobalAdmin() && !store.application.bDeveloper) || store.application.issue.close_date)" style="font-weight: bold;">{{store.application.issue.summary}}</p>
          <input ng-show="(common.isGlobalAdmin() || store.application.bDeveloper) && !store.application.issue.close_date" type="text" class="form-control" ng-model="store.application.issue.summary" placeholder="enter summary" style="width: 100%; font-weight: bold;">
          <table class="table table-condensed table-striped" style="margin-top: 10px;">
            <tr ng-repeat="comment in store.application.issue.comments">
              <td>
                <table class="table table-condensed" style="background: inherit;">
                  <tr><td style="white-space: nowrap;">{{comment.entry_date}}</td></tr>
                  <tr><td style="white-space: nowrap;"><a href="#/Users/{{comment.user_id}}">{{comment.last_name}}, {{comment.first_name}}</a> <small>({{comment.userid}})</small></td></tr>
                  <tr ng-show="!store.application.issue.close_date && !comment.bEdit && comment.userid == common.getUserID()"><td><button class="btn btn-sm btn-default glyphicon glyphicon-pencil pull-right" ng-click="comment.bEdit = true"></button></td></tr>
                </table>
              </td>
              <td>
                <pre ng-show="!store.application.issue.close_date || !comment.bEdit || comment.userid != common.getUserID()" style="background: inherit; color: inherit; white-space: pre-wrap;">{{comment.comments}}</pre>
                <textarea ng-show="!store.application.issue.close_date && comment.bEdit && comment.userid == common.getUserID()" ng-model="comment.comments" class="form-control" rows="5" style="width: 100%;" placeholder="enter comments">{{comment.comments}}</textarea>
                <button ng-show="!store.application.issue.close_date && comment.bEdit && comment.userid == common.getUserID()" class="btn btn-sm btn-default pull-right" ng-click="editIssueComment(comment)" style="margin: 10px 0px 0px 10px;">Save</button>
                <button ng-show="!store.application.issue.close_date && comment.bEdit && comment.userid == common.getUserID()" class="btn btn-sm btn-default pull-right" ng-click="comment.bEdit = false" style="margin: 10px 0px 0px 0px;">Cancel</button>
              </td>
            </tr>
            <tr ng-show="!store.application.issue.close_date && common.isValid(null)">
              <td></td>
              <td>
                <textarea ng-model="issue.comments" class="form-control" rows="5" style="width: 100%;" placeholder="enter comments"></textarea>
                <button class="btn btn-sm btn-default pull-right" ng-click="addIssueComment('close', store.application.issue.id, store.application.id)" style="margin: 10px 0px 0px 10px;">Close Issue</button>
                <button class="btn btn-sm btn-default pull-right" ng-click="addIssueComment('update', store.application.issue.id, store.application.id)" style="margin: 10px 0px 0px 0px;">Add Comments</button>
              </td>
            </tr>
          </table>
        </div>
      </div>
    </div>
  </div>
  <div ng-show="store.application.forms.Notify.active && (common.isGlobalAdmin() || store.application.bDeveloper)" class="table-responsive">
    <textarea ng-model="store.notification" class="form-control" placeholder="enter notification" rows="5"></textarea>
    <button class="btn btn-sm btn-default pull-right" ng-click="sendNotification()">Send Notification</button>
    <div ng-show="contacts">
      <h4 class="page-header">Sent Successfully</h4>
      <ul>
        <li ng-show="contact.sent" ng-repeat="(userid, contact) in contacts"><a href="#/Users/?userid={{userid}}">{{contact.name}}</a> <small>({{contact.userid}})</small></li>
      </ul>
      <h4 class="page-header">Sent Unsuccessfully</h4>
      <ul>
        <li ng-show="!contact.sent" ng-repeat="(userid, contact) in contacts"><a href="#/Users/?userid={{userid}}">{{contact.name}}</a> <small>({{contact.userid}})</small></li>
      </ul>
    </div>
  </div>
  <div ng-show="store.application.forms.Servers.active" class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th style="width: 100%;">Server</th>
        <th ng-show="common.isGlobalAdmin() || store.application.bDeveloper"></th>
        <th ng-show="common.isGlobalAdmin() || store.application.bDeveloper"></th>
      </tr>
      <tr ng-show="common.isGlobalAdmin() || store.application.bDeveloper">
        <td><select class="form-control" ng-model="store.server" ng-options="server.name for server in store.servers"></select></td>
        <td><button class="btn btn-xs btn-success glyphicon glyphicon-plus" ng-click="addServer()"></button></td>
      </tr>
      <tr ng-repeat="server in store.application.servers">
        <td><a href="#/Servers/{{server.server_id}}">{{server.name}}</a></td>
        <td ng-show="common.isGlobalAdmin() || store.application.bDeveloper" style="white-space: nowrap;"><button class="btn btn-xs btn-warning glyphicon glyphicon-pencil" data-toggle="modal" data-target="#serverModal" ng-click="serverDetails(server)"></button><button class="btn btn-xs btn-danger glyphicon glyphicon-remove" ng-click="removeServer(server.id)"></button></td>
      </tr>
    </table>
    <div id="serverModal" class="modal fad" role="dialog">
      <div class="modal-dialog">
        <div class="modal-content">
          <div class="modal-header">
            <button type="button" class="close" data-dismiss="modal">&times;</button>
            <h4 class="modal-title">Edit Monitoring Details - {{modalServer.name}}</h4>
          </div>
          <div class="modal-body table-responsive">
            <table class="table table-condensed table-striped">
              <tr>
                <th colspan="5"></th>
                <th colspan="2">Processes</th>
                <th colspan="2">Image</th>
                <th colspan="2">Resident</th>
                <th ng-show="common.isGlobalAdmin() || store.application.bDeveloper"></th>
                <th ng-show="common.isGlobalAdmin() || store.application.bDeveloper"></th>
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
                <th ng-show="common.isGlobalAdmin() || store.application.bDeveloper"></th>
                <th ng-show="common.isGlobalAdmin() || store.application.bDeveloper"></th>
              </tr>
              <tr ng-repeat="detail in modalServer.details">
                <td>
                  <span ng-show="!detail.bEdit">{{detail.daemon}}</span>
                  <input ng-show="detail.bEdit" type="text" class="form-control" ng-model="detail.daemon">
                </td>
                <td>
                  <span ng-show="!detail.bEdit">{{detail.version}}</span>
                  <input ng-show="detail.bEdit" type="text" class="form-control" ng-model="detail.version">
                </td>
                <td>
                  <span ng-show="!detail.bEdit">{{detail.owner}}</span>
                  <input ng-show="detail.bEdit" type="text" class="form-control" ng-model="detail.owner">
                </td>
                <td>
                  <span ng-show="!detail.bEdit">{{detail.script}}</span>
                  <input ng-show="detail.bEdit" type="text" class="form-control" ng-model="detail.script">
                </td>
                <td>
                  <span ng-show="!detail.bEdit">{{detail.delay}}</span>
                  <input ng-show="detail.bEdit" type="text" class="form-control" ng-model="detail.delay">
                </td>
                <td>
                  <span ng-show="!detail.bEdit">{{detail.min_processes}}</span>
                  <input ng-show="detail.bEdit" type="text" class="form-control" ng-model="detail.min_processes">
                </td>
                <td>
                  <span ng-show="!detail.bEdit">{{detail.max_processes}}</span>
                  <input ng-show="detail.bEdit" type="text" class="form-control" ng-model="detail.max_processes">
                </td>
                <td>
                  <span ng-show="!detail.bEdit">{{detail.min_image}}</span>
                  <input ng-show="detail.bEdit" type="text" class="form-control" ng-model="detail.min_image">
                </td>
                <td>
                  <span ng-show="!detail.bEdit">{{detail.max_image}}</span>
                  <input ng-show="detail.bEdit" type="text" class="form-control" ng-model="detail.max_image">
                </td>
                <td>
                  <span ng-show="!detail.bEdit">{{detail.min_resident}}</span>
                  <input ng-show="detail.bEdit" type="text" class="form-control" ng-model="detail.min_resident">
                </td>
                <td>
                  <span ng-show="!detail.bEdit">{{detail.max_resident}}</span>
                  <input ng-show="detail.bEdit" type="text" class="form-control" ng-model="detail.max_resident">
                </td>
                <td ng-show="common.isGlobalAdmin() || store.application.bDeveloper">
                  <button ng-show="!detail.bEdit" class="btn btn-xs btn-warning glyphicon glyphicon-pencil" ng-click="detail.bEdit = true"></button>
                  <button ng-show="detail.bEdit" class="btn btn-xs btn-warning glyphicon glyphicon-arrow-left" ng-click="detail.bEdit = false"></button>
                </td>
                <td ng-show="common.isGlobalAdmin() || store.application.bDeveloper">
                  <button ng-show="!detail.bEdit" class="btn btn-xs btn-danger glyphicon glyphicon-remove" ng-click="removeServerDetail(detail.id)"></button>
                  <button ng-show="detail.bEdit" class="btn btn-xs btn-success glyphicon glyphicon-ok" ng-click="editServerDetail(detail)"></button>
                </td>
              </tr>
              <tr ng-show="common.isGlobalAdmin() || store.application.bDeveloper">
                <td>
                  <input type="text" class="form-control" ng-model="serverDetail.daemon">
                </td>
                <td>
                  <input type="text" class="form-control" ng-model="serverDetail.version">
                </td>
                <td>
                  <input type="text" class="form-control" ng-model="serverDetail.owner">
                </td>
                <td>
                  <input type="text" class="form-control" ng-model="serverDetail.script">
                </td>
                <td>
                  <input type="text" class="form-control" ng-model="serverDetail.delay">
                </td>
                <td>
                  <input type="text" class="form-control" ng-model="serverDetail.min_processes">
                </td>
                <td>
                  <input type="text" class="form-control" ng-model="serverDetail.max_processes">
                </td>
                <td>
                  <input type="text" class="form-control" ng-model="serverDetail.min_image">
                </td>
                <td>
                  <input type="text" class="form-control" ng-model="serverDetail.max_image">
                </td>
                <td>
                  <input type="text" class="form-control" ng-model="serverDetail.min_resident">
                </td>
                <td>
                  <input type="text" class="form-control" ng-model="serverDetail.max_resident">
                </td>
                <td colspan="2">
                  <button ng-show="!detail.bEdit" class="btn btn-xs btn-success glyphicon glyphicon-plus" ng-click="addServerDetail()"></button>
                </td>
              </tr>
            </table>
          </div>
        </div>
      </div>
    </div>
  </div>
  {{/if}}
  `
  // }}}
}
