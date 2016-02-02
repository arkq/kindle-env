# collector - configure.ac
# Copyright (c) 2013-2016 Arkadiusz Bokowy

AC_INIT([collector], [0.1.0], [arkadiusz.bokowy@gmail.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_EXTRA_RECURSIVE_TARGETS([kindle-relink])

AC_PROG_CC
AM_PROG_CC_C_O


PKG_CHECK_MODULES(
	[GLIB20], [glib-2.0],
	[], [AC_MSG_ERROR([module glib-2.0 not found])],
)
PKG_CHECK_MODULES(
	[LIBCURL], [libcurl],
	[], [AC_MSG_ERROR([module libcurl not found])],
)
PKG_CHECK_MODULES(
	[SQLITE], [sqlite3],
	[], [AC_MSG_ERROR([module sqlite3 not found])],
)
PKG_CHECK_MODULES(
	[UUID], [uuid],
	[], [AC_MSG_ERROR([module uuid not found])],
)

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
