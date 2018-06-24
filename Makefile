VERSION      := 0
SUBVERSION   := 8
MINORVERSION := 1

#prefix := /usr
prefix := /yf/usr/local
CROSS_COMPILE := /home/steven/crosstools/x-tools/arm-cortex_a15-linux-uclibcgnueabihf/bin/

sbin := $(prefix)/sbin
man8 := $(prefix)/share/man/man8/

all: nethogs decpcap_test

runtests: test
	./test
	
	
# nethogs_testsum

CXX=$(CROSS_COMPILE)arm-cortex_a15-linux-uclibcgnueabihf-g++
CC=$(CROSS_COMPILE)arm-cortex_a15-linux-uclibcgnueabihf-gcc-6.3.0
CXXFLAGS=-g -Wall -Wextra -I/home/steven/crosstools/x-tools/arm-cortex_a15-linux-uclibcgnueabihf/arm-cortex_a15-linux-uclibcgnueabihf/sysroot/usr/include \
			  -I/home/steven/src/libpcap \
			  -L/home/steven/crosstools/x-tools/arm-cortex_a15-linux-uclibcgnueabihf/arm-cortex_a15-linux-uclibcgnueabihf/sysroot/usr/lib \
			  -L/home/steven/src/libpcap
CFLAGS=-Wall -Wextra -I/home/steven/crosstools/x-tools/arm-cortex_a15-linux-uclibcgnueabihf/arm-cortex_a15-linux-uclibcgnueabihf/sysroot/usr/include \
                          -I/home/steven/src/libpcap \
                          -L/home/steven/crosstools/x-tools/arm-cortex_a15-linux-uclibcgnueabihf/arm-cortex_a15-linux-uclibcgnueabihf/sysroot/usr/lib \
-L/home/steven/src/libpcap


#  -I  include 包含头文件
#  -L  库文件路径

#CFLAGS?=-Wall -Wextra -I$(YUNOS_ROOT)/yocto-layers/build/arago-tmp-external-linaro-toolchain/sysroots/dra7xx-evm/usr/include \
#	                    -L$(YUNOS_ROOT)/yocto-layers/build/arago-tmp-external-linaro-toolchain/sysroots/dra7xx-evm/usr/lib \
#	                    -L$(YUNOS_ROOT)/yocto-layers/build/arago-tmp-external-linaro-toolchain/sysroots/dra7xx-evm/lib


OBJS=packet.o connection.o process.o refresh.o decpcap.o cui.o inode2prog.o conninode.o devices.o

NCURSES_LIBS?=-lncurses

.PHONY: tgz

tgz: clean
	cd .. ; tar czvf nethogs-$(VERSION).$(SUBVERSION).$(MINORVERSION).tar.gz --exclude-vcs nethogs-0.8.1/*

.PHONY: check
check:
	echo "Not implemented"

install: nethogs nethogs.8
	install -d -m 755 $(DESTDIR)$(sbin)
	install -m 755 nethogs $(DESTDIR)$(sbin)
	install -d -m 755 $(DESTDIR)$(man8)
	install -m 644 nethogs.8 $(DESTDIR)$(man8)

test: test.cpp 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) test.cpp -o test -lpcap -lm ${NCURSES_LIBS} -DVERSION=\"$(VERSION)\" -DSUBVERSION=\"$(SUBVERSION)\" -DMINORVERSION=\"$(MINORVERSION)\"

nethogs: main.cpp nethogs.cpp $(OBJS)
	$(CXX) $(CXXFLAGS)  main.cpp $(OBJS) -o nethogs -lpcap -lm ${NCURSES_LIBS} -DVERSION=\"$(VERSION)\" -DSUBVERSION=\"$(SUBVERSION)\" -DMINORVERSION=\"$(MINORVERSION)\"
#lm  lib-m 用做运算的，这个集成到了libc中，在xtools中的sysroot路径下有所有的lib和头文件
#-ldbus-1 这个是自己加的，yunos自己编的libpacp需要用 ldbus-1,其头文件和lib理论上在x-tools下。
nethogs_testsum: nethogs_testsum.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) nethogs_testsum.cpp $(OBJS) -o nethogs_testsum -lpcap -lm ${NCURSES_LIBS} -DVERSION=\"$(VERSION)\" -DSUBVERSION=\"$(SUBVERSION)\" -DMINORVERSION=\"$(MINORVERSION)\"

decpcap_test: decpcap_test.cpp decpcap.o
	$(CXX) $(CXXFLAGS) decpcap_test.cpp decpcap.o -o decpcap_test -lpcap -lm

#-lefence

refresh.o: refresh.cpp refresh.h nethogs.h
	$(CXX) $(CXXFLAGS) -c refresh.cpp
process.o: process.cpp process.h nethogs.h
	$(CXX) $(CXXFLAGS) -c process.cpp
packet.o: packet.cpp packet.h nethogs.h
	$(CXX) $(CXXFLAGS) -c packet.cpp
connection.o: connection.cpp connection.h nethogs.h
	$(CXX) $(CXXFLAGS) -c connection.cpp
decpcap.o: decpcap.c decpcap.h
	$(CC) $(CXXFLAGS) -c decpcap.c
inode2prog.o: inode2prog.cpp inode2prog.h nethogs.h
	$(CXX) $(CXXFLAGS) -c inode2prog.cpp
conninode.o: conninode.cpp nethogs.h conninode.h
	$(CXX) $(CXXFLAGS) -c conninode.cpp
#devices.o: devices.cpp devices.h
#	$(CXX) $(CXXFLAGS) -c devices.cpp
cui.o: cui.cpp cui.h nethogs.h
	$(CXX) $(CXXFLAGS) -c cui.cpp -DVERSION=\"$(VERSION)\" -DSUBVERSION=\"$(SUBVERSION)\" -DMINORVERSION=\"$(MINORVERSION)\"

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -f nethogs
	rm -f test
	rm -f decpcap_test
