# [open]lipc - Makefile.am
# Copyright (c) 2016 Arkadiusz Bokowy

AM_CFLAGS = -I$(top_srcdir)/include
LDADD = -llipc

bin_PROGRAMS =

if WITH_LIPC_XXX_PROP
bin_PROGRAMS += lipc-get-prop lipc-set-prop
endif

if WITH_LIPC_PROBE
bin_PROGRAMS += lipc-probe
lipc_probe_CFLAGS = @GLIB20_CFLAGS@ @GIO20_CFLAGS@
lipc_probe_LDADD = $(LDADD) @GLIB20_LIBS@ @GIO20_LIBS@
endif

if ENABLE_KINDLE_ENV
AM_LDFLAGS = \
	-L$(sysrootdir)/usr/lib \
	-L$(top_srcdir)/../../kindle/lib \
	-L$(top_srcdir)/../../kindle/usr/lib
endif
