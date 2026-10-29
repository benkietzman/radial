Here are example scripts for Central that can be used in Maestro flows and called registered in schedules.

# footer
```
let r = {};
if (getRequest() !== undefined)
{
  r = getRequest();
}
if (r.email === undefined)
{
  r.email = 'john@doe.org';
}
if (r.company === undefined)
{
  r.company = 'John\'s Company';
}
if (r.server === undefined)
{
  r.server = 'doe.org';
}
if (r.userid === undefined)
{
  r.userid = 'ben';
}
if (r.website === undefined)
{
  r.website = 'https://'+r.server+'/central';
}
if (r.year === undefined)
{
  r.year = new Date().getFullYear();
}
if (db('dbCentralUsers', JSON.stringify({userid: r.userid})))
{
  let getUsers = JSON.parse(getResponse()).Response;
  if (getUsers.length == 1)
  {
    getUsers[0].link = 'https://'+r.server+'/central/#/Users/'+getUsers[0].id;
    getUsers[0].target = '_blank';
    r.engineer = getUsers[0];
  }
}
if (db('dbCentralApplications', JSON.stringify({name: 'Radial'})))
{
  let getApplications = JSON.parse(getResponse()).Response;
  if (getApplications.length == 1)
  {
    getApplications[0].application = 'Radial';
    if (getApplications[0].website != '')
    {
      getApplications[0].link = getApplications[0].website;
    }
    else
    {
      getApplications[0].link = 'https://'+r.server+'/central/#/Applications/'+getApplications[0].id;
    }
    getApplications[0].target = '_blank';
    r.power = getApplications[0];
  }
}
setStatus('okay');
setError('');
setResponse(JSON.stringify(r));
```

# homeUsefulTools
```
let links = [];
links.push('<a href="https://doe.og/" target="_blank">John\'s Website</a>');
setStatus('okay');
setError('');
setResponse(JSON.stringify(links));
```

# statistics
```
let b = false;
let e = '';
let strQuery = 'delete from statistic where `date` < date_format(date_sub(now(), interval 6 month), \'%Y-%m-%d\')'
if (dbupdate('central', strQuery))
{
  b = true;
}
else
{
  e = getError();
  chat('#central', 'Interface::dbupdate('+strQuery+') error:  '+e);
}
setStatus((b)?'okay':'error');
setError(e);
```

