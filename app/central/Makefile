# Radial
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2025-02-27
# copyright  : kietzman.org
# email      : ben@kietzman.org

prefix=/usr/local

all: bin/monitor

bin/monitor: ../../../common/libcommon.a obj/monitor.o bin
	g++ -o bin/monitor obj/monitor.o $(LDFLAGS) -L../../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz

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

obj:
	if [ ! -d obj ]; then mkdir obj; fi;

install: bin/monitor $(prefix)/radial/app/central
	install --mode=775 bin/monitor $(prefix)/radial/app/central/

$(prefix)/radial/app/central:
	mkdir -p $(prefix)/radial/app/central

clean:
	-rm -fr obj bin

uninstall:
	-rm -f $(prefix)/radial/app/central
