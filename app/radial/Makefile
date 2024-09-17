# Radial
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2024-01-09
# copyright  : kietzman.org
# email      : ben@kietzman.org

prefix=/usr/local

all: bin/terminal

bin/terminal: ../../../common/libcommon.a obj/terminal.o bin
	g++ -o bin/terminal obj/terminal.o $(LDFLAGS) -L../../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz

bin:
	if [ ! -d bin ]; then mkdir bin; fi;

../../../common/libcommon.a: ../../../common/Makefile
	cd ../../../common; make;

../../../common/Makefile: ../../../common/configure
	cd ../../../common; ./configure;

../../../common/configure:
	cd ../../../; git clone https://github.com/benkietzman/common.git

obj/terminal.o: terminal.cpp ../../../common/Makefile obj
	g++ -ggdb -Wall -c terminal.cpp -o obj/terminal.o $(CPPFLAGS) -I../../../common

obj:
	if [ ! -d obj ]; then mkdir obj; fi;

install: bin/terminal $(prefix)/radial/app/radial
	install --mode=775 bin/terminal $(prefix)/radial/app/radial/

$(prefix)/radial/app/radial: $(prefix)/radial/app
	mkdir $(prefix)/radial/app/radial

$(prefix)/radial/app: $(prefix)/radial
	mkdir $(prefix)/radial/app

$(prefix)/radial:
	mkdir $(prefix)/radial

clean:
	-rm -fr obj bin

uninstall:
	-rm -f $(prefix)/radial/app/radial
