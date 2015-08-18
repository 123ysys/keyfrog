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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ProcessManagerUnix.h"

#include "Debug.h"
#include "TermCode.h"

#include <exception>
#include <boost/lexical_cast.hpp>
// For reading /proc
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/graph/graph_utility.hpp>

namespace fs = boost::filesystem;
using namespace std;
using namespace keyfrog::proc;
using namespace keyfrog::TermCodes;

// FIXME: what about processes that cant be read (/proc/{pid} permissions)

namespace keyfrog {

    /**
     * Constructor 
     */
    ProcessManagerUnix::ProcessManagerUnix() {
    }

    /**
     * Destructor
     */
    ProcessManagerUnix::~ProcessManagerUnix() {
    }

    /**
     * Creates initial process tree
     */
    void ProcessManagerUnix::createProcTree() {
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        m_procTree.clear();
        m_pidToId.clear();
        fs::directory_iterator end_iter;

        // Get maps for used vertex properties
        for ( fs::directory_iterator dir_itr(m_procBase); dir_itr != end_iter; ++dir_itr ) {
            try {
                if ( fs::is_directory( *dir_itr ) ) {
                    // leaf() is removed
                    addConnectProcess(dir_itr->path().filename().string());
                }
            } catch ( const std::exception & ex ) {
                _dbg("Exception while enumerating processes: %s", ex.what());
            }
        }
        //dumpTree();
    }

    /**
     * Check whether given process is still running in system
     *
     * Not thread safe (not needed)
     */
    bool ProcessManagerUnix::processExists(const string & pidStr) {
        fs::path proc_path( m_procBase / pidStr );

        // Check if process exists and is directory
        if(!fs::exists(proc_path) || !fs::is_directory(proc_path))
            return false;
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
    bool ProcessManagerUnix::setProcessProperties(const ProcId & newProc) {
        try {
            // Find it's parent
            if( ! exists( m_procBase / m_procTree[newProc].pidStr / "stat" ) ) {
                throw std::runtime_error("file does not exist");
            }
            fs::ifstream statFile( m_procBase / m_procTree[newProc].pidStr / "stat" );
            string trm;
            statFile >> trm;
            statFile >> trm; // "(procname)"
            m_procTree[newProc].name = trm.substr(1, trm.size() - 2);
            statFile >> trm;
            statFile >> trm; // parent process pid
            m_procTree[newProc].ppid = boost::lexical_cast<int>(trm);
            m_procTree[newProc].ppidStr = trm;
            //_dbg("Parent of %s is: %s", pidStr.c_str(), trm.c_str());
            //_dbg("Name of %d is %s", m_procTree[newProc].pid, m_procTree[newProc].name.c_str());
        }

        catch( const std::exception & ex ) {
            const char * _procBase = m_procBase.c_str();
            const char * _pid = m_procTree[newProc].pidStr.c_str();

            _dbg( "%sError accessing process information at %s/%s/stat: %s%s" , cred, _procBase, _pid, ex.what(), creset);

            if( ! fs::exists( m_procBase ) ) {
                _dbg( "%sThe proc file system does not exist at %s. It is required by Keyfrog.%s",
                        cred, _procBase, creset);
            } else if( ! fs::exists( m_procBase / _pid ) ) {
                _qdbg( "%sProcess (pid %s) does not exist anymore at %s/%s%s",
                        cred, _pid, _procBase, _pid, creset);
            }

            return false;
        }

        return true;
    }
}
