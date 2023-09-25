///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-08-08
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
    this.c.attachEvent('commonWsReady_Feedback', (data) =>
    {
      this.m_bCommonWsReady = true;
      this.ready();
    });
    this.resetMenu();
    this.c.attachEvent('resetMenu', (data) =>
    {
      this.resetMenu();
    });
    Handlebars.registerHelper('haveDate', (val, options) =>
    {
      if (val && val != '0000-00-00 00:00:00')
      {
        return options.fn(this);
      }
      else
      {
        return options.inverse(this);
      }
    });
    Handlebars.registerHelper('showClosedOrOpen', (showClosed, open, options) =>
    {
      if (showClosed || open == 1)
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
    this.c.clearMenu();
    this.c.resetMenu();
  }
  // }}}
}
