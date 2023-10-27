// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-10-27
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
    let s = c.scope('Live',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Live');
      },
      // ]]]
      a: a,
      c: c,
      applications: null,
      classes: ['danger', 'info', 'success', 'warning'],
      'in': {}
    });
    // ]]]=
    // [[[ init()
    s.init = () =>
    {
      s.info.v = 'Retrieving applications...';
      s.applications = null;
      s.applications = [];
      let request = {Interface: 'database', Database: 'central_r', Query: 'select a.name from application a, application_account b, account_type c where a.id = b.application_id and b.type_id = c.id and c.type = \'Radial - WebSocket\' order by a.name'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          for (let i = 0; i < response.Response.length; i++)
          {
            if (c.isLocalAdmin(response.Response[i].name))
            {
              s.applications.push(response.Response[i].name);
            }
          }
          s.u();
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ send()
    s.send = () =>
    {
      if (c.isGlobalAdmin() || (s['in'].Application && c.isLocalAdmin(s['in'].Application)))
      {
        s.info.v = 'Sending message...';
        let request = {Interface: 'live', 'Function': 'message', Request: c.simplify(s['in'])};
        request.Request.Message.Action = 'message';
        c.wsRequest('radial', request).then((response) =>
        {
          let error = {};
          s.info.v = null;
          if (c.wsResponse(response, error))
          { 
            alert("Successfully sent message.");
          }
          else
          { 
            s.message.v = error.message;
          }
        });
      }
      else
      {
        s.message.v = 'You are not authorized to perform this action.';
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Live');
    s.u();
    if (a.ready())
    {
      s.init();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.init();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <h4 class="page-header">Live Messaging</h4>
  <p>
    Please be careful when using this tool.  Not selecting an Application will send the message to all applications.  Not inputting a User will send the message to all users for the given application.
  </p>
  <div class="row">
    <div class="col-md-6">
      <div class="input-group"><span class="input-group-text">Application</span><select class="form-control" c-model="in.Application"><option value="">-- All Applications --</option>{{#each applications}}<option value="{{.}}">{{.}}</option>{{/each}}</select></div>
    </div>
    <div class="col-md-3">
      <div class="input-group"><span class="input-group-text">User</span><input type="text" class="form-control" c-model="in.User"></div>
    </div>
    <div class="col-md-3">
      <div class="input-group"><span class="input-group-text">Class</span><select class="form-control" c-model="in.Message.Class">{{#each classes}}<option value="{{.}}">{{.}}</option>{{/each}}</select></div>
    </div>
  </div>
  <br>
  <div class="input-group"><span class="input-group-text">Title</span><input type="text" class="form-control" c-model="in.Message.Title"></div>
  <br>
  <b>Body</b><br>
  <textarea class="form-control" c-model="in.Message.Body" rows="5"></textarea>
  <br>
  <button class="btn btn-success bi bi-send float-end" c-click="send()">Send</button>
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger"></div>
  <div c-model="success" class="text-success"></div>
  `
  // ]]]
}
