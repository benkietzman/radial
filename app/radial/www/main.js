///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-10-23
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
let common = new Common(
{
  application: 'Radial',
  footer:
  {
    subject: 'Radial',
    version: '1.0'
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
    {path: '/Alert', name: 'Alert', component: '/radial/component/Alert.js'},
    {path: '/Home', name: 'Home', component: '/radial/component/Home.js'},
    {path: '/Irc', name: 'Irc', component: '/radial/component/Irc.js'},
    {path: '/Live', name: 'Live', component: '/radial/component/Live.js'},
    {path: '/Login', name: 'Login', component: '/include/common/js/component/Login.js'},
    {path: '/Logout', name: 'Logout', component: '/include/common/js/component/Logout.js'},
    {path: '/Ssh', name: 'Ssh', component: '/radial/component/Ssh.js'},
    {path: '/Status', name: 'Status', component: '/radial/component/Status.js'},
    {path: '/Terminal', name: 'Terminal', component: '/radial/component/Terminal.js'},
    {default: '/Home'}
  ]
});
common.enableJwt(true);
common.enableJwtInclusion(true);
common.setRedirectPath('https://'+location.host+'/radial');
common.setSecureLogin(true);
common.wsCreate('radial', location.host, '7797', true, 'radial');
let app = new App(
{
  common: common
});
