// vim: fmr=[[[,]]]
///////////////////////////////////////////
// author     : Ben Kietzman
// begin      : 2025-06-27
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
    let j = junction;
    let s = c.scope('HomeSamba',
    {
      // [[[ u()
      u: () =>
      {
        c.update('HomeSamba');
      },
      // ]]]
      a: a,
      c: c
    });
    // ]]]
    // [[[ directory()
    s.directory = (strItem) =>
    {     
      if (strItem == '..')
      { 
        let items = s.path.v.split('/');
        s.path.v = ''; 
        for (let i = 0; i < (items.length - 1); i++)
        {
          if (i > 0)
          {
            s.path.v += '/';
          }
          s.path.v += items[i];
        }
      }
      else if (strItem != '.')
      {
        if (s.path.v != '')
        {
          s.path.v += '/';
        }
        s.path.v += strItem;
      }
      s.directoryList();
    };
    // ]]]
    // [[[ directoryList()
    s.directoryList = () =>
    {
      if (s.user.v)
      {
        if (s.password.v)
        {
          if (s.domain.v)
          {
            if (s.share.v)
            {
              let request = [{Service: 'samba', 'Function': 'directoryExist', User: s.user.v, Password: s.password.v, Domain: s.domain.v}, {IP: s.ip.v, Share: s.share.v, Path: s.path.v}];
              j.request(request, true, (response) =>
              {
                let error = {};
                if (j.response(response, error))
                {
                  let request = [{Service: 'samba', 'Function': 'directoryList', User: s.user.v, Password: s.password.v, Domain: s.domain.v}, {IP: s.ip.v, Share: s.share.v, Path: s.path.v}];
                  j.request(request, true, (response) =>
                  {
                    let error = {};
                    if (j.response(response, error))
                    {
                      s.items = null;
                      s.items = []
                      for (let i = 0; i < response.Response[1].length; i++)
                      {
                        if (s.path.v != '' || response.Response[1][i] != '..')
                        {
                          s.items.push({name: response.Response[1][i]});
                        }
                      }
                      for (let i = 0; i < s.items.length; i++)
                      {
                        let request = [{Service: 'samba', 'Function': 'directoryExist', User: s.user.v, Password: s.password.v, Domain: s.domain.v, i: i},{IP: s.ip.v, Share: s.share.v, Path: s.path.v + '/' + s.items[i].name}];
                        j.request(request, true, (response) =>
                        {
                          let error = {};
                          if (j.response(response, error))
                          {
                            s.items[response.Response[0].i].type = 'directory';
                          }
                          else
                          {
                            s.items[response.Response[0].i].type = 'file';
                          }
                          s.u();
                        });
                      }
                    }
                    else
                    {
                      c.pushErrorMessage(error.message);
                    }
                    s.u();
                  });
                }
                else
                {
                  c.pushErrorMessage(error.message);
                }
                s.u();
              });
            }
            else
            {
              c.pushErrorMessage('Please provide the Share.');
            }
          }
          else
          {
            c.pushErrorMessage('Please provide the Domain.');
          }
        }
        else
        {
          c.pushErrorMessage('Please provide the Password.');
        }
      }
      else
      {
        c.pushErrorMessage('Please provide the User.');
      }
    };
    // ]]]
    // [[[ enter()
    s.enter = () =>
    {
      if (window.event.keyCode == 13)
      {
        s.directoryList();
      }
    };
    // ]]]
    // [[[ get()
    s.get = (strItem) =>
    {
      let strPath = s.path.v;
      if (strPath != '')
      {
        strPath += '/';
      }
      strPath += strItem;
      let request = [{Service: 'samba', 'Function': 'get', User: s.user.v, Password: s.password.v, Domain: s.domain.v}, {IP: s.ip.v, Share: s.share.v, Path: strPath}];
      j.request(request, true, (response) =>
      {
        let error = {};
        if (j.response(response, error))
        {
          let a = document.createElement('a');
          a.href = 'data:application/octet-stream;base64,' + encodeURIComponent(response.Response[1].Data);
          a.download = strItem;
          a.click();
        }
        else
        {
          c.pushErrorMessage(error.message);
        }
      });
    };
    // ]]]
    // [[[ main
    c.setMenu('Home', 'Samba');
    s.u();
    // ]]]
  },
  // ]]]
  // [[[ template
  template: `
  <div class="row" style="margin-top: 20px;">
    <div class="col input-group"><input type="text" class="form-control" c-keyup="enter()" id="user" c-model="user" placeholder="User"></div>
    <div class="col input-group"><input type="password" class="form-control" c-keyup="enter()" c-model="password" placeholder="Password"></div>
    <div class="col input-group"><input type="text" class="form-control" c-keyup="enter()" c-model="ip" placeholder="IP/Server"></div>
    <div class="col input-group"><input type="text" class="form-control" c-keyup="enter()" c-model="domain" placeholder="Domain"></div>
    <div class="col input-group"><input type="text" class="form-control" c-keyup="enter()" c-model="share" placeholder="Share"></div>
    <div class="col input-group"><input type="text" class="form-control" c-keyup="enter()" c-model="path" placeholder="Path"></div>
    <div class="col"><button class="btn btn-success" c-click="directoryList()">List</button></div>
  </div>
  <div class="row">
    {{#each items}}
    {{#if type}}
    {{#ifCond type "==" "directory"}}
    <div class="col" style="margin: 10px;">
      <button class="btn btn-secondary bi bi-folder" c-click="directory('{{../name}}')" style="white-space: nowrap;"> {{../name}}</button>
    </div>
    {{/ifCond}}
    {{/if}}
    {{/each}}
    {{#each items}}
    {{#if type}}
    {{#ifCond type "!=" "directory"}}
    <div class="col" style="margin: 10px;">
      <button class="btn btn-link" c-click="get('{{../name}}')" style="white-space: nowrap;"> {{../name}}</button>
    </div>
    {{/ifCond}}
    {{/if}}
    {{/each}}
  </div>
  `
  // ]]]
}
