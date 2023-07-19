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
    </table>
  </div>
  {{else}}
  <h3 class="page-header">{{application.name}}{{#if application.retirement_date}}<small class="text-danger"> --- RETIRED</small>{{/if}}</h3>
  <div c-model="info" class="text-warning">{{info}}</div>
  <div c-model="message" class="text-danger" style="font-weight:bold;">{{store.message}}</div>
  <nav class="container navbar navbar-dark bg-dark bg-gradient">
    <div class="container-fluid">
      <button type="button" class="navbar-toggle" data-bs-toggle="collapse" data-bs-target="#appnavigationbar" aria-control="appnavigationbar" aria-expanded="false", aria-lable="Toggle navigation">
        <span class="navbar-toggler-icon"></span>
      </button>
      <div class="collapse navbar-collapse" id="appnavigationbar">
        <ul class="navbar-nav me-auto mb-2 mb-lg-0">
          {{#each application.forms_order}}
          {{#each application.forms}}
          {{#ifCond @key '==' ../.}}
          <li class="nav-item"><a class="nav-link {{../active}}" href="#/Applications/{{../id}}/{{../../.}}">{{../../.}}</a></li>
          {{/ifCond}}
          {{/each}}
          {{/each}}
        </ul>
      </div>
    </div>
  </nav>
  {{#ifCond application.forms.General.active '&&' application}}
  {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
  <div class="float-end">
    {{^if application.bEdit}}
    <div style="white-space: nowrap;">
      <button class="btn btn-xs btn-warning" c-click="editApplication()">Edit</button>
      <button class="btn btn-xs btn-danger" c-click="removeApplication()" style="margin-left: 10px;">Remove</button>
    </div>
    {{/if}}
    <div ng-show="store.application.bEdit" style="white-space: nowrap;">
      <button class="btn btn-xs btn-warning glyphicon glyphicon-arrow-left" ng-click="preEditApplication(false)"></button>
      <button class="btn btn-xs btn-success glyphicon glyphicon-ok" ng-click="editApplication()" style="margin-left: 10px;"></button>
    </div>
  </div>
  {{/ifCond}}
  <table class="table table-condensed">
    <tr>
      <th style="white-space: nowrap;">
        Application ID:
      </th>
      <td>
        {{application.id}}
      </td>
    </tr>
    <tr>
      <th style="white-space: nowrap;">
        Application Name:
      </th>
      <td>
        {{#if application.bEdit}}
        <input type="text" class="form-control" c-model="application.name">
        {{else}}
        {{application.name}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th style="white-space: nowrap;">
        Creation Date:
      </th>
      <td>
        {{application.creation_date}}
      </td>
    </tr>
    {{#ifCond application.bEdit '||' application.retirement_date}}
    <tr>
      <th style="white-space: nowrap;">
        Retirement Date:
      </th>
      <td>
        {{#if application.bEdit}}
        <input type="text" class="form-control" c-model="application.retirement_date" placeholder="YYYY-MM-DD">
        {{else}}
        {{application.retirement_date}}
        {{/if}}
      </td>
    </tr>
    {{/ifCond}}
    {{ifCond application.bEdit '||' application.notify_priority_id}}
    <tr>
      <th style="white-space: nowrap;">
        Notify Priority:
      </th>
      <td>
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.notify_priority" ng-options="notify_priority.priority for notify_priority in store.notify_priorities">{{#each notify_priorities}}<option value="{{priority}}">{{.}}</option>{{/each}}</select>
        {{else}}
        {{application.notify_priority.priority}}
        {{/if}}
      </td>
    </tr>
    {{/ifCond}}
    {{#ifCond application.bEdit '||' application.website}}
    <tr>
      <th>
        Website:
      </th>
      <td>
        {{#if application.bEdit}}
        <input type="text" class="form-control" c-model="application.website">
        {{else}}
        <a href="{{application.website}}" target="_blank">{{application.website}}</a>
        {{/if}}
      </td>
    </tr>
    {{/ifCond}}
    {{#ifCond application.bedit '||' application.login_type_id}}
    <tr>
      <th style="white-space: nowrap;">
        Login Type:
      </th>
      <td style="white-space: nowrap;">
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.login_type">{{#each login_types}}<option value="{{.}}">{{type}}</option>{{/each}}</select>
        <div class="form-inline">
          <div class="input-group"><span class="input-group-text">Secure</span><select class="form-control" c-model="application.secure_port">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select></div>
          <div class="input-group"><span class="input-group-text">Auto-Register</span><select class="form-control" c-model="application.auto_register">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select></div>
          <div class="input-group"><span class="input-group-text">Account Check</span><select class="form-control" c-model="application.account_check">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select></div>
        </div>
        {{else}}
        {{application.login_type.type}}
        <br>
        Secure: {{application.secure_port.name}}
        <br>
        Auto-Register: {{application.auto_register.name}}
        <br>
        Account Check: {{application.account_check.name}}
        {{/if}}
      </td>
    </tr>
    {{/ifCond}}
    {{#ifCond application.bEdit '||' application.package_type_id}}
    <tr>
      <th style="white-space: nowrap;">
        Package Type:
      </th>
      <td>
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.package_type">{{#each package_types}}<option value="{{.}}">{{type}}</option>{{/each}}</select>
        {{else}}
        {{application.package_type.type}}
        {{/if}}
      </td>
    </tr>
    {{/ifCond}}
    <tr>
      <th>
        Dependable:
      </th>
      <td>
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.dependable">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select>
        {{else}}
        {{application.dependable.name}}
        {{/if}}
      </td>
    </tr>
    {{#ifCond application.bEdit '||' application.menu_id}}
    <tr>
      <th style="white-space: nowrap;">
        Menu Availability:
      </th>
      <td>
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.menu_access">{{#each menu_accesses}}<option value="{{.}}">{{type}}</option>{{/each}}</select>
        {{else}}
        {{application.menu_access.type}}
        {{/if}}
      </td>
    </tr>
    {{/ifCond}}
    {{#ifCond application.bEdit '||' (ifCond application.wiki.value '==' 1)}}
    <tr>
      <th>
        WIKI:
      </th>
      <td>
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.wiki">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select>
        {{else}}
        <a href="/wiki/index.php/{{urlEncode application.name}}" target="_blank">/wiki/index.php/{{application.name}}</a>
        {{/if}}
      </td>
    </tr>
    {{/ifCond}}
    <tr>
      <th>
        Description:
      </th>
      <td>
        {{#if application.bEdit}}
        <textarea class="form-control" c-model="application.description"></textarea>
        {{else}}
        <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{application.description}}</pre>
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
      <td>{{data.Owners}}</td>
      <td>{{numberShort data.NumberOfProcesses}}</td>
      <td>{{numberShort data.ImageSize}}</td>
      <td>{{numberShort data.ResidentSize}}</td>
      <td class="text-danger">{{data.Alarms}}</td>
    </tr>
    {{/each}}
  </table>
  {{/if}}
  {{/ifCond}}
  {{#ifCond application.forms.Accounts.active '&&' (ifCond isGlobalAdmin '||' application.bDeveloper)}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>User ID</th>
        <th>Encrypt</th>
        <th>Password</th>
        <th>Type</th>
        <th>Description</th>
        {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
        <th></th>
        {{/ifCond}}
      </tr>
      {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
      <tr>
        <td><input type="text" class="form-control" c-model="account.user_id" placeholder="User ID"></td>
        <td><select class="form-control" c-model="account.encrypt">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select></td>
        <td><input type="password" class="form-control" c-model="account.password" placeholder="Password"></td>
        <td><select class="form-control" c-model="account.type">{{#each account.types}}<option value="{{.}}">{{type}}</option>{{/each}}</select></td>
        <td><input type="text" class="form-control" c-model="account.description" placeholder="Description"></td>
        <td><button class="btn btn-xs btn-success" c-click="addAccount()">Add</button></td>
      </tr>
      {{/ifCond}}
      {{#each application.accounts}}
      <tr>
        <td>
          {{#if bEdit}}
          <input type="text" class="form-control" c-model="application.accounts.[{{@key}}].user_id" placeholder="User ID">
          {{else}}
          {{user_id}}
          {{/if}}
        </td>
        <td>
          {{#if bEdit}}
          <select class="form-control" c-model="application.accounts.[{{@key}}].encrypt">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select>
          {{else}}
          {{encrypt.name}}
          {{/if}}
        </td>
        <td>
          {{#if bEdit}}
          <input type="text" class="form-control" c-model="application.accounts.[{{@key}}].password" placeholder="Password">
          {{else}}
          {{password}}
          {{/if}}
        </td>
        <td>
          {{#if bEdit}}
          <select class="form-control" c-model="application.accounts.[{{@key}}].type">{{#each account.types}}<option value="{{.}}">{{type}}</option>{{/each}}</select>
          {{else}}
          {{type.type}}
          {{/if}}
        </td>
        <td>
          {{#if bEdit}}
          <textarea class="form-control" c-model="application.accounts.[{{@key}}].description" placeholder="Description"></textarea>
          {{else if description}}
          <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{description}}</pre>
          {{/if}}
        </td>
        {{#ifCond isGlobalAdmin '|| application.bDeveloper}}
        <td style="white-space: nowrap;">
          {{#if bEdit}}
          <button class="btn btn-xs btn-warning" c-click="preEditAccount({{.}}, false)">Cancel</button><button class="btn btn-xs btn-success" c-click="editAccount({{account}})" style="margin-left: 10px;">Save</button>
          {{else}}
          <button class="btn btn-xs btn-warning" ng-click="preEditAccount({{.}}, true)"></button><button class="btn btn-xs btn-danger" c-click="removeAccount({{id}})" style="margin-left: 10px;">Remove</button>
          {{/if}}
        </td>
        {{/ifCond}}
      </tr>
      {{/each}}
    </table>
  </div>
  {{/ifCond}}
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
        {{#ifCond (isLocalAdmin application.name) '||' application.bDeveloper}}
        <th></th>
        {{/ifCond}}
      </tr>
      {{#ifCond (isLocalAdmin application.name) '||' application.bDeveloper}}
      <tr>
        <td><input type="text" class="form-control" c-model="contact.[contactType.type].userid" placeholder="User ID"></td>
        <td><select class="form-control" ng-model="contact.[contactType.type].type">{{#each contactTypeOrder}}<option value="{{.}}">{{type}}</option>{{/each}}</select></td>
        <td><select class="form-control" ng-model="contact.[contactType.type].admin">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select></td>
        <td><select class="form-control" ng-model="contact.[contactType.type].locked">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select></td>
        <td><select class="form-control" ng-model="contact.[contactType.type].notify">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select></td>
        <td><input type="text" class="form-control" ng-model="contact.[contactType.type].description" placeholder="Description"></td>
        <td><button class="btn btn-xs btn-success" c-click="addContact({{contactType.type}})">Add</button></td>
      </tr>
      {{/ifCond}}
      {{#each application.contacts}}
      <tr>
        <td style="white-space:nowrap;">
          {{#if bEdit}}
          <input type="text" class="form-control" c-model="application.contacts.[{{@key}}].userid" placeholder="User ID">
          {{else}}
          <a href="#/Users/{{user_id}}">{{last_name}}, {{first_name}}</a> <small>({{userid}})</small>
          {{/if}}
        </td>
        <td style="white-space:nowrap;">
          {{#if bEdit}}
          <select class="form-control" c-model="application.contacts.[{{@key}}].type">{{#each contactTypeOrder}}<option value="{{.}}">{{type}}</option>{{/each}}</select>
          {{else}}
          {{type.type}}
          {{/if}}
        </td>
        <td>
          {{#if bEdit}}
          <select class="form-control" c-model="application.contacts.[{{@key}}].admin">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select>
          {{else}}
          {{admin.name}}
          {{/if}}
        </td>
        <td>
          {{#if bEdit}}
          <select class="form-control" c-model="application.contacts.[{{@key}}].locked">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select>
          {{else}}
          {{locked.name}}
          {{/if}}
        </td>
        <td>
          {{#if bEdit}}
          <select class="form-control" c-model="application.contacts.[{{@key}}].notify">{{#each a.m_noyes}}<option value="{{.}}">{{name}}</option>{{/each}}</select>
          {{else}}
          {{notify.name}}
          {{/if}}
        </td>
        <td>
          {{#if bEdit}}
          <input type="text" class="form-control" c-model="application.contacts.[{{@key}}].description" placeholder="Description">
          {{else if description}}
          <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{description}}</pre>
          {{/if}}
        </td>
        <td ng-show="common.isLocalAdmin() || store.application.bDeveloper" style="white-space: nowrap;">
          {{#if bEdit}}
          <button class="btn btn-xs btn-warning" c-click="preEditContact({{.}}, false)">Cancel</button><button class="btn btn-xs btn-success" c-click="editContact({{.}})" style="margin-left: 10px;">Save</button>
          {{else}}
          <button class="btn btn-xs btn-warning" c-click="preEditContact({{.}}, true)">Edit</button><button class="btn btn-xs btn-danger" c-click="removeContact({{id}})" style="margin-left: 10px;"></button>
          {{/if}}
        </td>
      </tr>
      {{/each}}
    </table>
  </div>
  {{/if}}
  {{#if application.forms.Depend.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Application</th>
        {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
        <th></th>
        {{/ifCond}}
      </tr>
      {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
      <tr>
        <td><select class="form-control" c-model="depend">{{#each dependApplications}}<option value="{{.}}">{{name}}</option>{{/each}}</select></td>
        <td><button class="btn btn-xs btn-success" c-click="addDepend()">Add</button></td>
      </tr>
      {{/ifCond}}
      {{#each application.depends}}
      <tr>
        <td><a href="#/Applications/{{application_id}}">{{name}}</a></td>
        {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
        <td><button class="btn btn-xs btn-danger" c-click="removeDepend({{id}})">Remove</button></td>
        {{/ifCond}}
      </tr>
      {{/each}}
    </table>
    <h4 class="page-header">Depend Upon {{application.name}}</h4>
    <table class="table table-condensed table-striped">
      <tr>
        <th>Application</th>
      </tr>
      {{#each application.dependents}}
      <tr>
        <td><a href="#/Applications/{{application_id}}">{{name}}</a></td>
      </tr>
      {{/each}}
    </table>
  </div>
  {{/if}}
  {{#if application.forms.Issues.active}}
  <div class="table-responsive">
    {{#if issue}}
    {{#if application.issue}}
    {{#ifCond isValid '&&' application.issue.close_date}}
    <button class="btn btn-sm btn-success float-end" c-click="editIssue(true)">Open</button>
    {{/ifCond}}
    <div class="row">
      <div class="col-md-3">
        <table class="table table-condensed card card-body card-inverse">
          <tr><th style="white-space: nowrap;">Issue #</th><td style="white-space: nowrap;">{{application.issue.id}}</td></tr>
          <tr ng-show="store.application.issue.open_date"><th>Open</th><td style="white-space: nowrap;">{{store.application.issue.open_date}}</td></tr>
          <tr ng-show="store.application.issue.close_date"><th>Close</th><td style="white-space: nowrap;">{{store.application.issue.close_date}}</td></tr>
          {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
          {{^if application.issue.close_date}}
          <tr><th style="white-space: nowrap;">On Hold</th><td><select c-model="application.issue.hold" class="form-control"><option value="0">No</option><option style="background: green; color: white;" value="1">Yes</option></select></td></tr>
          <tr><th>Priority</th><td><select c-model="application.issue.priority" class="form-control"><option value="1">Low</option><option style="color: orange;" value="2">Medium</option><option style="color: red;" value="3">High</option><option style="background: red; color: white;" value="4">Critical</option></select></td></tr>
          <tr><th>Due</th><td><input type="text" class="form-control" c-model="application.issue.due_date" placeholder="YYYY-MM-DD"></td></tr>
          <tr><th>Release</th><td><input type="text" class="form-control" c-model="application.issue.release_date" placeholder="YYYY-MM-DD"></td></tr>
          <tr><th>Assigned</th><td><input type="text" class="form-control" c-model="application.issue.assigned_userid" placeholder="User ID"></td></tr>
          <tr><th>Transfer</th><td><select class="form-control" c-model="application.issue.transfer">{{#each applications}}<option value="{{.}}">{{name}}</option>{{/each}}</select></td></tr>
          {{/if}}
          {{else}}
          {{#ifCond application.issue.hold '==' 1}}
          <tr><th></th><td style="margin-left: 10px; background: green; color: white; white-space: nowrap;">HOLD</td></tr>
          {{/ifCond}}
          {{#ifCond application.issue.priority '>=' 1}}
          <tr><th>Priority</th><td>{{#ifCond application.issue.priority '==' 1}}Low{{else ifCond application.issue.priority '==' 2}}<span style="color: orange;">Medium</span>{{else ifCond application.issue.priority '==' 3}}<span style="color: red;">High</span>{{else}}<span style="padding: 0px 2px; background: red; color: white;">Critical</span>{{/ifCond}}</td></tr>
          {{/ifCond}}
          {{#if application.issue.due_date}}
          <tr><th>Due</th><td style="white-space: nowrap;">{{application.issue.due_date}}</td></tr>
          {{/if}}
          {{#if application.issue.release_date}}
          <tr><th>Release</th><td style="white-space: nowrap;">{{application.issue.release_date}}</td></tr>
          {{/if}}
          {{#if application.issue.assigned}}
          <tr><th>Assigned</th><td style="white-space: nowrap;"><a href="#/Users/{{application.issue.assigned.id}}">{{application.issue.assigned.last_name}}, {{application.issue.assigned.first_name}}</a> <small>({{application.issue.assigned.userid}})</small></td></tr>
          {{/if}}
          {{/ifCond}}
        </table>
        {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
        <button class="btn btn-warning float-end" c-click="editIssue()">Save</button>
        {{/ifCond}}
      </div>
      <div class="col-md-9">
        {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
        <input type="text" class="form-control" c-model="application.issue.summary" placeholder="enter summary" style="width: 100%; font-weight: bold;">
        {{else}}
        <p style="font-weight: bold;">{{application.issue.summary}}</p>
        {{/ifCond}}
        <table class="table table-condensed table-striped" style="margin-top: 10px;">
          {{#each application.issue.comments}}
          <tr>
            <td>
              <table class="table table-condensed" style="background: inherit;">
                <tr><td style="white-space: nowrap;">{{entry_date}}</td></tr>
                <tr><td style="white-space: nowrap;"><a href="#/Users/{{user_id}}">{{last_name}}, {{first_name}}</a> <small>({{userid}})</small></td></tr>
                {{#ifCond !bEdit '&&' (ifCond userid '==' getUserID)}}
                <tr><td><button class="btn btn-sm btn-default float-end" c-click="preEditIssueComment({{.}}, true)"></button></td></tr>
                {{/ifCond}}
              </table>
            </td>
            <td>
              {{#ifCond bEdit '&&' (ifCond userid '==' getUserID)}}
              <textarea c-model="comments" class="form-control" rows="5" style="width: 100%;" placeholder="enter comments">{{comments}}</textarea>
              <button class="btn btn-sm btn-default float-end" c-click="editIssueComment({{comment}})" style="margin: 10px 0px 0px 10px;">Save</button>
              <button  class="btn btn-sm btn-default float-end" c-click="preEditIssueComment({{.}}, false)" style="margin: 10px 0px 0px 0px;">Cancel</button>
              {{else}}
              <pre  style="background: inherit; color: inherit; white-space: pre-wrap;">{{comments}}</pre>
              {{/ifCond}}
            </td>
          </tr>
          {{/each}}
          {{#if isValid}}
          <tr>
            <td></td>
            <td>
              <textarea c-model="issue.comments" class="form-control" rows="5" style="width: 100%;" placeholder="enter comments"></textarea>
              <button class="btn btn-sm btn-default float-end" c-click="addIssueComment('close', {{application.issue.id}}, {{application.id}})" style="margin: 10px 0px 0px 10px;">Close Issue</button>
              <button class="btn btn-sm btn-default float-end" c-click="addIssueComment('update', {{application.issue.id}}, {{application.id}})" style="margin: 10px 0px 0px 0px;">Add Comments</button>
            </td>
          </tr>
          {{/if}}
        </table>
      </div>
    </div>
    {{else}}
    <table class="table table-condensed table-striped">
      {{#if isValid}}
      <tr">
        <td style="width:25%;">
          <table class="table table-condensed" style="background: inherit;">
            <tr><th>Due</th><td><input type="text" c-model="issue.due_date" placeholder="YYYY-MM-DD"></td></tr>
            <tr><th>Priority</th><td><select c-model="issue.priority" class="form-control"><option value="1">Low</option><option style="color: orange;" value="2">Medium</option><option style="color: red;" value="3">High</option><option style="background: red; color: white;" value="4">Critical</option></select></td></tr>
            <tr><th>Assigned</th><td><input type="text" c-model="issue.assigned_userid" placeholder="User ID"></td></tr>
          </table>
        </td>
        <td style="width:75%;">
          <input type="text" class="form-control" c-model="issue.summary" placeholder="enter summary" style="width: 100%;">
          <br>
          <textarea c-model="issue.comments" class="form-control" rows="5" style="width: 100%;" placeholder="enter comments"></textarea>
          <button class="btn btn-sm btn-default float-end" c-click="addIssue()" style="margin-top: 10px;">Add Issue</button>
        </td>
      </tr>
      {{/if}}
    </table>
    <button class="btn btn-sm btn-default float-end" c-click="toggleClosedIssues()">{{#ifCond onlyOpenIssues '==' 0}}Hide{{else}}Show{{/ifCond}} Closed Issues</button>
    <table class="table table-condensed table-striped">
      {{#each application.issues}}
      <tr>
        <td>
          <table class="table table-condensed" style="background: inherit;">
            <tr><th style="white-space: nowrap;">Issue #</th><td><a href="#/Applications/{{application.id}}/Issues/{{id}}">{{id}}</a>{{#ifCond hold '==' 1}}<span style="margin-left: 20px; padding: 0px 2px; background: green; color: white;">HOLD</span>{{/ifCond}}</td></tr>
            {{#if open_date}}
            <tr><th>Open</th><td style="white-space: nowrap;">{{open_date}}</td></tr>
            {{/if}}
            {{#if due_date}}
            <tr><th>Due</th><td style="white-space: nowrap;">{{due_date}}</td></tr>
            {{/if}}
            {{#if release_date}}
            <tr><th>Release</th><td style="white-space: nowrap;">{{release_date}}</td></tr>
            {{/if}}
            {{#if close_date}}
            <tr><th>Close</th><td style="white-space: nowrap;">{{close_date}}</td></tr>
            {{/if}}
            {{#ifCond priority '>=' 1}}
            <tr><th>Priority</th>{{#ifCond priority '==' 1}}<td>Low</td>{{else ifCond priority '==' 2}}<td style="color: orange;">Medium</td>{{else ifCond priority '==' 3}}<td style="color: red;">High</td>{{else ifCond priority '>' 3}}<td style="color: white;"><span style="padding: 0px 2px; background: red;">Critical</span></td>{{/ifCond}}</tr>
            {{/ifCond}}
            {{#if comments}}
            <tr><td colspan="2"><a href="#/Users/{{comments[0].user_id}}">{{comments[0].last_name}}, {{comments[0].first_name}}</a> <small>({{comments[0].userid}})</small></td></tr>
            {{/if}}
            {{#if assigned}}
            <tr><td colspan="2"><a href="#/Users/{{assigned.id}}">{{assigned.last_name}}, {{assigned.first_name}}</a> <small>({{assigned.userid}})</small></td></tr>
            {{/if}}
          </table>
        </td>
        <td>
          {{#if summary}}
          <p style="font-weight: bold;">{{summary}}</p>
          {{/if}}
          {{#if comments}}
          <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{comments[0].comments}}</pre>
          {{/if}}
        </td>
      </tr>
      {{/each}}
    </table>
    {{/if}}
  </div>
  {{/if}}
  {{#ifCond application.forms.Notify.active '&&' (ifCond isGlobalAdmin '||' application.bDeveloper)}}
  <div class="table-responsive">
    <textarea c-model="notification" class="form-control" placeholder="enter notification" rows="5"></textarea>
    <button class="btn btn-sm btn-default float-end" c-click="sendNotification()">Send Notification</button>
    <div ng-show="contacts">
      <h4 class="page-header">Sent Successfully</h4>
      <ul>
        {{#each contacts}}
        {{#if sent}}
        <li><a href="#/Users/?userid={{userid}}">{{name}}</a> <small>({{userid}})</small></li>
        {{/if}}
        {{/each}}
      </ul>
      <h4 class="page-header">Sent Unsuccessfully</h4>
      <ul>
        {{#each contacts}}
        {{^if sent}}
        <li><a href="#/Users/?userid={{userid}}">{{name}}</a> <small>({{userid}})</small></li>
        {{/if}}
        {{/each}}
      </ul>
    </div>
  </div>
  {{/ifCond}}
  {{#if application.forms.Servers.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th style="width: 100%;">Server</th>
        {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
        <th colspan="2"></th>
        {{/ifCond}}
      </tr>
      {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
      <tr>
        <td><select class="form-control" c-model="server">{{#each servers}}<option value="{{.}}">{{name}}</option>{{/each}}</select></td>
        <td><button class="btn btn-xs btn-success" c-click="addServer()">Add</button></td>
      </tr>
      {{/ifCond}}
      {{#each application.servers}}
      <tr>
        <td><a href="#/Servers/{{server_id}}">{{name}}</a></td>
        {{#ifCond isGlobalAdimin '||' application.bDeveloper}}
        <td style="white-space: nowrap;"><button class="btn btn-xs btn-warning" data-toggle="modal" data-target="#serverModal" c-click="serverDetails({{.}})">Edit</button><button class="btn btn-xs btn-danger" c-click="removeServer({{id}})">Remove</button></td>
        {{/ifCond}}
      </tr>
      {{/each}}
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
                {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
                <th colspan="2"></th>
                {{/ifCond}}
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
                {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
                <th colspan="2"></th>
                {{/ifCond}}
              </tr>
              {{#each modalServer.details}}
              <tr>
                {{#if bEdit}}
                  <td><input type="text" class="form-control" ng-model="daemon"></td>
                  <td><input type="text" class="form-control" ng-model="version"></td>
                  <td><input type="text" class="form-control" ng-model="owner"></td>
                  <td><input type="text" class="form-control" ng-model="script"></td>
                  <td><input type="text" class="form-control" ng-model="delay"></td>
                  <td><input type="text" class="form-control" ng-model="min_processes"></td>
                  <td><input type="text" class="form-control" ng-model="max_processes"></td>
                  <td><input type="text" class="form-control" ng-model="min_image"></td>
                  <td><input type="text" class="form-control" ng-model="max_image"></td>
                  <td><input type="text" class="form-control" ng-model="min_resident"></td>
                  <td><input type="text" class="form-control" ng-model="max_resident"></td>
                  <td><button class="btn btn-xs btn-warning" c-click="preEditServerDetail({{.}}, false)">Cancel</button></td>
                  <td><button class="btn btn-xs btn-success" ng-click="editServerDetail({{.}})">Save</button></td>
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
                  <td><button class="btn btn-xs btn-warning" c-click="preEditServerDetail({{.}}, true)">Edit</button></td>
                  <td><button class="btn btn-xs btn-danger" c-click="removeServerDetail({{id}})">Remove</button></td>
                {{/if}}
              </tr>
              {{/each}}
              {{#ifCond isGlobalAdmin '||' application.bDeveloper}}
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
              {{/ifCond}}
            </table>
          </div>
        </div>
      </div>
    </div>
  </div>
  {{/if}}
  {{/if}}
  `
  // }}}
}
