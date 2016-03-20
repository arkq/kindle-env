# ktterm - configure.ac
# Copyright (c) 2013-2016 Arkadiusz Bokowy

AC_INIT([ktterm], [0.1.0], [arkadiusz.bokowy@gmail.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_EXTRA_RECURSIVE_TARGETS([kindle-relink])

AC_PROG_CC
AM_PROG_CC_C_O


PKG_CHECK_MODULES([JSONC], [json-c])
PKG_CHECK_MODULES([GTK], [gtk+-2.0])
PKG_CHECK_MODULES([VTE], [vte < 0.30])


AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
