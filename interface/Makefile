# Radial
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2022-04-21
# copyright  : kietzman.org
# email      : ben@kietzman.org
# {{{ prep work
prefix=/usr/local
all: bin/log bin/mysql bin/request bin/storage
# }}}

install: bin/auth bin/link bin/log bin/mysql bin/request bin/storage $(prefix)/radial/interface
	install --mode=777 bin/auth $(prefix)/radial/interface/
	install --mode=777 bin/link $(prefix)/radial/interface/
	install --mode=777 bin/log $(prefix)/radial/interface/
	install --mode=777 bin/mysql $(prefix)/radial/interface/
	install --mode=777 bin/request $(prefix)/radial/interface/
	install --mode=777 bin/storage $(prefix)/radial/interface/

# {{{ auth
auth: bin/auth $(prefix)/radial/interface
	install --mode=777 bin/auth $(prefix)/radial/interface/
bin/auth: ../../common/libcommon.a obj/auth.o obj/Base.o obj/Interface.o obj/Auth.o bin
	g++ -o bin/auth obj/auth.o obj/Base.o obj/Interface.o obj/Auth.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/auth.o: auth.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c auth.cpp -o obj/auth.o $(CPPFLAGS) -I../../common
obj/Auth.o: include/Auth.cpp obj
	g++ -ggdb -Wall -c include/Auth.cpp -o obj/Auth.o $(CPPFLAGS) -I../../common
# }}}
# {{{ link
link: bin/link $(prefix)/radial/interface
	install --mode=777 bin/link $(prefix)/radial/interface/
bin/link: ../../common/libcommon.a obj/link.o obj/Base.o obj/Interface.o obj/Link.o bin
	g++ -o bin/link obj/link.o obj/Base.o obj/Interface.o obj/Link.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/link.o: link.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c link.cpp -o obj/link.o $(CPPFLAGS) -I../../common
obj/Link.o: include/Link.cpp obj
	g++ -ggdb -Wall -c include/Link.cpp -o obj/Link.o $(CPPFLAGS) -I../../common
# }}}
# {{{ log
log: bin/log $(prefix)/radial/interface
	install --mode=777 bin/log $(prefix)/radial/interface/
bin/log: ../../common/libcommon.a obj/log.o obj/Base.o obj/Interface.o obj/Log.o bin
	g++ -o bin/log obj/log.o obj/Base.o obj/Interface.o obj/Log.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/log.o: log.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c log.cpp -o obj/log.o $(CPPFLAGS) -I../../common
obj/Log.o: include/Log.cpp obj
	g++ -ggdb -Wall -c include/Log.cpp -o obj/Log.o $(CPPFLAGS) -I../../common
# }}}
# {{{ mysql
mysql: bin/mysql $(prefix)/radial/interface
	install --mode=777 bin/mysql $(prefix)/radial/interface/
bin/mysql: ../../common/libcommon.a obj/mysql.o obj/Base.o obj/Interface.o obj/Mysql.o bin
	g++ -o bin/mysql obj/mysql.o obj/Base.o obj/Interface.o obj/Mysql.o $(LDFLAGS) -L/usr/lib64/mysql -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lmysqlclient -lnsl -lpthread -lrt -lssl -ltar -lz
obj/mysql.o: mysql.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c mysql.cpp -o obj/mysql.o $(CPPFLAGS) -I../../common
obj/Mysql.o: include/Mysql.cpp obj
	g++ -ggdb -Wall -c include/Mysql.cpp -o obj/Mysql.o $(CPPFLAGS) -I../../common
# }}}
# {{{ request
request: bin/request $(prefix)/radial/interface
	install --mode=777 bin/request $(prefix)/radial/interface/
bin/request: ../../common/libcommon.a obj/request.o obj/Base.o obj/Interface.o obj/Request.o bin
	g++ -o bin/request obj/request.o obj/Base.o obj/Interface.o obj/Request.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/request.o: request.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c request.cpp -o obj/request.o $(CPPFLAGS) -I../../common
obj/Request.o: include/Request.cpp obj
	g++ -ggdb -Wall -c include/Request.cpp -o obj/Request.o $(CPPFLAGS) -I../../common
# }}}
# {{{ storage
storage: bin/storage $(prefix)/radial/interface
	install --mode=777 bin/storage $(prefix)/radial/interface/
bin/storage: ../../common/libcommon.a obj/storage.o obj/Base.o obj/Interface.o obj/Storage.o bin
	g++ -o bin/storage obj/storage.o obj/Base.o obj/Interface.o obj/Storage.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz
obj/storage.o: storage.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c storage.cpp -o obj/storage.o $(CPPFLAGS) -I../../common
obj/Storage.o: include/Storage.cpp obj
	g++ -ggdb -Wall -c include/Storage.cpp -o obj/Storage.o $(CPPFLAGS) -I../../common
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
