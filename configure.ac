dnl Process this file with autoconf to produce a configure script.

AC_INIT([unshield],[1.4.3],[])
AC_CONFIG_AUX_DIR([autoconf])
AC_CONFIG_MACRO_DIR([autoconf-m4])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_HEADER([config.h])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
m4_ifdef([AM_MAINTAINER_MODE], [AM_MAINTAINER_MODE])

LT_INIT

dnl Checks for programs.
AC_PROG_CC
AC_C_CONST

AC_CHECK_FUNC([iconv],[AC_DEFINE(HAVE_ICONV,1,[iconv])], [])
AC_CHECK_HEADERS(inttypes.h stdint.h iconv.h unistd.h dlfcn.h fnmatch.h)
#PKG_CHECK_MODULES([ZLIB], [zlib])

AC_DEFINE_UNQUOTED([__USE_MINGW_ANSI_STDIO], 1, [Enable GNU style printf formatters])

# CMAKE compat
AC_SUBST(CMAKE_INSTALL_PREFIX, [${prefix}])
AC_SUBST(CMAKE_INSTALL_FULL_BINDIR, [${prefix}])
AC_SUBST(CMAKE_INSTALL_FULL_LIBDIR, [${libdir}])
AC_SUBST(CMAKE_INSTALL_FULL_INCLUDEDIR, [${includedir}])
AC_SUBST(PROJECT_VERSION, [${VERSION}])

CPPFLAGS="$CPPFLAGS -DAUTOTOOLS"
case "$CC" in
  gcc*) CPPFLAGS="$CPPFLAGS -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers" ;;
  *) ;;
esac

AC_OUTPUT([
Makefile
libunshield.pc
lib/Makefile
src/Makefile
])


