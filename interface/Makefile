# Radial
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2022-04-21
# copyright  : Ben Kietzman
# email      : ben@kietzman.org
# {{{ prep work
prefix=/usr/local
all: bin/alert bin/application bin/auth bin/central bin/command bin/data bin/database bin/db bin/feedback bin/irc bin/junction bin/jwt bin/link bin/log bin/logger bin/mysql bin/mythtv bin/request bin/secure bin/session bin/sqlite bin/ssh bin/status bin/storage bin/terminal bin/websocket
install: bin/alert bin/application bin/auth bin/central bin/command bin/data bin/database bin/db bin/feedback bin/irc bin/junction bin/jwt bin/link bin/live bin/log bin/logger bin/mysql bin/mythtv bin/request bin/secure bin/session bin/sqlite bin/ssh bin/status bin/storage bin/terminal bin/websocket $(prefix)/radial/interface
	install --mode=775 bin/alert $(prefix)/radial/interface/
	install --mode=775 bin/application $(prefix)/radial/interface/
	install --mode=775 bin/auth $(prefix)/radial/interface/
	install --mode=775 bin/central $(prefix)/radial/interface/
	install --mode=775 bin/command $(prefix)/radial/interface/
	install --mode=775 bin/data $(prefix)/radial/interface/
	install --mode=775 bin/database $(prefix)/radial/interface/
	install --mode=775 bin/db $(prefix)/radial/interface/
	install --mode=775 bin/feedback $(prefix)/radial/interface/
	install --mode=775 bin/irc $(prefix)/radial/interface/
	install --mode=775 bin/junction $(prefix)/radial/interface/
	install --mode=775 bin/jwt $(prefix)/radial/interface/
	install --mode=775 bin/link $(prefix)/radial/interface/
	install --mode=775 bin/live $(prefix)/radial/interface/
	install --mode=775 bin/log $(prefix)/radial/interface/
	install --mode=775 bin/logger $(prefix)/radial/interface/
	install --mode=775 bin/mysql $(prefix)/radial/interface/
	install --mode=775 bin/mythtv $(prefix)/radial/interface/
	install --mode=775 bin/request $(prefix)/radial/interface/
	install --mode=775 bin/secure $(prefix)/radial/interface/
	install --mode=775 bin/session $(prefix)/radial/interface/
	install --mode=775 bin/sqlite $(prefix)/radial/interface/
	install --mode=775 bin/ssh $(prefix)/radial/interface/
	install --mode=775 bin/status $(prefix)/radial/interface/
	install --mode=775 bin/storage $(prefix)/radial/interface/
	install --mode=775 bin/terminal $(prefix)/radial/interface/
	install --mode=775 bin/websocket $(prefix)/radial/interface/
# }}}

# {{{ alert
alert: bin/alert $(prefix)/radial/interface
	install --mode=775 bin/alert $(prefix)/radial/interface/
bin/alert: ../../common/libcommon.a obj/alert.o obj/Base.o obj/Interface.o obj/Alert.o bin
	g++ -o bin/alert obj/alert.o obj/Base.o obj/Interface.o obj/Alert.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/alert.o: alert.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c alert.cpp -o obj/alert.o $(CPPFLAGS) -I../../common
obj/Alert.o: include/Alert.cpp obj
	g++ -ggdb -Wall -c include/Alert.cpp -o obj/Alert.o $(CPPFLAGS) -I../../common
# }}}
# {{{ application
application: bin/application $(prefix)/radial/interface
	install --mode=775 bin/application $(prefix)/radial/interface/
bin/application: ../../common/libcommon.a obj/application.o obj/Base.o obj/Interface.o obj/Application.o bin
	g++ -o bin/application obj/application.o obj/Base.o obj/Interface.o obj/Application.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/application.o: application.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c application.cpp -o obj/application.o $(CPPFLAGS) -I../../common
obj/Application.o: include/Application.cpp obj
	g++ -ggdb -Wall -c include/Application.cpp -o obj/Application.o $(CPPFLAGS) -I../../common
# }}}
# {{{ auth
auth: bin/auth $(prefix)/radial/interface
	install --mode=775 bin/auth $(prefix)/radial/interface/
bin/auth: ../../common/libcommon.a obj/auth.o obj/Base.o obj/Interface.o obj/Auth.o bin
	g++ -o bin/auth obj/auth.o obj/Base.o obj/Interface.o obj/Auth.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/auth.o: auth.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c auth.cpp -o obj/auth.o $(CPPFLAGS) -I../../common
