dnl GVG_PATH_PROG(VAR, PROG, [action-it-found], [action-if-not-found])
dnl
dnl Checks for PROG using AC_PATH_PROG, but calling action-if-found
dnl or action-if-not-found depending on whether the program is found.
dnl VAR is set to the found program and AC_SUBST'd.
dnl
dnl The default action if action-if-not-found is not given is to call
dnl AC_MSG_ERROR with a sensible error message.
AC_DEFUN([GVG_PATH_PROG],
[
  AC_PATH_PROG([$1], [$2], [])
  AC_SUBST([$1])
  AS_IF([test "x$AS_TR_SH([$1])" = "x"],
        [AS_IF([test $# -lt 4],dnl if 4th arg missing, fallback to our default
               [AC_MSG_ERROR([Cannot find program "$2"])],
               [$4])],
        [$3])
])
