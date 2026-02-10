// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2026-02-05
// copyright  : Ben Kietzman
// email      : ben@kietzman.org
///////////////////////////////////////////
export default
{
  // [[[ controller()
  controller(id, nav)
  {
    // [[[ prep work
    let a = app;
    let c = common;
    let s = c.scope('Home',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Home');
        s.refresh();
      },
      // ]]]
      a: a,
      c: c,
      b: '',
      bAlign: false,
      bEscape: false,
      bg: 'black',
      bG0csi: false,
      bG1csi: false,
      bG2csi: false,
      bG3csi: false,
      bLoaded: false,
      bOsc: false,
      bSequence: false,
      fg: 'white',
      fw: 'normal',
      g: '',
      nState: 0,
      h: 24,
      m: '',
      n: '',
      r: '',
      s: [],
      t: '',
      w: 80,
      x: 0,
      y: 0
    });
    // ]]]
    // [[[ clear()
    s.clear = () =>
    {
      s.x = s.y = 0;
      for (let i = 0; i < s.h; i++)
      {
        for (let j = 0; j < s.w; j++)
        {
          s.s[i][j] = s.defaults();
        }
      }
    };
    // ]]]
    // [[[ defaults()
    s.defaults = () =>
    {
      return {background: s.bg, color: s.fg, value: ' ', weight: s.fw};
    };
    // ]]]
    // [[[ enter()
    s.enter = () =>
    {
      if (window.event.keyCode == 13)
      {
        s.launch();
      }
    };
    // ]]]
    // [[[ csi()
    s.csi = (d) =>
    {
      s.t += d;
      if (d == ';')
      {
        s.nState++;
      }
      else if (d == '?')
      {
      }
      else if (d >= '0' && d <= '9')
      {
        if (s.nState == 0)
        {
          s.n += d;
        }
        else if (s.nState == 1)
        {
          s.m += d;
        }
        else if (s.nState == 2)
        {
          s.r += d;
        }
        else if (s.nState == 3)
        {
          s.g += d;
        }
        else if (s.nState == 4)
        {
          s.b += d;
        }
      }
      else
      {
        let m = ((s.m != null)?Number(s.m):1);
        let n = ((s.n != null)?Number(s.n):1);
        switch (d)
        {
          // [[[ A - CUU - Cursor Up
          case 'A':
          {
            for (let i = 0; i < n; i++)
            {
              if (s.y > 0)
              {
                s.y--;
              }
            }
            break;
          }
          // ]]]
          // [[[ B - CUD - Cursor Down
          case 'B':
          {
            for (let i = 0; i < n; i++)
            {
              if (s.y < (s.h-1))
              {
                s.y++;
              }
            }
            break;
          }
          // ]]]
          // [[[ C - CUF - Cursor Forward
          case 'C':
          {
            for (let i = 0; i < n; i++)
            {
              if (s.x < (s.w-1))
              {
                s.x++;
              }
            }
            break;
          }
          // ]]]
          // [[[ D - CUB - Cursor Back
          case 'D':
          {
            for (let i = 0; i < n; i++)
            {
              if (s.x > 0)
              {
                s.x--;
              }
            }
            break;
          }
          // ]]]
          // [[[ E - CNL - Cursor Next Line
          case 'E':
          {
            for (let i = 0; i < n; i++)
            {
              if (s.y < (s.h-1))
              {
                s.y++;
                s.x = 0;
              }
            }
            break;
          }
          // ]]]
          // [[[ F - CPL - Cursor Previous Line
          case 'F':
          {
            for (let i = 0; i < n; i++)
            {
              if (s.y > 0)
              {
                s.y--;
                s.x = 0;
              }
            }
            break;
          }
          // ]]]
          // [[[ G - CHA - Cursor Horizontal Absolute
          case 'G':
          {
            s.x = n - 1;
            break;
          }
          // ]]]
          // [[[ H - CUP - Cursor Position
          case 'H':
          {
            if ((n-1) > 0 && (n-1) < s.h)
            {
              s.y = n - 1;
            }
            if ((m-1) > 0 && (m-1) < s.h)
            {
              s.x = m - 1;
            }
            break;
          }
          // ]]]
          // [[[ J - ED - Erase in Display
          case 'J':
          {
            if (s.n == '')
            {
              n = 0;
            }
            if (n == 0)
            {
              for (let i = s.x; i < s.w; i++)
              {
                s.s[s.y][i] = s.defaults();
              }
              for (let i = (s.y+1); i < s.h; i++)
              {
                for (let j = 0; j < s.w; j++)
                {
                  s.s[i][j] = s.defaults();
                }
              }
            }
            else if (n == 1)
            {
              for (let i = 0; i <= s.x; i++)
              {
                s.s[s.y][i] = s.defaults();
              }
              for (let i = 0; i < s.y; i++)
              {
                for (let j = 0; j < s.w; j++)
                {
                  s.s[i][j] = s.defaults();
                }
              }
            }
            else
            {
              for (let i = 0; i < s.h; i++)
              {
                for (let j = 0; j < s.w; j++)
                {
                  s.s[i][j] = s.defaults();
                }
              }
            }
            break;
          }
          // ]]]
          // [[[ K - EL - Erase in Line
          case 'K':
          {
            let x1 = 0, x2 = s.w;
            if (s.n == '')
            {
              n = 0;
            }
            if (n == 0)
            {
              x1 = s.x;
            }
            else if (n == 1)
            {
              x2 = s.x + 1;
            }
            for (let i = x1; i < x2; i++)
            {
              s.s[s.y][i] = s.defaults();
            }
            break;
          }
          // ]]]
          // [[[ S - SU - Scroll Up
          case 'S':
          {
            for (let i = 0; i < n; i++)
            {
              for (let j = 0; j < (s.y-1); j++)
              {
                for (let k = 0; k < s.w; k++)
                {
                  s.s[j][k].background = s.s[j+1][k].color;
                  s.s[j][k].color = s.s[j+1][k].color;
                  s.s[j][k].value = s.s[j+1][k].value;
                  s.s[j][k].weight = s.s[j+1][k].weight;
                }
              }
              for (let j = 0; j < s.w; j++)
              {
                s.s[s.y-1][j] = s.defaults();
              }
            }
            break;
          }
          // ]]]
          // [[[ T - SD - Scroll Down
          case 'T':
          {
            for (let i = 0; i < n; i++)
            {
              for (let j = 0; j < (s.y-1); j++)
              {
                for (let k = 0; k < s.w; k++)
                {
                  s.s[j+1][k].background = s.s[j][k].color;
                  s.s[j+1][k].color = s.s[j][k].color;
                  s.s[j+1][k].value = s.s[j][k].value;
                  s.s[j+1][k].weight = s.s[j][k].weight;
                }
              }
              for (let j = 0; j < s.w; j++)
              {
                s.s[0][j] = s.defaults();
              }
            }
            break;
          }
          // ]]]
          // [[[ f - HVP - Horizontal Vertical Position
          case 'f':
          {
            if ((n-1) > 0 && (n-1) < s.h)
            {
              s.y = n - 1;
            }
            if ((m-1) > 0 && (m-1) < s.h)
            {
              s.x = m - 1;
            }
            break;
          }
          // ]]]
          // [[[ m - SGR - Select Graphic Rendition
          case 'm':
          {
            if (s.n == '')
            {
              n = 0;
            }
            if (n == 38)
            {
              if (m == 2)
              {
                s.fg = 'rgb('+s.r+', '+s.g+', '+s.b+')';
              }
              else if (m == 5)
              {
                switch (Number(s.r))
                {
                  case 0: {s.fg = 'black'; break;}
                  case 1: {s.fg = 'darkred'; break;}
                  case 2: {s.fg = 'darkgreen'; break;}
                  case 3: {s.fg = 'darkyellow'; break;}
                  case 4: {s.fg = 'darkblue'; break;}
                  case 5: {s.fg = 'darkmagenta'; break;}
                  case 6: {s.fg = 'darkcyan'; break;}
                  case 7: {s.fg = 'lightgray'; break;}
                  case 8: {s.fg = 'gray'; break;}
                  case 9: {s.fg = 'red'; break;}
                  case 10: {s.fg = 'green'; break;}
                  case 11: {s.fg = 'yellow'; break;}
                  case 12: {s.fg = 'blue'; break;}
                  case 13: {s.fg = 'magenta'; break;}
                  case 14: {s.fg = 'cyan'; break;}
                  case 15: {s.fg = 'white'; break;}
                }
              }
            }
            else if (n == 48)
            {
              if (m == 2)
              {
                s.bg = 'rgb('+s.r+', '+s.g+', '+s.b+')';
              }
              else if (m == 5)
              {
                switch (Number(s.r))
                {
                  case 0: {s.bg = 'black'; break;}
                  case 1: {s.bg = 'darkred'; break;}
                  case 2: {s.bg = 'darkgreen'; break;}
                  case 3: {s.bg = 'darkyellow'; break;}
                  case 4: {s.bg = 'darkblue'; break;}
                  case 5: {s.bg = 'darkmagenta'; break;}
                  case 6: {s.bg = 'darkcyan'; break;}
                  case 7: {s.bg = 'lightgray'; break;}
                  case 8: {s.bg = 'gray'; break;}
                  case 9: {s.bg = 'red'; break;}
                  case 10: {s.bg = 'green'; break;}
                  case 11: {s.bg = 'yellow'; break;}
                  case 12: {s.bg = 'blue'; break;}
                  case 13: {s.bg = 'magenta'; break;}
                  case 14: {s.bg = 'cyan'; break;}
                  case 15: {s.bg = 'white'; break;}
                }
              }
            }
            else
            {
              let codes = [];
              codes.push(n);
              if (s.m != '')
              {
                codes.push(m);
              }
              for (let i = 0; i < codes.length; i++)
              {
                switch (codes[i])
                {
                  case 0: {s.bg = 'black'; s.fg = 'white'; s.fw = 'normal'; break;}
                  case 1: {s.fw = 'bold'; break;}
                  case 2: {s.fw = 'normal'; break;}
                  case 22: {s.fw = 'normal';}
                  case 30: {s.fg = 'black'; break;}
                  case 31: {s.fg = 'darkred'; break;}
                  case 32: {s.fg = 'darkgreen'; break;}
                  case 33: {s.fg = 'darkyellow'; break;}
                  case 34: {s.fg = 'darkblue'; break;}
                  case 35: {s.fg = 'darkmagenta'; break;}
                  case 36: {s.fg = 'darkcyan'; break;}
                  case 37: {s.fg = 'lightgray'; break;}
                  case 39: {s.fg = 'white'; break;}
                  case 40: {s.bg = 'black'; break;}
                  case 41: {s.bg = 'darkred'; break;}
                  case 42: {s.bg = 'darkgreen'; break;}
                  case 43: {s.bg = 'darkyellow'; break;}
                  case 44: {s.bg = 'darkblue'; break;}
                  case 45: {s.bg = 'darkmagenta'; break;}
                  case 46: {s.bg = 'darkcyan'; break;}
                  case 47: {s.bg = 'lightgray'; break;}
                  case 49: {s.bg = 'black'; break;}
                  case 90: {s.fg = 'gray'; break;}
                  case 91: {s.fg = 'red'; break;}
                  case 92: {s.fg = 'green'; break;}
                  case 93: {s.fg = 'yellow'; break;}
                  case 94: {s.fg = 'blue'; break;}
                  case 95: {s.fg = 'magenta'; break;}
                  case 96: {s.fg = 'cyan'; break;}
                  case 97: {s.fg = 'white'; break;}
                  case 100: {s.bg = 'gray'; break;}
                  case 101: {s.bg = 'red'; break;}
                  case 102: {s.bg = 'green'; break;}
                  case 103: {s.bg = 'yellow'; break;}
                  case 104: {s.bg = 'blue'; break;}
                  case 105: {s.bg = 'magenta'; break;}
                  case 106: {s.bg = 'cyan'; break;}
                  case 107: {s.bg = 'white'; break;}
                }
              }
            }
            break;
          }
          // ]]]
        }
        s.b = '';
        s.bCsi = false;
        s.bEscape = false;
        s.g = '';
        s.m = '';
        s.n = '';
        s.nState = 0;
        s.r = '';
        console.log(s.t);
        s.t = '';
      }
    };
    // ]]]
    // [[[ esc()
    s.esc = (d) =>
    {
      if (s.bCsi)
      {
        s.csi(d);
      }
      else if (s.bAlign)
      {
        s.bEscape = false;
        s.bAlign = false;
      }
      else if (s.bG0cs)
      {
        s.bEscape = false;
        s.bG0cs = false;
      }
      else if (s.bG1cs)
      {
        s.bEscape = false;
        s.bG1cs = false;
      }
      else if (s.bG2cs)
      {
        s.bEscape = false;
        s.bG2cs = false;
      }
      else if (s.bG3cs)
      {
        s.bEscape = false;
        s.bG3cs = false;
      }
      else if (s.bOsc)
      {
        s.bEscape = false;
        s.bOsc = false;
      }
      else if (s.bSequence)
      {
        s.bEscape = false;
        s.bSequence = false;
      }
      else if (d == '[') // CSI - Control Sequence Introducer prefix
      {
        s.bCsi = true;
      }
      else if (d == 'c') // RIS - Reset
      {
        s.clear();
        s.bEscape = false;
      }
      else if (d == 'D') // IND - Linefeed
      {
        s.y++;
        s.bEscape = false;
      }
      else if (d == 'E') // NEL - Newline
      {
        s.bEscape = false;
      }
      else if (d == 'H') // HTS - Set tab stop at current column
      {
        s.bEscape = false;
      }
      else if (d == 'M') // RI - Reverse linefeed
      {
        s.y--;
        s.bEscape = false;
      }
      else if (d == 'N') // SS2 - Single Shift Two
      {
        s.bEscape = false;
      }
      else if (d == 'O') // SS3 - Single Shift Three
      {
        s.bEscape = false;
      }
      else if (d == 'P') // DSC - Device Control String
      {
        s.bEscape = false;
      }
      else if (d == 'X') // SOS - Start of String
      {
        s.bEscape = false;
      }
      else if (d == 'Z') // DECID - DEC private identification
      {
        s.bEscape = false;
      }
      else if (d == '7') // DECSC - Save current state
      {
        s.bEscape = false;
      }
      else if (d == '8') // DECRC - Restore state most currently saved
      {
        s.bEscape = false;
      }
      else if (d == '\\') // ST - String Terminator
      {
        s.bEscape = false;
      }
      else if (d == '^') // PM - Private Message
      {
        s.bEscape = false;
      }
      else if (d == '_') // APC - Application Program Command
      {
        s.bEscape = false;
      }
      else if (d == '%') // Start sequence selecting
      {
        s.bSequence = true;
      }
      else if (d == '#') // DECALN - DEC screen alignment test
      {
        s.bAlign = true;
      }
      else if (d == '(') // Start sequence defining G0 character set
      {
        s.bG0cs = true;
      }
      else if (d == ')') // Start sequence defining G1 character set
      {
        s.bG1cs = true;
      }
      else if (d == '*') // Start sequence defining G2 character set
      {
        s.bG2cs = true;
      }
      else if (d == '+') // Start sequence defining G3 character set
      {
        s.bG3cs = true;
      }
      else if (d == '>') // DECPNM - Set numeric keypad mode
      {
        s.bEscape = false;
      }
      else if (d == '=') // DECPAM - Set application keypad mode
      {
        s.bEscape = false;
      }
      else if (d == ']') // OSC - Operating System Command prefix
      {
        s.bOsc = true;
      }
    };
    // ]]]
    // [[[ identify()
    s.identify = (d) =>
    {
      let n = d.charCodeAt(d);
//console.log(d+'('+n+')');
      if (s.bEscape)
      {
        s.esc(d);
      }
      else if (n == 27)
      {
        s.bEscape = true;
      }
      else
      {
        s.process(d);
      }
    };
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      for (let i = 0; i < s.h; i++)
      {
        let d = [];
        for (let j = 0; j < s.w; j++)
        {
          d.push(s.defaults());
        }
        s.s.push(d);
      }
      s.u();
    };
    // ]]]
    // [[[ launch()
    s.launch = (strServer) =>
    {
      let request = {Interface: 'emulator', 'Function': 'launch', Request: {Command: s.command.v}};
      s.clear();
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        if (!c.wsResponse(response, error))
        {
          c.pushErrorMessage(error.message);
        }
        s.u();
      });
    };
    // ]]]
    // [[[ process()
    s.process = (d) =>
    {
      let b = false;
      if (d == '\n' || d == '\r' || s.x >= s.w)
      {
        if (s.x >= s.w)
        {
          b = true;
        }
        if (d == '\n' || s.x >= s.w)
        {
          s.y++;
        }
        s.x = 0;
      }
      else if (d != '\a' && d != '\b' && d != '\t' && d != '\f')
      {
        b = true;
      }
      if (s.y >= s.h)
      {
        for (let j = 0; j < (s.y-1); j++)
        {
          for (let k = 0; k < s.w; k++)
          {
            s.s[j][k].background = s.s[j+1][k].color;
            s.s[j][k].color = s.s[j+1][k].color;
            s.s[j][k].value = s.s[j+1][k].value;
            s.s[j][k].weight = s.s[j+1][k].weight;
          }
        }
        for (let k = 0; k < s.w; k++)
        {
          s.s[s.y-1][k] = s.defaults();
        }
        s.y--;
      }
      if (b)
      {
        s.s[s.y][s.x] = s.defaults();
        s.s[s.y][s.x].value = d;
        s.x++;
      }
    };
    // ]]]
    // [[[ refresh()
    s.refresh = () =>
    {
      for (let i = 0; i < s.h; i++)
      {
        for (let j = 0; j < s.w; j++)
        {
          let e = document.getElementById('s_'+i+'_'+j);
          if (e)
          {
            e.innerHTML = s.s[i][j].value;
            e.style.backgroundColor = s.s[i][j].background;
            e.style.color = s.s[i][j].color;
            e.style.fontWeight = s.s[i][j].weight;
          }
        }
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Home');
    s.u();
    if (a.ready())
    {
      s.init();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.init();
      s.u();
    });
    c.attachEvent('commonWsMessage_Emulator', (data) =>
    {
      if (data.detail && data.detail.Action && data.detail.Action == 'data' && data.detail.Data)
      {
        let d = atob(data.detail.Data);
        for (let i = 0; i < d.length; i++)
        {
          s.identify(d[i]);
        }
        s.refresh();
      }
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div c-model="info" class="text-warning"></div>
  {{#isValid "Emulator"}}
  <div class="row">
    <div class="col-auto">
    <div class="input-group"><span class="input-group-text">Command</span><input type="text" class="form-control" c-keyup="enter()" c-model="command"><button class="btn btn-success" c-click="launch()">Launch</button></div>
    </div>
  </div>
  <div class="row">
    <div class="col-auto">
      <table class="table table-condensed" style="margin: 10px 0px 0px 0px; padding: 0px;">
      <tbody>
        {{#each ../s}}
        <tr style="margin: 0px; padding: 0px;">
          {{#each .}}
          <td style="margin: 0px; padding: 0px;"><pre id="s_{{@../index}}_{{@index}}" style="font-family: monospace; margin: 0px; padding: 0px;"></pre></td>
          {{/each}}
        </tr>
        {{/each}}
      </tbody>
      </table>
    </div>
  </div>
  {{else}}
  {{#if ../bLoaded}}
  {{^isValid}}
  <p class="fw-bold text-danger">Please login to use this application.</p>
  {{/isValid}}
  <p class="fw-bold text-danger">You must be registered as a contact for the Emulator application in Central.</p>
  {{/if}}
  {{/isValid}}
  `
  // ]]]
}
