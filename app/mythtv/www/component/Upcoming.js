// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2025-04-25
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
    let s = c.scope('Upcoming',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Upcoming');
      },
      // ]]]
      a: a,
      c: c,
      bLoaded: false
    });
    // ]]]
    // [[[ getProgramDetails()
    s.getProgramDetails = (ChanId, StartTimestamp) =>
    {
      s.details = null;
      let CStart = new Date(StartTimestamp);
      let request = {Interface: 'mythtv', 'Function': 'backend', Request: {Service: 'Guide', Command: 'GetProgramDetails', Get: {ChanId: ChanId, StartTime: CStart.toISOString().split('.')[0].replace('T', ' ')}}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {   
          response.Response = JSON.parse(response.Response);
          s.details = response.Response.Program;
console.log(s.details);
          let t = new Date(s.details.StartTime);
          t = new Date(t - t.getTimezoneOffset() * 60 * 1000);
          s.details.StartTime = t.toISOString().split('.')[0].replace('T', ' ');
          t = new Date(s.details.EndTime);
          t = new Date(t - t.getTimezoneOffset() * 60 * 1000);
          s.details.EndTime = t.toISOString().split('.')[0].replace('T', ' ');
          if (s.details.Airdate)
          {   
            t = new Date(s.details.Airdate);
            t = new Date(t - t.getTimezoneOffset() * 60 * 1000);
            s.details.Airdate = t.toISOString().split('.')[0].replace('T', ' ');
          }
          if (s.details.Recording)
          {
            if (s.details.Recording.StartTs)
            {
              t = new Date(s.details.Recording.StartTs);
              t = new Date(t - t.getTimezoneOffset() * 60 * 1000);
              s.details.Recording.StartTs = t.toISOString().split('.')[0].replace('T', ' ');
            }
            if (s.details.Recording.EndTs)
            {
              t = new Date(s.details.Recording.EndTs);
              t = new Date(t - t.getTimezoneOffset() * 60 * 1000);
              s.details.Recording.EndTs = t.toISOString().split('.')[0].replace('T', ' ');
            }
          }
          s.details.actors = '';
          s.details.guests = '';
          s.details.directors = '';
          if (s.details.Cast && s.details.Cast.CastMembers && s.details.Cast.CastMembers.CastMember)
          {
            for (let i = 0; i < s.details.Cast.CastMembers.CastMember.length; i++)
            {
              if (s.details.Cast.CastMembers.CastMember[i].Role == 'actor')
              {
                if (s.details.actors.length > 0)
                {
                  s.details.actors += ', ';
                }
                s.details.actors += s.details.Cast.CastMembers.CastMember[i].Name;
              }
              else if (s.details.Cast.CastMembers.CastMember[i].Role == 'director')
              {
                if (s.details.directors.length > 0)
                {
                  s.details.directors += ', ';
                }
                s.details.directors += s.details.Cast.CastMembers.CastMember[i].Name;
              }
              else if (s.details.Cast.CastMembers.CastMember[i].Role == 'guest' || s.details.Cast.CastMembers.CastMember[i].Role == 'guest_star')
              {
                if (s.details.guests.length > 0)
                {
                  s.details.guests += ', ';
                }
                s.details.guests += s.details.Cast.CastMembers.CastMember[i].Name;
              }
            }
          }
        }
        else
        {
          s.modalServerMessage.v = error.message;
        }
        c.loadModal('Upcoming', 'detailsModal', true);
      });
    };
    // ]]]
    // [[[ getUpcoming()
    s.getUpcoming = () =>
    {
      s.programs = null;
      s.programs = [];
      let request = {Interface: 'mythtv', 'Function': 'backend', Request: {Service: 'Dvr', Command: 'GetUpcomingList'}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          response.Response = JSON.parse(response.Response);
          let p = response.Response.ProgramList.Programs.Program;
          for (let i = 0; i < p.length; i++)
          {
            let t = new Date(p[i].StartTime);
            p[i].StartTimestamp = t.getTime();
            t = new Date(t - t.getTimezoneOffset() * 60 * 1000);
            p[i].StartTime = t.toISOString().split('.')[0].replace('T', ' ');
            t = new Date(p[i].EndTime);
            p[i].EndTimestamp = t.getTime();
            t = new Date(t - t.getTimezoneOffset() * 60 * 1000);
            p[i].EndTime = t.toISOString().split('.')[0].replace('T', ' ');
            s.programs.push(p[i]);
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
        s.getUpcoming();
      }
      else
      {
        s.u();
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Upcoming');
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
      <tr c-click="getProgramDetails({{Channel.ChanId}}, {{StartTimestamp}})" data-bs-target="#detailsModal" style="cursor: pointer;">
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
  <div id="detailsModal" class="modal modal-lg">
    <div class="modal-dialog modal-dialog-scrollable">
      <div class="modal-content">
        <div class="modal-header">
          <h4 class="modal-title">Program Details</h4>
          <button type="button" class="btn-close" data-bs-dismiss="modal"></button>
        </div>
        <div class="modal-body" class="table-responsive">
          <table class="table table-condensed table-striped">
          <tbody>
            {{#if details.Title}}
            <tr><th>Title</th><td>{{details.Title}}{{#if details.SubTitle}} - {{details.SubTitle}}{{/if}}</td></tr>
            {{/if}}
            {{#if details.Description}}
            <tr><th>Description</th><td>{{details.Description}}</td></tr>
            {{/if}}
            {{#if details.actors}}
            <tr><th>Actors</th><td>{{details.actors}}</td></tr>
            {{/if}}
            {{#if details.guests}}
            <tr><th>Guest Stars</th><td>{{details.guests}}</td></tr>
            {{/if}}
            {{#if details.directors}}
            <tr><th>Directors</th><td>{{details.directors}}</td></tr>
            {{/if}}
            {{#if details.Category}}
            <tr><th>Category</th><td>{{details.Category}}</td></tr>
            {{/if}}
            {{#if details.CatType}}
            <tr><th>Type</th><td>{{details.CatType}}{{#if details.SeriesId}} ({{details.SeriesId}}){{/if}}</td></tr>
            {{#ifCond details.Season ">" 0}}
            <tr><th>Season</th><td>{{../details.Season}}</td></tr>
            {{/ifCond}}
            {{#ifCond details.Episode ">" 0}}
            <tr><th>Episode</th><td>{{../details.Episode}}</td></tr>
            {{/ifCond}}
            {{/if}}
            {{#if details.Airdate}}
            <tr><th>Original Airdate</th><td>{{details.Airdate}}</td></tr>
            {{/if}}
            {{#if details.ProgramId}}
            <tr><th>Program ID</th><td>{{details.ProgramId}}</td></tr>
            {{/if}}
            <tr><th>MythTV Status</th><td>{{#if details.Recording}}{{details.Recording.StatusName}} {{details.Recording.StartTs}}{{else}}Not Recording{{/if}}</td></tr>
          </tbody>
          </table>
        </div>
        <div class="modal-footer">
          <div c-model="modalServerMessage" class="text-danger fw-bold"></div>
          <button type="button" class="btn btn-danger" data-bs-dismiss="modal">Close</button>
        </div>
      </div>
    </div>
  </div>
  `
  // ]]]
}
