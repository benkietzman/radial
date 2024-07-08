// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2024-07-08
// copyright  : kietzman.org
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
    let s = c.scope('Chat',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Chat');
        s.resize();
      },
      // ]]]
      a: a,
      c: c,
      blurred: false,
      histories: {},
      history: [],
      message: null,
      user: new Observable,
      users: {}
    });
    // ]]]
    // [[[ chat()
    s.chat = () =>
    {
      if (c.isValid() && s.user.v)
      {
        let message = {Action: 'chat', Message: s.message.v, User: c.getUserID(), FirstName: c.getUserFirstName(), LastName: c.getUserLastName()};
        if (!s.histories[s.user.v])
        { 
          s.histories[s.user.v] = [];
        } 
        s.histories[s.user.v].push(message);
        while (s.histories[s.user.v].length > 50)
        {
          s.histories[s.user.v].shift();
        } 
        s.history = s.histories[s.user.v];
        let request = {Interface: 'live', 'Function': 'message', Request: {User: s.user.v, Message: message}};
        s.message.v = null;
        c.wsRequest(c.m_strAuthProtocol, request).then((response) => {});
        s.u();
        document.getElementById('radial_history').scrollTop = document.getElementById('radial_history').scrollHeight;
        document.getElementById('radial_message').focus();
      }       
    };        
    // ]]]  
    // [[[ convert()
    s.convert = (m) =>
    {     
      let b = false;
      let h = '';
      let i = false;
      let u = false;
              
      for (let n = 0; n < m.length; n++)
      {
        switch (m[n])
        {
          case '\u0002':
          {
            if (b)
            {
              b = false;
              h += '</b>';
            }
            else
            {
              b = true;
              h += '<b>';
            }
            break;
          }
          case '\u0003':
          {
            if (isFinite(m.substr((n + 1), 2)) && m.substr((n + 1), 2) != '  ')
            {
              h += '<span style="color:';
              switch (m.substr((n + 1), 2))
              {
                case '00': h += 'white'; break;
                case '01': h += 'black'; break;
                case '02': h += 'blue'; break;
                case '03': h += 'green'; break;
                case '04': h += 'red'; break;
                case '05': h += 'brown'; break;
                case '06': h += 'magenta'; break;
                case '07': h += 'orange'; break;
                case '08': h += 'yellow'; break;
                case '09': h += 'light-green'; break;
                case '10': h += 'cyan'; break;
                case '11': h += 'light-cyan'; break;
                case '12': h += 'light-blue'; break;
                case '13': h += 'pink'; break;
                case '14': h += 'grey'; break;
                case '15': h += 'light-grey'; break;
              }
              h += ';';
              if (m.substr((n + 3), 1) == ',' && isFinite(m.substr((n + 4), 2)) && m.substr((n + 4), 2) != '  ')
              {
                h += ' background:';
                switch (m.substr((n + 4), 2))
                {
                  case '00': h += 'white'; break;
                  case '01': h += 'black'; break;
                  case '02': h += 'blue'; break;
                  case '03': h += 'green'; break;
                  case '04': h += 'red'; break;
                  case '05': h += 'brown'; break;
                  case '06': h += 'magenta'; break;
                  case '07': h += 'orange'; break;
                  case '08': h += 'yellow'; break;
                  case '09': h += 'light-green'; break;
                  case '10': h += 'cyan'; break;
                  case '11': h += 'light-cyan'; break;
                  case '12': h += 'light-blue'; break;
                  case '13': h += 'pink'; break;
                  case '14': h += 'grey'; break;
                  case '15': h += 'light-grey'; break;
                }
                h += ';';
                n += 3;
              }
              h += ' border-radius: 10px; border-style:solid; border-width:0px;">';
              n += 2;
            }
            else
            {
              h += '</span>';
            }
            break;
          }
          case '\u001D':
          {
            if (i)
            {
              i = false;
              h += '</i>';
            }
            else
            {
              i = true;
              h += '<i>';
            }
            break;
          }
          case '\u001F':
          {
            if (u)
            {
              u = false;
              h += '</u>';
            }
            else
            {
              u = true;
              h += '<u>';
            }
            break;
          }
          default:
          {
            h += m[n];
          }
        }
      }

      return h;
    };
    // ]]]
    // [[[ enter()
    s.enter = () =>
    {
      if (window.event.keyCode == 13)
      {
        s.chat();
      }
    };
    // ]]]
    // [[[ hist()
    s.hist = () =>
    {
      if (!s.histories[s.user.v])
      {
        s.histories[s.user.v] = [];
      }
      s.history = s.histories[s.user.v];
      s.users[s.user.v].unread = 0;
      s.u();
      document.getElementById('radial_message').focus();
      document.getElementById('radial_history').scrollTop = document.getElementById('radial_history').scrollHeight;
    };
    // ]]]
    // [[[ init()
    s.init = () =>
    {
      c.attachEvent('commonWsMessage_'+c.application, (data) =>
      {
        if (c.isDefined(data.detail) && c.isDefined(data.detail.Action) && c.isDefined(data.detail.User))
        {
          // [[[ chat
          if (data.detail.Action == 'chat' && c.isDefined(data.detail.Message))
          {
            let bFound = false;
            if (!s.users[data.detail.User])
            {
              s.users[data.detail.User] = {icon: '&#x0001F4A1;', sessions: [], unread: 0};
            }
            s.users[data.detail.User].connected = true;
            if (c.isDefined(s.users[data.detail.User].FirstName))
            {
              data.detail.FirstName = s.users[data.detail.User].FirstName;
            }
            if (c.isDefined(s.users[data.detail.User].LastName))
            {
              data.detail.LasName = s.users[data.detail.User].LastName;
            }
            if (c.isDefined(data.detail.FirstName))
            {
              s.users[data.detail.User].FirstName = data.detail.FirstName;
            }
            if (c.isDefined(data.detail.LastName))
            {
              s.users[data.detail.User].LastName = data.detail.LastName;
            }
            s.sort();
            for (let i = 0; !bFound && i < s.users[data.detail.User].sessions.length; i++)
            {
              if (s.users[data.detail.User].sessions[i] == data.detail.wsRequestID)
              {
                bFound = true;
              }
            }
            if (!bFound)
            {
              s.users[data.detail.User].sessions.push(data.detail.wsRequestID);
            }
            if (s.user.v != data.detail.User)
            {
              s.users[data.detail.User].unread++;
            }
            if (!s.histories[data.detail.User])
            {
              s.histories[data.detail.User] = [];
            }
            data.detail.Message = s.convert(data.detail.Message);
            s.histories[data.detail.User].push(data.detail);
            while (s.histories[data.detail.User].length > 50)
            {
              s.histories[data.detail.User].shift();
            }
            s.u();
            document.getElementById('radial_message').focus();
            document.getElementById('radial_history').scrollTop = document.getElementById('radial_history').scrollHeight;
            if (s.blurred && 'Notification' in window)
            {
              Notification.requestPermission((permission) =>
              {
                let strTitle = 'Radial Chat - ';
                if (c.isDefined(data.detail.LastName))
                {
                  strTitle += data.detail.LastName + ', ';
                }
                if (c.isDefined(data.detail.FirstName))
                {
                  strTitle += data.detail.FirstName;
                }
                strTitle += ' (' + data.detail.User + ')';
                let notification = new Notification(strTitle, {body: data.detail.Message, dir: 'auto'});
                setTimeout(() =>
                {
                  notification.close();
                }, 5000);
              });
            }
          }
          // ]]]
          // [[[ connect
          else if (data.detail.Action == 'connect')
          {
            if (c.getUserID() != data.detail.User)
            {
              if (!s.users[data.detail.User])
              {
                s.users[data.detail.User] = {icon: '&#x0001F4A1;', sessions: [], unread: 0};
              }
              s.users[data.detail.User].connected = true;
              if (c.isDefined(data.detail.FirstName))
              {
                s.users[data.detail.User].FirstName = data.detail.FirstName;
              }
              if (c.isDefined(data.detail.LastName))
              {
                s.users[data.detail.User].LastName = data.detail.LastName;
              }
              s.sort();
              s.users[data.detail.User].sessions.push(data.detail.wsRequestID);
              s.u();
            }
          }
          // ]]]
          // [[[ disconnect
          else if (data.detail.Action == 'disconnect')
          {
            if (c.getUserID() != data.detail.User && s.users[data.detail.User])
            {
              let sessions = [];
              for (let i = 0; i < s.users[data.detail.User].length; i++)
              {
                if (s.users[data.detail.User].sessions[i] != data.detail.wsRequestID)
                {
                  sessions.push(s.users[data.detail.User].sessions[i]);
                }
              }
              if (sessions.length > 0)
              {
                s.users[data.detail.User].sessions = null;
                s.users[data.detail.User].sessions = sessions;
              }
              else
              {
                s.users[data.detail.User].connected = false;
              }
              s.u();
              document.getElementById('radial_message').focus();
              document.getElementById('radial_history').scrollTop = document.getElementById('radial_history').scrollHeight;
            }
          }
          // ]]]
        }
      });
      s.users = null;
      s.users = {'radial_bot': {connected: true, FirstName: 'Radial', icon: '&#x0001F916;', sessions: [], unread: 0}};
      let request = {Interface: 'live', 'Function': 'list', Request: {Scope: 'all'}};
      c.wsRequest(c.m_strAuthProtocol, request).then((response) =>
      {
        let error = {};
        if (c.wsResponse(response, error) && c.isDefined(response.Response))
        {
          for (let [strSession, data] of Object.entries(response.Response))
          {
            if (c.isDefined(data.User) && c.getUserID() != data.User)
            {
              if (!s.users[data.User])
              {
                s.users[data.User] = {connected: true, FirstName: data.FirstName, LastName: data.LastName, icon: '&#x0001F4A1;', sessions: [], unread: 0};
              }
              s.users[data.User].sessions.push(data.wsRequestID);
            }
          }
          s.sort();
          s.u();
        }
      });
    };
    // ]]]
    // [[[ resize()
    s.resize = () =>
    {
      let history = document.getElementById('radial_history');
      let maxHeight = document.documentElement.clientHeight - 240;
      history.style.minHeight = maxHeight + 'px';
      history.style.maxHeight = maxHeight + 'px';
      history.scrollTop = history.scrollHeight;
      document.getElementById('radial_message').focus();
    };
    // ]]]
    // [[[ sort()
    s.sort = () =>
    {
      for (let key of Object.keys(s.users))
      {
        s.users[key].display = '';
        if (c.isDefined(s.users[key].LastName))
        {
          s.users[key].display = s.users[key].LastName + ', ';
        }
        if (c.isDefined(s.users[key].FirstName))
        {
          s.users[key].display += s.users[key].FirstName + ' ';
        }
        s.users[key].display += '(' + key + ')';
      }
      var arr = [];
      for (let key in s.users)
      {
        let obj = {};
        obj[key] = s.users[key];
        obj[key].User = key;
        obj.tempSortName = s.users[key].display.toLowerCase();
        arr.push(obj);
      }
      arr.sort((a, b) =>
      {
        let at = a.tempSortName, bt = b.tempSortName;
        return ((at > bt)?1:((at < bt)?-1:0));
      });
      let result = [];
      for (let i = 0, l = arr.length; i < l; i++)
      {
        let id = null;
        let obj = arr[i];
        delete obj.tempSortName;
        for (let key in obj)
        {
          if (obj.hasOwnProperty(key))
          {
            id = key;
          }
        }
        let item = obj[id];
        result.push(item);
      }
      s.users = null;
      s.users = {};
      for (let i = 0; i < result.length; i++)
      {
        s.users[result[i].User] = result[i];
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Chat');
    c.attachEvent('commonWsConnected', (data) =>
    {
      s.init();
    });
    window.addEventListener('resize', () =>
    {
      s.resize();
    });
    window.onblur = () =>
    {
      s.blurred = true;
    };
    window.onfocus = () =>
    {
      s.blurred = false;
    };
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
    {{#isValid}}
    <select class="form-select form-select-sm" c-model="user" c-change="hist()" style="font-family: monospace, monospace; margin-bottom: 10px;">
      {{#each @root.users}}
      <option value="{{@key}}"{{#if unread}} class="bg-warning"{{/if}}>{{#if connected}}{{{icon}}}{{else}}&nbsp;{{/if}}&nbsp;{{#if LastName}}{{LastName}}, {{/if}}{{FirstName}} ({{@key}}){{#if unread}} [{{unread}}]{{/if}}</option>
      {{/each}}
    </select>
    <div class="card card-body card-inverse table-responsive" id="radial_history" style="padding: 0px;">
      <table class="table table-condensed table-striped" style="margin: 0px;">
        {{#each @root.history}}
        <tr>
          <td><pre style="background: inherit; color: inherit; margin: 0px; white-space: pre-wrap;">{{#if FirstName}}{{FirstName}}{{else}}{{User}}{{/if}}</pre></td>
          <td><pre style="background: inherit; color: inherit; margin: 0px; white-space: pre-wrap;">{{{Message}}}</pre></td>
        </tr>
        {{/each}}
      </table>
    </div>
    <input type="text" class="form-control form-control-sm" id="radial_message" c-model="message" c-keyup="enter()" placeholder="Type message and hit enter..." style="font-family: monospace, monospace; margin-top: 10px;">
    {{/isValid}}
  `
  // ]]]
}
