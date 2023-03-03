# Radial
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2022-04-21
# copyright  : kietzman.org
# email      : ben@kietzman.org
# {{{ prep work
prefix=/usr/local
all: bin/auth bin/central bin/centralmon bin/database bin/irc bin/jwt bin/link bin/log bin/mysql bin/request bin/secure bin/storage bin/websocket
install: bin/auth bin/central bin/centralmon bin/database bin/irc bin/jwt bin/link bin/live bin/log bin/mysql bin/request bin/secure bin/storage bin/websocket $(prefix)/radial/interface
	install --mode=775 bin/auth $(prefix)/radial/interface/
	install --mode=775 bin/central $(prefix)/radial/interface/
	install --mode=775 bin/centralmon $(prefix)/radial/interface/
	install --mode=775 bin/database $(prefix)/radial/interface/
	install --mode=775 bin/irc $(prefix)/radial/interface/
	install --mode=775 bin/jwt $(prefix)/radial/interface/
	install --mode=775 bin/link $(prefix)/radial/interface/
	install --mode=775 bin/live $(prefix)/radial/interface/
	install --mode=775 bin/log $(prefix)/radial/interface/
	install --mode=775 bin/mysql $(prefix)/radial/interface/
	install --mode=775 bin/request $(prefix)/radial/interface/
	install --mode=775 bin/secure $(prefix)/radial/interface/
	install --mode=775 bin/storage $(prefix)/radial/interface/
	install --mode=775 bin/websocket $(prefix)/radial/interface/
# }}}

# {{{ auth
auth: bin/auth $(prefix)/radial/interface
	install --mode=775 bin/auth $(prefix)/radial/interface/
bin/auth: ../../common/libcommon.a obj/auth.o obj/Base.o obj/Interface.o obj/Auth.o bin
	g++ -o bin/auth obj/auth.o obj/Base.o obj/Interface.o obj/Auth.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/auth.o: auth.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c auth.cpp -o obj/auth.o $(CPPFLAGS) -I../../common
obj/Auth.o: include/Auth.cpp obj
	g++ -ggdb -Wall -c include/Auth.cpp -o obj/Auth.o $(CPPFLAGS) -I../../common
# }}}
# {{{ central
central: bin/central $(prefix)/radial/interface
	install --mode=775 bin/central $(prefix)/radial/interface/
bin/central: ../../common/libcommon.a obj/central.o obj/Base.o obj/Interface.o obj/Central.o bin
	g++ -o bin/central obj/central.o obj/Base.o obj/Interface.o obj/Central.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/central.o: central.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c central.cpp -o obj/central.o $(CPPFLAGS) -I../../common
obj/Central.o: include/Central.cpp obj
	g++ -ggdb -Wall -c include/Central.cpp -o obj/Central.o $(CPPFLAGS) -I../../common
# }}}
# {{{ centralmon
centralmon: bin/centralmon $(prefix)/radial/interface
	install --mode=775 bin/centralmon $(prefix)/radial/interface/
bin/centralmon: ../../common/libcommon.a obj/centralmon.o obj/Base.o obj/Interface.o obj/CentralMon.o bin
	g++ -o bin/centralmon obj/centralmon.o obj/Base.o obj/Interface.o obj/CentralMon.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/centralmon.o: centralmon.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c centralmon.cpp -o obj/centralmon.o $(CPPFLAGS) -I../../common
obj/CentralMon.o: include/CentralMon.cpp obj
	g++ -ggdb -Wall -c include/CentralMon.cpp -o obj/CentralMon.o $(CPPFLAGS) -I../../common
# }}}
# {{{ database
database: bin/database $(prefix)/radial/interface
	install --mode=775 bin/database $(prefix)/radial/interface/
bin/database: ../../common/libcommon.a obj/database.o obj/Base.o obj/Interface.o obj/Database.o bin
	g++ -o bin/database obj/database.o obj/Base.o obj/Interface.o obj/Database.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/database.o: database.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c database.cpp -o obj/database.o $(CPPFLAGS) -I../../common
obj/Database.o: include/Database.cpp obj
	g++ -ggdb -Wall -c include/Database.cpp -o obj/Database.o $(CPPFLAGS) -I../../common
# }}}
# {{{ irc
irc: bin/irc $(prefix)/radial/interface
	install --mode=775 bin/irc $(prefix)/radial/interface/
bin/irc: ../../common/libcommon.a obj/irc.o obj/Base.o obj/Interface.o obj/Irc.o bin
	g++ -o bin/irc obj/irc.o obj/Base.o obj/Interface.o obj/Irc.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssh -lssl -ltar -lz
obj/irc.o: irc.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c irc.cpp -o obj/irc.o $(CPPFLAGS) -I../../common
obj/Irc.o: include/Irc.cpp obj
	g++ -ggdb -Wall -c include/Irc.cpp -o obj/Irc.o $(CPPFLAGS) -I../../common
# }}}
# {{{ jwt
jwt: bin/jwt $(prefix)/radial/interface
	install --mode=775 bin/jwt $(prefix)/radial/interface/
bin/jwt: ../../common/libcommon.a obj/jwt.o obj/Base.o obj/Interface.o obj/Jwt.o bin
	g++ -o bin/jwt obj/jwt.o obj/Base.o obj/Interface.o obj/Jwt.o $(LDFLAGS) -L/usr/local/lib -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -ljwt -lmjson -lnsl -lpthread -lrt -lssh -lssl -ltar -lz
