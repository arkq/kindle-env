# collector - Makefile.am
# Copyright (c) 2013 Arkadiusz Bokowy

bin_PROGRAMS = collector

collector_SOURCES = \
	main.c \
	find.c

collector_CFLAGS = \
	@GLIB20_CFLAGS@ \
	@LIBCURL_CFLAGS@ \
	@SQLITE_CFLAGS@ \
	@UUID_CFLAGS@

collector_LDADD = \
	@GLIB20_LIBS@ \
	@LIBCURL_LIBS@ \
	@SQLITE_LIBS@ \
	@UUID_LIBS@
