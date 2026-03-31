#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# Project : cobol-llm
# Licence : GNU Lesser General Public License v2.1
#
# Copyright (c) 2026 Dennis Drown

AC_PREREQ([2.69])
AC_INIT([cobol-llm], [0.1.0], [])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([lib/c/cob_curl.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign -Wall -Werror -Wno-portability])

# Checks for programs
AC_PROG_CC
AM_PROG_AR
LT_INIT([shared disable-static])


# Checks for libraries
PKG_CHECK_MODULES([CURL], [libcurl >= 7.32.0],
    [],
    [AC_MSG_ERROR([libcurl 7.32.0 or later is required])])

PKG_CHECK_MODULES([CJSON], [libcjson],
    [],
    [AC_MSG_ERROR([cJSON not found. Please install libcjson-dev.])])


# Checks for C header files
AC_CHECK_HEADERS([stdint.h string.h stdio.h],
    [],
    [AC_MSG_ERROR([Required header not found])])


# Check for GNUCobol
AC_PATH_PROG([COBC], [cobc], [no])
if test "x$COBC" = "xno"; then
    AC_MSG_ERROR([GNUCobol (cobc) not found. Please install GNUCobol.])
fi

AC_PATH_PROG([COB_CONFIG], [cob-config], [no])
if test "x$COB_CONFIG" = "xno"; then
    AC_MSG_ERROR([cob-config not found. Please install GNUCobol.])
fi

# Retrieve GNUCobol version for information
AC_MSG_CHECKING([GNUCobol version])
COBC_VERSION=`$COBC --version | head -1`
AC_MSG_RESULT([$COBC_VERSION])
AC_SUBST([COBC])

# GNUCobol compile flags
COBCFLAGS="-fixed -Wall"
AC_SUBST([COBCFLAGS])

# GNUCobol link flags for modules (no main entry point)
COBCFLAGS_MODULE="-m"
AC_SUBST([COBCFLAGS_MODULE])

# GNUCobol link flags for executable programs
COBCFLAGS_PROG="-x -fstatic-call"
AC_SUBST([COBCFLAGS_PROG])


# Identify build files
AC_CONFIG_FILES([
    Makefile
    lib/c/Makefile
    examples/Makefile
])

AC_OUTPUT

AC_MSG_NOTICE([
-------------------------------------------------
  cobol-llm version : $PACKAGE_VERSION
  GNUCobol          : $COBC_VERSION
  COBCFLAGS         : $COBCFLAGS

  C compiler        : $CC
  CFLAGS            : $CFLAGS
  libcurl CFLAGS    : $CURL_CFLAGS
  libcurl LIBS      : $CURL_LIBS
  cJSON CFLAGS      : $CJSON_CFLAGS
  cJSON LIBS        : $CJSON_LIBS
-------------------------------------------------
])
