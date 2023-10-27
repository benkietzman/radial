///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-10-23
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
    this.m_bCommonWsReady = false;
    this.m_strApplication = null;
    this.c.attachEvent('commonAuthReady', (data) =>
    {
      this.m_bCommonAuthReady = true;
      this.ready();
    });
    this.c.attachEvent('commonWsReady_Radial', (data) =>
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
    let unIndex;
    this.c.clearMenu();
    this.c.menu = {left: [], right: []};
    unIndex = 0;
    this.c.menu.left[unIndex++] = {value: 'Home', href: '/Home', icon: 'house', active: null};
    this.c.menu.left[unIndex++] = {value: 'Alert', href: '/Alert', icon: 'megaphone', active: null};
    unIndex = 0;
    this.c.menu.right[unIndex++] = {value: 'Status', href: '/Status', icon: 'gear', active: null};
    this.c.resetMenu();
  }
  // }}}
}
