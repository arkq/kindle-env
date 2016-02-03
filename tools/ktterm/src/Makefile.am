# ktterm - Makefile.am
# Copyright (c) 2013-2016 Arkadiusz Bokowy

bin_PROGRAMS = ktterm

ktterm_SOURCES = \
	main.c \
	keyboard.c \
	ktutils.c

ktterm_CFLAGS = \
	-DDATADIR=\"$(pkgdatadir)\" \
	@JSONC_CFLAGS@ \
	@GTK_CFLAGS@ \
	@VTE_CFLAGS@

ktterm_LDADD = \
	@JSONC_LIBS@ \
	@GTK_LIBS@ \
	@VTE_LIBS@

ktpatch.o: ktpatch.c
	$(CC) $(CFLAGS) $(ktterm_CFLAGS) -c -o ktpatch.o "$(srcdir)/ktpatch.c"

kindle-relink-local: $(ktterm_OBJECTS) ktpatch.o
	$(CCLD) $(CFLAGS) -o ktterm $(ktterm_OBJECTS) ktpatch.o \
		-Wl,-rpath-link=$(top_srcdir)/../../kindle/usr/lib \
		$(top_srcdir)/../../kindle/usr/lib/libglib-2.0.so \
		$(top_srcdir)/../../kindle/usr/lib/libgobject-2.0.so \
		$(top_srcdir)/../../kindle/usr/lib/libgtk-x11-2.0.so \
		$(top_srcdir)/../../kindle/usr/lib/libgdk-x11-2.0.so \
		$(top_srcdir)/../../kindle/usr/lib/libgdk_pixbuf-2.0.so \
		$(top_srcdir)/../../kindle/usr/lib/libatk-1.0.so \
		$(top_srcdir)/../../kindle/usr/lib/libcairo.so \
		$(top_srcdir)/../../kindle/usr/lib/libgio-2.0.so \
		$(top_srcdir)/../../kindle/usr/lib/libpango-1.0.so \
		$(top_srcdir)/../../kindle/usr/lib/libpangocairo-1.0.so.0 \
		$(top_srcdir)/../../kindle/usr/lib/libXinerama.so \
		-lX11 -lm -lncurses \
		-Wl,-Bstatic -ljson-c -lvte -Wl,-Bdynamic
