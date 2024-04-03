// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2022-10-12
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
let common = new Common(
{
  application: 'Finance',
  footer:
  {
    subject: 'Finance',
    version: '2.0'
  },
  loads:
  {
    'centralMenu': '/include/common/js/component/centralMenu.js',
    'footer': '/include/common/js/component/footer.js',
    'menu': '/include/common/js/component/menu.js',
    'messages': '/include/common/js/component/messages.js',
    'progress': '/finance/component/progress.js',
    'radialStatus': '/include/common/js/component/radialStatus.js'
  },
  routes:
  [
    {path: '/Summary', name: 'Summary', component: '/finance/component/Summary.js'},
    {path: '/Summary/Assumption', name: 'SummaryAssumption', component: '/finance/component/SummaryAssumption.js'},
    {path: '/Summary/Forecast', name: 'SummaryForecast', component: '/finance/component/SummaryForecast.js'},
    {path: '/Summary/Plan', name: 'SummaryPlan', component: '/finance/component/SummaryPlan.js'},
    {path: '/Income', name: 'Income', component: '/finance/component/Income.js'},
    {path: '/Income/Employment', name: 'IncomeEmployment', component: '/finance/component/IncomeEmployment.js'},
    {path: '/Income/Welfare', name: 'IncomeWelfare', component: '/finance/component/IncomeWelfare.js'},
    {path: '/Expense', name: 'Expense', component: '/finance/component/Expense.js'},
    {path: '/Expense/Auto', name: 'ExpenseAuto', component: '/finance/component/ExpenseAuto.js'},
    {path: '/Expense/Charity', name: 'ExpenseCharity', component: '/finance/component/ExpenseCharity.js'},
    {path: '/Expense/Food', name: 'ExpenseFood', component: '/finance/component/ExpenseFood.js'},
    {path: '/Expense/Home', name: 'ExpenseHome', component: '/finance/component/ExpenseHome.js'},
    {path: '/Expense/Medical', name: 'ExpenseMedical', component: '/finance/component/ExpenseMedical.js'},
    {path: '/Expense/Misc', name: 'ExpenseMisc', component: '/finance/component/ExpenseMisc.js'},
    {path: '/Expense/Spend', name: 'ExpenseSpend', component: '/finance/component/ExpenseSpend.js'},
    {path: '/Expense/Utility', name: 'ExpenseUtility', component: '/finance/component/ExpenseUtility.js'},
    {path: '/Asset', name: 'Asset', component: '/finance/component/Asset.js'},
    {path: '/Asset/Liquid', name: 'AssetLiquid', component: '/finance/component/AssetLiquid.js'},
    {path: '/Asset/Metal', name: 'AssetMetal', component: '/finance/component/AssetMetal.js'},
    {path: '/Asset/Property', name: 'AssetProperty', component: '/finance/component/AssetProperty.js'},
    {path: '/Asset/Stock', name: 'AssetStock', component: '/finance/component/AssetStock.js'},
    {path: '/Liability', name: 'Liability', component: '/finance/component/Liability.js'},
    {path: '/Liability/Compound', name: 'LiabilityCompound', component: '/finance/component/LiabilityCompound.js'},
    {path: '/Liability/Simple', name: 'LiabilitySimple', component: '/finance/component/LiabilitySimple.js'},
    {default: '/Summary'}
  ]
});
let junction = new ServiceJunction;
common.enableJwt(true);
common.enableJwtInclusion(true);
common.setRedirectPath('https://'+location.host+'/finance');
common.setSecureLogin(true);
common.wsCreate('radial', location.host, '7797', true, 'radial');
let app = new App(
{
  common: common,
  junction: junction
});
