/*********************************************************************************
 *   Copyright (C) 2006-2013 by Sebastian Gniazdowski                            *
 *   All Rights reserved.                                                        *
 *                                                                               *
 *   Redistribution and use in source and binary forms, with or without          *
 *   modification, are permitted provided that the following conditions          *
 *   are met:                                                                    *
 *   1. Redistributions of source code must retain the above copyright           *
 *      notice, this list of conditions and the following disclaimer.            *
 *   2. Redistributions in binary form must reproduce the above copyright        *
 *      notice, this list of conditions and the following disclaimer in the      *
 *      documentation and/or other materials provided with the distribution.     *
 *   3. Neither the name of the Keyfrog nor the names of its contributors        *
 *      may be used to endorse or promote products derived from this software    *
 *      without specific prior written permission.                               *
 *                                                                               *
 *   THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND     *
 *   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE       *
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  *
 *   ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE    *
 *   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL  *
 *   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS     *
 *   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       *
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT  *
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY   *
 *   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF      *
 *   SUCH DAMAGE.                                                                *
 *********************************************************************************/

#if HAVE_CONFIG_H       /* HAVE_CONFIG_H */
#include <config.h>
#else                           /* HAVE_CONFIG_H */
#include <FallbackConfigH.h>
#endif                          /* HAVE_CONFIG_H */

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include "Daemon.h"
#include "ProcessManagerLinux.h"
#include "ProcessManagerMac.h"
#include "ProcessManagerFBSD.h"
#include "RawEvent.h"
#include "CallbackClosure.h"
#include "Configuration.h"
#include "StorageSqlite.h"
#include "Debug.h"
#include "TermCode.h"

#include <exception>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
using namespace keyfrog::TermCodes;
namespace fs = boost::filesystem;

namespace keyfrog {
    /** 
     * @brief FIXME
     */
    bool Daemon::createDefaultConfig(string homeDir) {
        string configDirPath = homeDir + "/.keyfrog";
        fs::path configDir = fs::path(configDirPath);
        if(!fs::exists(configDir)) {
            _dbg("Config dir does not exist - creating");
            try {
                fs::create_directory(configDir);
            } catch( const exception & ex ) {
                _dbg("Exception while creating ~/.keyfrog directory: %s", ex.what());
                return false;
            }
        } else {
            _dbg("Config dir exist");
        }
        return true;
    }

    /**
     * Initializes application code.
     */
    Daemon::Daemon(bool asDaemon) : m_xConnected(false) {
#ifdef HOST_IS_OSX
        m_processManager = new ProcessManagerMac();
#elif defined HOST_IS_LINUX
        m_processManager = new ProcessManagerLinux();
#elif defined HOST_IS_FBSD
        m_processManager = new ProcessManagerFBSD();
#endif
        m_eventFilter = new EventFilter( m_wim, *m_processManager );
        m_processMonitor = new ProcessMonitor();

#ifndef _KF_COLORS
        TermCodes::nullOutput();
#endif

        char * homeEnvVar = getenv("HOME");
        if(homeEnvVar == NULL) {
            throw exception();
        }
        string homeDir = homeEnvVar;
        m_configuration.setHomeDir(homeDir);

        string configPath = homeDir + "/.keyfrog/config";
        m_configuration.setConfigPath(configPath);
        m_configuration.options().setDaemonMode(asDaemon);

        // Read configuration
        createDefaultConfig(homeDir);
        m_configReader.setConfiguration(m_configuration);
        m_configReader.readConfig();
        m_eventFilter->setFilterConfig(m_configuration.filterConfig());

        // Create database
        m_storageBackend = new StorageSqlite();
        m_storage = new StorageManager(m_storageBackend);
        m_storage->connect(homeDir + "/.keyfrog/keyfrog.db");

    }

    /**
     * Main task is closing database and disconnecting from Xserver
     */
    Daemon::~Daemon() {
        m_eventFilter->stop();
        delete m_processMonitor;
        delete m_eventFilter;
        delete m_processManager;
    }

    /// FIXME Fix configuration
    bool Daemon::connectXserver(string displayName) {
        // Connect to Xserver
        while ( 1 ) {
            if(m_eventFilter->connect(displayName)) {
                _dbg("%sSuccessfully connected to Xserver (DISPLAY=%s)%s", cboldGreen, displayName.c_str(), creset);
                m_xConnected = true;
                break;
            }
            _dbg("%sConnect to X server (DISPLAY=%s) failed, waiting 60 seconds before trying again%s", cboldRed, displayName.c_str(), creset);
            // Wait a bit..
            boost::xtime xt;
#if BOOST_VERSION >= 105000
            boost::xtime_get(&xt, boost::TIME_UTC_);
#else
            boost::xtime_get(&xt, boost::TIME_UTC);
#endif
            xt.sec += 60;
            boost::thread::sleep(xt);
        }
        m_eventFilter->start();
        return true;
    }

    /** 
     * return success or failure
     */
    bool Daemon::daemonize() {
        pid_t   pid;
        int     fd_err,fd_out;

        _dbg("%sForking into background / daemonize()%s", cboldGreen, creset);

        if((pid = fork()) < 0) {
            _err("Could not fork");
            return false;
        } else if(pid != 0) {
            exit(0);
        }

        setsid();
        if( -1 == chdir("/") ) {
            _err("Could not chdir(/)");
        }
        umask(0002);

#ifdef _KF_DEBUG
        if(     
                (fd_err = open("/tmp/kf.err", O_WRONLY | O_CREAT | O_APPEND , 0660)) == -1 ||
                (fd_out = open("/tmp/kf.out", O_RDWR | O_CREAT | O_APPEND, 0660)) == -1
          ) {
            _err("Redirect input/output to /tmp/kf.{err,out} failed");
            return false;
        }
#else
        if((fd_out = open("/dev/null",O_RDWR, 0)) == -1) {
            _err("Redirect input/output to /dev/null failed");
            return false;
        }
        fd_err = fd_out;
#endif
        dup2(fd_out,STDIN_FILENO);
        dup2(fd_out,STDOUT_FILENO);
        dup2(fd_err,STDERR_FILENO);

        if(fd_err > 2)
            close(fd_err);
        if(fd_err!=fd_out && fd_out > 2)
            close(fd_out);
        return true;
    }

    /**
     * Application main loop
     */
    int Daemon::run() {
        if(m_configuration.options().daemonMode()) {
            if(!daemonize()) {
                throw exception();
            }
        }

        if(!m_xConnected)
            if(!connectXserver()) {
                return 1;
            }

        m_processMonitor->init(m_processManager);
        // Run process monitor thread
        boost::thread thProcMon(boost::bind(&ProcessMonitor::eventLoop, boost::ref(*m_processMonitor)));

        m_processManager->createProcTree();

        while(1) {
            // Wait for event from X11
            Event event = m_eventFilter->nextEvent();
            switch (event.type()) {
                case kfKeyPress:
                    // TODO: config option for this
                    if(event.groupId() != -1) {
                        m_storage->addKeyPress(event.groupId());
                    }
                    break;
                case kfFocusIn:
                    break;
                case kfDestroyNotify:
                    m_wim.findAndUseWindow(event.destWin());
                    m_wim.invalidateEntry();
                    break;
                default:
                    _dbg("Unknown type! (%d)", event.type());
                    break;
            }
        }
        return EXIT_SUCCESS;
    }
}
