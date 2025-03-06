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
      let request = {Interface: 'database', Database: 'central_r', Query: 'select a.id, a.name from server a, application_server b, application c where a.id=b.server_id and b.application_id=c.id and c.name = \'Central Monitor\' order by a.name'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          s.servers = response.Response;
          s.u();
          c.addInterval('ServersStatus', 'monitor', s.monitor, 300000);
          s.monitor();
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ monitor()
    s.monitor = () =>
    {
      for (let i = 0; i < s.servers.length; i++)
      {
        if (s.servers[i].name)
        {
          s.info.v = 'Retrieving status for server '+s.servers[i].name+'...';
          let request = {Interface: 'central', 'Function': 'monitorSystem', Request: {server: s.servers[i].name, i: i}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              let i = response.Request.i;
              s.servers[i].monitor = response.Response;
              s.servers[i].monitor.data.cpuUsage = Math.round(s.servers[i].monitor.data.cpuUsage);
              s.servers[i].monitor.data.mainUsage = Math.round(s.servers[i].monitor.data.mainUsed * 100 / s.servers[i].monitor.data.mainTotal);
              s.servers[i].monitor.data.swapUsage = Math.round(s.servers[i].monitor.data.swapUsed * 100 / s.servers[i].monitor.data.swapTotal);
              s.servers[i].alarms = [];
              if (s.servers[i].monitor.alarms)
              { 
                s.servers[i].alarms.push(s.servers[i].monitor.alarms);
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
                          let request = {Interface: 'central', 'Function': 'monitorProcess', Request: {server: s.servers[i].name, process: s.servers[i].applications[j].daemons[k].daemon, i: i, j: j, k: k}};
                          c.wsRequest('radial', request).then((response) =>
                          {
                            let error = {};
                            s.info.v = null;
                            if (c.wsResponse(response, error))
                            {
                              let i = response.Request.i;
                              let j = response.Request.j;
                              let k = response.Request.k;
                              s.servers[i].applications[j].daemons[k].monitor = response.Response;
                              if (s.servers[i].applications[j].daemons[k].monitor.alarms)
                              {
                                s.servers[i].alarms.push(s.servers[i].applications[j].daemons[k].monitor.alarms);
                              }
                              s.u();
                            }
                            else
                            {
                              c.pushErrorMessage(error.message);
                            }
                          });
                        }
                        s.u();
                      }
                      else
                      {
                        c.pushErrorMessage(error.message);
                      }
                    });
                  }
                  s.u();
                }
                else
                {
                  c.pushErrorMessage(error.message);
                }
              });
              s.u();
            }
            else
            {
              c.pushErrorMessage(error.message);
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
        <td valign="top" title="{{number monitor.data.processes 0}}">
          {{numberShort monitor.data.processes 0}}
        </td>
        <td valign="top">
          <div class="progress" style="width: 100px;">
            <div class="progress-bar" role="progressbar" aria-valuenow="{{monitor.data.cpuUsage}}" aria-valuemin="0" aria-valuemax="100" style="width: {{numberShort monitor.data.cpuUsage 0}}%;">
              {{numberShort monitor.data.cpuUsage 0}}%
            </div>
          </div>
        </td>
        <td valign="top">
          <div class="progress" style="width: 100px;">
            <div class="progress-bar" role="progressbar" aria-valuenow="{{monitor.data.mainUsage}}" aria-valuemin="0" aria-valuemax="100" style="width: {{numberShort monitor.data.mainUsage 0}}%;">
              {{numberShort monitor.data.mainUsage 0}}%
            </div>
          </div>
        </td>
        <td valign="top">
          <div class="progress" style="width: 100px;">
            <div class="progress-bar" role="progressbar" aria-valuenow="{{monitor.data.swapUsage}}" aria-valuemin="0" aria-valuemax="100" style="width: {{numberShort monitor.data.swapUsage 0}}%;">
              {{numberShort monitor.data.swapUsage 0}}%
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
