# Keyfrog

The repository contains Keyfrog & Keyvis source.

Example Keyfrog run:

    $ keyfrog --nb --display $DISPLAY

KeyFrog monitors the keyboard and visualizes its usage statistics. The user
can obtain detailed information about keyboard activity: the intensity of
keyboard usage, how was it distributed in time, which applications were
used, etc. This may be very useful, for example, to developers to monitor
their productivity. The environment being monitored is the X Window System
(text applications are explicitly supported if run inside an X terminal).

## Building

Generic installation instructions are described in INSTALL. In short, run:

    $ ./autogen.sh ; ./configure ; make && make install

Running autogen.sh might be optional -- if "configure" already exists, skip
autogen. On the other hand, rerunning autogen.sh might help in case of
unexpected configure problems.

To include Keyvis, add --with-keyvis option to configure. It will use Ant to
build NetBeans project. You will need jars (look below) and Java developer
tools (JDK). Instead of building Keyvis you can also download a binary package
from sf.net.


## Using Keyfrog
### 1. Daemon - KEYFROG

Try running in foreground (keyfrog --nb, also look at --help) -- observing
keyfrog debug output makes it easy to tune configuration (~/.keyfrog/config).
The output is also saved to ~/.keyfrog/keyfrog.log (unless the configure
option --disable-debug is used).

Look at doc/sample-config. You probably want to copy it to ~/.keyfrog/config.
It's easy to edit.

Keyfrog collects keyboard usage statistics into ~/.keyfrog/keyfrog.db
database (SQLite). From there, separate GUI application "Keyvis"
presents visualization.


### 2. Visualization - KEYVIS

Use Keyvis to visualize collected data. Its sources are in keyvis
sub-directory. Make sure all jar files are downloaded before building
the NetBeans project (look into keyvis/README). Keyvis is installed in
/opt/keyvis by default. Make a symlink:

    $ ln -s /opt/keyvis/keyvis /usr/bin

Then start Keyvis by typing "keyvis" in shell.

You can also download a binary release (zip file) from SourceForge. It
includes the jars and is ready to run.

Charts will be interesting after running keyfrog daemon for a few days or
more.

For more information about Keyfrog go to SourceForge project page:
http://sf.net/projects/keyfrog

## RECORD extension

Keyfrog relies on RECORD extension of X11 server. On Ubuntu, install packages
libxtst6, libxtst-dev -- these are for Xorg server. Other package names:
xorg-libXtst (macports). After installing, restart your desktop.

On OS X the RECORD extension needs to be enabled first (restart X11 after
issuing one of the "defaults" command):

    # if you use MacPorts Xserver (Xquartz)
    $ defaults write org.macports.X11 enable_test_extensions -boolean true

    # if you use Apple's original Xquartz Xserver
    $ defaults write org.x.X11 enable_test_extensions -boolean true

    # if you use the open source Xquartz Xserver (OS X 10.7 and later)
    $ defaults write org.macosforge.xquartz.X11 enable_test_extensions -boolean true

Keyfrog is capable of monitoring applications being run INSIDE ANY X11
TERMINAL. So you can track whether you use "vim" or "mc", or other console
program. This feature reads /proc file system -- so you can "only" monitor
applications that run on the same machine on which keyfrog daemon runs. X11
server can be remote.

