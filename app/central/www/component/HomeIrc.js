// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-12-15
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
    let s = c.scope('HomeIrc',
    {
      // [[[ u()
      u: () =>
      {
        c.update('HomeIrc');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ changeChannels()
    s.changeChannels = () =>
    {
      s.target.v = '';
      for (let i = 0; i < s.selectChannels.v.length; i++)
      {
        if (i > 0)
        {
          s.target.v += ', ';
        }
        s.target.v += s.selectChannels.v[i];
      }
    };
    // ]]]
    // [[[ load()
    s.load = () =>
    {
      s.info.v = 'Retrieving channels...';
      let request = {Interface: 'irc', 'Function': 'channels'};
      c.wsRequest('radial', request).then((response) =>
      {
        let error = {};
        s.info.v = null;
        if (c.wsResponse(response, error))
        {
          s.channels = null;
          s.channels = [];
          for (let channel of Object.keys(response.Response))
          {
            s.channels.push({name: channel, present: response.Response[channel]});
          }
          s.u();
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
      s.u();
    };
    // ]]]
    // [[[ send()
    s.send = () =>
    {
      if (s.target.v)
      {
        if (s.mess.v)
        {
          let channels = s.target.v.split(',');
          for (let i = 0; i < channels.length; i++)
          {
            s.message.v = null;
            s.success.v = null;
            s.info.v = 'Sending chat...';
            let request = {Interface: 'irc', 'Function': 'chat', Target: channels[i].trim(), Message: '<ETX>08,03 ' + c.getUserFirstName() + ' ' + c.getUserLastName() + ' (' + c.getUserID() + ') <ETX> ' + s.mess.v};
            c.wsRequest('radial', request).then((response) =>
            {
              let error = {};
              s.info.v = null;
              if (c.wsResponse(response, error))
              {
                s.success.v = 'Successfully sent chat.';
              }
              else
              {
                c.pushErrorMessage(error.message);
              }
            });
          }
        }
        else
        {
          c.pushErrorMessage('Please provide the Message.');
        }
      }
      else
      {
        c.pushErrorMessage('Please provide the Target.');
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Home', 'IRC');
    s.u();
    if (a.ready())
    {
      s.load();
    }
    else
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.load();
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <h4 class="page-header">Send IRC Chat Message</h4>
  <div class="row">
    <div class="col-md-4">
      <p>
        Use the provided form to send a chat message to a given channel/user on IRC.  The message will be processed by the irc interface within Radial.
      </p>
      <p>
        List of all channels:
      </p>
      <select class="form-control" c-model="selectChannels" c-change="changeChannels()" size="2" style="height: 200px;" multiple>
        {{#each channels}}
        <option{{#if present}} class="text-success"{{/if}} value="{{name}}">{{name}}</option>
        {{/each}}
      </select>
      <p>
        <small>Chatbot resides in <span class="text-success">green</span> channels.</small>
      </p>
    </div>
    <div class="col-md-8">
      <p>
        You can send your message to any channel regardless of whether the radial chatbot resides in that channel.  You can send your message to any user, but if that user is not online at that time, the message will be undeliverable.
      </p>
      {{#isValid}}
      <div class="input-group"><span class="input-group-text">Channels/Users</span><input type="text" class="form-control" c-model="target"></div>
      <br>
      <textarea class="form-control" c-model="mess" placeholder="Type message here..."></textarea>
      <br>
      <button class="btn btn-success bi bi-send float-end" c-click="send()" title="Send"></button>
      {{else}}
      <b class="text-warning">Please login in order to use this form.</b>
      {{/isValid}}
      <div c-model="info" class="text-warning"></div>
      <div c-model="success" class="text-success"></div>
    </div>
  </div>
  `
  // ]]]
}
