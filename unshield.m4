dnl $Id$ vim: syntax=config
dnl Check for libunshield

AC_DEFUN([AM_PATH_LIBUNSHIELD], [

  AC_ARG_WITH(libunshield,
    AC_HELP_STRING(
      [--with-libunshield[=DIR]],
      [Search for libunshield in DIR/include and DIR/lib]),
      [
				CPPFLAGS="$CPPFLAGS -I${withval}/include"
				LDFLAGS="$LDFLAGS -L${withval}/lib"
			]
    )

  AC_ARG_WITH(libunshield-include,
    AC_HELP_STRING(
      [--with-libunshield-include[=DIR]],
      [Search for libunshield header files in DIR]),
      [
				CPPFLAGS="$CPPFLAGS -I${withval}"
			]
    )

  AC_ARG_WITH(libunshield-lib,
    AC_HELP_STRING(
      [--with-libunshield-lib[=DIR]],
      [Search for libunshield library files in DIR]),
      [
				LDFLAGS="$LDFLAGS -L${withval}"
			]
    )

	AC_CHECK_LIB(unshield,unshield_open,,[
		AC_MSG_ERROR([Can't find unshield library])
		])
	AC_CHECK_HEADERS(libunshield.h,,[
		AC_MSG_ERROR([Can't find libunshield.h])
		])

])
