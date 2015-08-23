bin_PROGRAMS = keyfrog
keyfrog_SOURCES = keyfrog.cpp CallbackClosure.cpp ConfigReader.cpp Configuration.cpp Daemon.cpp Debug.cpp \
    EventFilter.cpp Event.cpp EventMonitorX11.cpp EventMonitorMac.cpp FilterConfig.cpp \
    Group.cpp Options.cpp ProcessManager.cpp ProcessManagerMac.cpp ProcessManagerLinux.cpp ProcessManagerFBSD.cpp \
    ProcessMonitor.cpp RawEvent.cpp Regex.cpp Storage.cpp StorageManager.cpp StorageSqlite.cpp \
    TermCode.cpp KfWindow.cpp KfWindowCache.cpp XErrorUtil.cpp \
    Common.cpp ProcessTree.cpp ProcessProperties.cpp ProcessMap.cpp

# libxml2 is hardcoded because of problems with ubuntu

# set the include path found by configure
AM_CXXFLAGS = $(all_includes) $(X11_CFLAGS) $(XTST_CFLAGS) $(LIBUTIL_CFLAGS) $(LIBXML2_CFLAGS) $(SQLITE3_CFLAGS) $(BOOST_CPPFLAGS)

# the library search path.
keyfrog_LDFLAGS = $(all_libraries) $(X11_LIBS) $(XTST_LIBS) $(LIBUTIL_LIBS) $(LIBXML2_LIBS) $(SQLITE3_LIBS) $(CARBON_FRAMEWORK)

keyfrog_LDADD = $(BOOST_SYSTEM_LIB) $(BOOST_THREAD_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_PROGRAM_OPTIONS_LIB)

noinst_HEADERS = CallbackClosure.h ConfigReader.h Configuration.h Daemon.h Debug.h \
		EventFilter.h Event.h EventInternal.h EventMonitor.h EventMonitorX11.h EventMonitorMac.h FilterConfig.h \
		Group.h Options.h ProcessManager.h ProcessManagerMac.h ProcessManagerLinux.h ProcessManagerFBSD.h \
		ProcessMonitor.h RawEvent.h Regex.h Storage.h StorageManager.h StorageSqlite.h \
		TermCode.h  KfWindow.h KfWindowCache.h XErrorUtil.h \
		Common.h ProcessTree.h ProcessProperties.h ProcessMap.h

