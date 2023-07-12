// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-12
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // {{{ controller()
  controller(id, nav)
  {
    let a = app;
    let c = common;
    let s = c.scope('Home',
    {
      // {{{ u()
      u: () =>
      {
        c.render(id, 'Home', this);
      },
      // }}}
      a: a,
      c: c,
      message: null
    });
    c.setMenu('Home', null);
  },
  // }}}
  // {{{ template
  template: `
  {{#if message}}
  <div class="text-danger" style="font-weight:bold;"><br><br>{{message}}<br><br></div>
  {{/if}}
  <div class="row">
    <div class="page-header row" style="margin-top: 0px;">
      <div class="col-md-6">
        <h3 style="margin-top: 0px;"><small>Welcome to</small><br>Central<small></small></h3>
      </div>
      <div class="col-md-6">
        <div class="row">
          <div class="col-md-4">
            <div ng-show="store.numApplications" class="well" style="padding: 10px; text-align: center;">
              <a class="text-warning" href="#/Applications"><b>Applications:</b> {{numberShort numApplications}}</a>
            </div>
          </div>
          <div class="col-md-4">
            <div ng-show="store.numServers" class="well" style="padding: 10px; text-align: center;">
              <a class="text-success" href="#/Servers"><b>Servers:</b> {{numberShort numServers}}</a>
            </div>
          </div>
          <div class="col-md-4">
            <div ng-show="store.numUsers" class="well" style="padding: 10px; text-align: center;">
              <a class="text-info" href="#/Users"><b>Users:</b> {{numberShort numUsers}}</a>
            </div>
          </div>
        </div>
      </div>
    </div>
    <div class="row">
      <div class="col-md-12">
        <p>
          This website organizes and tracks <a href="/central/#/Applications">application</a>, <a href="/central/#/Servers">server</a>, and <a href="/central/#/Users">user</a> information.  This website offers many different areas of value for automation teams.  It keeps software engineers and server administrators more efficient by providing a central location to store associated information.
        </p>
        <p>
          Central provides many useful and vital capabilities at a more detailed level.  For instance, Central provides the ability to manage application issues which allows developers the ability to organize and prioritize their workload.  Central provides a front-end to the centralized web-based security modules allowing applications to easily switch between various authentication mechanisms for their websites.  Central has hooks into the <a href="/central/#/Applications/54">System Information</a> (aka: Central Monitor) application which actively monitors the health of servers as well as the daemonized services of applications.
        </p>
        {{#if info}}
        <div class="text-warning"><br><br>{{info}}<br><br></div>
        {{/if}}
      </div>
    </div>
  </div>
  <div class="row">
    <div class="col-md-5">
      <div class="panel panel-success" style="display: table; box-shadow: 3px 3px 4px black;">
        <div class="panel-heading" style="font-weight:bold;">
          Front Door
        </div>
        <div class="panel-body">
          <p>
            Please use the <a href="/central/#/Home/FrontDoor">Front Door</a> when creating a new issue for an application.  The Front Door provides a comprehensive list of applications from which to choose.
          </p>
          <p>
            The primary/backup developers of the given application will be notified of the newly created issue.  The developers will also receive a weekly <a href="/central/#/Applications/Workload">Workload</a> reminder of all outstanding open issues for which they are the primary/backup developers.
          </p>
        </div>
      </div>
    </div>
    <div class="col-md-7">
      <div class="panel panel-warning" style="display: table; box-shadow: 3px 3px 4px black;">
        <div class="panel-heading" style="font-weight:bold;">
          Workload
        </div>
        <div class="panel-body">
          <p>
            Developers may access the <a href="/central/#/Applications/Workload">Workload</a> section of Central in order to view their list of open application issues for which they are registered as a primary/backup developer.  The Workload displays issues sorted by priority, due date, and open date.  This helps to keep developers organized in how they tackle their daily issues.
          </p>
          <p>
            Central sends out a weekly Workload email to all developers who have open issues.  The email is sent out every Monday morning and contains a list of open application issues organized similarly to the Workload section of Central.  This helps to keep issues on the radar for developers who may not visit the Workload section of Central regularly.
          </p>
        </div>
      </div>
    </div>
  </div>
  `
  // }}}
}
