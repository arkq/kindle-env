# [open]lipc - configure.ac
# Copyright (c) 2016 Arkadiusz Bokowy

AC_INIT([openlipc], [0.1.0], [arkadiusz.bokowy@gmail.com])
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


AC_ARG_WITH([lipc-xxx-prop],
	[AS_HELP_STRING([--without-lipc-prop], [omit lipc-get/set-prop replacements])],
	[], [with_lipc_xxx_prop=yes])
AM_CONDITIONAL([WITH_LIPC_XXX_PROP], [test "x$with_lipc_xxx_prop" = "xyes"])

AC_ARG_WITH([lipc-probe],
	[AS_HELP_STRING([--without-lipc-probe], [omit lipc-probe replacement])],
	[], [with_lipc_probe=yes])
AM_CONDITIONAL([WITH_LIPC_PROBE], [test "x$with_lipc_probe" = "xyes"])
AM_COND_IF([WITH_LIPC_PROBE], [
	PKG_CHECK_MODULES([GLIB20], [glib-2.0])
	PKG_CHECK_MODULES([GIO20], [gio-2.0])
])


AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
