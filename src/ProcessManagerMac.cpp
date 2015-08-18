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

#ifdef HOST_IS_OSX

#include "ProcessManagerMac.h"

#include <cerrno>
#include <cstdlib>
#include <sys/sysctl.h>

#include <exception>
#include <boost/lexical_cast.hpp>
// For reading /proc
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/graph/graph_utility.hpp>

#include "Debug.h"

using namespace boost;
namespace fs = boost::filesystem;
using namespace std;
using namespace keyfrog::proc;

typedef struct kinfo_proc kinfo_proc;
static bool GetUnixProcesses( std::vector< kinfo_proc > & procList );

namespace keyfrog {

    /**
     * Constructor 
     */
    ProcessManagerMac::ProcessManagerMac() {
        m_procBase = "n/a";
    }

    /**
     * Destructor
     */
    ProcessManagerMac::~ProcessManagerMac() {
    }

    /**
     * Creates initial process tree
     */
    void ProcessManagerMac::createProcTree() {
        std::vector< kinfo_proc > procList;
        GetUnixProcesses( procList );

        _dbg("Mac Process count: %u!", procList.size());
    }

    /**
     * Check whether given process is still running in system
     *
     * Not thread safe (not needed)
     */
    bool ProcessManagerMac::processExists(const string & pidStr) {
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
    bool ProcessManagerMac::setProcessProperties(const ProcId & newProc) {
        return true;
    }

}

/**
 * Returns a list of all OSX processes - not only the Cocoa applications.
 * The vector is being cleared
 */
static bool GetUnixProcesses( std::vector< kinfo_proc > & procList ) {
    // First call sysctl with result = NULL and length = 0 - to get length
    // of required buffer for all the kinfo_proc structures - in bytes
    //
    // Next, translate byte size into amount of std::vector<> elements - and grow
    // the vector by that number of elements

    kinfo_proc* result = NULL;
    bool done = false;
    int err;

    kinfo_proc template_kinfo;
    memset( &template_kinfo, 0, sizeof( kinfo_proc ) );
    // Clear the result vector
    procList.clear();

    do {
        // Get byte-size of the result buffer (it will hold multiple kinfo_proc structures)
        size_t length = 0;
        static const int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };

        err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                NULL, &length,
                NULL, 0 );

        if( -1 == err ) {
            err = errno;
        }

        if( err )
            return false;

        if( length % sizeof(kinfo_proc) ) {
            // sysctl returned a weird byte-size, that
            // isn't a multiply of sizeof(kinfo_proc)
            return false;
        }

        int vsize = length / sizeof( kinfo_proc );
        // Grow std::vector<>
        procList.resize( vsize, template_kinfo );

        // Fill the prepared std::vector<> buffer
        size_t new_length = length;
        err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                &procList.front(), &new_length,
                NULL, 0 );

        if( -1 == err ) {
            err = errno;
        }

        if( new_length % sizeof(kinfo_proc) ) {
            // Incorrect, weird size
            procList.clear();
            return false;
        } else if( !err ) {
            done = true;
            // Are there any unused elements?
            if( new_length < length ) {
                int new_vsize = new_length / sizeof( kinfo_proc );
                // std::vector<> will not touch unremoved elements
                procList.resize( new_vsize, template_kinfo );
            }
        } else if( ENOMEM == err ) {
            // The only error that is tolerated
            procList.clear();
            err = 0;
        } else {
            procList.clear();
            return false;
        }
    } while( !done );

    return true;
}

#endif

// vim:et:sw=4:ts=4:sts=4
