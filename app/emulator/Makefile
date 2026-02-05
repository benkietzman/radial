# Emulator
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2026-02-05
# copyright  : Ben Kietzman
# email      : ben@kietzman.org

all: bin/emulator

bin/emulator: ../../../common/libcommon.a obj/emulator.o bin
	g++ -o bin/emulator obj/emulator.o $(LDFLAGS) -L../../../common -lbz2 -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssh -lssl -ltar -lz

bin:
	if [ ! -d bin ]; then mkdir bin; fi;

../../../common/libcommon.a: ../../../common/Makefile
	cd ../../../common; make;

../../../common/Makefile: ../../../common/configure
	cd ../../../common; ./configure;

../../../common/configure:
	cd ../../../; git clone https://github.com/benkietzman/common.git

obj/emulator.o: emulator.cpp ../../../common/Makefile obj
	g++ -ggdb -Wall -c emulator.cpp -o obj/emulator.o $(CPPFLAGS) -I../../../common

obj:
	if [ ! -d obj ]; then mkdir obj; fi;

install: bin/emulator $(prefix)/radial/app/emulator
	install --mode=775 bin/emulator $(prefix)/radial/app/emulator/emulator_preload

install_service:
	if [ ! -f /etc/systemd/user/radial_emulator.service ]; then install --mode=644 radial_emulator.service /etc/systemd/user/; fi;

$(prefix)/radial/app/emulator:
	mkdir -p $(prefix)/radial/app/emulator

clean:
	-rm -fr obj bin

uninstall:
	-rm -f $(prefix)/radial/app/emulator
