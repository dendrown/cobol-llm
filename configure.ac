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

AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Checks for programs
AC_PROG_CC
AM_PROG_AR
LT_INIT([shared disable-static])

# Checks for libraries
PKG_CHECK_MODULES([CURL], [libcurl >= 7.32.0],
    [],
    [AC_MSG_ERROR([libcurl 7.32.0 or later is required])])

# Checks for header files
AC_CHECK_HEADERS([stdint.h string.h stdio.h],
    [],
    [AC_MSG_ERROR([Required header not found])])

AC_CONFIG_FILES([
    Makefile
    lib/c/Makefile
])

AC_OUTPUT

AC_MSG_NOTICE([
-------------------------------------------------
  cobol-llm version : $PACKAGE_VERSION
  C compiler        : $CC
  CFLAGS            : $CFLAGS
  libcurl CFLAGS    : $CURL_CFLAGS
  libcurl LIBS      : $CURL_LIBS
-------------------------------------------------
])
