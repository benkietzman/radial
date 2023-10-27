// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2023-10-27
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
    let s = c.scope('Home',
    {
      // [[[ u()
      u: () =>
      {
        c.update('Home');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ main
    c.setMenu('Home');
    s.u();
    if (!a.ready())
    {
      s.info.v = 'Authenticating session...';
    }
    c.attachEvent('appReady', (data) =>
    {
      s.info.v = null;
    });
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div c-model="info" class="text-warning"></div>
  <div c-model="message" class="text-danger" style="font-weight:bold;"></div>
  <h4 class="page-header">Radial</h4>
  <p>
  Provides a radial design with a central hub and adjunct interfaces.
  </p>
  <h4 class="page-header">Client Instructions</h4>
  <p>
  The client application should connect to Radial on either port 7234 (encrypted socket) or 7797 (WebSocket).  The request is written in JSON format using name/value pairs and the line should end with a new line character.  Every JSON request should have the "Interface" argument containing a valid registered interface.  The request may or may not have additional arguments.  It depends on the requirements of the interface being run.
  </p>
  <p>
  After a request has been sent by the client, Radial will be return the details of the original request as well as any response data.  If Radial was able to successfully process the request, a field of "Status" will be returned with a value of "okay".  If Radial was unable to successfully process the request, a field of "Status" will be returned with a value of "error" and an error message will be provided within the field "Error".
  </p>
  <p>
  Radial is capable of processing requests on multiple socket connections in parallel.  It is also capable of processing multiple requests per socket connection.  Radial returns responses based on the order of completion not the order of arrival.  Radial never terminates the socket connection.  It is always the client who must terminate the connection when finished sending and receiving data.
  </p>
  `
  // ]]]
}
