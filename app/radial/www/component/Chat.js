// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-12-15
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
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]=
    // [[[ channel()
    s.channel = (strChannel) =>
    {
      s.target.v = strChannel;
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
          s.channels = response.Response;
          s.u();
        }
        else
        {
          s.message.v = error.message;
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
          s.message.v = null;
          s.success.v = null;
          s.info.v = 'Sending chat...';
          let request = {Interface: 'irc', 'Function': 'chat', Target: s.target.v, Message: '<ETX>08,03 ' + c.getUserFirstName() + ' ' + c.getUserLastName() + ' (' + c.getUserID() + ') <ETX> ' + s.mess.v};
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
              s.message.v = error.message;
            }
          });
        }
        else
        {
          s.message.v = 'Please provide the Message.';
        }
      }
      else
      {
        s.message.v = 'Please provide the Target.';
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Chat');
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
  <h4 class="page-header">Send Chat</h4>
  <div class="row">
    <div class="col-md-6">
      <p>
        Use the provided form to send a chat message to a given channel/user on IRC.  The message will be processed by the irc interface within Radial.
      </p>
      <p>
        You can send your chat to any channel regardless of whether the radial chatbot resides in that channel.  You can send your chat to any user, but if that user is not online at the time, the chat message will be undelivered.
      </p>
      <p>
        Here is a list of channels in which the radial chatbot currently resides:
      </p>
      <ul class="list-group">
        {{#each channels}}
        <button class="list-group-item btn btn-link" c-click="channel('{{.}}')" style="text-align: left;">{{.}}</button>
        {{/each}}
      </ul>
    </div>
    <div class="col-md-6">
      {{#isValid}}
      <div class="input-group"><span class="input-group-text">Channel/User</span><input type="text" class="form-control" c-model="target"></div>
      <br>
      <textarea class="form-control" c-model="mess" placeholder="Type message here..."></textarea>
      <br>
      <button class="btn btn-success bi bi-send float-end" c-click="send()" title="Send"></button>
      {{else}}
      <b class="text-warning">Please login in order to use this form.</b>
      {{/isValid}}
      <div c-model="info" class="text-warning"></div>
      <div c-model="message" class="text-danger"></div>
      <div c-model="success" class="text-success"></div>
    </div>
  </div>
  `
  // ]]]
}