obj/jwt.o: jwt.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c jwt.cpp -o obj/jwt.o $(CPPFLAGS) -I../../common
obj/Jwt.o: include/Jwt.cpp obj
	g++ -ggdb -Wall -c include/Jwt.cpp -o obj/Jwt.o $(CPPFLAGS) -I/usr/local/include -I../../common
# }}}
# {{{ link
link: bin/link $(prefix)/radial/interface
	install --mode=775 bin/link $(prefix)/radial/interface/
bin/link: ../../common/libcommon.a obj/link.o obj/Base.o obj/Interface.o obj/Link.o bin
	g++ -o bin/link obj/link.o obj/Base.o obj/Interface.o obj/Link.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/link.o: link.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c link.cpp -o obj/link.o $(CPPFLAGS) -I../../common
obj/Link.o: include/Link.cpp obj
	g++ -ggdb -Wall -c include/Link.cpp -o obj/Link.o $(CPPFLAGS) -I../../common
# }}}
# {{{ live
live: bin/live $(prefix)/radial/interface
	install --mode=775 bin/live $(prefix)/radial/interface/
bin/live: ../../common/libcommon.a obj/live.o obj/Base.o obj/Interface.o obj/Live.o bin
	g++ -o bin/live obj/live.o obj/Base.o obj/Interface.o obj/Live.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/live.o: live.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c live.cpp -o obj/live.o $(CPPFLAGS) -I../../common
obj/Live.o: include/Live.cpp obj
	g++ -ggdb -Wall -c include/Live.cpp -o obj/Live.o $(CPPFLAGS) -I../../common
# }}}
# {{{ log
log: bin/log $(prefix)/radial/interface
	install --mode=775 bin/log $(prefix)/radial/interface/
bin/log: ../../common/libcommon.a obj/log.o obj/Base.o obj/Interface.o obj/Log.o bin
	g++ -o bin/log obj/log.o obj/Base.o obj/Interface.o obj/Log.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/log.o: log.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c log.cpp -o obj/log.o $(CPPFLAGS) -I../../common
obj/Log.o: include/Log.cpp obj
	g++ -ggdb -Wall -c include/Log.cpp -o obj/Log.o $(CPPFLAGS) -I../../common
# }}}
# {{{ mysql
mysql: bin/mysql $(prefix)/radial/interface
	install --mode=775 bin/mysql $(prefix)/radial/interface/
bin/mysql: ../../common/libcommon.a obj/mysql.o obj/Base.o obj/Interface.o obj/Mysql.o bin
	g++ -o bin/mysql obj/mysql.o obj/Base.o obj/Interface.o obj/Mysql.o $(LDFLAGS) -L/usr/lib64/mysql -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lmysqlclient -lnsl -lpthread -lrt -lssl -ltar -lz
obj/mysql.o: mysql.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c mysql.cpp -o obj/mysql.o $(CPPFLAGS) -I../../common
obj/Mysql.o: include/Mysql.cpp obj
	g++ -ggdb -Wall -c include/Mysql.cpp -o obj/Mysql.o $(CPPFLAGS) -I../../common
# }}}
# {{{ request
request: bin/request $(prefix)/radial/interface
	install --mode=775 bin/request $(prefix)/radial/interface/
bin/request: ../../common/libcommon.a obj/request.o obj/Base.o obj/Interface.o obj/Request.o bin
	g++ -o bin/request obj/request.o obj/Base.o obj/Interface.o obj/Request.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/request.o: request.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c request.cpp -o obj/request.o $(CPPFLAGS) -I../../common
obj/Request.o: include/Request.cpp obj
	g++ -ggdb -Wall -c include/Request.cpp -o obj/Request.o $(CPPFLAGS) -I../../common
# }}}
# {{{ secure
secure: bin/secure $(prefix)/radial/interface
	install --mode=775 bin/secure $(prefix)/radial/interface/
bin/secure: ../../common/libcommon.a obj/secure.o obj/Base.o obj/Interface.o obj/Secure.o bin
	g++ -o bin/secure obj/secure.o obj/Base.o obj/Interface.o obj/Secure.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/secure.o: secure.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c secure.cpp -o obj/secure.o $(CPPFLAGS) -I../../common
obj/Secure.o: include/Secure.cpp obj
	g++ -ggdb -Wall -c include/Secure.cpp -o obj/Secure.o $(CPPFLAGS) -I../../common
# }}}
# {{{ storage
storage: bin/storage $(prefix)/radial/interface
	install --mode=775 bin/storage $(prefix)/radial/interface/
bin/storage: ../../common/libcommon.a obj/storage.o obj/Base.o obj/Interface.o obj/Storage.o bin
	g++ -o bin/storage obj/storage.o obj/Base.o obj/Interface.o obj/Storage.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/storage.o: storage.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c storage.cpp -o obj/storage.o $(CPPFLAGS) -I../../common
obj/Storage.o: include/Storage.cpp obj
	g++ -ggdb -Wall -c include/Storage.cpp -o obj/Storage.o $(CPPFLAGS) -I../../common
# }}}
# {{{ websocket
websocket: bin/websocket $(prefix)/radial/interface
	install --mode=775 bin/websocket $(prefix)/radial/interface/
bin/websocket: ../../common/libcommon.a obj/websocket.o obj/Base.o obj/Interface.o obj/Websocket.o bin
	g++ -o bin/websocket obj/websocket.o obj/Base.o obj/Interface.o obj/Websocket.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lwebsockets -lz
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
