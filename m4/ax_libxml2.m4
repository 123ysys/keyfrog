#
# AX_LIBXML2([MINIMUM_VERSION])
#

AC_DEFUN([AX_LIBXML2],
[
  xml_min_ver=$1
  xml_config=xml2-config
  have_libxml2=no
  LIBXML2_CFLAGS=
  LIBXML2_LIBS=

  AC_ARG_WITH([libxml2], AC_HELP_STRING([--with-libxml2@<:@=ARG@:>@],
	      [use libxml2 library from a standard location (ARG=yes)
		  or from the specified location (ARG=<path>) @<:@ARG=yes@:>@]))

  if test "x$with_libxml2" = "xno"; then
      AC_MSG_ERROR([libxml2 is required, no disable is possible (--with-libxml2=no given)])
  fi

  if test "x$with_libxml2" = "xyes"; then
	with_libxml2=
  fi

  if test "x$with_libxml2" != x; then
	xml_config=$with_libxml2/bin/$xml_config
  fi

  AC_MSG_CHECKING([libxml2 $xml_config >= $xml_min_ver])
  if ! $xml_config --version > /dev/null 2>&1; then
	AC_MSG_ERROR([libxml2 not found (see config.log for details).])
  fi

  ver=`$xml_config --version | $AWK -F. '{ printf "%d", ([$]1 * 1000 + [$]2) * 1000 + [$]3}'`
  min_ver=`echo $xml_min_ver | $AWK -F. '{ printf "%d", ([$]1 * 1000 + [$]2) * 1000 + [$]3}'`
  if test "$ver" -ge "$min_ver"; then
	LIBXML2_LIBS="`$xml_config --libs`"
	LIBXML2_CFLAGS="`$xml_config --cflags`"
	have_libxml2=yes
	AC_MSG_RESULT([yes])
  else
	AC_MSG_ERROR([libxml2 >= $xml_min_ver not found])
  fi

  AC_SUBST([LIBXML2_CFLAGS])
  AC_SUBST([LIBXML2_LIBS])
])

