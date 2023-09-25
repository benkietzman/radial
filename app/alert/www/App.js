// vim600: fdm=marker
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-09-25
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
    this.c.attachEvent('commonAuthReady', (data) =>
    {
      this.m_bCommonAuthReady = true;
      this.ready();
    });
    this.c.attachEvent('commonWsReady_Alert', (data) =>
    {
      this.m_bCommonWsReady = true;
      this.ready();
    });
    this.resetMenu();
    common.attachEvent('resetMenu', (data) =>
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
    this.c.clearMenu();
    this.c.resetMenu();
  }
  // }}}
}
