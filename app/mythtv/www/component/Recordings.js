// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2025-04-25
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
    let s = c.scope('Recordings',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Recordings');
      },
      // ]]]
      a: a,
      c: c,
      bLoaded: false
    });
    // ]]]
    // [[[ getRecordings()
    s.getRecordings = () =>
    {
      s.programs = null;
      s.programs = [];
      let request = {Interface: 'mythtv', 'Function': 'dvrGetRecordedList'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          let p = response.Response.ProgramList.Programs.Program;
          for (let i = 0; i < p.length; i++)
          {
            if (p[i].Recording.RecGroup == 'Default')
            {
              let t = new Date(p[i].StartTime);
              p[i].StartTime = (new Date(t - t.getTimezoneOffset() * 60 * 1000)).toISOString().split('.')[0].replace('T', ' ');
              s.programs.push(p[i]);
            }
          }
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      s.bLoaded = true;
      if (c.isValid('MythTV'))
      {
        s.getRecordings();
      }
      else
      {
        s.u();
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Recordings');
    s.u();
    if (a.ready())
    {
      s.init();
    }
    c.attachEvent('appReady', (data) =>
    {
      s.init();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  {{#isValid "MythTV"}}
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
    <thead>
      <tr>
        <th>Recorded</th>
        <th>Title</th>
        <th>SubTitle</th>
        <th>Description</th>
        <th>Channel</th>
        <th>Episode</th>
        <th>Category</th>
      </tr>
    </thead>
    <tbody>
      {{#each ../programs}}
      <tr>
        <td>{{StartTime}}</td>
        <td>{{Title}}</td>
        <td>{{SubTitle}}</td>
        <td>{{Description}}</td>
        <td>{{Channel.ChanNum}} {{Channel.CallSign}}</td>
        <td>S{{Season}}E{{Episode}}</td>
        <td>{{Category}}</td>
      </tr>
      {{/each}}
    </tbody>
    </table>
  </div>
  {{else}}
  {{#if ../bLoaded}}
  {{^isValid}}
  <p class="fw-bold text-danger">Please login to use this application.</p>
  {{/isValid}}
  <p class="fw-bold text-danger">You must be registered as a contact for the MythTV application in Central.</p>
  {{/if}}
  {{/isValid}}
  `
  // ]]]
}
