#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT([GeanyVG], [0.1], [ban@herbesfolles.org])
AC_CONFIG_SRCDIR([src/gvg-plugin.c])
AC_CONFIG_AUX_DIR([build/auxf])
AC_CONFIG_MACRO_DIR([build/m4])
AM_INIT_AUTOMAKE([1.10 -Wall -Werror foreign])
AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
LT_PREREQ([2.2.0])
LT_INIT
AC_PROG_CC

AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums], [no])
AC_SUBST([GLIB_MKENUMS])
AS_IF([test "x$GLIB_MKENUMS" = "xno"],
      [AC_MSG_ERROR([glib-mkenums not found, are GLib dev tools installed?])])

# Checks for libraries.
PKG_CHECK_MODULES([GVG], [glib-2.0 >= 2.16
                          gio-2.0
                          gtk+-2.0 >= 2.14
                          libxml-2.0])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([pipe close kill waitpid],,
               [AC_MSG_ERROR([some required system functions are missing])])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
