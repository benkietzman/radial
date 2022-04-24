# Radial
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2022-04-21
# copyright  : kietzman.org
# email      : ben@kietzman.org

prefix=/usr/local

all: bin/hub

bin/hub: ../common/libcommon.a obj/hub.o obj/Base.o obj/Hub.o bin
	g++ -o bin/hub obj/hub.o obj/Base.o obj/Hub.o $(LDFLAGS) -L../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz

bin:
	if [ ! -d bin ]; then mkdir bin; fi;

../common/libcommon.a: ../common/Makefile
	cd ../common; make;

../common/Makefile: ../common/configure
	cd ../common; ./configure;

../common/configure:
	cd ../; git clone https://github.com/benkietzman/common.git

obj/hub.o: hub.cpp ../common/Makefile obj
	g++ -ggdb -Wall -c hub.cpp -o obj/hub.o $(CPPFLAGS) -I../common

obj/Base.o: include/Base.cpp obj
	g++ -ggdb -Wall -c include/Base.cpp -o obj/Base.o $(CPPFLAGS) -I../common

obj/Hub.o: include/Hub.cpp obj
	g++ -ggdb -Wall -c include/Hub.cpp -o obj/Hub.o $(CPPFLAGS) -I../common

obj:
	if [ ! -d obj ]; then mkdir obj; fi;

install: bin/hub $(prefix)/radial
	install --mode=777 bin/hub $(prefix)/radial/hub_preload

install_service:
	if [ ! -f /lib/systemd/system/radial.service ]; then install --mode=644 radial.service /lib/systemd/system/; fi;

$(prefix)/radial:
	mkdir $(prefix)/radial

clean:
	-rm -fr obj bin

uninstall:
	-rm -f $(prefix)/radial
