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

#ifdef HOST_IS_FBSD

#include "ProcessManagerFBSD.h"
#include "Debug.h"

#include <exception>
#include <boost/lexical_cast.hpp>

#include <sys/user.h>
#include <libutil.h>

using namespace std;

namespace keyfrog {

    /**
     * Constructor
     */
    ProcessManagerFBSD::ProcessManagerFBSD() {
    }

    /**
     * Destructor
     */
    ProcessManagerFBSD::~ProcessManagerFBSD() {
    }

    /**
     * Creates initial process tree
     */
    void ProcessManagerFBSD::createProcTree() {
        boost::recursive_mutex::scoped_lock lock( m_accessMutex );

        int numallproc = 0;
        struct kinfo_proc* allproc = kinfo_getallproc( &numallproc );
        if( NULL == allproc ) {
            return;
        }

        _dbg("FBSD Process count: %d!", numallproc);

        // Creates a map with all processes that will be feed to m_processTree
        ProcessMap processMap;
        for ( int i = 0; i < numallproc; ++i ) {
            try {
                // Get the instantly available data
                pid_t pid = allproc[i].ki_pid;
                pid_t ppid = allproc[i].ki_ppid;
                string name = allproc[i].ki_comm;

                processMap[ pid ] = ProcessProperties();
                setProcessProperties( processMap[ pid ], pid, true, ppid, true, name );
            } catch ( const std::exception & ex ) {
                _dbg("Exception while adding processes: %s", ex.what());
            }
        }

        boost::recursive_mutex::scoped_lock lock2( m_processTree.mutex() );
        m_processTree.clear();
        m_processTree.addConnectProcesses( processMap );

        //dumpTree();
    }

    /**
     * Check whether given process is still running in system
     *
     * Not thread safe (not needed)
     */
    bool ProcessManagerFBSD::processExists(const string & pidStr) {
        // TODO
        return true;
    }

    /**
     * When this method is called only pid and pidStr properties are set
     * (and they are required).
     *
     * Not thread safe, because it's private
     *
     * @return success/failure
     */
    bool ProcessManagerFBSD::setProcessProperties(ProcessProperties & newProperties, pid_t pid,
                                                    bool ppid_known, pid_t ppid,
                                                    bool name_known, const std::string & name
                                                ) {
        newProperties.pid = pid;
        try {
            newProperties.pidStr = boost::lexical_cast< string >( pid );
        } catch( const std::exception & ex ) {
            return false;
        }

        if( !ppid_known ) {
            // TODO unimplemented
            newProperties.ppid_known = false;
        } else {
            newProperties.ppid = ppid;
            try {
                newProperties.ppidStr = boost::lexical_cast< string >( ppid );
            } catch( const std::exception & ex ) {
                return false;
            }
            newProperties.ppid_known = true;
        }

        if( !name_known ) {
            // TODO unimplemented
            newProperties.name_known = false;
        } else {
            newProperties.name = name;
            newProperties.name_known = true;
        }

        //_dbg( "Path of %d is: %s", pid, pathbuf );
        return true;
    }

}

#endif
