dnl $Id$ vim: syntax=config
dnl Check for libsynce

AC_DEFUN(AM_PATH_LIBSYNCE, [

  AC_ARG_WITH(libsynce,
    AC_HELP_STRING(
      [--with-libsynce[=DIR]],
      [Search for libsynce in DIR/include and DIR/lib]),
      [
				CPPFLAGS="$CPPFLAGS -I${withval}/include"
				LDFLAGS="$LDFLAGS -L${withval}/lib"
			]
    )

  AC_ARG_WITH(libsynce-include,
    AC_HELP_STRING(
      [--with-libsynce-include[=DIR]],
      [Search for libsynce header files in DIR]),
      [
				CPPFLAGS="$CPPFLAGS -I${withval}"
			]
    )

  AC_ARG_WITH(libsynce-lib,
    AC_HELP_STRING(
      [--with-libsynce-lib[=DIR]],
      [Search for libsynce library files in DIR]),
      [
				LDFLAGS="$LDFLAGS -L${withval}"
			]
    )

	AC_CHECK_LIB(synce,main,,[
		AC_MSG_ERROR([Can't find synce library])
		])
	AC_CHECK_HEADERS(synce.h,,[
		AC_MSG_ERROR([Can't find synce.h])
		])

])
