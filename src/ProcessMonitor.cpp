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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "ProcessMonitor.h"
#include "TermCode.h"
#include "Debug.h"
#include <exception>
#include <boost/thread/xtime.hpp>
#include <boost/thread/thread.hpp>

using namespace std;
using namespace boost;
using namespace keyfrog::TermCodes;

namespace keyfrog {

    /**
     * Constructor 
     */
    ProcessMonitor::ProcessMonitor() {
    }

    /**
     * Destructor
     */
    ProcessMonitor::~ProcessMonitor() {
    }

    /**
     * Prepare monitor
     */
    bool ProcessMonitor::init(ProcessManager *procMan) {
        m_procMan = procMan;
        return true;
    }

    /** 
     * It's usable probably only with threads
     */
    void ProcessMonitor::eventLoop() {
        // Inotify doesn't detect events on /proc. Is there other way? 
        // Workaround: process tree is recreated every 5 seconds (TODO: config option for this)
        while ( 1 ) {
            _dbg("%s//// procMonitor loop (|V|=%d) ////%s", cboldGreen, m_procMan->processTree().count(), creset);
            m_procMan->createProcTree();

            // Wait a bit..
            xtime xt;
#if BOOST_VERSION >= 105000
            xtime_get(&xt, TIME_UTC_);
#else
            xtime_get(&xt, TIME_UTC);
#endif
            xt.sec += 5;
            thread::sleep(xt);
        }
    }
}
