# Radial
Provides a radial design with a central hub and adjunct interfaces.

# Available Interfaces
This is a list of interfaces hosted by Radial.  The Common Library contains Radial classes written in C++ as well as PHP.  Those classes can reach all the interfaces via the request function. Beyond that, there are a number of specialized functions within the Radial classes fine tuned to the needs of particular interfaces.

* auth:  Provides authentication/authorization via Warden.
* central:  Provides functional hooks into Central.
* centralmon:  Provides functional hooks into Central Monitor.
* command:  Provides the ability to execute commands on the local server and return the stdout results.
* database:  Provides access to preconfigured databases of various types.
* db:  Provides access to simplified database functions that black box the underlying SQL logic.
* feedback:  Provides functional hooks into Feedback.
* irc:  Provides functional hooks into an IRC chat bot.
* junction:  Provides functional hooks into the Service Junction.
* jwt:  Provides functional hooks into JSON Web Tokens (JWT).
* link:  Links more than one Radial instance together across a network.
* live:  Provides live message passing between applications and users connected over WebSockets.
* log:  Provides logging capabilities.
* logger:  Provides functional hooks to Logger.
* mysql:  Provides functional hooks to MySQL.
* request:  Provides the ability for upstream applications to communicate with Radial via a request/response protocol over socket connections.
* secure:  Provides login functionality for websites connected to Radial over WebSocket connections.
* storage:  Provides common memory storage synchronized across Radial instances.
* websocket:  Provides the ability for upstream websites to communicate with Radial via a request/response protocol over WebSocket connections.

# Route Key
Within the code you will notice the following short keys used within JSON associative arrays:

* _d:  destination - Designates the flow of the request.  Either toward the target (t) or source (s).
* _f:  function - Designates an internal function.
* _l:  link - Contains an internally imbedded request transmitted across links.
* _s:  source - Designates the source interface of the request.
* _t:  target - Designates the target interface of the request.
* _r:  request - Contains an internally imbedded request transmitted across interfaces.
