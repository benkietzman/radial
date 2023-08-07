// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-08-07
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
    let s = c.scope('ApplicationsStatistics',
    {
      // [[[ u()
      u: () =>
      {
        c.update('ApplicationsStatistics');
      },
      // ]]]
      a: a,
      c: c,
      bLoading: true,
      displays: ['-- All --', 'Applications', 'Browsers', 'Operating Systems', 'Robots'],
      showDatas: ['no', 'yes'],
      uniques: ['no', 'yes']
    });
    // ]]]
    // [[[ disp()
    s.disp = () =>
    {
      if (s.application.id)
      {
        s.displays = ['-- All --', 'Browsers', 'Operating Systems', 'Robots'];
        if (s.display.v == 'Applications')
        {
          s.display.v = s.displays[0];
        }
      }
      else
      {
        s.displays = ['-- All --', 'Applications', 'Browsers', 'Operating Systems', 'Robots'];
      }
      s.loadStatistics();
    };
    // ]]]
    // [[[ loadApplications()
    s.loadApplications = () =>
    {
      s.info.v = 'Retrieving applications...';
      let request = {Interface: 'database', Database: 'central_r', Query: 'select distinct b.id, b.name from statistic a, application b where a.application_id = b.id order by b.name'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          s.applications = response.Response;
        }
        else
        {
          s.message.v = error.message;
        }
        s.applications.unshift({id: null, name: '-- All --'});
        s.application.v = s.applications[0];
        if (s.bLoading)
        {
          s.bLoading = false;
          s.loadStatistics();
        }
        s.u();
      });
    };
    // ]]]
    // [[[ loadStatistics()
    s.loadStatistics = () =>
    {
      let strQuery = 'select a.date, '+((s.unique.v == 'yes')?'count(a.count)':'sum(a.count)')+' count';
      if (s.display.v != '-- All --')
      {
        strQuery += ', b.name';
      }
      strQuery += ' from statistic a';
      if (s.display.v == '-- All --')
      {
        strQuery += ' where 1 = 1';
      }
      else if (s.display.v == 'Applications')
      {
        strQuery += ', application b where a.application_id = b.id';
      }
      else if (s.display.v == 'Browsers')
      {
        strQuery += ', browser b where a.browser_id = b.id and b.id != 0';
      }
      else if (s.display.v == 'Operating Systems')
      {
        strQuery += ', operating_system b where a.operating_system_id = b.id and b.id != 0';
      }
      else if (s.display.v == 'Robots')
      {
        strQuery += ', robot b where a.robot_id = b.id and b.id != 0';
      }
      if (s.application.v.id)
      {
        strQuery += ' and a.application_id = '+s.application.v.id;
      }
      if (s.date_start.v)
      {
        strQuery += ' and a.date >= \''+s.date_start.v+'\'';
      }
      if (s.date_end.v)
      {
        strQuery += ' and a.date <= \''+s.date_end.v+'\'';
      }
      strQuery += ' group by';
      if (s.display.v != '-- All --')
      {
        strQuery += ' name,';
      }
      strQuery += ' a.date order by a.date';
      if (s.display.v != '-- All --')
      {
        strQuery += ', name';
      }
      s.info.v = 'Retrieving statistics...';
      let request = {Interface: 'database', Database: 'central_r', Query: strQuery};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          s.statistics = response.Response;
          s.u();
        }
        else
        {
          s.message.v = error.message;
        }
      });
    };
    // ]]]
    // [[[ main
    c.setMenu('Applications', 'Statistics');
    s.display = s.displays[0];
    s.showData = s.showDatas[0];
    s.unique = s.uniques[0];
    if (!s.date_start)
    { 
      let d = new Date(Date.now() - (1000 * 60 * 60 * 24 * 30));
      s.date_start = d.getFullYear().toString().padStart(4, '0') + '-' + (d.getMonth() + 1).toString().padStart(2, '0') + '-' + d.getDate().toString().padStart(2, '0');
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
    c.attachEvent('render', (data) =>
    {
      if (data.detail.name == 'ApplicationsStatistics')
      {
        if (c.isDefined(s.statistics))
        {
          s.total = 0;
          let d = {labels: [], datasets: []};
          let chart = {};
          let labels = {};
          for (let i = 0; i < s.statistics.length; i++)
          {
            labels[s.statistics[i].date] = i;
          }
          for (let [key, value] of Object.entries(labels))
          {
            d.labels.push(key);
          }
          for (let i = 0; i < s.statistics.length; i++)
          {
            s.total += parseInt(s.statistics[i].count);
            if (!c.isDefined(s.statistics[i].name))
            {
              s.statistics[i].name = 'count';
            }
            if (!c.isDefined(chart[s.statistics[i].name]))
            {
              chart[s.statistics[i].name] = {};
              for (let [key, value] of Object.entries(labels))
              {
                chart[s.statistics[i].name][key] = 0;
              }
            }
            chart[s.statistics[i].name][s.statistics[i].date] = s.statistics[i].count;
          }
          for (let [key, value] of Object.entries(chart))
          {
            let data = [];
            for (let [subKey, subValue] of Object.entries(value))
            {
              data.push(subValue);
            }
            d.datasets.push({label: key, data: data});
          }
          const ctx = document.getElementById('chart');
          if (c.isDefined(s.chart) && !c.isNull(s.chart))
          {
            s.chart.destroy();
            delete s.chart;
          }
          s.chart = new Chart(ctx, {type: 'line', data: d});
        }
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div class="row">
    <div class="col-md-3">
      <div class="page-header">
        <h5>Search Options</h5>
      </div>
      <div class="input-group"><span class="input-group-text">Application</span><select c-model="application" c-change="disp()" class="form-control" c-json>{{#each applications}}<option value="{{json .}}">{{name}}</option>{{/each}}</select></div>
      <div class="input-group"><span class="input-group-text">Display</span><select c-model="display" class="form-control" c-change="loadStatistics()">{{#each displays}}<option value="{{.}}">{{.}}</option>{{/each}}</select></div>
      <div class="input-group"><span class="input-group-text">Show Data</span><select c-model="showData" c-change="u()" class="form-control">{{#each showDatas}}<option value="{{.}}">{{.}}</option>{{/each}}</select></div>
      <div class="input-group"><span class="input-group-text">Unique</span><select c-model="unique" c-change="loadStatistics()" class="form-control">{{#each uniques}}<option value="{{.}}">{{.}}</option>{{/each}}</select></div>
      <div class="card card-body card-inverse">
        <div class="input-group"><span class="input-group-text">Start</span><input type="text" c-model="date_start" class="form-control" placeholder="YYYY-MM-DD"></div>
        <div class="input-group"><span class="input-group-text">End</span><input type="text" c-model="date_end" class="form-control" placeholder="YYYY-MM-DD"></div>
        <button class="btn btn-primary float-end" c-click="loadStatistics()">Search</button>
      </div>
    </div>
    <div class="col-md-9">
      <h3 class="page-header">Statistics</h3>
      <div class="input-group float-end"><span class="input-group-text">Narrow</span><input type="text" class="form-control" id="narrow" c-model="narrow" c-render placeholder="Narrow Results"></div>
      <div c-model="info" class="text-warning"></div>
      <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
      {{#if statistics}}
      <canvas id="chart"></canvas>
      {{#ifCond showData.val '==' 'yes'}}
      <div class="table-responsive">
        <table class="table table-condensed table-striped">
          <tr>
            <th>Date</th>
            <th>Name</th>
            <th>Count</th>
          </tr>
          {{#eachFilter @root.statistics "name" narrow}}
          <tr>
            <td style="white-space: nowrap;">{{date}}</td>
            <td>{{name}}</td>
            <td>{{count}}</td>
          </tr>
          {{/eachFilter}}
          <tr>
            <th>Total</td>
            <td></td>
            <td>{{@root.total}}</td>
          </tr>
        </table>
      </div>
      {{/ifCond}}
      {{/if}}
    </div>
  </div>
  `
  // ]]]
}
