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
    let a = app;
    let c = common;
    let s = c.scope('index',
    {
      // [[[ u()
      u: () =>
      {
        c.update('index');
      },
      // ]]]
      a: a,
      c: c
    });
    // [[[ send()
    s.send = () =>
    {
      if (s.user.v)
      {
        if (s.message.v)
        {
          s.info.v = 'Sending alert...';
          let request = {Interface: 'alert', Request: {User: s.user.v, Message: s.message.v + ' - Sent by: ' + c.getUserFirstName() + ' ' + c.getUserLastName() + ' (' + c.getUserID() + ')'}};
          c.wsRequest('radial', request).then((response) =>
          {
            let error = {};
            s.info.v = null;
            if (c.wsResponse(response, error))
            {
              alert('The alert has been sent.');
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
  },
  // ]]]
  // [[[ template
  template: `
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
  <h3 class="page-header">Send Alert Message</h3>
  <div class="row">
    <div class="col-md-6">
      <p>
        Use the form on the right in order to send an alert message to the given user.  The alert will be sent through and processed by the alert interface within Radial.
      </p>
      <p>
        That interface will attempt to send the alert to the following devices:
      </p>
      <ul>
        <li><b>email</b>:  Sends an email to the email address defined within Central.</li>
        <li><b>chat</b>:  Sends a private chat message to the user on IRC.</li>
        <li><b>live</b>:  Displays a notification box to the user on any website utilizing the Common framework for which the user is logged into that website.
        <li><b>text</b>:  Sends an email to the pager address defined within Central.</li>
      </ul>
    </div>
    <div class="col-md-6">
      {{#isValid}}
      <div class="form-group"><div class="input-group"><input type="text" class="form-control" c-model="user" placeholder="User" value="User"></div></div>
      <div class="form-group"><div class="input-group"><textarea class="form-control" c-model="message" placeholder="Type message here..."></textarea></div></div>
      <button class="btn btn-success float-end" c-click="send()">Send</button>
      {{else}}
      <b class="text-warning">Please login in order to use this form.</b>
      {{/isValid}}
    </div>
  </div>
  `
  // ]]]
}