# workload
```
let b = false;
let e = '';
let people = {};
let strFrom = 'john@doe.org';
let strServer = 'doe.org';
let strQuery = 'select a.name application_name, b.id, b.application_id, b.assigned_id, date_format(b.close_date, \'%Y-%m-%d\') close_date, date_format(b.due_date, \'%Y-%m-%d\') due_date, b.hold, date_format(b.open_date, \'%Y-%m-%d\') open_date, b.priority, date_format(b.release_date, \'%Y-%m-%d\') release_date, b.summary from application a, application_issue b where a.id = b.application_id and b.close_date is null order by b.priority desc, b.due_date, b.open_date';
if (dbquery('central_r', strQuery))
{
  let getApplicationIssue = JSON.parse(getResponse()).Response;
  b = true;
  for (let i = 0; i < getApplicationIssue.length; i++)
  {
    let assigns = [];
    if (getApplicationIssue[i].assigned_id != '')
    {
      strQuery = 'select first_name, last_name, userid from person where id = '+getApplicationIssue[i].assigned_id;
      if (dbquery('central_r', strQuery))
      {
        let getAssigned = JSON.parse(getResponse()).Response;
        if (getAssigned.length > 0)
        {
          getApplicationIssue[i].assigned_first_name = getAssigned[0].first_name;
          getApplicationIssue[i].assigned_last_name = getAssigned[0].last_name;
          getApplicationIssue[i].assigned_userid = getAssigned[0].userid;
        }
      }
      else
      {
        chat('#central', 'Interface::dbquery('+strQuery+') error:  '+getError());
      }
      strQuery = 'select id, comments, user_id from issue_comment where issue_id = '+getApplicationIssue[i].id+' order by entry_date, id limit 1';
      if (dbquery('central_r', strQuery))
      {
        let getIssueComment = JSON.parse(getResponse()).Response;
        if (getIssueComment.length > 0)
        {
          getApplicationIssue[i].comments = getIssueComment[0].comments;
          if (getIssueComment[0].user_id != '')
          {
            getApplicationIssue[i].requester_id = getIssueComment[0].user_id;
            strQuery = 'select first_name, last_name, userid from person where id = '+getIssueComment[0].user_id;
            if (dbquery('central_r', strQuery))
            {
              let getRequester = JSON.parse(getResponse()).Response;
              if (getRequester.length > 0)
              {
                getApplicationIssue[i].requester_first_name = getRequester[0].first_name;
                getApplicationIssue[i].requester_last_name = getRequester[0].last_name;
                getApplicationIssue[i].requester_userid = getRequester[0].userid;
              }
              else
              {
                chat('#central', 'dbquery('+strQuery+') error ['+getApplicationIssue[i].id+']:  No results returned.');
              }
            }
           else
            {
              chat('#central', 'dbquery('+strQuery+') error ['+getApplicationIssue[i].id+']:  '+getError());
            }
          }
        }
      }
      else
      {
        chat('#central', 'dbquery('+strQuery+') error ['+getApplicationIssue[i].id+']:  '+getError());
      }
      if (getApplicationIssue[i].assigned_id !== undefined && getApplicationIssue[i].assigned_id != '')
      {
        assigns.push(getApplicationIssue[i].assigned_id);
      }
      else
      {
        strQuery = 'select a.contact_id from application_contact a, contact_type b where a.type_id = b.id and a.application_id = '+getApplicationIssue[i].application_id+' and b.type in (\'Primary Developer\', \'Backup Developer\')';
        if (dbquery('central_r', strQuery))
        {
          let getDeveloper = JSON.parse(getResponse()).Response;
          for (let j = 0; j < getDeveloper.length; j++)
          {
            assigns.push(getDeveloper[j].contact_id);
          }
        }
      }
      assigns = [...new Set(assigns)].sort();
      for (let j = 0; j < assigns.length; j++)
      {
        if (people[assigns[j]] === undefined)
        {
          people[assigns[j]] = [];
        }
        people[assigns[j]].push(getApplicationIssue[i]);
      }
    }
  }
}
else
{
  e = getError();
  chat('#central', 'dbquery('+strQuery+') error:  '+e);
}
Object.entries(people).forEach(([key, person]) =>
{
  strQuery = 'select first_name, last_name, userid, email from person where id = '+key;
  if (dbquery('central_r', strQuery))
  {
    let getPerson = JSON.parse(getResponse()).Response;
    if (getPerson.length > 0)
    {
      let to = [];
      let strSubject = 'Central:  Workload';
      let strText = '';
      let strHtml = '';
      to.push(getPerson[0].email);
      strText += 'NOTICE:  You are viewing this email in plain-text mode.  This email contains much more information when viewed in HTML mode.'+"\n\n";
      strText += 'OPEN APPLICATION ISSUES ('+person.length+')'+"\n\n";
      strHtml += '<html>';
      strHtml += '<body style="background:#f3f3f3;padding:10px;">';
      strHtml += '<h3>Open Application Issues ('+person.length+')</h3>';
      strHtml += '<p>';
      strHtml += 'This is the weekly <a href="https://'+strServer+'/central/#/Applications/Workload">Workload</a> email.  It provides your personalized workload of open application issues.  The open issues listed below are being pulled from applications for which you are registered as either a primary or backup developer.  The issues are sorted according to priority, due date, and open date.';
      strHtml += '</p>';
      for (let i = 0; i < person.length; i++)
      {
        strText += 'Issue #'+person[i].id+':  https://'+strServer+'/central/#/Applications/Issues/'+person[i].id+"\n";
        strHtml += '<div style="margin:10px 5px;border-style:solid;border-width:1px;border-color:#cccccc;border-radius:10px;background:white;box-shadow: 3px 3px 4px #888888;padding:10px;">';
        strHtml += '<table>';
        strHtml += '<tr>';
        strHtml += '<td valign="top">';
        strHtml += '<a href="https://'+strServer+'/central/#/Applications/'+person[i].application_id+'" style="font-weight: bold;">'+person[i].application_name+'</a>';
        strHtml += '<table>';
        strHtml += '<tr><td style="white-space: nowrap;">Issue #:</td><td><a href="https://'+strServer+'/central/#/Applications/Issues/'+person[i].id+'">'+person[i].id+'</a>'+((person[i].hold == '1')?'<span style="margin-left: 20px; padding: 0px 2px; background: green; color: white;">HOLD</span>':'')+'</td></tr>';
        strHtml += '<tr><td style="white-space: nowrap;">Open:</td><td>'+person[i].open_date+'</td></tr>';
        if (person[i].due_date != '')
        {
          strHtml += '<tr><td style="white-space: nowrap;">Due:</td><td>'+person[i].due_date+'</td></tr>';
        }
        if (person[i].release_date != '')
        {
          strHtml += '<tr><td style="white-space: nowrap;">Release:</td><td>'+person[i].release_date+'</td></tr>';
        }
        if (person[i].priority != '')
        {
          strHtml += '<tr>';
          strHtml += '<td style="white-space: nowrap;">Priority:</td>';
          strHtml += '<td>';
          if (person[i].priority == '4')
          {
            strHtml += '<span style="color: red; font-weight: bold;">CRITICAL</span>';
          }
          else if (person[i].priority == '3')
          {
            strHtml += '<span style="color: red;">High</span>';
          }
          else if (person[i].priority == '2')
          {
            strHtml += '<span style="color: orange;">Medium</span>';
          }
          else if (person[i].priority == '1')
          {
            strHtml += '<span>Low</span>';
          }
          else
          {
            strHtml += '<span>none</span>';
          }
          strHtml += '</td>';
          strHtml += '</tr>';
        }
        if (person[i].requester_userid !== undefined && person[i].requester_userid != '')
        {
          strHtml += '<tr><td style="white-space: nowrap;">Requester:</td><td style="white-space: nowrap;"><a href="https://'+strServer+'/central/#/Users/'+person[i].requester_id+'">'+person[i].requester_last_name+', '+person[i].requester_first_name+'</a> <small>('+person[i].requester_userid+')</small></td></tr>';
        }
        if (person[i].assigned_userid !== undefined && person[i].assigned_userid != '')
        {
          strHtml += '<tr><td style="white-space: nowrap;">Assigned:</td><td style="white-space: nowrap;"><a href="https://'+strServer+'/central/#/Users/'+person[i].assigned_id+'">'+person[i].assigned_last_name+', '+person[i].assigned_first_name+'</a> <small>('+person[i].assigned_userid+')</small></td></tr>';
        }
        strHtml += '</table>';
        strHtml += '</td>';
        strHtml += '<td valign="top">';
        if (person[i].summary != '')
        {
          strHtml += '<p style="margin: 0px 20px 20px 20px; font-weight: bold;">'+person[i].summary+'</p>';
        }
        if (person[i].comments != '')
        {
          strHtml += '<pre style="margin: 0px 20px 20px 20px; white-space: pre-wrap;">'+person[i].comments+'</pre>';
        }
        strHtml += '</td>';
        strHtml += '</tr>';
        strHtml += '</table>';
        strHtml += '</div>';
      }
      strHtml += '<p>';
      strHtml += 'Please use <a href="https://'+strServer+'/central/#/Home/Issues">Issues</a> to create a new issue for an application.  Issues provides a comprehensive list of applications from which to choose.';
      strHtml += '</p>';
      strHtml += '<p>';
      strHtml += 'This message was sent by <a href="https://'+strServer+'/central" style="text-decoration:none;">Central</a>';
      strHtml += '</p>';
      strHtml += '</body>';
      strHtml += '</html>';
      email(strFrom, JSON.stringify(to), '', '', strSubject, strText, strHtml, '');
    }
    else
    {
      chat('#central', 'dbquery('+strQuery+') error ['+key+']:  No results returned.');
    }
  }
  else
  {
    chat('#central', 'dbquery('+strQuery+') error ['+key+']:  '+getError());
  }
});
setStatus((b)?'okay':'error');
setError(e);
```
