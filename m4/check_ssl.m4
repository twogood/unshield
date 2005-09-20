dnl @synopsis CHECK_SSL
dnl
dnl This macro will check various standard spots for OpenSSL including
dnl a user-supplied directory. The user uses '--with-ssl' or
dnl '--with-ssl=/path/to/ssl' as arguments to configure.
dnl
dnl If OpenSSL is found the include directory gets added to CFLAGS and
dnl CXXFLAGS as well as '-DHAVE_SSL', '-lssl' & '-lcrypto' get added to
dnl LIBS, and the libraries location gets added to LDFLAGS. Finally
dnl 'HAVE_SSL' gets set to 'yes' for use in your Makefile.in I use it
dnl like so (valid for gmake):
dnl
dnl     HAVE_SSL = @HAVE_SSL@
dnl     ifeq ($(HAVE_SSL),yes)
dnl         SRCS+= @srcdir@/my_file_that_needs_ssl.c
dnl     endif
dnl
dnl For bsd 'bmake' use:
dnl
dnl     .if ${HAVE_SSL} == "yes"
dnl         SRCS+= @srcdir@/my_file_that_needs_ssl.c
dnl     .endif
dnl
dnl @category InstalledPackages
dnl @author Mark Ethan Trostler <trostler@juniper.net>
dnl @version 2003-01-28
dnl @license AllPermissive

AC_DEFUN([CHECK_SSL],
[AC_MSG_CHECKING(if ssl is wanted)
AC_ARG_WITH(ssl,
[  --with-ssl enable ssl [will check /usr/local/ssl
                            /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /usr ]
],
[   AC_MSG_RESULT(yes)
    for dir in $withval /usr/local/ssl /usr/lib/ssl /usr/ssl /usr/pkg /usr/local /usr; do
        ssldir="$dir"
        if test -f "$dir/include/openssl/ssl.h"; then
            found_ssl="yes";
            CFLAGS="$CFLAGS -I$ssldir/include/openssl -DHAVE_SSL";
            CXXFLAGS="$CXXFLAGS -I$ssldir/include/openssl -DHAVE_SSL";
            break;
        fi
        if test -f "$dir/include/ssl.h"; then
            found_ssl="yes";
            CFLAGS="$CFLAGS -I$ssldir/include/ -DHAVE_SSL";
            CXXFLAGS="$CXXFLAGS -I$ssldir/include/ -DHAVE_SSL";
            break
        fi
    done
    if test x_$found_ssl != x_yes; then
        AC_MSG_ERROR(Cannot find ssl libraries)
    else
        printf "OpenSSL found in $ssldir\n";
        LIBS="$LIBS -lssl -lcrypto";
        LDFLAGS="$LDFLAGS -L$ssldir/lib";
        HAVE_SSL=yes
    fi
    AC_SUBST(HAVE_SSL)
],
[
    AC_MSG_RESULT(no)
])
])dnl
