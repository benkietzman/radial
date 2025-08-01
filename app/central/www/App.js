///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-07-12
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
class App
{
  // {{{ constructor()
  constructor(options)
  {
    this.c = options.common;
    this.j = options.junction;
    this.m_bReady = false;
    this.m_bCommonAuthReady = false;
    this.m_bCommonFooterReady = false;
    this.m_bCommonWsReady = false;
    this.m_letters = ['#', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'ALL'];
    this.m_noyes = [{name: 'No', value: 0}, {name: 'Yes', value: 1}];
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
    Handlebars.registerHelper('statusShowRestart', (nodes, options) =>
    {
      let bShow = false;
      for (let n of Object.keys(nodes))
      {
        if (nodes[n].PID)
        {
          bShow = true;
        }
      }
      if (bShow)
      {
        return options.fn(this);
      }
      else
      {
        return options.inverse(this);
      }
    });
    Handlebars.registerHelper('statusShowStart', (nodes, options) =>
    {
      let bShow = false;
      for (let n of Object.keys(nodes))
      {
        if (!nodes[n].PID)
        {
          bShow = true;
        }
      }
      if (bShow)
      {
        return options.fn(this);
      }
      else
      {
        return options.inverse(this);
      }
    });
    Handlebars.registerHelper('statusShowStop', (nodes, options) =>
    {
      let bShow = false;
      for (let n of Object.keys(nodes))
      {
        if (nodes[n].PID)
        {
          bShow = true;
        }
      }
      if (bShow)
      {
        return options.fn(this);
      }
      else
      {
        return options.inverse(this);
      }
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
    this.c.menu.left[unIndex] = {value: 'Home', href: '/Home', icon: 'house', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    unSubIndex = 0;
    if (this.c.isValid())
    {
      this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'IRC', href: '/Home/Irc', icon: 'chat-left', active: null};
      this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Issues', href: '/Home/Issues', icon: 'ticket', active: null};
      this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Live', href: '/Home/Live', icon: 'lightning', active: null};
      this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Notify', href: '/Home/Notify', icon: 'send', active: null};
    }
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Samba', href: '/Home/Samba', icon: 'hdd-network', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'SSH', href: '/Home/Ssh', icon: 'terminal', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Terminal', href: '/Home/Terminal', icon: 'terminal', active: null};
    unIndex++;
    this.c.menu.left[unIndex] = {value: 'Applications', href: '/Applications', icon: 'app', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    unSubIndex = 0;
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Accounts', href: '/Applications/Accounts', icon: 'wallet2', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Issues', href: '/Applications/Issues', icon: 'ticket', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Repositories', href: '/Applications/Repositories', icon: 'inbox', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Schedule', href: '/Applications/Schedule', icon: 'calendar', active: null};
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Statistics', href: '/Applications/Statistics', icon: 'graph-up', active: null};
    if (this.c.isValid())
    {
      this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Workload', href: '/Applications/Workload', icon: 'person-workspace', active: null};
    }
    unIndex++;
    this.c.menu.left[unIndex] = {value: 'Groups', href: '/Groups', icon: 'people', active: null};
    unIndex++;
    this.c.menu.left[unIndex] = {value: 'Servers', href: '/Servers', icon: 'server', active: null};
    this.c.menu.left[unIndex].submenu = {left: [], right: []};
    unSubIndex = 0;
    this.c.menu.left[unIndex].submenu.left[unSubIndex++] = {value: 'Status', href: '/Servers/Status', icon: 'stopwatch', active: null};
    unIndex++;
    this.c.menu.left[unIndex] = {value: 'Users', href: '/Users', icon: 'people', active: null};
    if (this.c.isValid())
    {
      unIndex = 0;
      this.c.menu.right[unIndex++] = {value: 'Chat', href: '/Chat', icon: 'chat', active: null};
      this.c.menu.right[unIndex++] = {value: 'Profile', href: '/Users/?userid='+this.c.getUserID(), icon: 'person', active: null};
      this.c.menu.right[unIndex++] = {value: 'Status', href: '/Status', icon: 'gear', active: null};
    }
    this.c.resetMenu();
  }
  // }}}
  // {{{ setNoYes()
  setNoYes(item)
  {
    let bFound = false;
    let nIndex = 0;

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
