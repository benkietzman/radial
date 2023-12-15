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
          let request = {Interface: 'irc', 'Function': 'chat', Target: s.target.v, Message: s.mess.v + ' - Sent by: ' + c.getUserFirstName() + ' ' + c.getUserLastName() + ' (' + c.getUserID() + ')'};
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
    if (!a.ready())
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
      s.u();
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
