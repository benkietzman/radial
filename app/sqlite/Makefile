# Radial
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2024-10-24
# copyright  : kietzman.org
# email      : ben@kietzman.org

prefix=/usr/local

all: bin/sqlite

bin/sqlite: ../../../common/libcommon.a obj/sqlite.o bin
	g++ -o bin/sqlite obj/sqlite.o $(LDFLAGS) -L../../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz

bin:
	if [ ! -d bin ]; then mkdir bin; fi;

../../../common/libcommon.a: ../../../common/Makefile
	cd ../../../common; make;

../../../common/Makefile: ../../../common/configure
	cd ../../../common; ./configure;

../../../common/configure:
	cd ../../../; git clone https://github.com/benkietzman/common.git

obj/sqlite.o: sqlite.cpp ../../../common/Makefile obj
	g++ -ggdb -Wall -c sqlite.cpp -o obj/sqlite.o $(CPPFLAGS) -I../../../common

obj:
	if [ ! -d obj ]; then mkdir obj; fi;

install: bin/sqlite $(prefix)/radial/app/radial
	install --mode=775 bin/sqlite $(prefix)/radial/app/sqlite/

$(prefix)/radial/app/sqlite:
	mkdir -p $(prefix)/radial/app/sqlite

clean:
	-rm -fr obj bin

uninstall:
	-rm -f $(prefix)/radial/app/sqlite
