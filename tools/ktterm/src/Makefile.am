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
	@LIPC_LIBS@ \
	@GTK_LIBS@ \
	@VTE_LIBS@

if ENABLE_KINDLE_ENV

AM_LDFLAGS = \
	-L$(sysrootdir)/usr/lib \
	-L$(top_srcdir)/../../kindle/lib \
	-L$(top_srcdir)/../../kindle/usr/lib

ktterm_SOURCES += ktpatch.c

ktterm$(EXEEXT): $(ktterm_OBJECTS)
	@rm -f ktterm$(EXEEXT)
	$(ktterm_LINK) $(ktterm_OBJECTS) \
		@GTK_LIBS@ @LIPC_LIBS@ -lX11 -lm -lncurses \
		-Wl,-Bstatic -ljson-c -lvte -Wl,-Bdynamic

endif
