# Radial
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2022-04-21
# copyright  : kietzman.org
# email      : ben@kietzman.org

prefix=/usr/local

all: bin/log bin/mysql bin/request

bin/log: ../../common/libcommon.a obj/log.o obj/Base.o obj/Interface.o obj/Log.o bin
	g++ -o bin/log obj/log.o obj/Base.o obj/Interface.o obj/Log.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz

bin/mysql: ../../common/libcommon.a obj/mysql.o obj/Base.o obj/Interface.o obj/Mysql.o bin
	g++ -o bin/mysql obj/mysql.o obj/Base.o obj/Interface.o obj/Mysql.o $(LDFLAGS) -L/usr/lib64/mysql -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lmysqlclient -lnsl -lpthread -lrt -lssl -ltar -lz

bin/request: ../../common/libcommon.a obj/request.o obj/Base.o obj/Interface.o obj/Request.o bin
	g++ -o bin/request obj/request.o obj/Base.o obj/Interface.o obj/Request.o $(LDFLAGS) -L../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz

bin:
	if [ ! -d bin ]; then mkdir bin; fi;

../../common/libcommon.a: ../../common/Makefile
	cd ../../common; make;

../../common/Makefile: ../../common/configure
	cd ../../common; ./configure;

../../common/configure:
	cd ../../; git clone https://github.com/benkietzman/common.git

obj/log.o: log.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c log.cpp -o obj/log.o $(CPPFLAGS) -I../../common

obj/mysql.o: mysql.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c mysql.cpp -o obj/mysql.o $(CPPFLAGS) -I../../common

obj/request.o: request.cpp ../../common/Makefile obj
	g++ -ggdb -Wall -c request.cpp -o obj/request.o $(CPPFLAGS) -I../../common

obj/Base.o: ../include/Base.cpp obj
	g++ -ggdb -Wall -c ../include/Base.cpp -o obj/Base.o $(CPPFLAGS) -I../../common

obj/Interface.o: include/Interface.cpp obj
	g++ -ggdb -Wall -c include/Interface.cpp -o obj/Interface.o $(CPPFLAGS) -I../../common

obj/Log.o: include/Log.cpp obj
	g++ -ggdb -Wall -c include/Log.cpp -o obj/Log.o $(CPPFLAGS) -I../../common

obj/Mysql.o: include/Mysql.cpp obj
	g++ -ggdb -Wall -c include/Mysql.cpp -o obj/Mysql.o $(CPPFLAGS) -I../../common

obj/Request.o: include/Request.cpp obj
	g++ -ggdb -Wall -c include/Request.cpp -o obj/Request.o $(CPPFLAGS) -I../../common

obj:
	if [ ! -d obj ]; then mkdir obj; fi;

install: bin/log bin/mysql bin/request $(prefix)/radial/interface
	install --mode=777 bin/log $(prefix)/radial/interface/
	install --mode=777 bin/mysql $(prefix)/radial/interface/
	install --mode=777 bin/request $(prefix)/radial/interface/

$(prefix)/radial/interface: $(prefix)/radial
	if [ ! -d $(prefix)/radial/interface ]; then mkdir $(prefix)/radial/interface; fi;

$(prefix)/radial:
	if [ ! -d $(prefix)/radial ]; then mkdir $(prefix)/radial; fi;

clean:
	-rm -fr obj bin
