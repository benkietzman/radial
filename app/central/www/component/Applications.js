// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-17
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
    let s = c.scope('Applications',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Applications');
      },
      // ]]]
      a: a,
      c: c,
      d: {},
      bNotified: false,
      issue: {priority: '1'},
      issueList: true,
      onlyOpenIssues: 1,
      serverDetail: {delay: 0, min_processes: 0, max_processes: 0, min_image: 0, max_image: 0, min_resident: 0, max_resident: 0}
    });
    // ]]]
    // [[[ addAccount()
    s.addAccount = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationAccountAdd', Request: c.simplify(s.account)};
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
    // ]]]
    // [[[ addApplication()
    s.addApplication = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationAdd', Request: c.simplify(s.d.application)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          document.location.href = '#/Applications/?application=' + encodeURIComponent(s.d.application.name.v);
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ addContact()
    s.addContact = () =>
    {
      s.contact.application_id = s.application.id;
      let request = {Interface: 'central', 'Function': 'applicationUserAdd', Request: c.simplify(s.contact)};
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
    // ]]]
    // [[[ addDepend()
    s.addDepend = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationDependAdd', Request: {application_id: s.application.id, dependant_id: s.depend.v.id}};
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
    // ]]]
    // [[[ addIssue()
    s.addIssue = () =>
    {
      s.info.v = 'Adding issue...';
      let request = {Interface: 'central', 'Function': 'applicationIssueAdd', Request: {application_id: s.application.id, application_name: s.application.name, summary: s.issue.summary.v, due_date: s.issue.due_date.v, priority: s.issue.priority.v, assigned_userid: s.issue.assigned_userid.v, comments: s.issue.comments.v, server: location.host}};
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
    // ]]]
    // [[[ addIssueComment()
    s.addIssueComment = (strAction, nIssueID, nApplicationID) =>
    {
      s.info.v = 'Adding comment...';
      let request = {Interface: 'central', 'Function': 'applicationIssueCommentAdd', Request: {issue_id: nIssueID, comments: s.issue.comments.v, action: strAction, application_id: nApplicationID, server: location.host}};
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
          document.location.href = '#/Applications/' + nApplicationID + '/Issues/' + nIssueID;
          s.showForm('Issues');
        }
        else
        {
          s.info.v = null;
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ addServer()
    s.addServer = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationServerAdd', Request: {application_id: s.application.id, server_id: s.server.v.id}};
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
    // ]]]
    // [[[ addServerDetail()
    s.addServerDetail = () =>
    {
      s.modalServerInfo.v = 'Adding server detail...';
      let request = {Interface: 'central', 'Function': 'applicationServerDetailAdd', Request: c.simplify(s.serverDetail)};
      request.Request.application_server_id = s.modalServer.id;
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.modalServerInfo.v = null;
        if (c.wsResponse(response, error))
        {
          s.serverDetails(s.modalServer.id);
          s.sysInfoUpdate();
        }
        else
        {
          s.modalServerMessage.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ editAccount()
    s.editAccount = (nIndex) =>
    {
      let request = {Interface: 'central', 'Function': 'applicationAccountEdit', Request: c.simplify(s.application.accounts[nIndex])};
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
    // ]]]
    // [[[ editApplication()
    s.editApplication = () =>
    {
      let request = {Interface: 'central', 'Function': 'applicationEdit', Request: c.simplify(s.application)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          s.application = null;
          s.loadApplication();
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
      let request = {Interface: 'central', 'Function': 'applicationUserEdit', Request: c.simplify(s.application.contacts[nIndex])};
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
    // ]]]
    // [[[ editIssue()
    s.editIssue = (bOpen) =>
    {
      if (c.isDefined(bOpen) && bOpen)
      {
        s.application.issue.close_date = null;
      }
      let request = {Interface: 'central', 'Function': 'applicationIssueEdit', Request: c.simplify(s.application.issue)};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          if (c.isDefined(s.application.issue.transfer) && c.isDefined(s.application.issue.transfer.v) && c.isDefined(s.application.issue.transfer.v.id) && s.application.issue.transfer.v.id != s.application.id)
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
                document.location.href = '/central/#/Applications/?application=' + encodeURIComponent(s.application.issue.transfer.v.name) + '&form=Issues&issue_id=' + s.application.issue.id;
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
    // ]]]
    // [[[ editIssueComment()
    s.editIssueComment = (nIndex) =>
    {
      s.info.v = 'Updating issue...';
      let request = {Interface: 'central', 'Function': 'applicationIssueCommentEdit', Request: {id: comment.id, comments: s.application.issue.comments[nIndex].comments.v}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          s.application.issue.comments[nIndex].bEdit = false;
          s.u();
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ editServerDetail()
    s.editServerDetail = (nIndex) =>
    {
      s.modalServerInfo.v = 'Updating server detail...';
      let request = {Interface: 'central', 'Function': 'applicationServerDetailEdit', Request: c.simplify(s.modalServer.details[nIndex])};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.modalServerInfo.v = null;
        if (c.wsResponse(response, error))
        {
          s.serverDetails(s.modalServer.id);
          s.sysInfoUpdate();
        }
        else
        {
          s.modalServerMessage.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ initForms()
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
    // ]]]
    // [[[ loadApplication()
    s.loadApplication = () =>
    {
      s.preLoad();
      if (c.isParam(nav, 'id') || c.isParam(nav, 'application'))
      {
        let strForm = ((c.isParam(nav, 'form'))?c.getParam(nav, 'form'):'General');
        if (c.isDefined(s.application) && s.application != null && (c.getParam(nav, 'id') == s.application.id || c.getParam(nav, 'application') == s.application.name))
        {
          s.showForm(strForm);
        }
        else
        {
          s.info.v = 'Retrieving application...';
          s.application = null;
          let request = {Interface: 'central', 'Function': 'application', Request: {form: strForm}};
          if (c.isParam(nav, 'id'))
          {
            request.Request.id = c.getParam(nav, 'id');
          }
          else
          {
            request.Request.name = c.getParam(nav, 'application');
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
              s.application.bLocalAdmin = c.isLocalAdmin(s.application.name);
              s.initForms();
              s.showForm(strForm);
              if (s.application.bDeveloper)
              {
                s.application.forms.Accounts = {value: 'Accounts', active: null};
                s.application.forms_order.splice(1, 0, 'Accounts');
                s.application.forms.Notify = {value: 'Notify', active: null};
                s.application.forms_order.splice(5, 0, 'Notify');
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
                    s.application.bLocalAdmin = true;
                    s.application.forms.Accounts = {value: 'Accounts', active: null};
                    s.application.forms_order.splice(1, 0, 'Accounts');
                    s.application.forms.Notify = {value: 'Notify', active: null};
                    s.application.forms_order.splice(5, 0, 'Notify');
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
    // ]]]
    // [[[ loadApplications()
    s.loadApplications = () =>
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
        s.applications = null;
        s.applications = [];
        s.u();
        s.info.v = 'Retrieving applications...';
        let request = {Interface: 'central', 'Function': 'applications', Request: {}};
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
            s.applications = response.Response;
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
                  s.u();
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
        s.loadApplication();
      }
    };
    // ]]]
    // [[[ preEditAccount()
    s.preEditAccount = (nIndex, bEdit) =>
    {
      s.application.accounts[nIndex].bEdit = bEdit;
      if (!bEdit)
      {
        s.application.accounts[nIndex] = c.simplify(s.application.accounts[nIndex]);
      }
      s.u();
    };
    // ]]]
    // [[[ preEditApplication()
    s.preEditApplication = (bEdit) =>
    {
      s.application.bEdit = bEdit;
      if (!bEdit)
      {
        s.application = c.simplify(s.application);
      }
      s.u();
    };
    // ]]]
    // [[[ preEditContact()
    s.preEditContact = (nIndex, bEdit) =>
    {
      s.application.contacts[nIndex].bEdit = bEdit;
      if (!bEdit)
      {
        s.application.contacts[nIndex] = c.simplify(s.application.contacts[nIndex]);
      }
      s.u();
    };
    // ]]]
    // [[[ preEditIssueComment()
    s.preEditIssueComment = (nIndex, bEdit) =>
    {
      s.application.issue.comments[nIndex].bEdit = bEdit;
      if (!bEdit)
      {
        s.application.issue.comments[nIndex] = c.simplify(s.application.issue.comments[nIndex]);
      }
      s.u();
    };
    // ]]]
    // [[[ preEditServerDetail()
    s.preEditServerDetail = (nIndex, bEdit) =>
    {
      s.modalServer.details[nIndex].bEdit = bEdit;
      if (!bEdit)
      {
        s.modalServer.details[nIndex] = c.simplify(s.modalServer.details[nIndex]);
      }
      s.u();
      let e = document.querySelector('div.modal-backdrop');
      e.parentNode.removeChild(e);
      document.querySelector('body').style.overflow = 'auto';
      let modal = new bootstrap.Modal(document.getElementById('serverModal'));
      modal.show();
    };
    // ]]]
    // [[[ preLoad()
    s.preLoad = () =>
    {
      // [[[ get contact types
      if (!c.isDefined(s.contact_types))
      {
        s.contact_types = [{type: 'Primary Developer'}, {type: 'Backup Developer'}, {type: 'Primary Contact'}, {type: 'Contact'}];
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
              s.message.v = error.message;
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
            s.message.v = error.message;
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
            s.message.v = error.message;
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
            s.message.v = error.message;
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
            s.message.v = error.message;
          }
        });
      }
      // ]]]
    };
    // ]]]
    // [[[ removeAccount()
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
    // ]]]
    // [[[ removeApplication()
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
    // ]]]
    // [[[ removeContact()
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
    // ]]]
    // [[[ removeDepend()
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
    // ]]]
    // [[[ removeServer()
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
    // ]]]
    // [[[ removeServerDetail()
    s.removeServerDetail = (nID) =>
    {
      if (confirm('Are you sure you want to remove this application server detail?'))
      {
        s.modalServerInfo.v = 'Removing server detail...';
        let request = {Interface: 'central', 'Function': 'applicationServerDetailRemove', Request: {id: nID}};
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.modalServerInfo.v = null;
          if (c.wsResponse(response, error))
          {
            s.serverDetails(s.modalServer.id);
            s.sysInfoUpdate();
          }
          else
          {
            s.modalServerMessage.v = error.message;
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
      if (confirm('Are you sure you want to send this application notification?'))
      {
        let request = {Interface: 'central', 'Function': 'applicationNotify', Request: {id: s.application.id, notification: s.notification.v, server: location.host}};
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
    // [[[ serverDetails()
    s.serverDetails = (nID) =>
    {
      s.modalServer = null;
      for (let i = 0; i < s.application.servers.length; i++)
      {
        if (s.application.servers[i].id == nID)
        {
          s.modalServer = s.application.servers[i];
        }
      }
      s.modalServerInfo.v = 'Retrieving server details...';
      let request = {Interface: 'central', 'Function': 'applicationServerDetails', Request: {application_server_id: nID}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.modalServerInfo.v = null;
        if (c.wsResponse(response, error))
        {
          s.modalServer.details = response.Response;
          s.u();
          let e = document.querySelector('div.modal-backdrop');
          e.parentNode.removeChild(e);
          document.querySelector('body').style.overflow = 'auto';
          let modal = new bootstrap.Modal(document.getElementById('serverModal'));
          modal.show();
        }
        else
        {
          s.modalServerMessage.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ showForm()
    s.showForm = (strForm) =>
    {
      s.info.v = null;
      s.initForms();
      if (!c.isDefined(s.application.forms[strForm]))
      {
        strForm = 'General';
      }
      for (let key of Object.keys(s.application.forms))
      {
        s.application.forms[key].active = null;
      }
      s.application.forms[strForm].active = 'active';
      // [[[ General
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
      // ]]]
      // [[[ Accounts
      else if (strForm == 'Accounts')
      {
        if (s.application.bDeveloper)
        {
          if (!c.isDefined(s.application.accounts) || s.application.accounts == null)
          {
            s.account = {application_id: s.application.id, encrypt: a.m_noyes[0]};
            s.application.accounts = null;
            s.application.accounts = [];
            s.u();
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
                    s.u();
                  }
                  else
                  {
                    s.message.v = error.message;
                  }
                });
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
          s.message.v = 'You are not authorized to view this data.';
        }
      }
      // ]]]
      // [[[ Contacts
      else if (strForm == 'Contacts')
      {
        if (!c.isDefined(s.application.contacts) || s.application.contacts == null)
        {
          s.contact = {application_id: s.application.id, type: s.contact_types[3], admin: a.m_noyes[0], locked: a.m_noyes[0], notify: a.m_noyes[1]};
          s.application.contacts = null;
          s.application.contacts = [];
          s.u();
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
      // [[[ Depend
      else if (strForm == 'Depend')
      {
        if (!c.isDefined(s.application.depends) || s.application.depends == null || !c.isDefined(s.application.dependents) || s.application.dependents == null)
        {
          s.info.v = 'Retrieving dependents...';
          let request = {Interface: 'central', 'Function': 'dependentsByApplicationID', Request: {application_id: s.application.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              s.application.depends = response.Response.depends;
              s.application.dependents = response.Response.dependents;
              if (s.application.bDeveloper)
              {
                let request = {Interface: 'central', 'Function': 'applications', Request: {dependable: 1}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    s.dependApplications = response.Response;
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
      }
      // ]]]
      // [[[ Issues
      else if (strForm == 'Issues')
      {
        if (c.isParam(nav, 'issue_id'))
        {
          if (s.issueList && c.isDefined(s.issue))
          {
            delete s.issue;
          }
          s.issueList = false;
          if (!c.isDefined(s.application.issue) || s.application.issue == null || c.getParam(nav, 'issue_id') != s.application.issue.id)
          {
            s.application.issue = null;
            s.info.v = 'Retrieving issue...';
            let request = {Interface: 'central', 'Function': 'applicationIssue', Request: {id: c.getParam(nav, 'issue_id'), comments: 1}};
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
                if (c.isValid() && s.application.issue.close_date == '')
                {
                  s.application.issue.bIsValidOpen = true;
                }
                if (s.application.bDeveloper)
                {
                  if (s.application.issue.close_date == '')
                  {
                    s.application.issue.bDeveloperOpen = true;
                  }
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
        }
        else
        {
          if (!s.issueList && c.isDefined(s.issue))
          {
            delete s.issue;
            s.issue = {priority: '1'};
          }
          s.issueList = true;
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
                s.u();
              }
              else
              {
                s.message.v = error.message;
              }
            });
          }
        }
        s.u();
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
        if (!c.isDefined(s.application.servers) || s.application.servers == null)
        {
          s.info.v = 'Retrieving servers...';
          let request = {Interface: 'central', 'Function': 'serversByApplicationID', Request: {application_id: s.application.id}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              s.application.servers = response.Response;
              if (s.application.bDeveloper)
              {
                let request = {Interface: 'central', 'Function': 'servers', Request: {}};
                c.wsRequest('radial', request).then((response) =>
                {
                  let error = {};
                  if (c.wsResponse(response, error))
                  {
                    s.servers = response.Response;
                    s.server = s.servers[0];
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
      }
      // ]]]
      s.u();
    };
    // ]]]
    // [[[ sysInfoStatus()
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
                      s.u();
                    }
                    else
                    {
                      s.message.v = error.message;
                    }
                  });
                }
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
    c.setMenu('Applications');
    s.list = true;
    if (c.isParam(nav, 'id') || c.isParam(nav, 'application'))
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
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <!-- [[[ applications -->
  {{#if list}}
  <h3 class="page-header">Applications</h3>
  <div class="input-group float-end"><span class="input-group-text">Narrow</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow Results"></div>
  {{#each a.m_letters}}
  <div style="display: inline-block;">
    <a href="#/Applications/?letter={{urlEncode .}}">
      <button class="btn btn-sm btn-{{#ifCond . "==" @root.letter}}warning{{else}}primary{{/ifCond}}">{{.}}</button>
    </a>
  </div>
  {{/each}}
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Application</th><th>Primary Developers</th><th>Servers</th>
      </tr>
      {{#isValid "Central"}}
      <tr>
        <td><input type="text" class="form-control" c-model="d.application.name" placeholder="Application Name"></td>
        <td><button class="btn btn-primary" c-click="addApplication()">Add Application</button></td>
      </tr>
      {{/isValid}}
      {{#eachFilter applications "name" narrow}}
      <tr style="{{application.style}}">
        <td valign="top">
          <a href="#/Applications/{{id}}">{{name}}</a>
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
            {{#each servers}}
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
  <!-- [[[ application -->
  {{else}}
  <h3 class="page-header">{{application.name}}{{#if application.retirement_date}}<small class="text-danger"> --- RETIRED</small>{{/if}}</h3>
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
  <nav class="container navbar navbar-expand-lg navbar-dark bg-dark bg-gradient">
    <div class="container-fluid">
      <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#appnavigationbar" aria-controls="appnavigationbar" aria-expanded="false", aria-label="Toggle navigation">
        <span class="navbar-toggler-icon"></span>
      </button>
      <div class="collapse navbar-collapse" id="appnavigationbar">
        <ul class="navbar-nav me-auto mb-2 mb-lg-0">
          {{#each application.forms_order}}
          <li class="nav-item"><a class="nav-link {{#with (lookup @root.application.forms .)}}{{active}}{{/with}}" href="#/Applications/{{@root.application.id}}/{{.}}">{{.}}</a></li>
          {{/each}}
        </ul>
      </div>
    </div>
  </nav>
  <!-- [[[ general -->
  {{#if application.forms.General.active}}
  {{#if application.bDeveloper}}
  <div class="float-end">
    {{#if application.bEdit}}
    <div style="white-space: nowrap;">
      <button class="btn btn-xs btn-warning" c-click="preEditApplication(false)">Cancel</button>
      <button class="btn btn-xs btn-success" c-click="editApplication()" style="margin-left: 10px;">Save</button>
    </div>
    {{else}}
    <div style="white-space: nowrap;">
      <button class="btn btn-xs btn-warning" c-click="preEditApplication(true)">Edit</button>
      <button class="btn btn-xs btn-danger" c-click="removeApplication()" style="margin-left: 10px;">Remove</button>
    </div>
    {{/if}}
  </div>
  {{/if}}
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
    <tr>
      <th style="white-space: nowrap;">
        Notify Priority:
      </th>
      <td>
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.notify_priority" c-json>{{#each notify_priorities}}<option value="{{json .}}">{{priority}}</option>{{/each}}</select>
        {{else}}
        {{application.notify_priority.priority}}
        {{/if}}
      </td>
    </tr>
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
    <tr>
      <th style="white-space: nowrap;">
        Login Type:
      </th>
      <td style="white-space: nowrap;">
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.login_type" c-json>{{#each login_types}}<option value="{{json .}}">{{type}}</option>{{/each}}</select>
        <div class="form-inline">
          <div class="input-group"><span class="input-group-text">Secure</span><select class="form-control" c-model="application.secure_port" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
          <div class="input-group"><span class="input-group-text">Auto-Register</span><select class="form-control" c-model="application.auto_register" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
          <div class="input-group"><span class="input-group-text">Account Check</span><select class="form-control" c-model="application.account_check" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
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
    <tr>
      <th style="white-space: nowrap;">
        Package Type:
      </th>
      <td>
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.package_type" c-json>{{#each package_types}}<option value="{{json .}}">{{type}}</option>{{/each}}</select>
        {{else}}
        {{application.package_type.type}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th>
        Dependable:
      </th>
      <td>
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.dependable" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
        {{else}}
        {{application.dependable.name}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th style="white-space: nowrap;">
        Menu Availability:
      </th>
      <td>
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.menu_access" c-json>{{#each menu_accesses}}<option value="{{json .}}">{{type}}</option>{{/each}}</select>
        {{else}}
        {{application.menu_access.type}}
        {{/if}}
      </td>
    </tr>
    <tr>
      <th>
        WIKI:
      </th>
      <td>
        {{#if application.bEdit}}
        <select class="form-control" c-model="application.wiki" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
        {{else}}
        {{#ifCond application.wiki.value "==" 1}}
        <a href="/wiki/index.php/{{urlEncode @root.application.name}}" target="_blank">/wiki/index.php/{{@root.application.name}}</a>
        {{/ifCond}}
        {{/if}}
      </td>
    </tr>
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
  <!-- [[[ accounts -->
  {{#if application.forms.Accounts.active}}
  {{#if application.bDeveloper}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>User ID</th>
        <th>Encrypt</th>
        <th>Password</th>
        <th>Type</th>
        <th>Description</th>
        <th></th>
      </tr>
      <tr>
        <td><input type="text" class="form-control" c-model="account.user_id" placeholder="User ID"></td>
        <td><select class="form-control" c-model="account.encrypt" c-json>{{#each a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
        <td><input type="password" class="form-control" c-model="account.password" placeholder="Password"></td>
        <td><select class="form-control" c-model="account.type" c-json>{{#each account.types}}<option value="{{json .}}">{{type}}</option>{{/each}}</select></td>
        <td><input type="text" class="form-control" c-model="account.description" placeholder="Description"></td>
        <td><button class="btn btn-xs btn-success" c-click="addAccount()">Add</button></td>
      </tr>
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
          <select class="form-control" c-model="application.accounts.[{{@key}}].encrypt" c-json>{{#each @root.a.m_noyes}}<option value="{{json .}}">{{name}}</option>{{/each}}</select>
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
          <select class="form-control" c-model="application.accounts.[{{@key}}].type" c-json>{{#each @root.account.types}}<option value="{{json .}}">{{type}}</option>{{/each}}</select>
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
        <td style="white-space: nowrap;">
          {{#if bEdit}}
          <button class="btn btn-xs btn-warning" c-click="preEditAccount({{@key}}, false)">Cancel</button><button class="btn btn-xs btn-success" c-click="editAccount({{@key}})" style="margin-left: 10px;">Save</button>
          {{else}}
          <button class="btn btn-xs btn-warning" c-click="preEditAccount({{@key}}, true)">Edit</button><button class="btn btn-xs btn-danger" c-click="removeAccount({{id}})" style="margin-left: 10px;">Remove</button>
          {{/if}}
        </td>
      </tr>
      {{/each}}
    </table>
  </div>
  {{/if}}
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
  <!-- [[[ dependents -->
  {{#if application.forms.Depend.active}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Application</th>
        {{#if application.bDeveloper}}
        <th></th>
        {{/if}}
      </tr>
      {{#if application.bDeveloper}}
      <tr>
        <td><select class="form-control" c-model="depend" c-json>{{#each dependApplications}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td>
        <td><button class="btn btn-xs btn-success" c-click="addDepend()">Add</button></td>
      </tr>
      {{/if}}
      {{#each application.depends}}
      <tr>
        <td><a href="#/Applications/{{application_id}}">{{name}}</a></td>
        {{#if @root.application.bDeveloper}}
        <td><button class="btn btn-xs btn-danger" c-click="removeDepend({{id}})">Remove</button></td>
        {{/if}}
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
  <!-- ]]] -->
  <!-- [[[ issues -->
  {{#if application.forms.Issues.active}}
  <div class="table-responsive">
    <!-- [[[ issues -->
    {{#if issueList}}
    <table class="table table-condensed table-striped">
      {{#isValid}}
      <tr>
        <td style="width:25%;">
          <table class="table table-condensed" style="background: inherit;">
            <tr><th>Due</th><td><input class="form-control" type="text" c-model="issue.due_date" placeholder="YYYY-MM-DD"></td></tr>
            <tr><th>Priority</th><td><select c-model="issue.priority" class="form-control"><option value="1">Low</option><option style="color: orange;" value="2">Medium</option><option style="color: red;" value="3">High</option><option style="background: red; color: white;" value="4">Critical</option></select></td></tr>
            <tr><th>Assigned</th><td><input class="form-control" type="text" c-model="issue.assigned_userid" placeholder="User ID"></td></tr>
          </table>
        </td>
        <td style="width:75%;">
          <input type="text" class="form-control" c-model="issue.summary" placeholder="enter summary" style="width: 100%;" autofocus>
          <br>
          <textarea c-model="issue.comments" class="form-control" rows="5" style="width: 100%;" placeholder="enter comments"></textarea>
          <button class="btn btn-primary float-end" c-click="addIssue()" style="margin-top: 10px;">Add Issue</button>
        </td>
      </tr>
      {{/isValid}}
    </table>
    <button class="btn btn-primary float-end" c-click="toggleClosedIssues()">{{#ifCond onlyOpenIssues "==" 0}}Hide{{else}}Show{{/ifCond}} Closed Issues</button>
    <table class="table table-condensed table-striped">
      {{#each application.issues}}
      <tr>
        <td>
          <table class="table table-condensed" style="background: inherit;">
            <tr><th style="white-space: nowrap;">Issue #</th><td><a href="#/Applications/{{@root.application.id}}/Issues/{{id}}">{{id}}</a>{{#ifCond hold "==" 1}}<span style="margin-left: 20px; padding: 0px 2px; background: green; color: white;">HOLD</span>{{/ifCond}}</td></tr>
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
            {{#ifCond priority ">=" 1}}
            <tr><th>Priority</th>{{#ifCond priority "==" 1}}<td>Low</td>{{else ifCond priority "==" 2}}<td style="color: orange;">Medium</td>{{else ifCond priority "==" 3}}<td style="color: red;">High</td>{{else ifCond priority ">" 3}}<td style="color: white;"><span style="padding: 0px 2px; background: red;">Critical</span></td>{{/ifCond}}</tr>
            {{/ifCond}}
            {{#if comments}}
            <tr><td colspan="2"><a href="#/Users/{{comments.[0].user_id}}">{{comments.[0].last_name}}, {{comments.[0].first_name}}</a> <small>({{comments.[0].userid}})</small></td></tr>
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
          <pre style="background: inherit; color: inherit; white-space: pre-wrap;">{{comments.[0].comments}}</pre>
          {{/if}}
        </td>
      </tr>
      {{/each}}
    </table>
    <!-- ]]] -->
    <!-- [[[ issue -->
    {{else}}
    {{#if application.issue}}
    {{#if application.bDeveloper}}
    {{#if application.issue.close_date}}
    <button class="btn btn-sm btn-primary float-end" c-click="editIssue(true)">Reopen</button>
    {{/if}}
    {{/if}}
    <div class="row">
      <div class="col-md-4">
        <table class="table table-condensed card card-body card-inverse">
          <tr><th style="white-space: nowrap;">Issue #</th><td style="white-space: nowrap;">{{application.issue.id}}</td></tr>
          {{#if application.issue.open_date}}
          <tr><th>Open</th><td style="white-space: nowrap;">{{application.issue.open_date}}</td></tr>
          {{/if}}
          {{#if application.issue.close_date}}
          <tr><th>Close</th><td style="white-space: nowrap;">{{application.issue.close_date}}</td></tr>
          {{/if}}
          {{#if application.issue.bDeveloperOpen}}
          <tr><th style="white-space: nowrap;">On Hold</th><td><select c-model="application.issue.hold" class="form-control"><option value="0">No</option><option style="background: green; color: white;" value="1">Yes</option></select></td></tr>
          <tr><th>Priority</th><td><select c-model="application.issue.priority" class="form-control"><option value="1">Low</option><option style="color: orange;" value="2">Medium</option><option style="color: red;" value="3">High</option><option style="background: red; color: white;" value="4">Critical</option></select></td></tr>
          <tr><th>Due</th><td><input type="text" class="form-control" c-model="application.issue.due_date" placeholder="YYYY-MM-DD"></td></tr>
          <tr><th>Release</th><td><input type="text" class="form-control" c-model="application.issue.release_date" placeholder="YYYY-MM-DD"></td></tr>
          <tr><th>Assigned</th><td><input type="text" class="form-control" c-model="application.issue.assigned_userid" placeholder="User ID"></td></tr>
          <tr><th>Transfer</th><td><select class="form-control" c-model="application.issue.transfer" c-json>{{#each applications}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></td></tr>
          {{else}}
          {{#ifCond application.issue.hold "==" 1}}
          <tr><th></th><td style="margin-left: 10px; background: green; color: white; white-space: nowrap;">HOLD</td></tr>
          {{/ifCond}}
          {{#if application.issue.priority}}
          <tr><th>Priority</th><td>{{#ifCond @root.application.issue.priority "==" 1}}Low{{else ifCond @root.application.issue.priority "==" 2}}<span style="color: orange;">Medium</span>{{else ifCond @root.application.issue.priority "==" 3}}<span style="color: red;">High</span>{{else}}<span style="padding: 0px 2px; background: red; color: white;">Critical</span>{{/ifCond}}</td></tr>
          {{/if}}
          {{#if application.issue.due_date}}
          <tr><th>Due</th><td style="white-space: nowrap;">{{application.issue.due_date}}</td></tr>
          {{/if}}
          {{#if application.issue.release_date}}
          <tr><th>Release</th><td style="white-space: nowrap;">{{application.issue.release_date}}</td></tr>
          {{/if}}
          {{#if application.issue.assigned}}
          <tr><th>Assigned</th><td style="white-space: nowrap;"><a href="#/Users/{{application.issue.assigned.id}}">{{application.issue.assigned.last_name}}, {{application.issue.assigned.first_name}}</a> <small>({{application.issue.assigned.userid}})</small></td></tr>
          {{/if}}
          {{/if}}
        </table>
        {{#if application.issue.bDeveloperOpen}}
        <button class="btn btn-warning float-end" c-click="editIssue()">Save</button>
        {{/if}}
      </div>
      <div class="col-md-8">
        {{#if application.issue.bDeveloperOpen}}
        <input type="text" class="form-control" c-model="application.issue.summary" placeholder="enter summary" style="width: 100%; font-weight: bold;">
        {{else}}
        <p style="font-weight: bold;">{{application.issue.summary}}</p>
        {{/if}}
        <table class="table table-condensed table-striped" style="margin-top: 10px;">
          {{#each application.issue.comments}}
          <tr>
            <td>
              <table class="table table-condensed" style="background: inherit;">
                <tr><td style="white-space: nowrap;">{{entry_date}}</td></tr>
                <tr><td style="white-space: nowrap;"><a href="#/Users/{{user_id}}">{{last_name}}, {{first_name}}</a> <small>({{userid}})</small></td></tr>
                {{#if bEdit}}
                {{#ifCond userid "==" getUserID}}
                <tr><td><button class="btn btn-primary float-end" c-click="preEditIssueComment({{@key}}, true)">Edit</button></td></tr>
                {{/ifCond}}
                {{/if}}
              </table>
            </td>
            <td>
              {{#if bEdit}}
              {{#ifCond userid "==" getUserID}}
              <textarea c-model="comments" class="form-control" rows="5" style="width: 100%;" placeholder="enter comments">{{comments}}</textarea>
              <button class="btn btn-primary float-end" c-click="editIssueComment({{@key}})" style="margin: 10px 0px 0px 10px;">Save</button>
              <button  class="btn btn-primary float-end" c-click="preEditIssueComment({{@key}}, false)" style="margin: 10px 0px 0px 0px;">Cancel</button>
              {{else}}
              <pre  style="background: inherit; color: inherit; white-space: pre-wrap;">{{comments}}</pre>
              {{/ifCond}}
              {{else}}
              <pre  style="background: inherit; color: inherit; white-space: pre-wrap;">{{comments}}</pre>
              {{/if}}
            </td>
          </tr>
          {{/each}}
          {{#if application.issue.bIsValidOpen}}
          <tr>
            <td></td>
            <td>
              <textarea c-model="issue.comments" class="form-control" rows="5" style="width: 100%;" placeholder="enter comments"></textarea>
              <button class="btn btn-primary float-end" c-click="addIssueComment('close', {{@root.application.issue.id}}, {{@root.application.id}})" style="margin: 10px 0px 0px 10px;">Close Issue</button>
              <button class="btn btn-primary float-end" c-click="addIssueComment('update', {{@root.application.issue.id}}, {{@root.application.id}})" style="margin: 10px 0px 0px 0px;">Add Comments</button>
            </td>
          </tr>
          {{/if}}
        </table>
      </div>
    </div>
    {{/if}}
    {{/if}}
    <!-- ]]] -->
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
  <!-- [[[ servers -->
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
  {{/if}}
  <!-- ]]] -->
  `
  // ]]]
}
