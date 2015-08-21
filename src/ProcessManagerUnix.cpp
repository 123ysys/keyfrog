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

        fs::directory_iterator end_iter;

        // Creates a map with all processes that will be feed to m_processTree
        ProcessMap processMap;
        try {
            for ( fs::directory_iterator dir_itr(m_procBase); dir_itr != end_iter; ++dir_itr ) {
                if ( fs::is_directory( *dir_itr ) ) {
                    pid_t pid;
                    try {
                        pid = boost::lexical_cast< pid_t >( dir_itr->path().filename().string() );
                    } catch( const std::exception & ex ) {
                        //_dbg("Exception while enumerating processes [1]: %s", ex.what());
                        continue;
                    }
                    processMap[ pid ] = ProcessProperties();
                    setProcessProperties( processMap[ pid ], pid, false, 0, false, "" );
                }
            }
        } catch ( const std::exception & ex ) {
            _dbg("Exception while enumerating processes [2]: %s", ex.what());
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
    bool ProcessManagerUnix::setProcessProperties(ProcessProperties & newProcProp, pid_t pid,
                                            bool ppid_known, pid_t ppid,
                                            bool name_known, const std::string & name
                                        ) {
        try {
            newProcProp.pid = pid;
            newProcProp.pidStr = boost::lexical_cast<string>(pid);

            // Find it's parent
            if( ! exists( m_procBase / newProcProp.pidStr / "stat" ) ) {
                throw std::runtime_error("file does not exist");
            }
            fs::ifstream statFile( m_procBase / newProcProp.pidStr / "stat" );

            string trm;
            statFile >> trm;
            statFile >> trm; // "(procname)"

            if( !name_known ) {
                newProcProp.name = trm.substr(1, trm.size() - 2);
            } else {
                newProcProp.name = name;
            }
            newProcProp.name_known = true;

            statFile >> trm;

            // Is ppid and ppidStr already set?
            if( !ppid_known ) {
                statFile >> trm; // parent process pid
                pid_t ppid2 = boost::lexical_cast<int>(trm);
                newProcProp.ppidStr = trm;
                newProcProp.ppid = ppid2;
            } else {
                newProcProp.ppidStr = boost::lexical_cast<string>(ppid);
                newProcProp.ppid = ppid;
            }
            newProcProp.ppid_known = true;

            //_dbg("Parent of %s is: %s", pidStr.c_str(), trm.c_str());
            //_dbg("Name of %d is %s", m_procTreeGraph[newProc].pid, m_procTreeGraph[newProc].name.c_str());
        }

        catch( const std::exception & ex ) {
            const char * _procBase = m_procBase.c_str();
            const char * _pid = newProcProp.pidStr.c_str();

            // Check if it is special pid 0
            if( 0 == newProcProp.pid ) {
                newProcProp.ppid = 0;
                newProcProp.ppidStr = string("0");
                newProcProp.name = string("[void process 0]");
            } else {
                _dbg( "%sError adding new process. Was accessing information at %s/%s/stat: %s%s",
                        cred, _procBase, _pid, ex.what(), creset);

                if( ! fs::exists( m_procBase ) ) {
                    _dbg( "%sThe proc file system does not exist at %s. It is required by Keyfrog.%s",
                            cred, _procBase, creset);
                } else if( ! fs::exists( m_procBase / _pid ) ) {
                    _qdbg( "%sProcess (pid %s) does not exist anymore at %s/%s%s",
                            cred, _pid, _procBase, _pid, creset);
                }

                return false;
            }
        }

        return true;
    }
}
