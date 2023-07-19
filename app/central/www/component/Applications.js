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
    s.editIssue = (bOpen) =>
    {
      if (c.isDefined(bOpen) && bOpen)
      {
        s.application.issue.close_date = null;
      }
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
    // {{{ preEditAccount()
    s.preEditAccount = (account, bEdit) =>
    {
      account.bEdit = bEdit;
    };
    // }}}
    // {{{ preEditApplication()
    s.preEditApplication = (bEdit) =>
    {
      s.application.bEdit = bEdit;
    };
    // }}}
    // {{{ preEditContact()
    s.preEditContact = (contact, bEdit) =>
    {
      contact.bEdit = bEdit;
    };
    // }}}
    // {{{ preEditIssueComment()
    s.preEditIssueComment = (comment, bEdit) =>
    {
      comment.bEdit = bEdit;
    };
    // }}}
    // {{{ preEditServerDetail()
    s.preEditServerDetail = (detail, bEdit) =>
    {
      detail.bEdit = bEdit;
    };
    // }}}
    // {{{ removeAccount()
    s.removeAccount = (nID) =>
    {
      if (confirm('Are you sure you want to remove this application account?'))
      {
        let request = {Interface: 'central', 'Function': 'applicationAccountRemove', Request: {id: nID}};
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
      }
    };
    // }}}
    // {{{ removeApplication()
    s.removeApplication = () =>
    {
      if (confirm('Are you sure you want to remove this application?'))
      {
        let request = {Interface: 'central', 'Function': 'applicationRemove', Request: {id: s.application.id}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.application = null;
            document.location.href = '#/Applications';
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
    };
    // }}}
    // {{{ removeContact()
    s.removeContact = (nID) =>
    {
      if (confirm('Are you sure you want to remove this application contact?'))
      {
        let request = {Interface: 'central', 'Function': 'applicationUserRemove', Request: {id: nID}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.application.contacts  = null;
            s.showForm('Contacts');
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
    };
    // }}}
    // {{{ removeDepend()
    s.removeDepend = (nID) =>
    {
      if (confirm('Are you sure you want to remove this application depend?'))
      {
        let request = {Interface: 'central', 'Function': 'applicationDependRemove', Request: {id: nID}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.application.depends  = null;
            s.application.dependents  = null;
            s.showForm('Depend');
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
    };
    // }}}
    // {{{ removeServer()
    s.removeServer = (nID) =>
    {
      if (confirm('Are you sure you want to remove this application server?'))
      {
        let request = {Interface: 'central', 'Function': 'applicationServerRemove', Request: {id: nID}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.application.servers  = null;
            s.showForm('Servers');
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
    };
    // }}}
    // {{{ removeServerDetail()
    s.removeServerDetail = (nID) =>
    {
      if (confirm('Are you sure you want to remove this application server detail?'))
      {
        let request = {Interface: 'central', 'Function': 'applicationServerDetailRemove', Request: {id: nID}};
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
      }
    };
    // }}}
    // {{{ sendNotification()
    s.sendNotification = () =>
    {
      if (confirm('Are you sure you want to send this application notification?'))
      {
        let request = {Interface: 'central', 'Function': 'applicationNotify', Request: {id: s.application.id, notification: s.notification, server: location.host}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            s.application.contacts  = response.Response;
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
    };
    // }}}
    // {{{ serverDetails()
    s.serverDetails = (server) =>
    {
      let request = {Interface: 'central', 'Function': 'applicationServerDetails', Request: {application_server_id: server.id}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.modalServer.details = response.Response;
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // }}}
    // {{{ showForm()
    s.showForm = (strForm) =>
    {
      s.info.v = null;
      s.initForms();
      if (!c.isDefined(s.application.forms[strForm]))
      {
        strForm = 'General';
      }
      s.application.forms.forEach((value, key) =>
      {
        value.active = null;
      });
      s.application.forms[strForm].active = 'active';
      // {{{ General
      if (strForm == 'General')
      {
        for (let i = 0; i < s.login_types.length; i++)
        {
          if (s.login_types[i].id == s.application.login_type_id)
          {
            s.application.login_type = s.login_types[i];
          }
        }
        for (let i = 0; i < s.menu_accesses.length; i++)
        {
          if (s.menu_accesses[i].id == s.application.menu_id)
          {
            s.application.menu_access = s.menu_accesses[i];
          }
        }
        for (let i = 0; i < s.notify_priorities.length; i++)
        {
          if (s.notify_priorities[i].id == s.application.notify_priority_id)
          {
            s.application.notify_priority = s.notify_priorities[i];
          }
        }
        for (let i = 0; i < s.package_types.length; i++)
        {
          if (s.package_types[i].id == s.application.package_type_id)
          {
            s.application.package_type = s.package_types[i];
          }
        }
        s.application.account_check = a.setNoYes(s.application.account_check);
        s.application.auto_register = a.setNoYes(s.application.auto_register);
        s.application.dependable = a.setNoYes(s.application.dependable);
        s.application.secure_port = a.setNoYes(s.application.secure_port);
        s.application.wiki = a.setNoYes(s.application.wiki);
      }
      // }}}
      // {{{ Accounts
      else if (strForm == 'Accounts')
      {
        if (c.isGlobalAdmin() || s.application.bDeveloper)
        {
          if (!c.isDefined(s.application.accounts) || s.application.accounts == null)
          {
            s.account = {application_id: s.application.id, encrypt: a.m_noyes[0]};
            s.application.accounts = null;
            s.application.accounts = [];
            s.info.v = 'Retrieving accounts...';
            let request = {Interface: 'central', 'Function': 'applicationAccountsByApplicationID', Request: {application_id: s.application.id}};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              s.info.v = null;
              if (c.wsResponse(response, error))
              {
                for (let i = 0; i < response.Response.length; i++)
                {
                  s.application.accounts.push(response.Response[i]);
                }
                let request = {Interface: 'central', 'Function': 'accountTypes', Request: {}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    s.account.types = response.Response;
                    s.account.type = s.account.types[0];
                    for (let i = 0; i < s.application.accounts.length; i++)
                    {
                      for (let j = 0; j < a.m_noyes.length; j++)
                      {
                        if (s.application.accounts[i].encrypt.value == a.m_noyes[j].value)
                        {
                          s.application.accounts[i].encrypt = a.m_noyes[j];
                        }
                      }
                      for (let j = 0; j < s.account.types.length; j++)
                      {
                        if (s.application.accounts[i].type.id == s.account.types[j].id)
                        {
                          s.application.accounts[i].type = s.account.types[j];
                        }
                      }
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
                s.message.v = error.message;
              }
            });
          }
        }
        else
        {
          s.message.v = 'You are not authorized to view this data.';
        }
      }
      // }}}
      // {{{ Contacts
      else if (strForm == 'Contacts')
      {
        if (!c.isDefined(s.application.contacts) || s.application.contacts == null)
        {
          s.contact = {};
          for (let i = 0; i < s.contactTypeOrder.length; i++)
          {
            s.contact[s.contactTypeOrder[i]] = {application_id: s.application.id, type: s.contactTypeOrder[i], admin: a.m_noyes[0], locked: a.m_noyes[0], notify: a.m_noyes[1]};
          }
          s.application.contacts = null;
          s.application.contacts = [];
          s.info.v = 'Retrieving contacts...';
          let request = {Interface: 'central', 'Function': 'applicationUsersByApplicationID', Request: {application_id: s.application.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              for (let i = 0; i < response.Response.length; i++)
              {
                for (let j = 0; j < s.contactTypeOrder.length; j++)
                {
                  if (response.Response[i].type.type == s.contactTypeOrder[j].type)
                  {
                    response.Response[i].type = s.contactTypeOrder[j];
                  }
                }
                for (let j = 0; j < a.m_noyes.length; j++)
                {
                  if (response.Response[i].admin.value == a.m_noyes[j].value)
                  {
                    response.Response[i].admin = a.m_noyes[j];
                  }
                  if (response.Response[i].locked.value == a.m_noyes[j].value)
                  {
                    response.Response[i].locked = a.m_noyes[j];
                  }
                  if (response.Response[i].notify.value == a.m_noyes[j].value)
                  {
                    response.Response[i].notify = a.m_noyes[j];
                  }
                }
                s.application.contacts.push(response.Response[i]);
              }
            }
            else
            {
              s.message.v = error.message;
            }
          });
        }
      }
      // }}}
      // {{{ Depend
      else if (strForm == 'Depend')
      {
        if (!c.isDefined(s.application.depends) || s.application.depends == null || !c.isDefined(s.application.dependents) || s.application.dependents == null)
        {
          s.info.v = 'Retrieving dependents...';
          let request = {Interface: 'central', 'Function': 'dependentsByApplicationID', Request: {application_id: s.application.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info = null;
            if (c.wsResponse(response, error))
            {
              s.application.depends = response.Response.depends;
              s.application.dependents = response.Response.dependents;
              if (c.isGlobalAdmin() || s.application.bDeveloper)
              {
                let request = {Interface: 'central', 'Function': 'applications', Request: {dependable: 1}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    s.dependApplications = response.Response;
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
      }
      // }}}
      // {{{ Issues
      else if (strForm == 'Issues')
      {
        if (c.isObject(nav.data) && c.isDefined(nav.data.issue_id))
        {
          s.issue = true;
          if (!c.isDefined(s.application.issue) || s.application.issue == null || nav.data.issue_id != s.application.issue.id)
          {
            s.application.issue = null;
            s.info.v = 'Retrieving issue...';
            let request = {Interface: 'central', 'Function': 'applicationIssue', Request: {id: nav.data.issue_id, comments: 1}};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              s.info.v = null;
              if (c.wsResponse(response, error))
              {
                s.application.issue = response.Response;
                if (s.application.issue.assigned && s.application.issue.assigned.userid)
                {
                  s.application.issue.assigned_userid = s.application.issue.assigned.userid;
                }
                if (c.isGlobalAdmin() || s.application.bDeveloper)
                {
                  s.applications = null;
                  let request = {Interface: 'central', 'Function': 'applications', Request: {}};
                  c.wsRequest('radial', request).then((response) =>
                  {
                    let error = {};
                    if (c.wsResponse(response, error))
                    {
                      s.applications = response.Response;
                      for (let i = 0; i < s.applications.length; i++)
                      {
                        if (s.applications[i].id == s.application.id)
                        {
                          s.application.issue.transfer = s.applications[i];
                        }
                      }
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
        }
        else
        {
          s.issue.v = false;
          s.issue = {priority: '1'};
          if (!c.isDefined(s.application.issues) || s.application.issues == null)
          {
            s.info.v = 'Retrieving issues...';
            let request = {Interface: 'central', 'Function': 'applicationIssuesByApplicationID', Request: {application_id: s.application.id, comments: 1, open: s.onlyOpenIssues}};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              s.info.v = null;
              if (c.wsResponse(response, error))
              {
                s.application.issues = response.Response;
              }
              else
              {
                s.message.v = error.message;
              }
            });
          }
        }
      }
      // }}}
      // {{{ Notify
      else if (strForm == 'Notify')
      {
      }
      // }}}
      // {{{ Servers
      else if (strForm == 'Servers')
      {
        if (!c.isDefined(s.application.servers) || s.application.servers == null)
        {
          s.info.v = 'Retrieving servers...';
          let request = {Interface: 'central', 'Function': 'serversByApplicationID', Request: {application_id: $scope.store.application.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info = null;
            if (c.wsResponse(response, error))
            {
              s.application.servers = response.Response;
              if (c.isGlobalAdmin() || s.application.bDeveloper)
              {
                let request = {Interface: 'central', 'Function': 'servers', Request: {}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    s.servers = response.Response;
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
      }
      // }}}
    };
    // }}}
    // {{{ sysInfoStatus()
    s.sysInfoStatus = () =>
    {
      if (s.application && s.application.id)
      {
        let request = {Interface: 'central', 'Function': 'serverDetailsByApplicationID', Request: {application_id: s.application.id}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          if (c.wsResponse(response, error))
          {
            if (response.Response)
            {
              for (let i = 0; i < response.Response.length; i++)
              {
                if (response.Response[i].daemon)
                {
                  let request = {Interface: 'junction', Request: [{Service: 'sysInfo', Action: 'process', Server: response.Response[i].name, Process: response.Response[i].daemon, server_id: response.Response[i].server_id}]};
                  c.wsRequest('radial', request).then((response) =>
                  {
                    let error = {};
                    if (c.wsResponse(response, error))
                    {
                      let nIndex = -1;
                      if (!s.application.sysInfo)
                      {
                        s.application.sysInfo = [];
                      }
                      for (let i = 0; i < s.application.sysInfo.length; i++)
                      {
                        if (s.application.sysInfo[i].ServerID == response.Request[0].server_id && s.application.sysInfo[i].Daemon == response.Request[0].Process)
                        {
                          nIndex = i;
                        }
                      }
                      if (nIndex == -1)
                      {
                        let info = {};
                        nIndex = s.application.sysInfo.length;
                        info.ServerID = response.Request[0].server_id;
                        info.Server = response.Request[0].Server;
                        info.Daemon = response.Request[0].Process;
                        s.application.sysInfo[nIndex] = info;
                      }
                      s.application.sysInfo[nIndex].data = response.Response[1];
                    }
                    else
                    {
                      s.message.v = error.message;
                    }
                  });
                }
              }
            }
          }
          else
          {
            s.message.v = error.message;
          }
        });
      }
    };
    // }}}
    // {{{ sysInfoUpdate()
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
    // }}}
    // {{{ toggleClosedIssues()
    s.toggleClosedIssues = () =>
    {
      s.application.issues = null;
      s.onlyOpenIssues = ((s.onlyOpenIssues == 1)?0:1);
      s.showForm('Issues');
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
      {{/eachFilter}}
    </table>
  </div>
  {{else}}
  {{/if}}
  `
  // }}}
}
