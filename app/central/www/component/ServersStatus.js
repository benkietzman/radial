// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-28
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
    let s = c.scope('ServersStatus',
    {
      // [[[ u()
      u: () =>
      {
        c.update('ServersStatus');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ loadServers()
    s.loadServers = () =>
    {
      s.servers = null;
      s.u();
      s.info.v = 'Retrieving servers...';
      let request = {Interface: 'database', Database: 'central_r', Query: 'select a.id, a.name from server a, application_server b, application c where a.id=b.server_id and b.application_id=c.id and c.name = \'System Information\' order by a.name'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          s.servers = response.Response;
          s.u();
          c.addInterval('ServersStatus', 'sysInfo', s.sysInfo, 300000);
          s.sysInfo();
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ sysInfo()
    s.sysInfo = () =>
    {
      for (let i = 0; i < s.servers.length; i++)
      {
        if (s.servers[i].name)
        {
          s.info.v = 'Retrieving status for server '+s.servers[i].name+'...';
          let request = {Interface: 'junction', Request: [{Service: 'sysInfo', Action: 'system', Server: s.servers[i].name, i: i}]};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              let i = response.Request[0].i;
              s.servers[i].sysInfo = response.Response[1];
              s.servers[i].sysInfo.CpuUsage = Math.round(s.servers[i].sysInfo.CpuUsage);
              s.servers[i].sysInfo.MainUsage = Math.round(s.servers[i].sysInfo.MainUsed * 100 / s.servers[i].sysInfo.MainTotal);
              s.servers[i].sysInfo.SwapUsage = Math.round(s.servers[i].sysInfo.SwapUsed * 100 / s.servers[i].sysInfo.SwapTotal);
              s.servers[i].alarms = null;
              if (s.servers[i].sysInfo.Alarms)
              {
                s.servers[i].alarms = s.servers[i].sysInfo.Alarms.split(',');
              }
              s.info.v = 'Retrieving applications for server '+s.servers[i].name+'...';
              let request = {Interface: 'database', Database: 'central_r', Query: 'select distinct a.id, a.name, b.id application_server_id from application a, application_server b, application_server_detail c where a.id=b.application_id and b.id=c.application_server_id and b.server_id = '+s.servers[i].id+' and c.daemon is not null and c.daemon != \'\' order by a.name', Request: {i: i}};
              c.wsRequest('radial', request).then((response) =>
              {
                let error = {};
                s.info.v = null;
                if (c.wsResponse(response, error))
                {
                  let i = response.Request.i;
                  s.servers[i].applications = response.Response;
                  for (let j = 0; j < s.servers[i].applications.length; j++)
                  {
                    s.info.v = 'Retrieving processes for application '+s.servers[i].applications[j].name+' for server '+s.servers[i].name+'...';
                    var request = {Interface: 'database', Database: 'central_r', Query: 'select daemon from application_server_detail where application_server_id = '+s.servers[i].applications[j].application_server_id+' and daemon is not null and daemon != \'\' order by daemon', Request: {i: i, j: j}};
                    c.wsRequest('radial', request).then((response) =>
                    {
                      let error = {};
                      s.info.v = null;
                      if (c.wsResponse(response, error))
                      {
                        let i = response.Request.i;
                        let j = response.Request.j;
                        s.servers[i].applications[j].daemons = response.Response;
                        for (let k = 0; k < s.servers[i].applications[j].daemons.length; k++)
                        {
                          s.info.v = 'Retrieving status for process '+s.servers[i].applications[j].daemons[k].daemon+' for application '+s.servers[i].applications[j].name+' for server '+s.servers[i].name+'...';
                          let request = {Interface: 'junction', Request: [{Service: 'sysInfo', Action: 'process', Server: s.servers[i].name, Process: s.servers[i].applications[j].daemons[k].daemon, i: i, j: j, k: k}]};
                          c.wsRequest('radial', request).then((response) =>
                          {
                            let error = {};
                            s.info.v = null;
                            if (c.wsResponse(response, error))
                            {
                              let i = response.Request[0].i;
                              let j = response.Request[0].j;
                              let k = response.Request[0].k;
                              s.servers[i].applications[j].daemons[k].sysInfo = response.Response[1];
                              if (s.servers[i].applications[j].daemons[k].sysInfo.Alarms)
                              {
                                let alarms = s.servers[i].applications[j].daemons[k].sysInfo.Alarms.split(',');
                                if (!s.servers[i].alarms)
                                {
                                  s.servers[i].alarms = [];
                                }
                                for (let l = 0; l < alarms.length; l++)
                                {
                                  s.servers[i].alarms.push(s.servers[i].applications[j].name+' - '+s.servers[i].applications[j].daemons[k].daemon+' - '+alarms[l]);
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
    };
    // ]]]
    // [[[ main
    c.setMenu('Servers', 'Status');
    s.u();
    if (a.ready())
    {
      s.loadServers();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.loadServers();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <h3 class="page-header">Server Status</h3>
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
  <div class="table-responsive">
    <table class="table table-condensed table-striped">
      <tr>
        <th>Server</th>
        <th>Processes</th>
        <th>CPU Usage</th>
        <th>Memory Usage</th>
        <th>Swap Usage</th>
        <th>Alarms</th>
      </tr>
      {{#each servers}}
      <tr>
        <td valign="top"><a href="#/Servers/{{id}}">{{name}}</a></td>
        <td valign="top">
          {{numberShort sysInfo.NumberOfProcesses 0}}
        </td>
        <td valign="top">
          <div class="progress" style="width: 100px;">
            <div class="progress-bar" role="progressbar" aria-valuenow="{{sysInfo.CpuUsage}}" aria-valuemin="0" aria-valuemax="100" style="width: {{numberShort sysInfo.CpuUsage 0}}%;">
              {{numberShort sysInfo.CpuUsage 0}}%
            </div>
          </div>
        </td>
        <td valign="top">
          <div class="progress" style="width: 100px;">
            <div class="progress-bar" role="progressbar" aria-valuenow="{{sysInfo.MainUsage}}" aria-valuemin="0" aria-valuemax="100" style="width: {{numberShort sysInfo.MainUsage 0}}%;">
              {{numberShort sysInfo.MainUsage 0}}%
            </div>
          </div>
        </td>
        <td valign="top">
          <div class="progress" style="width: 100px;">
            <div class="progress-bar" role="progressbar" aria-valuenow="{{sysInfo.SwapUsage}}" aria-valuemin="0" aria-valuemax="100" style="width: {{numberShort sysInfo.SwapUsage 0}}%;">
              {{numberShort sysInfo.SwapUsage 0}}%
            </div>
          </div>
        </td>
        <td valign="top" class="text-danger">
          {{#if alarms}}
          <ul class="list-group">
            {{#each alarms}}
            <li class="list-group-item text-danger">{{.}}</li>
            {{/each}}
          </ul>
          {{/if}}
        </td>
      </tr>
      {{/each}}
    </table>
  </div>
  `
  // ]]]
}
