///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2025-10-17
// copyright  : Ben Kietzman
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
    this.m_strApplication = 'Builder';
    this.m_strInterface = 'builder';
    this.c.attachEvent('commonAuthReady', (data) =>
    {
      this.m_bCommonAuthReady = true;
      this.ready();
    });
    this.c.attachEvent('commonWsReady_Builder', (data) =>
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
    let unIndex;
    this.c.clearMenu();
    this.c.menu = {left: [], right: []};
    unIndex = 0;
    this.c.menu.left[unIndex++] = {value: 'Home', href: '/Home', icon: 'house', active: null};
    if (this.c.isValid())
    {
      unIndex = 0;
      this.c.menu.right[unIndex++] = {value: 'Status', href: '/Status', icon: 'gear', active: null};
    }
    this.c.resetMenu();
  }
  // }}}
}
