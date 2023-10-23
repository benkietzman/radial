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
    {path: '/', name: 'index', component: '/radial/component/index.js'},
    {path: '/Login', name: 'Login', component: '/include/common/js/component/Login.js'},
    {path: '/Logout', name: 'Logout', component: '/include/common/js/component/Logout.js'},
    {default: '/'}
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
