AC_PREREQ(2.59c)

AC_INIT([pa-applet], [1.0.0], [], [pa-applet])

AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

AC_PROG_CC_C99
if test "x$ac_cv_prog_cc_c99" = "xno"; then
    AC_MSG_ERROR([C compiler must be C99 compliant])
fi
AC_HEADER_STDC

PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GTK3], [gtk+-3.0])
PKG_CHECK_MODULES([LIBPULSE], [libpulse])
PKG_CHECK_MODULES([LIBPULSE_GLIB], [libpulse-mainloop-glib])
PKG_CHECK_MODULES([LIBNOTIFY], [libnotify])
PKG_CHECK_MODULES([XLIB], [x11])
PKG_CHECK_MODULES([LIBCANBERRA], [libcanberra])

AC_CONFIG_FILES([Makefile man/Makefile src/Makefile])
AC_OUTPUT