obj/Auth.o: include/Auth.cpp obj
	g++ -ggdb -Wall -c include/Auth.cpp -o obj/Auth.o $(CPPFLAGS) -I../../common
# }}}
# {{{ central
central: bin/central $(prefix)/radial/interface
	install --mode=775 bin/central $(prefix)/radial/interface/
bin/central: ../../common/libcommon.a obj/central.o obj/Base.o obj/Interface.o obj/Central.o bin
	g++ -o bin/central obj/central.o obj/Base.o obj/Interface.o obj/Central.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/central.o: central.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c central.cpp -o obj/central.o $(CPPFLAGS) -I../../common
obj/Central.o: include/Central.cpp obj
	g++ -ggdb -Wall -c include/Central.cpp -o obj/Central.o $(CPPFLAGS) -I../../common
# }}}
# {{{ command
command: bin/command $(prefix)/radial/interface
	install --mode=775 bin/command $(prefix)/radial/interface/
bin/command: ../../common/libcommon.a obj/command.o obj/Base.o obj/Interface.o obj/Command.o bin
	g++ -o bin/command obj/command.o obj/Base.o obj/Interface.o obj/Command.o $(LDFLAGS) -L/usr/local/lib -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/command.o: command.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c command.cpp -o obj/command.o $(CPPFLAGS) -I../../common
obj/Command.o: include/Command.cpp obj
	g++ -ggdb -Wall -c include/Command.cpp -o obj/Command.o $(CPPFLAGS) -I/usr/local/include -I../../common
# }}}
# {{{ data
data: bin/data $(prefix)/radial/interface
	install --mode=775 bin/data $(prefix)/radial/interface/
bin/data: ../../common/libcommon.a obj/data.o obj/Base.o obj/Interface.o obj/Data.o bin
	g++ -o bin/data obj/data.o obj/Base.o obj/Interface.o obj/Data.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/data.o: data.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c data.cpp -o obj/data.o $(CPPFLAGS) -I../../common
obj/Data.o: include/Data.cpp obj
	g++ -ggdb -Wall -c include/Data.cpp -o obj/Data.o $(CPPFLAGS) -I../../common
# }}}
# {{{ database
database: bin/database $(prefix)/radial/interface
	install --mode=775 bin/database $(prefix)/radial/interface/
bin/database: ../../common/libcommon.a obj/database.o obj/Base.o obj/Interface.o obj/Database.o bin
	g++ -o bin/database obj/database.o obj/Base.o obj/Interface.o obj/Database.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/database.o: database.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c database.cpp -o obj/database.o $(CPPFLAGS) -I../../common
obj/Database.o: include/Database.cpp obj
	g++ -ggdb -Wall -c include/Database.cpp -o obj/Database.o $(CPPFLAGS) -I../../common
# }}}
# {{{ db
db: bin/db $(prefix)/radial/interface
	install --mode=775 bin/db $(prefix)/radial/interface/
bin/db: ../../common/libcommon.a obj/db.o obj/Base.o obj/Interface.o obj/Db.o bin
	g++ -o bin/db obj/db.o obj/Base.o obj/Interface.o obj/Db.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/db.o: db.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c db.cpp -o obj/db.o $(CPPFLAGS) -I../../common
obj/Db.o: include/Db.cpp obj
	g++ -ggdb -Wall -c include/Db.cpp -o obj/Db.o $(CPPFLAGS) -I../../common
# }}}
# {{{ feedback
feedback: bin/feedback $(prefix)/radial/interface
	install --mode=775 bin/feedback $(prefix)/radial/interface/
bin/feedback: ../../common/libcommon.a obj/feedback.o obj/Base.o obj/Interface.o obj/Feedback.o bin
	g++ -o bin/feedback obj/feedback.o obj/Base.o obj/Interface.o obj/Feedback.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/feedback.o: feedback.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c feedback.cpp -o obj/feedback.o $(CPPFLAGS) -I../../common
obj/Feedback.o: include/Feedback.cpp obj
	g++ -ggdb -Wall -c include/Feedback.cpp -o obj/Feedback.o $(CPPFLAGS) -I../../common
# }}}
# {{{ irc
irc: bin/irc $(prefix)/radial/interface
	install --mode=775 bin/irc $(prefix)/radial/interface/
bin/irc: ../../common/libcommon.a obj/irc.o obj/Base.o obj/Interface.o obj/Irc.o bin
	g++ -o bin/irc obj/irc.o obj/Base.o obj/Interface.o obj/Irc.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/irc.o: irc.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c irc.cpp -o obj/irc.o $(CPPFLAGS) -I../../common
