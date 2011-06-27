#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([tsparser], [20110219], [koukou@users.sourceforge.jp])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/tsparser.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MKTIME
AC_CHECK_FUNCS([memmove memset])

AC_CONFIG_FILES([Makefile
                 src/ARIB/Makefile
                 src/Makefile
                 src/TS/Makefile])
AC_OUTPUT
