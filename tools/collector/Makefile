#
#   kbooks - makefile
#   Copyright (c) 2012 Arkadiusz Bokowy
#

CC      = armv7a-softfp-linux-gnueabi-gcc
PKGC    = armv7a-softfp-linux-gnueabi-pkg-config
STRIP   = armv7a-softfp-linux-gnueabi-strip
#CC      = gcc
#PKGC    = pkg-config

CFLAGS  = -pipe -Wall -O2
CFLAGS += `${PKGC} --cflags glib-2.0`
LDFLAGS  = `${PKGC} --libs glib-2.0`
LDFLAGS += -luuid -lsqlite3 -lcurl -lssl -lcrypto

OBJS    = main.o find.o
PROG    = kbooks

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS)

%.o: src/%.c
	$(CC) $(CFLAGS) -c $<

strip:
	$(STRIP) $(PROG)

clean:
	rm -f *.o
	rm -rf kmpkg/

build: $(PROG) strip
	mkdir -p kmpkg/usr/bin/
	cp kbooks kmpkg/usr/bin/
	tar --transform 's/^kmpkg//' -czf kbooks-0.1.0.ktgz kmpkg/*
