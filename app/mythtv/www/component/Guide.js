// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2025-04-28
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
    let s = c.scope('Guide',
    {
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
          }
          s.details.guests = '';
          for (let i = 0; i < s.details.Cast.CastMembers.CastMember.length; i++)
          {
            if (s.details.Cast.CastMembers.CastMember[i].Role == 'guest' || s.details.Cast.CastMembers.CastMember[i].Role == 'guest_star')
            {
              if (s.details.guests.length > 0)
              {
                s.details.guests += ', ';
              }
              s.details.guests += s.details.Cast.CastMembers.CastMember[i].Name;
            }
          }
          s.details.directors = '';
          for (let i = 0; i < s.details.Cast.CastMembers.CastMember.length; i++)
          {
            if (s.details.Cast.CastMembers.CastMember[i].Role == 'director')
            {
              if (s.details.directors.length > 0)
              {
                s.details.directors += ', ';
              }
              s.details.directors += s.details.Cast.CastMembers.CastMember[i].Name;
            }
          }
        }
        else
        {
          s.modalServerMessage.v = error.message;
        }
        c.loadModal('Guide', 'detailsModal', true);
        s.resize();
      });
    };
    // ]]]
    // [[[ getGuide()
    s.getGuide = () =>
    {
      s.channels = null;
      s.channels = {};
      let CNow = new Date();
      let CStart = new Date(CNow.getFullYear(), CNow.getMonth(), CNow.getDate() , CNow.getHours(), 0, 0, 0);
      //let CEnd = new Date(CStart.getTime() + (14 * 24 * 60 * 60 * 1000));
      let CEnd = new Date(CStart.getTime() + (16 * 60 * 60 * 1000));
      let request = {Interface: 'mythtv', 'Function': 'backend', Request: {Service: 'Guide', Command: 'GetProgramGuide', Get: {StartTime: CStart.toISOString().split('.')[0].replace('T', ' '), EndTime: CEnd.toISOString().split('.')[0].replace('T', ' ')}, StartTimestamp: CStart.getTime(), EndTimestamp: CEnd.getTime()}};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error))
        {
          response.Response = JSON.parse(response.Response);
          s.cols = [];
          let bFirst = true;
          let CStartTime = response.Request.StartTimestamp;
          let CEndTime = response.Request.EndTimestamp;
          while (CStartTime < CEndTime)
          {
            let startTime = new Date(CStartTime);
            let title = '';
            if (bFirst || startTime.getHours() == 0)
            {
              bFirst = false;
              title = (startTime.getMonth() + 1) + '/' + startTime.getDate() + ' ';
            }
            title += startTime.getHours().toString().padStart(2, '0') + ':00';
            s.cols.push(title);
            CStartTime += 3600 * 1000;
          }
          for (let i = 0; i < response.Response.ProgramGuide.Channels.ChannelInfo.length; i++)
          {
            if (response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs && response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program)
            {
              let bFirst = true;
              let strChannel = response.Response.ProgramGuide.Channels.ChannelInfo[i].ChanNum + ' ' + response.Response.ProgramGuide.Channels.ChannelInfo[i].CallSign;
              s.channels[strChannel] = {ChanId: response.Response.ProgramGuide.Channels.ChannelInfo[i].ChanId, programs: []};
              for (let j = 0; j < response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program.length; j++)
              {
                if (response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].Recording)
                {
                  if (response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].Recording.StatusName == 'WillRecord')
                  {
                    response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].color = 'warning';
                  }
                  else if (response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].Recording.StatusName == 'Pending' || response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].Recording.StatusName == 'Recording' || response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].Recording.StatusName == 'Tuning')
                  {
                    response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].color = 'success';
                  }
                  else
                  {
                    response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].color = 'danger';
                  }
                }
                else
                {
                  response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].color = 'secondary';
                }
                response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].TitleShort = response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].Title;
                if (response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].TitleShort.length > 20)
                {
                  response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].TitleShort = response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].TitleShort.substr(0, 17) + '...';
                }
                let t = new Date(response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].StartTime);
                response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].StartTimeShort = t.getHours().toString().padStart(2, '0') + ':' + t.getMinutes().toString().padStart(2, '0');
                response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].StartTimestamp = t.getTime();
                t = new Date(t - t.getTimezoneOffset() * 60 * 1000);
                response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].StartTime = t.toISOString().split('.')[0].replace('T', ' ');
                t = new Date(response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].EndTime);
                response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].EndTimeShort = t.getHours().toString().padStart(2, '0') + ':' + t.getMinutes().toString().padStart(2, '0');
                response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].EndTimestamp = t.getTime();
                t = new Date(t - t.getTimezoneOffset() * 60 * 1000);
                response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].EndTime = t.toISOString().split('.')[0].replace('T', ' ');
                if (bFirst)
                {
                  if (response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].EndTimestamp > response.Request.StartTimestamp)
                  {
                    bFirst = false;
                    response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].colspan = (response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].EndTimestamp - response.Request.StartTimestamp) / 1000 / 60;
                    s.channels[strChannel].programs.push(response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j]);
                  }
                }
                else
                {
                  response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].colspan = (response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].EndTimestamp - response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j].StartTimestamp) / 1000 / 60;
                  s.channels[strChannel].programs.push(response.Response.ProgramGuide.Channels.ChannelInfo[i].Programs.Program[j]);
                }
              }
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
        s.getGuide();
      }
      else
      {
        s.u();
      }
    };
    // ]]]
    // [[[ resize()
    s.resize = () =>
    {
      let guide = document.getElementById('radial_guide');
      let maxHeight = document.documentElement.clientHeight - 140;
      guide.style.minHeight = maxHeight + 'px';
      guide.style.maxHeight = maxHeight + 'px';
    };
    // ]]]
    // [[[ u()
    s.u = () =>
    {
      c.update('Guide');
      s.resize();
    };
    // ]]]
    // [[[ main
    c.setMenu('Guide');
    s.u();
    if (a.ready())
    {
      s.init();
    }
    c.attachEvent('appReady', (data) =>
    {
      s.init();
    });
    window.addEventListener('resize', () =>
    {
      s.resize();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div class="table-responsive" id="radial_guide" style="padding: 0px;">
    {{#isValid "MythTV"}}
    <table class="table table-condensed table-striped">
    <thead>
      <tr>
        <th class="text-left" style="position: sticky; left: 0; top: 0;">Channel</th>
        {{#each ../cols}}
        <th class="text-left text-nowrap" colspan="60" style="position: sticky; top: 0;">{{.}}</th>
        {{/each}}
      </tr>
    </thead>
    <tbody>
      {{#each ../channels}}
      <tr>
        <td class="text-left text-nowrap" style="position: sticky; left: 0;">{{@key}}</td>
        {{#each programs}}
        <td c-click="getProgramDetails({{../ChanId}}, {{StartTimestamp}})" class="bg-{{color}}-subtle bg-gradient border border-dark text-nowrap" colspan="{{colspan}}" data-bs-target="#detailsModal" style="cursor: pointer;" title="[{{StartTimeShort}}-{{EndTimeShort}}] {{Title}}">{{TitleShort}}</td>
        {{/each}}
      </tr>
      {{/each}}
    </tbody>
    </table>
    {{else}}
    {{#if ../bLoaded}}
    {{^isValid}}
    <p class="fw-bold text-danger">Please login to use this application.</p>
    {{/isValid}}
    <p class="fw-bold text-danger">You must be registered as a contact for the MythTV application in Central.</p>
    {{/if}}
    {{/isValid}}
  </div>
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
