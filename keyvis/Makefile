#
# This file is dirty.
# TODO: integrate autotools properly
#
DEPS=$(wildcard src/org/keyfrog/keyvis/* src/org/keyfrog/gfx/*)

all: jar

jar: dist/keyvis.jar
	@:

dist/keyvis.jar: $(DEPS)
	@ant jar || if :; then echo "***"; echo "*** In case of Keyvis build problems see keyvis/README"; echo "***"; exit 1; fi
	@mkdir -p dist/lib
	cp -v libs/*.jar dist/lib

clean:
	ant clean

install: jar
	mkdir -p /opt/keyvis/lib
	cp -v keyvis README dist/keyvis.jar /opt/keyvis
	cp -v dist/lib/*.jar /opt/keyvis/lib

uninstall:
	-rm -vrf /opt/keyvis

distdir:
	@:
