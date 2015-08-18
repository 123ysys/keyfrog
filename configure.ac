AC_INIT([Keyfrog],[1.1])
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE

m4_ifndef([PKG_CHECK_MODULES],  [m4_apply([AC_DEFUN], [[PKG_CHECK_MODULES], [echo "Skipping missing pkg-config (target package: $1)"]] )] )

AC_LANG([C++])
AC_PROG_CXX
AC_PROG_AWK
LT_INIT

case x"$host_os" in
  *darwin*)
    AC_DEFINE([HOST_IS_OSX],[],
              [Defined if host is OSX])
        CARBON_FRAMEWORK="-Wl,-framework,Carbon"
        AC_SUBST([CARBON_FRAMEWORK])
    ;;
  *)
    AC_DEFINE([HOST_IS_OTHER_UNIX],[],
              [Defined if host is a general Unix-like system])
    ;;
esac

#
# Boost
#

AX_BOOST_BASE([1.40],[ax_boost="yes"],[ax_boost="no"])
if ! test "x$ax_boost" = "xyes" ; then
    AC_MSG_ERROR([Boost is mandatory. Please install boost library (or try adding --with-boost=/opt/local, or similar option - see ./configure --help for more information).])
fi
AX_BOOST_SYSTEM
if test "x$BOOST_SYSTEM_LIB" = "x" ; then
    AC_MSG_ERROR([Boost.System is mandatory. Please install boost library with system support.])
fi
AX_BOOST_THREAD
if ! test "x$ax_boost_thread" = "xyes" ; then
    AC_MSG_ERROR([Boost.Thread is mandatory. Please install boost library with threads support.])
fi
AX_BOOST_FILESYSTEM
if ! test "x$ax_boost_filesystem" = "xyes" ; then
    AC_MSG_ERROR([Boost.Filesystem is mandatory. Please install boost library with filesystem support.])
fi
AX_BOOST_PROGRAM_OPTIONS
if ! test "x$ax_boost_program_options" = "xyes" ; then
    AC_MSG_ERROR([Boost.Program_options is mandatory. Please install boost library with program options support.])
fi

#
# SQLITE
#

if test "x$with_sqlite3" = "x" ; then
    PKG_CHECK_MODULES([SQLITE3], [sqlite3 >= 3.1], [have_sqlite3=yes], [have_sqlite3=no])
fi

if ! test "x$have_sqlite3" = "xyes" ; then
    echo "Searching for a non pkg-config sqlite3 installation.."
    # TODO: fix the macro to detect incorrect with-path
    AX_SQLITE3
    if ! test "x$ax_sqlite3" = "xyes" ; then
        AC_MSG_ERROR([Please install sqlite3 library (version 3.1 or greater)])
    else
        SQLITE3_LIBS="$SQLITE3_LDFLAGS"
    fi
fi

AC_SUBST([SQLITE3_CFLAGS])
AC_SUBST([SQLITE3_LIBS])

#
# LIBXML2
#

if test "x$with_libxml2" = "x" ; then
    PKG_CHECK_MODULES([LIBXML2], [libxml-2.0 >= 2.6.12], [have_libxml2=yes], [have_libxml2=no])
fi

