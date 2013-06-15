# ktterm - Makefile.am
# Copyright (c) 2013 Arkadiusz Bokowy

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

static-relink: $(ktterm_OBJECTS)
	$(CCLD) $(CFLAGS) -o ktterm $(ktterm_OBJECTS) @GTK_LIBS@ -lX11 -lm -lncurses -Wl,-Bstatic -ljson-c -lvte -Wl,-Bdynamic
