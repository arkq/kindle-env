#
#   ktterm - makefile
#   Copyright (c) 2013 Arkadiusz Bokowy
#

CC      = armv7a-softfp-linux-gnueabi-gcc
PKGC    = armv7a-softfp-linux-gnueabi-pkg-config
STRIP   = armv7a-softfp-linux-gnueabi-strip
#CC      = gcc
#PKGC    = pkg-config

CFLAGS  = -pipe -Wall -Os
CFLAGS += `${PKGC} --cflags vte`
LDFLAGS = `${PKGC} --libs gtk+-2.0` -lX11 -lm -lncurses
LDSFLAGS = -Wl,-Bstatic -ljansson -lvte -Wl,-Bdynamic

OBJS    = main.o ktutils.o keyboard.o
PROG    = ktterm

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS) $(LDSFLAGS)

%.o: src/%.c
	$(CC) $(CFLAGS) -c $<

strip:
	$(STRIP) $(PROG)

clean:
	rm -f *.o
	rm -rf kmpkg/

build: $(PROG) strip
	mkdir -p kmpkg/usr/bin/
	mkdir -p kmpkg/usr/share/ktterm/
	cp ktterm kmpkg/usr/bin/
	cp share/* kmpkg/usr/share/ktterm/
	tar --transform 's/^kmpkg//' -czf ktterm-0.1.0.ktgz kmpkg/*
