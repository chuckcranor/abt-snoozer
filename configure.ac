#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([abt-snoozer], [0.1], [],[],[]) 
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

AC_CANONICAL_TARGET
AC_CANONICAL_SYSTEM
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])

# we should remove this soon, only needed for automake 1.10 and older
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([README.md])
AC_CONFIG_HEADERS([abt-snoozer-config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CXXCPP

AC_PROG_MKDIR_P

AC_REQUIRE_CPP

AC_CHECK_SIZEOF([long int])

dnl
dnl Verify pkg-config
dnl
PKG_PROG_PKG_CONFIG
if test "x$PKG_CONFIG" == "x"; then
   AC_MSG_ERROR([Could not find pkg-config utility!])
fi

PKG_CHECK_MODULES([ARGOBOTS],[argobots],[],
   [AC_MSG_ERROR([Could not find working argobots installation!])])
LIBS="$ARGOBOTS_LIBS $LIBS"
CPPFLAGS="$ARGOBOTS_CFLAGS $CPPFLAGS"
CFLAGS="$ARGOBOTS_CFLAGS $CFLAGS"

dnl =========================================================================
dnl ==== Check for libev ====================================================
dnl =========================================================================

AC_ARG_WITH([libev],
            [AS_HELP_STRING([[--with-libev=[DIR]]], [Use libev from specified directory])],
            [
                CPPFLAGS="$CPPFLAGS -I${with_libev}/include"
                # note: add rpath too because stock libev install uses
                # shared libs
                LDFLAGS="$LDFLAGS -Wl,-rpath=${with_libev}/lib -L${with_libev}/lib"
                SNOOZER_PKGCONFIG_LIBS="${SNOOZER_PKGCONFIG_LIBS} -L${with_libev}/lib -lev -lm"
            ],
            [
                SNOOZER_PKGCONFIG_LIBS="${SNOOZER_PKGCONFIG_LIBS} -lev -lm"
            ]
            )

AC_CHECK_HEADER([ev.h],[],
         [AC_MSG_ERROR([Could not find libev header in
          ${with_libev}/include!])])

AC_CHECK_LIB([ev],[ev_once],[evok=1; LIBS="-lev $LIBS"],
             [evok=0])
if test "A$evok" == "A0"; then
   AC_CHECK_LIB([m], [log], [], [])
   dnl Needs to be a different function to avoid hitting cache
   AC_CHECK_LIB([ev], [ev_break], [],
             [AC_MSG_ERROR([Could not find useable
           libev library at prefix ${with_libev}/lib!])])
fi


AC_SUBST(SNOOZER_PKGCONFIG_LIBS)
AC_CONFIG_FILES([Makefile maint/abt-snoozer.pc])
AC_OUTPUT
 
