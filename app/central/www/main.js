// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-12
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
let common = new Common(
{
  application: 'Central',
  footer:
  {
    subject: 'Central',
    userid: 'ben', // TODO
    version: '2.0'
  },
  loads:
  {
    'centralMenu': '/include/common/js/component/centralMenu.js',
    'footer': '/include/common/js/component/footer.js',
    'menu': '/include/common/js/component/menu.js',
    'messages': '/include/common/js/component/messages.js',
    'radialStatus': '/include/common/js/component/radialStatus.js'
  },
  routes:
  [
    {path: '/Applications', name: 'Applications', component: '/central/component/Applications.js'},
    {path: '/Applications/Accounts', name: 'ApplicationsAccounts', component: '/central/component/ApplicationsAccounts.js'},
    {path: '/Applications/Accounts/:user', name: 'ApplicationsAccounts', component: '/central/component/ApplicationsAccounts.js'},
    {path: '/Applications/Issues', name: 'ApplicationsIssues', component: '/central/component/ApplicationsIssues.js'},
    {path: '/Applications/Issues/:id', name: 'ApplicationsIssues', component: '/central/component/ApplicationsIssues.js'},
    {path: '/Applications/Schedule', name: 'ApplicationsSchedule', component: '/central/component/ApplicationsSchedule.js'},
    {path: '/Applications/Statistics', name: 'ApplicationsStatistics', component: '/central/component/ApplicationsStatistics.js'},
    {path: '/Applications/Workload', name: 'ApplicationsWorkload', component: '/central/component/ApplicationsWorkload.js'},
    {path: '/Applications/:id/:form/:issue_id', name: 'Applications', component: '/central/component/Applications.js'},
    {path: '/Applications/:id/:form', name: 'Applications', component: '/central/component/Applications.js'},
    {path: '/Applications/:id', name: 'Applications', component: '/central/component/Applications.js'},
    {path: '/Home', name: 'Home', component: '/central/component/Home.js'},
    {path: '/Home/FrontDoor', name: 'HomeFrontDoor', component: '/central/component/HomeFrontDoor.js'},
    {path: '/Home/FrontDoor/:id', name: 'HomeFrontDoor', component: '/central/component/HomeFrontDoor.js'},
    {path: '/Login', name: 'Login', component: '/include/common/js/component/Login.js'},
    {path: '/Logout', name: 'Logout', component: '/include/common/js/component/Logout.js'},
    {path: '/Servers', name: 'Servers', component: '/central/component/Servers.js'},
    {path: '/Servers/Status', name: 'ServersStatus', component: '/central/component/ServersStatus.js'},
    {path: '/Servers/:id/:form', name: 'Servers', component: '/central/component/Servers.js'},
    {path: '/Servers/:id', name: 'Servers', component: '/central/component/Servers.js'},
    {path: '/Users', name: 'Users', component: '/central/component/Users.js'},
    {path: '/Users/:id/:form', name: 'Users', component: '/central/component/Users.js'},
    {path: '/Users/:id/', name: 'Users', component: '/central/component/Users.js'},
    {default: '/Home'}
  ]
});
common.enableJwt(true);
common.enableJwtInclusion(true);
common.setRedirectPath('https://kietzman.org/finance'); // TODO
common.setSecureLogin(true);
common.wsCreate('radial', 'kietzman.org', '7797', true, 'radial'); // TODO
let app = new App(
{
  common: common
});