obj/Irc.o: include/Irc.cpp obj
	g++ -ggdb -Wall -c include/Irc.cpp -o obj/Irc.o $(CPPFLAGS) -I../../common
# }}}
# {{{ junction
junction: bin/junction $(prefix)/radial/interface
	install --mode=775 bin/junction $(prefix)/radial/interface/
bin/junction: ../../common/libcommon.a obj/junction.o obj/Base.o obj/Interface.o obj/Junction.o bin
	g++ -o bin/junction obj/junction.o obj/Base.o obj/Interface.o obj/Junction.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/junction.o: junction.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c junction.cpp -o obj/junction.o $(CPPFLAGS) -I../../common
obj/Junction.o: include/Junction.cpp obj
	g++ -ggdb -Wall -c include/Junction.cpp -o obj/Junction.o $(CPPFLAGS) -I../../common
# }}}
# {{{ jwt
jwt: bin/jwt $(prefix)/radial/interface
	install --mode=775 bin/jwt $(prefix)/radial/interface/
bin/jwt: ../../common/libcommon.a obj/jwt.o obj/Base.o obj/Interface.o obj/Jwt.o bin
	g++ -o bin/jwt obj/jwt.o obj/Base.o obj/Interface.o obj/Jwt.o $(LDFLAGS) -L/usr/local/lib -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -ljwt -lmjson -lpthread -lrt -lssl -ltar -lz
obj/jwt.o: jwt.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c jwt.cpp -o obj/jwt.o $(CPPFLAGS) -I../../common
obj/Jwt.o: include/Jwt.cpp obj
	g++ -ggdb -Wall -c include/Jwt.cpp -o obj/Jwt.o $(CPPFLAGS) -I/usr/local/include -I../../common
# }}}
# {{{ link
link: bin/link $(prefix)/radial/interface
	install --mode=775 bin/link $(prefix)/radial/interface/
bin/link: ../../common/libcommon.a obj/link.o obj/Base.o obj/Interface.o obj/Link.o bin
	g++ -o bin/link obj/link.o obj/Base.o obj/Interface.o obj/Link.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/link.o: link.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c link.cpp -o obj/link.o $(CPPFLAGS) -I../../common
obj/Link.o: include/Link.cpp obj
	g++ -ggdb -Wall -c include/Link.cpp -o obj/Link.o $(CPPFLAGS) -I../../common
# }}}
# {{{ live
live: bin/live $(prefix)/radial/interface
	install --mode=775 bin/live $(prefix)/radial/interface/
bin/live: ../../common/libcommon.a obj/live.o obj/Base.o obj/Interface.o obj/Live.o bin
	g++ -o bin/live obj/live.o obj/Base.o obj/Interface.o obj/Live.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/live.o: live.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c live.cpp -o obj/live.o $(CPPFLAGS) -I../../common
obj/Live.o: include/Live.cpp obj
	g++ -ggdb -Wall -c include/Live.cpp -o obj/Live.o $(CPPFLAGS) -I../../common
# }}}
# {{{ log
log: bin/log $(prefix)/radial/interface
	install --mode=775 bin/log $(prefix)/radial/interface/
bin/log: ../../common/libcommon.a obj/log.o obj/Base.o obj/Interface.o obj/Log.o bin
	g++ -o bin/log obj/log.o obj/Base.o obj/Interface.o obj/Log.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/log.o: log.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c log.cpp -o obj/log.o $(CPPFLAGS) -I../../common
obj/Log.o: include/Log.cpp obj
	g++ -ggdb -Wall -c include/Log.cpp -o obj/Log.o $(CPPFLAGS) -I../../common
# }}}
# {{{ logger
logger: bin/logger $(prefix)/radial/interface
	install --mode=775 bin/logger $(prefix)/radial/interface/
bin/logger: ../../common/libcommon.a obj/logger.o obj/Base.o obj/Interface.o obj/Logger.o bin
	g++ -o bin/logger obj/logger.o obj/Base.o obj/Interface.o obj/Logger.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/logger.o: logger.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c logger.cpp -o obj/logger.o $(CPPFLAGS) -I../../common
obj/Logger.o: include/Logger.cpp obj
	g++ -ggdb -Wall -c include/Logger.cpp -o obj/Logger.o $(CPPFLAGS) -I../../common
# }}}
# {{{ mysql
mysql: bin/mysql $(prefix)/radial/interface
	install --mode=775 bin/mysql $(prefix)/radial/interface/
