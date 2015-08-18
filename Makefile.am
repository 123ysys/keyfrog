# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.9
ACLOCAL_AMFLAGS = -I m4

# FIXME
kfdocdir=$(pkgdatadir)/doc

if COND_KEYVIS
  MAYBE_KEYVIS = keyvis
endif

SUBDIRS = src doc $(MAYBE_KEYVIS)
DIST_SUBDIRS = src doc

EXTRA_DIST=LICENSE.BSD \
COPYING \
Doxyfile \
autogen.sh \
keyvis/keyvis \
keyvis/build.xml \
keyvis/Makefile \
keyvis/README \
keyvis/manifest.mf \
keyvis/libs/README \
keyvis/nbproject/build-impl.xml \
keyvis/nbproject/genfiles.properties \
keyvis/nbproject/project.properties \
keyvis/nbproject/project.xml \
keyvis/src/org/keyfrog/gfx/Keyfrog_flat_pho.gif \
keyvis/src/org/keyfrog/keyvis/AboutWin.form \
keyvis/src/org/keyfrog/keyvis/AboutWin.java \
keyvis/src/org/keyfrog/keyvis/KeyVisWin.form \
keyvis/src/org/keyfrog/keyvis/KeyVisWin.java \
keyvis/src/org/keyfrog/keyvis/LineChartFactory.java \
keyvis/src/org/keyfrog/keyvis/Main.java