if ! test "x$have_libxml2" = "xyes" ; then
    echo "Searching for a non pkg-config libxml2 installation.."
    AX_LIBXML2(2.6.12)

    if ! test "x$have_libxml2" = "xyes" ; then
        echo "Manually searching for libxml2.."

        #
        # Library file for libxml2
        #
        # TODO: check multiple directories, like in the includes case

        AC_CHECK_LIB([xml2], [xmlInitParser],
                    [LIBXML2_LIBS=-lxml2],
                    [LIBXML2_LIBS=])

        if test "x$LIBXML2_LIBS" = "x"; then
            echo "libxml2 library file now searched in: \$xml2_lib_dir /usr/lib64 /usr/lib /usr/local/lib /opt/lib"
            BAKLDFLAGS="$LDFLAGS"

            for i in $xml2_lib_dir /usr/lib64 /usr/lib /usr/local/lib /opt/lib; do
                LDFLAGS="-L$i"
                unset ac_cv_lib_xml2_xmlParseFile
                AC_CHECK_LIB([xml2], [xmlInitParser],
                            [LIBXML2_LIBS="-L$i -lxml2"],
                            [LIBXML2_LIBS=])
                if test "x$LIBXML2_LIBS" != "x"; then
                    echo "libxml2 library is under path: $i"
                    break
                fi
            done

            LDFLAGS="$BAKLDFLAGS"
        fi

        if test "x$LIBXML2_LIBS" = "x"; then
            AC_MSG_ERROR([Please install libxml2 library (version 2.6.12 or greater)])
        fi

        #
        # Header files for libxml2
        #

        # Are system include dirs sufficient?
        AC_TRY_COMPILE([#include <libxml/xmlmemory.h>], [int a = 1;],
            [LIBXML2_CFLAGS=" "],
            [LIBXML2_CFLAGS=]
        )

        # Check set of directories
        if test "x$LIBXML2_CFLAGS" = "x"; then
            echo "libxml2 headers now searched in: \$xml2_include_dir /usr/include/libxml2 /usr/include /usr/local/include/libxml2 /usr/local/include /opt/include/libxml2 /opt/include"
            BAKCPPFLAGS="$CPPFLAGS"

            for i in $xml2_include_dir /usr/include/libxml2 /usr/include /usr/local/include/libxml2 /usr/local/include /opt/include/libxml2 /opt/include ; do
                CPPFLAGS="-I$i"
                AC_TRY_COMPILE([#include <libxml/xmlmemory.h>], [int a = 1;],
                    [LIBXML2_CFLAGS="-I$i"],
                    [LIBXML2_CFLAGS=]
                )
                if test "x$LIBXML2_CFLAGS" != "x"; then
                    echo "libxml2 headers are under path: $i"
                    break
                fi
            done

            AC_CHECK_HEADERS([libxml/xmlmemory.h],[],[AC_MSG_ERROR([Please install libxml2 library (version 2.6.12 or greater)])])
            AC_CHECK_HEADERS([libxml/parser.h],[],[AC_MSG_ERROR([Please install libxml2 library (version 2.6.12 or greater)])])

            CPPFLAGS="$BAKCPPFLAGS"
        fi

        have_libxml2=yes
    fi
fi

AC_SUBST([LIBXML2_CFLAGS])
AC_SUBST([LIBXML2_LIBS])

#
# X11
#

if test "x$with_x" = "x" ; then
    PKG_CHECK_MODULES([X11], [x11], [have_x11=yes], [have_x11=no])
fi

if ! test "x$have_x11" = "xyes" ; then
    echo "Searching for a non pkg-config X11 environment.."
    AC_PROG_SED
    # Fall back to standard procedure of finding X11
    AC_PATH_X
    if test x"$no_x" = "yes" ; then
        AC_MSG_ERROR([Please install Xorg, Xquartz or other Xserver (including developer packages, i.e. those with -dev or -devel suffix in name)])
    fi
    if test x"$x_includes" != x"NONE" && test -n "$x_includes" ; then
        X11_CFLAGS=-I`echo $x_includes | $SED -e "s/:/ -I/g"`
    fi
    if test x"$x_libraries" != x"NONE" && test -n "$x_libraries" ; then
        X11_LIBS=-L`echo $x_libraries | $SED -e "s/:/ -L/g"`
    fi
    X11_LIBS="$X11_LIBS -lX11"
fi

AC_SUBST([X11_CFLAGS])
AC_SUBST([X11_LIBS])

#
# Xtst
#

PKG_CHECK_MODULES([XTST], [xtst], [have_xtst=yes], [have_xtst=no])
if ! test "x$have_xtst" = "xyes" ; then
    echo "Searching for a non pkg-config X test extension.."

    BAKCPPFLAGS="$CPPFLAGS"
    BAKLDFLAGS="$LDFLAGS"
    CPPFLAGS="$X11_CFLAGS"
    LDFLAGS="$X11_LIBS"

    AC_CHECK_HEADERS([X11/extensions/record.h],
                     [found_recordext=yes; break;],,[#include <X11/Xlib.h>])

    AC_CHECK_LIB([Xtst], [XRecordCreateContext], [found_recordext_lib=yes])
    if test "x$found_recordext_lib" = "xyes" ; then
        XTST_LIBS="-lXtst"
    fi

    CPPFLAGS="$BAKCPPFLAGS"
    LDFLAGS="$BAKLDFLAGS"
else
    found_recordext=yes
fi

AC_SUBST([XTST_CFLAGS])
AC_SUBST([XTST_LIBS])

#
# JAVAC, ANT
#

AC_ARG_WITH([keyvis],
      AS_HELP_STRING([--with-keyvis=@<:@ARG@:>@],[build Keyvis application @<:@default=no@:>@; you will need ant, JDK1.6 and additional jar libraries (see keyvis/README)
      ]),
      [
      if test "$withval" = "no" ; then
          want_keyvis="no"
      elif test "$withval" = "yes" ; then
          want_keyvis="yes"
      fi
      ],
      [want_keyvis="no"]
)

if test "x$want_keyvis" = "xyes" ; then
        AX_PROG_JAVA_CC(javac)
        if ! test "x$ax_javac" = "xyes" ; then
                AC_MSG_NOTICE([***])
                AC_MSG_NOTICE([*** Warning: Java JDK not found. Keyvis will not be build.])
                AC_MSG_NOTICE([***])
                want_keyvis="no"
        else
                AC_CHECK_TOOL(ANT,ant)
                if test -z "$ANT" ; then
                        AC_MSG_NOTICE([***])
                        AC_MSG_NOTICE([*** Warning: Ant build tool not found. Keyvis will not be build.])
                        AC_MSG_NOTICE([***])
                        want_keyvis="no"
                fi
        fi
fi

#
# Debug
#

want_debug="yes"
AC_ARG_ENABLE(debug,
               AS_HELP_STRING([--disable-debug],[compile without debugging info]),
               [if test "x$enableval" = "xno" ; then
                want_debug="no"
            fi])

if test "x$want_debug" = "xyes" ; then
    AC_DEFINE([_KF_DEBUG],[1],[Turns on debug messages and advanced debug functionality])
else
    AC_DEFINE([NDEBUG],[1],[Turns off standard debugging functionality])
fi

#
# Color output using terminal escape codes
#

want_colors="yes"
AC_ARG_ENABLE(colors,
               AS_HELP_STRING([--disable-colors],[dont colorify the output]),
               [if test "x$enableval" = "xno" ; then
                want_colors="no"
            fi])

if test "x$want_colors" = "xyes" ; then
    AC_DEFINE([_KF_COLORS],[1],[If enabled output messages will use colors])
fi

AM_CONDITIONAL([COND_KEYVIS], [test "x$want_keyvis" = "xyes"])

# BOOST_CPPFLAGS can have non preprocessor options like -pthread
add_LDFLAGS="$X11_LIBS $XTST_LIBS $LIBXML2_LIBS $SQLITE3_LIBS $BOOST_LDFLAGS $BOOST_SYSTEM_LIB $BOOST_THREAD_LIB $BOOST_FILESYSTEM_LIB $BOOST_PROGRAM_OPTIONS_LIB $CARBON_FRAMEWORK"
add_CXXFLAGS="$X11_CFLAGS $XTST_CFLAGS $LIBXML2_CFLAGS $SQLITE3_CFLAGS $BOOST_CPPFLAGS"

AC_CONFIG_FILES(Makefile src/Makefile doc/Makefile)
AC_OUTPUT

echo
echo "****************************************************************"
echo "Using following libraries: libxml2, boost, sqlite3"
echo "  LDFLAGS: $add_LDFLAGS"
echo "  CXXFLAGS: $add_CXXFLAGS"
if ! test "x$found_recordext" = "xyes" ; then
        echo "Warning: no RECORD extension found, Keyfrog might not compile (try installing packages libxtst & libxtst-dev or similar)"
fi
if test "x$want_debug" = "xyes" ; then
        echo "Debug support: yes"
else
        echo "Debug support: no"
fi
if test "x$want_colors" = "xyes" ; then
        echo "Color output: yes"
else
        echo "Color output: no"
fi
if test "x$want_keyvis" = "xyes" ; then
        echo "Keyvis will be build and installed to /opt/keyvis"
else
        echo "Keyvis will not be build"
fi
echo "****************************************************************"
echo