bin/mysql: ../../common/libcommon.a obj/mysql.o obj/Base.o obj/Interface.o obj/Mysql.o bin
	g++ -o bin/mysql obj/mysql.o obj/Base.o obj/Interface.o obj/Mysql.o $(LDFLAGS) -L/usr/lib64/mysql -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lmysqlclient -lpthread -lrt -lssl -ltar -lz
obj/mysql.o: mysql.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c mysql.cpp -o obj/mysql.o $(CPPFLAGS) -I../../common
obj/Mysql.o: include/Mysql.cpp obj
	g++ -ggdb -Wall -c include/Mysql.cpp -o obj/Mysql.o $(CPPFLAGS) -I../../common
# }}}
# {{{ mythtv
mythtv: bin/mythtv $(prefix)/radial/interface
	install --mode=775 bin/mythtv $(prefix)/radial/interface/
bin/mythtv: ../../common/libcommon.a obj/mythtv.o obj/Base.o obj/Interface.o obj/MythTv.o bin
	g++ -o bin/mythtv obj/mythtv.o obj/Base.o obj/Interface.o obj/MythTv.o $(LDFLAGS) -L/usr/lib64/mythtv -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/mythtv.o: mythtv.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c mythtv.cpp -o obj/mythtv.o $(CPPFLAGS) -I../../common
obj/MythTv.o: include/MythTv.cpp obj
	g++ -ggdb -Wall -c include/MythTv.cpp -o obj/MythTv.o $(CPPFLAGS) -I../../common
# }}}
# {{{ request
request: bin/request $(prefix)/radial/interface
	install --mode=775 bin/request $(prefix)/radial/interface/
bin/request: ../../common/libcommon.a obj/request.o obj/Base.o obj/Interface.o obj/Request.o bin
	g++ -o bin/request obj/request.o obj/Base.o obj/Interface.o obj/Request.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/request.o: request.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c request.cpp -o obj/request.o $(CPPFLAGS) -I../../common
obj/Request.o: include/Request.cpp obj
	g++ -ggdb -Wall -c include/Request.cpp -o obj/Request.o $(CPPFLAGS) -I../../common
# }}}
# {{{ secure
secure: bin/secure $(prefix)/radial/interface
	install --mode=775 bin/secure $(prefix)/radial/interface/
bin/secure: ../../common/libcommon.a obj/secure.o obj/Base.o obj/Interface.o obj/Secure.o bin
	g++ -o bin/secure obj/secure.o obj/Base.o obj/Interface.o obj/Secure.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/secure.o: secure.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c secure.cpp -o obj/secure.o $(CPPFLAGS) -I../../common
obj/Secure.o: include/Secure.cpp obj
	g++ -ggdb -Wall -c include/Secure.cpp -o obj/Secure.o $(CPPFLAGS) -I../../common
# }}}
# {{{ session
session: bin/session $(prefix)/radial/interface
	install --mode=775 bin/session $(prefix)/radial/interface/
bin/session: ../../common/libcommon.a obj/session.o obj/Base.o obj/Interface.o obj/Session.o bin
	g++ -o bin/session obj/session.o obj/Base.o obj/Interface.o obj/Session.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/session.o: session.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c session.cpp -o obj/session.o $(CPPFLAGS) -I../../common
obj/Session.o: include/Session.cpp obj
	g++ -ggdb -Wall -c include/Session.cpp -o obj/Session.o $(CPPFLAGS) -I../../common
# }}}
# {{{ sqlite
sqlite: bin/sqlite $(prefix)/radial/interface
	install --mode=775 bin/sqlite $(prefix)/radial/interface/
bin/sqlite: ../../common/libcommon.a obj/sqlite.o obj/Base.o obj/Interface.o obj/Sqlite.o bin
	g++ -o bin/sqlite obj/sqlite.o obj/Base.o obj/Interface.o obj/Sqlite.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lsqlite3 -lssl -ltar -lz
obj/sqlite.o: sqlite.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c sqlite.cpp -o obj/sqlite.o $(CPPFLAGS) -I../../common
obj/Sqlite.o: include/Sqlite.cpp obj
	g++ -ggdb -Wall -c include/Sqlite.cpp -o obj/Sqlite.o -DSQLITE_DEFAULT_FOREIGN_KEYS=1 $(CPPFLAGS) -I../../common
# }}}
# {{{ ssh
ssh: bin/ssh $(prefix)/radial/interface
	install --mode=775 bin/ssh $(prefix)/radial/interface/
bin/ssh: ../../common/libcommon.a obj/ssh.o obj/Base.o obj/Interface.o obj/Ssh.o bin
	g++ -o bin/ssh obj/ssh.o obj/Base.o obj/Interface.o obj/Ssh.o $(LDFLAGS) -L/usr/local/lib -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssh -lssl -ltar -lz
