// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-12
// copyright  : kietzman.org
// email      : ben@kietzman.org
///////////////////////////////////////////
class App
{
  // {{{ constructor()
  constructor(options)
  {
    this.c = options.common;
    this.m_bReady = false;
    this.m_bCommonAuthReady = false;
    this.m_bCommonFooterReady = false;
    this.m_bCommonWsReady = false;
    this.m_letters = ['#', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'ALL'];
    this.m_noyes = [{name: 'No', value: '0'}, {name: 'Yes', value: '1'}];
    this.m_strApplication = null;
    this.c.attachEvent('commonAuthReady', (data) =>
    {
      this.m_bCommonAuthReady = true;
      this.ready();
    });
    this.c.attachEvent('commonFooterReady', (data) =>
    {
      this.m_bCommonFooterReady = true;
      this.ready();
    });
    this.c.attachEvent('commonWsReady_Central', (data) =>
    {
      this.m_bCommonWsReady = true;
      this.ready();
    });
    this.resetMenu();
    this.c.attachEvent('resetMenu', (data) =>
    {
      this.resetMenu();
    });
  }
  // }}}
  // {{{ ready()
  ready(response, error)
  {
    if (!this.m_bReady && this.m_bCommonAuthReady && this.m_bCommonWsReady)
    {
      this.m_bReady = true;
      this.c.dispatchEvent('appReady', null);
    }

    return this.m_bReady;
  }
  // }}}
  // {{{ resetMenu()
  resetMenu()
  {
    let unIndex, unSubIndex;
    this.c.clearMenu();
    this.c.menu = {left: [], right: []};
    unIndex = 0;
    this.c.menu.left[unIndex] = {value: 'Home', href: '/Home', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    unSubIndex = 0;
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'FrontDoor', href: '/Home/FrontDoor', active: null};
    unIndex++;
    this.c.menu.left[unIndex] = {value: 'Applications', href: '/Applications', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    unSubIndex = 0;
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Accounts', href: '/Applications/Accounts', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Issues', href: '/Applications/Issues', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Schedule', href: '/Applications/Schedule', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Statistics', href: '/Applications/Statistics', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Workload', href: '/Applications/Workload', active: null};
    unIndex++;
    this.c.menu.left[unIndex] = {value: 'Servers', href: '/Servers', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    unSubIndex = 0;
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Status', href: '/Servers/Status', active: null};
    unIndex++;
    this.c.menu.left[unIndex] = {value: 'Users', href: '/Users', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    this.c.resetMenu();
  }
  // }}}
  // {{{ setNoYes()
  setNoYes(item)
  {
    let bFound = false;;
    let nIndex = 0

    for (nIndex = 0; !bFound && nIndex < this.m_noyes.length; nIndex++)
    {
      if (item.value == this.m_noyes[nIndex].value)
      {
        bFound = true;
      }
    }
    nIndex--;
    if (!bFound)
    {
      nIndex = 0;
    }

    return this.m_noyes[nIndex];
  }
  // }}}
}
