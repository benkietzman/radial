///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2025-04-25
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
let common = new Common(
{
  application: 'MythTV',
  footer:
  {
    subject: 'MythTV',
    version: '1.0'
  },
  loads:
  {
    'centralMenu': '/include/common/js/component/centralMenu.js',
    'footer': '/include/common/js/component/footer.js',
    'menu': '/include/common/js/component/menu.js',
    'messages': '/include/common/js/component/messages.js',
    'radialChat': '/include/common/js/component/radialChat.js',
    'radialStatus': '/include/common/js/component/radialStatus.js'
  },
  routes:
  [
    {path: '/Guide', name: 'Guide', component: '/mythtv/component/Guide.js'},
    {path: '/Recordings', name: 'Recordings', component: '/mythtv/component/Recordings.js'},
    {path: '/Login', name: 'Login', component: '/include/common/js/component/Login.js'},
    {path: '/Logout', name: 'Logout', component: '/include/common/js/component/Logout.js'},
    {path: '/Status', name: 'Status', component: '/include/common/js/component/RadialStatus.js'},
    {path: '/Upcoming', name: 'Upcoming', component: '/mythtv/component/Upcoming.js'},
    {default: '/Guide'}
  ]
});
common.enableJwt(true);
common.enableJwtInclusion(true);
common.setRedirectPath('https://'+location.host+'/mythtv');
common.setSecureLogin(true);
common.wsCreate('radial', location.host, '7797', true, 'radial');
let app = new App(
{
  common: common
});
