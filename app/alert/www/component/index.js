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
    if (c.isParam(nav, 'user'))
    {
      s.user.v = c.getParam(nav, 'user');
    }
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
  {{#isValid}}
  <div class="form-group"><div class="input-group"><input type="text" class="form-control" c-model="user" placeholder="User" value="User"></div></div>
  <div class="form-group"><div class="input-group"><textarea class="form-control" c-model="message" placeholder="Type message here..."></textarea></div></div>
  <button id="send" class="btn btn-success" c-click="send()">Send</button>
  {{else}}
  <b class="text-danger">PLEASE LOGIN</b>
  {{/isValid}}
  `
  // ]]]
}
