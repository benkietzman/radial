# Radial
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2025-02-27
# copyright  : kietzman.org
# email      : ben@kietzman.org

prefix=/usr/local

all: bin/centralmon

bin/centralmon: ../../../common/libcommon.a obj/centralmon.o bin
	g++ -o bin/centralmon obj/centralmon.o $(LDFLAGS) -L../../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz

bin:
	if [ ! -d bin ]; then mkdir bin; fi;

../../../common/libcommon.a: ../../../common/Makefile
	cd ../../../common; make;

../../../common/Makefile: ../../../common/configure
	cd ../../../common; ./configure;

../../../common/configure:
	cd ../../../; git clone https://github.com/benkietzman/common.git

obj/centralmon.o: centralmon.cpp ../../../common/Makefile obj
	g++ -ggdb -Wall -c centralmon.cpp -o obj/centralmon.o $(CPPFLAGS) -I../../../common

obj:
	if [ ! -d obj ]; then mkdir obj; fi;

install: bin/centralmon $(prefix)/radial/app/centralmon
	install --mode=775 bin/centralmon $(prefix)/radial/app/centralmon/

$(prefix)/radial/app/centralmon:
	mkdir -p $(prefix)/radial/app/centralmon

clean:
	-rm -fr obj bin

uninstall:
	-rm -f $(prefix)/radial/app/centralmon
