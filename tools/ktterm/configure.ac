# ktterm - configure.ac
# Copyright (c) 2013-2016 Arkadiusz Bokowy

AC_INIT([ktterm], [0.1.0], [arkadiusz.bokowy@gmail.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_PROG_CC
AM_PROG_CC_C_O


AC_ARG_ENABLE([kindle-env],
	[AS_HELP_STRING([--enable-kindle-env], [use Kindle development environment])])
AM_CONDITIONAL([ENABLE_KINDLE_ENV], [test "x$enable_kindle_env" = "xyes"])
AM_COND_IF([ENABLE_KINDLE_ENV], [
	AC_MSG_CHECKING([whether Kindle root image is mounted])
	AC_CONFIG_SRCDIR([../../kindle/etc/upstart/kindle.conf])
	AC_MSG_RESULT([yes])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#ifndef __arm__
		error: Not an ARM compiler!
		#endif]])], [],
		[AC_MSG_FAILURE([ARM-compatible compiler is required])])
	AC_SUBST([sysrootdir], [`$CC -v 2>&1 |grep -o 'sysroot=\S*' |cut -c 9-`])
])


PKG_CHECK_MODULES([JSONC], [json-c])
PKG_CHECK_MODULES([GTK], [gtk+-2.0])
PKG_CHECK_MODULES([VTE], [vte < 0.30])

AC_ARG_WITH([lipc],
	[AS_HELP_STRING([--with-lipc], [link against a proprieatary lipc library])])
AM_CONDITIONAL([WITH_LIPC], [test "x$with_lipc" = "xyes"])
AM_COND_IF([WITH_LIPC], [
	AC_CHECK_HEADERS([openlipc.h], [], [AC_MSG_ERROR([Header openlipc.h not found!])])
	AC_SUBST([LIPC_LIBS], [-llipc])
])


AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
