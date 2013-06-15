# ktterm - configure.ac
# Copyright (c) 2013 Arkadiusz Bokowy

AC_INIT([ktterm], [0.1.0], [arkadiusz.bokowy@gmail.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_PROG_CC
AM_PROG_CC_C_O


PKG_CHECK_MODULES(
	[JSONC], [json-c],
	[], [AC_MSG_ERROR([module json-c not found])],
)
PKG_CHECK_MODULES(
	[GTK], [gtk+-2.0],
	[], [AC_MSG_ERROR([module GTK+ 2.0 not found])],
)
PKG_CHECK_MODULES(
	[VTE], [vte],
	[], [AC_MSG_ERROR([module VTE not found])],
)


AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
