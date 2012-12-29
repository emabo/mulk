#-----------------------------------------------------------------------------
# Copyright (C) 2008, 2009, 2010, 2011, 2012 - Emanuele Bovisio
#
# This file is part of Mulk.
#
# Mulk is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Mulk is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Mulk.  If not, see <http://www.gnu.org/licenses/>.
#
# In addition, as a special exception, the copyright holders give
# permission to link the code of portions of this program with the
# OpenSSL library under certain conditions as described in each
# individual source file, and distribute linked combinations
# including the two.
# You must obey the GNU General Public License in all respects
# for all of the code used other than OpenSSL.  If you modify
# file(s) with this exception, you may extend this exception to your
# version of the file(s), but you are not obligated to do so.  If you
# do not wish to do so, delete this exception statement from your
# version.  If you delete this exception statement from all source
# files in the program, then also delete it here.
#-----------------------------------------------------------------------------

AC_PREREQ(2.61)
AC_INIT([mulk], [0.7.0], [http://sourceforge.net/projects/mulk])
LT_PREREQ([2.2.6])
LT_INIT
AM_INIT_AUTOMAKE([mulk], [0.7.0])
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
SHARED_VERSION_INFO="0:1:0"

# Initialize maintainer mode
AM_MAINTAINER_MODE

# Check for package options
AC_ARG_ENABLE([debug],
  [  --enable-debug          turn on debugging [[default=no]]],
  [enable_debug=$enableval], [enable_debug=no])
AC_ARG_ENABLE([recursion],
  [  --enable-recursion      enable recursion support [[default=yes]]],
  [enable_recursion=$enableval], [enable_recursion=yes])
AC_ARG_ENABLE([metalink],
  [  --enable-metalink       enable Metalink support [[default=yes]]],
  [enable_metalink=$enableval], [enable_metalink=yes])
AC_ARG_ENABLE([checksum],
  [  --enable-checksum       enable checksum verification support [[default=yes]]],
  [enable_checksum=$enableval], [enable_checksum=yes])
AC_ARG_ENABLE([nls],
  [  --enable-nls            enable Native Language Support [[default=yes]]],
  [enable_nls=$enableval], [enable_nls=yes])

# Checks for arguments.
AC_ARG_WITH(libcurl-includes,
  AC_HELP_STRING([--with-libcurl-includes=DIR], [Specify location of libcurl headers]),[
  CFLAGS="$CFLAGS -I$withval"
])

AC_ARG_WITH(libcurl-libs,
  AC_HELP_STRING([--with-libcurl-libs=DIR],[Specify location of libcurl libs]),[
  LDFLAGS="$LDFLAGS -L$withval"
])

AC_ARG_WITH(libcurl,
  AC_HELP_STRING([--with-libcurl=DIR],[Where to look for libcurl]),[
  OPT_CURL="$withval"
  if test X"$OPT_CURL" = Xno; then
    AC_MSG_ERROR([Cannot compile without libcurl. Try at http://curl.haxx.se/])
  else
    if test X"$OPT_CURL" = Xyes; then
      AC_MSG_WARN([libcurl is always enabled. Cannot compile without it.])
    else
      CFLAGS="$CFLAGS -I$OPT_CURL/include"
      LDFLAGS="$LDFLAGS -L$OPT_CURL/lib"
    fi
  fi
])

AC_ARG_WITH(uriparser-includes,
  AC_HELP_STRING([--with-uriparser-includes=DIR], [Specify location of uriparser headers]),[
  CFLAGS="$CFLAGS -I$withval"
])

AC_ARG_WITH(uriparser-libs,
  AC_HELP_STRING([--with-uriparser-libs=DIR],[Specify location of uriparser libs]),[
  LDFLAGS="$LDFLAGS -L$withval"
])

AC_ARG_WITH(uriparser,
  AC_HELP_STRING([--with-uriparser=DIR],[Where to look for uriparser]),[
  OPT_URIPARSER="$withval"
  if test X"$OPT_URIPARSER" = Xno; then
    AC_MSG_ERROR([Cannot compile without liburiparser. Try at http://uriparser.sourceforge.net/])
  else
    if test X"$OPT_URIPARSER" = Xyes; then
      AC_MSG_WARN([uriparser is always enabled. Cannot compile without it.])
    else
      CFLAGS="$CFLAGS -I$OPT_URIPARSER/include"
      LDFLAGS="$LDFLAGS -L$OPT_URIPARSER/lib"
    fi
  fi
])

AC_ARG_WITH(libmetalink-includes,
  AC_HELP_STRING([--with-libmetalink-includes=DIR], [Specify location of libmetalink headers]),[
  CFLAGS="$CFLAGS -I$withval"
  enable_metalink=yes
])

AC_ARG_WITH(libmetalink-libs,
  AC_HELP_STRING([--with-libmetalink-libs=DIR],[Specify location of libmetalink libs]),[
  LDFLAGS="$LDFLAGS -L$withval"
  enable_metalink=yes
])

AC_ARG_WITH(libmetalink,
  AC_HELP_STRING([--with-libmetalink=DIR],[Where to look for libmetalink]),[
  OPT_METALINK="$withval"
  if test X"$OPT_METALINK" = Xno; then
    enable_metalink=no
  else
    if test X"$OPT_METALINK" = Xyes; then
      enable_metalink=yes
    else
      CFLAGS="$CFLAGS -I$OPT_METALINK/include"
      LDFLAGS="$LDFLAGS -L$OPT_METALINK/lib"
      enable_metalink=yes
    fi
  fi
])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])

# Checks for libraries.
AC_CHECK_LIB([curl], [curl_multi_init], [], AC_MSG_ERROR([Cannot compile without libcurl. see README.]))
AC_CACHE_VAL(my_cv_curl_vers,[
  my_cv_curl_vers=NONE
  check="7.19.4"
  check_hex="071304" 

  AC_MSG_CHECKING([for curl >= $check])

  if eval curl-config --version 2>/dev/null >/dev/null; then
    ver=`curl-config --version | sed -e "s/libcurl //g"`
    hex_ver=`curl-config --vernum | tr 'a-f' 'A-F'`
    ok=`echo "ibase=16; if($hex_ver>=$check_hex) $hex_ver else 0" | bc`
    my_cv_curl_vers="$ver"

    if test x$ok != x0; then
      AC_MSG_RESULT([$my_cv_curl_vers])
    else
      AC_MSG_RESULT(FAILED)
	  AC_MSG_WARN([$ver is not recommended. Need version $check or higher to perform intensive downloads.])
    fi
  else
    AC_MSG_RESULT(FAILED)
    AC_MSG_WARN([curl-config was not found])
  fi
])

AC_CHECK_LIB([uriparser], [uriParseUriA], [], AC_MSG_ERROR([Cannot compile without liburiparser. see README.]))

if test "x$enable_recursion" = "xyes"; then
  AC_CHECK_LIB([tidy], [tidyGetChild], [], AC_MSG_ERROR([Cannot compile without libtidy. Disable recursion support (--disable-recursion) or see README.]))
fi

if test "x$ac_cv_lib_tidy_tidyGetChild" = "xyes"; then
  AC_DEFINE([ENABLE_RECURSION], [1], [Define to 1 if recursion support is enabled.])
  AM_CONDITIONAL([ENABLE_RECURSION], true)
else
  enable_recursion=no
  AM_CONDITIONAL([ENABLE_RECURSION], false)
fi

if test "x$enable_metalink" = "xyes"; then
  AC_CHECK_LIB([metalink], [metalink_parse_file], [], AC_MSG_ERROR([Cannot compile without libmetalink. Disable Metalink support (--disable-metalink) or try at http://launchpad.net/libmetalink/]))

  if test "x$enable_checksum" = "xyes"; then
    AC_CHECK_LIB([crypto],[SHA1_Init], [], AC_MSG_ERROR([Cannot compile without libcrypto. Disable checksum verification support (--disable-checksum) or try at http://www.openssl.org/]))
  fi
fi

if test "x$ac_cv_lib_metalink_metalink_parse_file" = "xyes"; then
  AC_DEFINE([ENABLE_METALINK], [1], [Define to 1 if Metalink support is enabled.])
  AM_CONDITIONAL([ENABLE_METALINK], true)

  if test "x$ac_cv_lib_crypto_SHA1_Init" = "xyes"; then
    AC_DEFINE([ENABLE_CHECKSUM], [1], [Define to 1 if Metalink checksum verification support is enabled.])
    AM_CONDITIONAL([ENABLE_CHECKSUM], true)
  else
    enable_checksum=no
    AM_CONDITIONAL([ENABLE_CHECKSUM], false)
  fi
else
  enable_metalink=no
  AM_CONDITIONAL([ENABLE_METALINK], false)
  enable_checksum=no
  AM_CONDITIONAL([ENABLE_CHECKSUM], false)
fi

if test "x$enable_debug" = "xyes"; then
  AC_DEFINE([DEBUG], [1], [Define to 1 if debug is enabled.])
fi
AM_CONDITIONAL([DEBUG], [test "x$enable_debug" = "xyes"])
if test "x$enable_nls" = "xyes"; then
  AC_DEFINE([ENABLE_NLS], [1], [Define to 1 if NLS is enabled.])
fi
AM_CONDITIONAL([ENABLE_NLS], [test "x$enable_nls" = "xyes"])
AM_CONDITIONAL([ENABLE_STATIC], [test "x$enable_static" = "xyes"])
AM_CONDITIONAL([ENABLE_SHARED], [test "x$enable_shared" = "xyes"])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS([unistd.h malloc.h ctype.h errno.h stdarg.h getopt.h stdint.h libintl.h])
AC_CHECK_HEADERS([curl/curl.h curl/multi.h],, AC_MSG_ERROR([Cannot compile without libcurl. Try at http://curl.haxx.se/]))
if test "x$enable_recursion" = "xyes"; then
  AC_CHECK_HEADER([tidy.h],
    [AC_DEFINE([HAVE_TIDY_H], [1], [Define to 1 if you have the <tidy.h> header file.])],
    [AC_CHECK_HEADER([tidy/tidy.h],
      [AC_DEFINE([HAVE_TIDY_TIDY_H], [1], [Define to 1 if you have the <tidy/tidy.h> header file.])],
      [AC_MSG_ERROR([Cannot compile without libtidy. Try at http://tidy.sourceforge.net/])])
    ]
  )
fi
AC_CHECK_HEADER([uriparser/Uri.h],, AC_MSG_ERROR([Cannot compile without liburiparser. Try at http://uriparser.sourceforge.net/]))
if test "x$enable_checksum" = "xyes"; then
  AC_CHECK_HEADER([openssl/sha.h],, AC_MSG_ERROR([Cannot compile without libcrypto. Disable checksum verification support (--disable-checksum) or try at http://www.openssl.org/])) 
  AC_CHECK_HEADER([openssl/md2.h], [AC_DEFINE([HAVE_OPENSSL_MD2], [1], [Define to 1 if you have the <openssl/md2.h> header file.])])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_C_CONST
gl_VISIBILITY
AC_SYS_LARGEFILE

# Checks for library functions.
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
case "$target" in
  *mingw*)
    ;;
  *)
    AC_FUNC_MALLOC
    AC_FUNC_REALLOC
    ;;
esac
AC_CHECK_FUNCS([mkdir select truncate rename remove rmdir])
AC_CHECK_FUNCS([strchr strstr strcmp strncmp strcpy strncpy strlen strerror tolower strtok])
AC_CHECK_FUNCS([malloc calloc free])
AC_CHECK_FUNCS([vsnprintf])
AC_FUNC_FSEEKO

# It should compile also with -Werror
if test "X$GCC" = "Xyes"; then
  CFLAGS="${CFLAGS} -Wshadow -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls"
  if test "x$enable_debug" = "xyes"; then
    CFLAGS="${CFLAGS} -Werror"
  fi
fi

AC_SUBST(SHARED_VERSION_INFO)
AC_CONFIG_FILES([Makefile include/Makefile include/mulk/Makefile lib/Makefile src/Makefile man/Makefile po/Makefile.in win32/Makefile win32/libmulk/Makefile win32/mulk/Makefile])
AC_OUTPUT

AC_MSG_NOTICE([Configured to build mulk:

  mulk version:    ${VERSION}
  libmulk version: ${SHARED_VERSION_INFO}
  libcurl version: ${my_cv_curl_vers}
  Build:           ${build}
  Install prefix:  ${prefix}
  Compiler:        ${CC}
  CFLAGS:          ${CFLAGS}
  LDFLAGS:         ${LDFLAGS}
  LIBS:            ${LIBS}
  static library   ${enable_static}
  shared library   ${enable_shared}
  Debug:           ${enable_debug}
  Recursion:       ${enable_recursion}
  Metalink:        ${enable_metalink}
  Checksum:        ${enable_checksum}
  NLS:             ${enable_nls}
])


