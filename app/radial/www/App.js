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
    Handlebars.registerHelper('showRestart', (nodes, options) =>
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
    Handlebars.registerHelper('showStart', (nodes, options) =>
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
    Handlebars.registerHelper('showStop', (nodes, options) =>
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
    this.c.clearMenu();
    this.c.resetMenu();
  }
  // }}}
}
