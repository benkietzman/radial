# Radial
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2025-02-27
# copyright  : Ben Kietzman
# email      : ben@kietzman.org

prefix=/usr/local

all: bin/monitor bin/monitor_read bin/monitor_restart

bin/monitor: ../../../common/libcommon.a obj/monitor.o bin
	g++ -o bin/monitor obj/monitor.o $(LDFLAGS) -L../../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz

bin/monitor_read: ../../../common/libcommon.a obj/monitor_read.o bin
	g++ -o bin/monitor_read obj/monitor_read.o $(LDFLAGS) -L../../../common -lcommon -lb64 -lcrypto -lexpat -lmjson -lssl

bin/monitor_restart: ../../../common/libcommon.a obj/monitor_restart.o bin
	g++ -o bin/monitor_restart obj/monitor_restart.o $(LDFLAGS) -L../../../common -lcommon -lb64 -lcrypto -lexpat -lmjson -lssl

bin:
	if [ ! -d bin ]; then mkdir bin; fi;

../../../common/libcommon.a: ../../../common/Makefile
	cd ../../../common; make;

../../../common/Makefile: ../../../common/configure
	cd ../../../common; ./configure;

../../../common/configure:
	cd ../../../; git clone https://github.com/benkietzman/common.git

obj/monitor.o: monitor.cpp ../../../common/Makefile obj
	g++ -ggdb -Wall -c monitor.cpp -o obj/monitor.o $(CPPFLAGS) -I../../../common

obj/monitor_read.o: monitor_read.cpp ../../../common/Makefile obj
	g++ -ggdb -Wall -c monitor_read.cpp -o obj/monitor_read.o $(CPPFLAGS) -I../../../common

obj/monitor_restart.o: monitor_restart.cpp ../../../common/Makefile obj
	g++ -ggdb -Wall -c monitor_restart.cpp -o obj/monitor_restart.o $(CPPFLAGS) -I../../../common

obj:
	if [ ! -d obj ]; then mkdir obj; fi;

install: bin/monitor bin/monitor_restart $(prefix)/radial/app/central
	install --mode=775 bin/monitor $(prefix)/radial/app/central/monitor_preload
	install --mode=775 bin/monitor_read $(prefix)/radial/app/central/
	install --mode=775 bin/monitor_restart $(prefix)/radial/app/central/

install_service:
	if [ ! -f /lib/systemd/system/radial_central_monitor.service ]; then install --mode=644 radial_central_monitor.service /lib/systemd/system/; fi;

$(prefix)/radial/app/central:
	mkdir -p $(prefix)/radial/app/central

clean:
	-rm -fr obj bin

uninstall:
	-rm -f $(prefix)/radial/app/central
