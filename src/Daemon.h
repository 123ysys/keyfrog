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

#ifndef KEYFROG_DAEMON_H
#define KEYFROG_DAEMON_H

#include "ProcessManager.h"
#include "ProcessMonitor.h"
#include "EventFilter.h"
#include "EventMonitorX11.h"
#include "Storage.h"
#include "StorageManager.h"
#include "ConfigReader.h"

#include <cstdlib>
#include <string>

namespace keyfrog {
    /**
     * @author Sebastian Gniazdowski
     */
    class Daemon {
        /// General interface to events
        EventFilter *m_eventFilter;
        /// Statistics storage
        Storage *m_storageBackend;
        Storage *m_storage;
        /// Creates FilterConfig etc.
        ConfigReader m_configReader;
        /// Window properties cache
        KfWindowCache m_wim;
        /// Daemon configuration
        Configuration m_configuration;
        /// Process manager
        ProcessManager* m_processManager;
        /// Process monitor 
        ProcessMonitor* m_processMonitor;
        /// Is X server connected
        bool m_xConnected;

        public:
        Daemon(bool asDaemon = true);

        ~Daemon();

        /// Connect to X server
        bool connectXserver(std::string displayName = ":0");

        /// Runs daemon, returns exit code
        int run();

        /// Creates default config
        bool createDefaultConfig(std::string homeDir);

        /// Forks into background
        bool daemonize();
        private:
    };
}

#endif
