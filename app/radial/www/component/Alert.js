// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-09-25
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
    let s = c.scope('Alert',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Alert');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]=
    // [[[ send()
    s.send = () =>
    {
      if (s.user.v)
      {
        if (s.mess.v)
        {
          s.message.v = null;
          s.success.v = null;
          s.info.v = 'Sending alert...';
          let request = {Interface: 'alert', Request: {User: s.user.v, Message: s.mess.v + ' - Sent by: ' + c.getUserFirstName() + ' ' + c.getUserLastName() + ' (' + c.getUserID() + ')'}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              s.success.v = 'The alert has been sent.';
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
        s.message.v = 'Please provide the User.';
      }
    };
    // ]]]
    // [[[ main
    c.setMenu('Alert');
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
  <h4 class="page-header">Send Alert Message</h4>
  <div class="row">
    <div class="col-md-6">
      <p>
        Use the provided form to send an alert message to a given user.  The message will be processed by the alert interface within Radial.
      </p>
      <p>
        That interface will attempt to send to the following devices:
      </p>
      <ul>
        <li><b>chat</b>:  Sends a private chat message to the user on IRC.</li>
        <li><b>email</b>:  Sends an email to the email address defined within Central.</li>
        <li><b>live</b>:  Displays a notification box to the user on any website utilizing the Common framework for which the user is logged into that website.
        <li><b>text</b>:  Sends an email to the pager address defined within Central.</li>
      </ul>
    </div>
    <div class="col-md-6">
      {{#isValid}}
      <div class="input-group"><span class="input-group-text">User</span><input type="text" class="form-control" c-model="user"></div>
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
