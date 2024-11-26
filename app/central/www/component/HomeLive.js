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
    let s = c.scope('HomeLive',
    {
      // [[[ u()
      u: () =>
      {
        c.update('HomeLive');
      },
      // ]]]
      a: a,
      c: c,
      classes: ['danger', 'info', 'success', 'warning'],
      messages: false
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
        s.messaged = null;
        s.u();
        s.loadApplications();
      }
      else
      {
        s.info.v = 'Please login to send a live message.';
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
        s.info.v = 'Please login to send a live message.';
      }
    };
    // ]]]
    // [[[ loadApplications()
    s.loadApplications = () =>
    {
      if (c.isValid())
      {
        s.info.v = 'Retrieving applications...';
        let request = {Interface: 'database', Database: 'central_r', Query: 'select id, name from application where website is not null order by name'};
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
            s.message.v = error.message;
          }
        });
      }
      else
      {
        s.info.v = 'Please login to send a live message.';
      }
    };
    // ]]]
    // [[[ send()
    s.send = () =>
    {
      if (c.isValid())
      {
        if (s.application.v)
        {
          if (s.title.v)
          {
            if (s.body.v)
            {
              s.info.v = 'Sending notification...';
              s.messaged.v = null;
              let request = {Interface: 'live', 'Function': 'message', Request: {Application: s.application.v.name, Message: {Action: 'message', Class: s['class'].v, Title: s.title.v, Body: s.body.v + '\n\n-- ' + c.getUserFirstName() + ' ' + c.getUserLastName() + ' (' + c.getUserID() + ')'}}};
              c.wsRequest('radial', request).then((response) =>
              {
                let error = {};
                if (c.wsResponse(response, error))
                {
                  s.info.v = null;
                  s.messaged.v = 'Message has been sent.';
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
              alert('Please provide the Body.');
            }
          }
          else
          {
            alert('Please provide the Title.');
          }
        }
        else
        {
          alert('Please select an Application.');
        }
      }
      else
      {
        s.info.v = 'Please login to send a live message.';
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Home', 'Live');
    s['class'] = s.classes[0];
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
  <h3 class="page-header">Send a Live Message</h3>
  <p>
    Live Messages are sent to applications as alert boxes.
  </p>
  <div class="row" style="margin-bottom: 10px;">
    <div class="col-md-12">
      <div c-model="info" class="text-warning"></div>
      <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
      {{#isValid}}
      <div class="row">
        <div class="col-md-3" style="padding-top: 10px;">
          {{#if ../applications}}
          <div class="input-group"><span class="input-group-text">App</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow"></div>
          <select class="form-control" c-model="application" c-change="loadApplication()" size="2" style="height: 200px;" c-json>
            {{#eachFilter ../applications 'name' ../narrow}}
            <option value="{{json .}}">{{name}}</option>
            {{/eachFilter}}
          </select>
          {{/if}}
        </div>
        <div class="col-md-9">
          {{#if ../display}}
          <div class="card border border-primary-subtle">
            <div class="card-header bg-primary fw-bold">
              {{../application.val.name}}
            </div>
            <div class="card-body bg-primary-subtle">
              <div class="row" style="margin-top: 10px;">
                <div class="col-md-4">
                  <div class="input-group"><span class="input-group-text bg-primary">Class</span><select class="form-control bg-primary-subtle" c-model="class">{{#each ../classes}}<option value="{{.}}">{{.}}</option>{{/each}}</select></div>
                </div>
                <div class="col-md-8">
                  <div class="input-group"><span class="input-group-text bg-primary">Title</span><input type="text" class="form-control bg-primary-subtle" c-model="title"></div>
                </div>
              </div>
              <div class="row" style="margin-top: 10px;">
                <div class="col-md-12">
                  <textarea c-model="body" class="form-control bg-primary-subtle" rows="5" style="width: 100%;" placeholder="enter message"></textarea>
                </div>
              </div>
              <div class="row" style="margin-top: 10px;">
                <div class="col-md-12">
                  <button class="btn btn-primary bi bi-send float-end" c-click="send()" title="Send Message"></button>
                </div>
              </div>
            </div>
            {{#if messages}}
            <div class="card-footer">
              <div c-model="messaged" class="text-success"></div>
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
