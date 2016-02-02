# collector - Makefile.am
# Copyright (c) 2013-2016 Arkadiusz Bokowy

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

kindle-relink-local: $(collector_OBJECTS)
	$(CCLD) $(CFLAGS) -o collector $(collector_OBJECTS) \
		$(top_srcdir)/../../kindle/lib/libuuid.so \
		$(top_srcdir)/../../kindle/usr/lib/libcurl.so \
		$(top_srcdir)/../../kindle/usr/lib/libglib-2.0.so \
		$(top_srcdir)/../../kindle/usr/lib/libsqlite3.so