obj/ssh.o: ssh.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c ssh.cpp -o obj/ssh.o $(CPPFLAGS) -I../../common
obj/Ssh.o: include/Ssh.cpp obj
	g++ -ggdb -Wall -c include/Ssh.cpp -o obj/Ssh.o $(CPPFLAGS) -I/usr/local/include -I../../common
# }}}
# {{{ status
status: bin/status $(prefix)/radial/interface
	install --mode=775 bin/status $(prefix)/radial/interface/
bin/status: ../../common/libcommon.a obj/status.o obj/Base.o obj/Interface.o obj/Status.o bin
	g++ -o bin/status obj/status.o obj/Base.o obj/Interface.o obj/Status.o $(LDFLAGS) -L/usr/local/lib -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/status.o: status.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c status.cpp -o obj/status.o $(CPPFLAGS) -I../../common
obj/Status.o: include/Status.cpp obj
	g++ -ggdb -Wall -c include/Status.cpp -o obj/Status.o $(CPPFLAGS) -I/usr/local/include -I../../common
# }}}
# {{{ storage
storage: bin/storage $(prefix)/radial/interface
	install --mode=775 bin/storage $(prefix)/radial/interface/
bin/storage: ../../common/libcommon.a obj/storage.o obj/Base.o obj/Interface.o obj/Storage.o bin
	g++ -o bin/storage obj/storage.o obj/Base.o obj/Interface.o obj/Storage.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/storage.o: storage.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c storage.cpp -o obj/storage.o $(CPPFLAGS) -I../../common
obj/Storage.o: include/Storage.cpp obj
	g++ -ggdb -Wall -c include/Storage.cpp -o obj/Storage.o $(CPPFLAGS) -I../../common
# }}}
# {{{ terminal
terminal: bin/terminal $(prefix)/radial/interface
	install --mode=775 bin/terminal $(prefix)/radial/interface/
bin/terminal: ../../common/libcommon.a obj/terminal.o obj/Base.o obj/Interface.o obj/Terminal.o bin
	g++ -o bin/terminal obj/terminal.o obj/Base.o obj/Interface.o obj/Terminal.o $(LDFLAGS) -L/usr/local/lib -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz
obj/terminal.o: terminal.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c terminal.cpp -o obj/terminal.o $(CPPFLAGS) -I../../common
obj/Terminal.o: include/Terminal.cpp obj
	g++ -ggdb -Wall -c include/Terminal.cpp -o obj/Terminal.o $(CPPFLAGS) -I/usr/local/include -I../../common
# }}}
# {{{ websocket
websocket: bin/websocket $(prefix)/radial/interface
	install --mode=775 bin/websocket $(prefix)/radial/interface/
bin/websocket: ../../common/libcommon.a obj/websocket.o obj/Base.o obj/Interface.o obj/Websocket.o bin
	g++ -o bin/websocket obj/websocket.o obj/Base.o obj/Interface.o obj/Websocket.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lwebsockets -lz
obj/websocket.o: websocket.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c websocket.cpp -o obj/websocket.o $(CPPFLAGS) -I../../common
obj/Websocket.o: include/Websocket.cpp obj
	g++ -ggdb -Wall -c include/Websocket.cpp -o obj/Websocket.o $(CPPFLAGS) -I../../common
# }}}

# {{{ post work
bin:
	if [ ! -d bin ]; then mkdir bin; fi;
../../common/libcommon.a: ../../common/Makefile
	cd ../../common; make;
../../common/Makefile: ../../common/configure
	cd ../../common; ./configure;
../../common/configure:
	cd ../../; git clone https://github.com/benkietzman/common.git
obj/Base.o: ../include/Base.cpp obj
	g++ -ggdb -Wall -c ../include/Base.cpp -o obj/Base.o $(CPPFLAGS) -I../../common
obj/Interface.o: include/Interface.cpp obj
	g++ -ggdb -Wall -c include/Interface.cpp -o obj/Interface.o $(CPPFLAGS) -I../../common
obj:
	if [ ! -d obj ]; then mkdir obj; fi;
$(prefix)/radial/interface: $(prefix)/radial
	if [ ! -d $(prefix)/radial/interface ]; then mkdir $(prefix)/radial/interface; fi;
$(prefix)/radial:
	if [ ! -d $(prefix)/radial ]; then mkdir $(prefix)/radial; fi;
clean:
	-rm -fr obj bin
# }}}
