#!/bin/sh
error=0

for libtoolize in glibtoolize libtoolize; do
        LIBTOOLIZE=`which $libtoolize 2>/dev/null`
        if test "$LIBTOOLIZE"; then
                break;
        fi
done

HAVE_PKG_CONFIG=1
(pkg-config --version) </dev/null >/dev/null 2>&1 || {
        echo "Warning: No pkg-config found"
        HAVE_PKG_CONFIG=0
}

(autoconf --version) </dev/null >/dev/null 2>&1 || {
        echo "Error: No autoconf found. Install packages like 'autoconf', 'automake', 'autotools-dev' (exact names depend on your system)"
        error=1
}

($LIBTOOLIZE --version) </dev/null >/dev/null 2>&1 || {
        echo "Error: No libtool found (e.g. /usr/bin/libtool). Install package 'libtool' (exact name depends on your system)"
        error=1
}

(automake --version) </dev/null >/dev/null 2>&1 || {
        echo "Error: No automake found. Install packages like 'automake', 'autotools-dev' (exact names depend on your system)"
        error=1
}

(aclocal --version) </dev/null >/dev/null 2>&1 || {
        echo "Error: No aclocal found"
        echo "aclocal should be part of automake package. Install packages like 'automake', 'autotools-dev' (exact names depend on your system)"
        error=1
}

if test "$error" -eq 1; then
        echo "Error occurred (description should be printed above), aborting"
        exit 1
fi

if ! test -f /usr/share/aclocal/pkg.m4; then
    if test -f /usr/local/share/aclocal/pkg.m4; then
        # Not too happy to go into /usr/local
        # Doing this only if pkg-config has been found
        # otherwise no messing with aclocal includes
        # TODO: other paths like /opt
        if test "$HAVE_PKG_CONFIG" -eq 1; then
            export ACLOCAL='aclocal -I /usr/local/share/aclocal'
        fi
    fi
fi

reconf() {
    if test -d "autom4te.cache"; then
        rm -rf autom4te.cache
    fi

        echo "Running aclocal..."
        aclocal -I m4
        echo "Running autoheader..."
        autoheader
        echo "Running libtoolize..."
        $LIBTOOLIZE --force --copy
        echo "Running automake..."
        automake --add-missing --copy
        echo "Running autoconf..."
        autoconf
}

if [ "$1" = "noautoreconf" ]; then
    reconf
else
    if [ "$1" = "force" ]; then
        FORCE="-f"
    fi
    if [ "$1" = "-f" ]; then
        FORCE="-f"
    fi
    autoreconf -iv $FORCE
fi

echo ""
# Only a warning message
if test "$HAVE_PKG_CONFIG" -eq 1; then
    pkgcheckdef=`grep PKG_CHECK_MODULES aclocal.m4 | grep AC_DEFUN`
    if test "x$pkgcheckdef" = "x"; then
        echo "warning: PKG_CHECK_MODULES isn't defined but pkg-config was found"
        echo ""
    fi
fi

echo "Now type './configure' to generate Makefile files for KeyFrog (use --with-keyvis to include KeyVis, the GUI)"

