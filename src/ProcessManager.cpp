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

#include "ProcessManager.h"

#include "Debug.h"

#include <exception>
#include <boost/lexical_cast.hpp>
// For reading /proc
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/graph/graph_utility.hpp>

namespace fs = boost::filesystem;
using namespace std;
using namespace keyfrog::proc;

// FIXME: what about processes that cant be read (/proc/{pid} permissions)

namespace keyfrog {

    /**
     * Public constructor 
     */
    ProcessManager::ProcessManager() : m_procBase( "/proc" ) {
    }

    /**
     * Public destructor
     */
    ProcessManager::~ProcessManager() {
    }

    /**
     * Complete add process method. Adds tree node, sets
     * properties, adds edge to parent.
     */
    bool ProcessManager::addConnectProcess(const string & pidStr) {
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        // Add this process
        ProcId newProc = addProcess(pidStr);

        // Invalid PID?
        if( 0 == newProc ) {
            return false;
        }

        // Skip process 0 -- it's a special pid, parent of processes that do not have parent
        if( 0 == m_procTree[newProc].pid ) {
            return false;
        }

        // Find it's parent
        if(!setProcessProperties(newProc)) {
            _dbg("ERROR: setProcessProperties failed for pid: %s", pidStr.c_str());
            // FIXME: remove newProc from graph?
            return false;
        }

        pid_t ppid = m_procTree[newProc].ppid;
        // If parent does not exist in graph, add him first
        if(0 == m_pidToId.count(ppid))
            addConnectProcess(ppid);
        // Add edge parent -> newProc
        add_edge(m_pidToId[ppid], newProc, m_procTree);
        return true;
    }

    /** 
     * Updates given process (its properties, children, edges, etc.)
     * including removal if needed.
     */
    void ProcessManager::updateProcess(pid_t pid) {
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        // Proc exists in map?
        if(0 == m_pidToId.count(pid)) {
            // XXX Don't add, just return?
            addConnectProcess(pid);
            return;
        }

        // XXX Check if they exists in graph?
        ProcId procId = m_pidToId[ pid ];
        ProcId parentId = m_pidToId[ m_procTree[ procId ].ppid ];

        string pidStr = m_procTree[ procId ].pidStr;

        // Remove edge from parent
        remove_edge(parentId, procId, m_procTree);

        // Recursive for all children
        ProcDescIter eit, eit_end;
        for(tie(eit, eit_end) = out_edges(procId, m_procTree); eit != eit_end; ++eit) {
            ProcId targetProcId = target(*eit, m_procTree);
            updateProcess( m_procTree[ targetProcId ].pid );
        }

        if(!processExists(pidStr)) {
            // Remove vertex from graph
            removeProcess(procId);
        } else {
            // Refresh process info
            setProcessProperties(procId);
            // Add connection from new parent
            pid_t ppid = m_procTree[procId].ppid;
            add_edge(m_pidToId[ppid], procId, m_procTree);
        }
    }

    /**
     * @warning Does not remove edges related to removed vertex! 
     */
    void ProcessManager::removeProcess(ProcId procId) {
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        m_pidToId.erase(m_procTree[procId].pid);
        remove_vertex(procId, m_procTree);
    }

    /**
     * Adds process vertex to graph. Sets it's two properties: pid and pidStr.
     *
     * Not thread safe, because it's private
     */
    ProcId ProcessManager::addProcess(const string & pidStr) {
        ProcId newProc;
        try {
            int pid, ppid;
            // Will throw exception if pid string is not integer
            pid = boost::lexical_cast<int>(pidStr);

            // Vertex of given process already exists?
            if(0 != m_pidToId.count(pid))
                return m_pidToId[pid];

            // Add pid to graph
            newProc = add_vertex(m_procTree);
            m_procTree[newProc].pid = pid;
            m_procTree[newProc].pidStr = pidStr;
            m_pidToId[pid] = newProc;

            // Special pid 0?
            if( 0 == pid ) {
                m_procTree[ newProc ].ppid = 0;
                m_procTree[ newProc ].ppidStr = string("0");
                m_procTree[ newProc ].name = string("[empty process 0]");
            }
        } catch( const std::exception & ex ) {
            // FIXME
            return 0;
        }
        return newProc;
    }

    /**
     * Converts pid_t into string
     *
     * Thread safe
     */
    bool ProcessManager::addConnectProcess(pid_t pid) {
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        string pidStr;
        try {
            pidStr = boost::lexical_cast<string>(pid);
        } catch( const std::exception & ex ) {
            return false;
        }
        return addConnectProcess(pidStr);
    }

    /**
     * Draft look of graph
     */
    void ProcessManager::dumpTree(string filename) {
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        print_graph(m_procTree, get(&proc::ProcProperties::pid, m_procTree));
    }

    int ProcessManager::procCount() {
        return num_vertices(m_procTree);
    }

    set< pair<pid_t, std::string> > ProcessManager::fetchDescendants(pid_t pid) {
        set< pair<pid_t, string> > retSet;
        if(0 == m_pidToId.count(pid)) {
            _dbg("NO SUCH PID %d! ", pid);
            return retSet;
        }

        // The pid must exist (see count(key) above)
        ProcId procId = m_pidToId[pid];
        fetchDescendants_sub(retSet, procId);

        _qldbg("-----:::: ");
        for( set< pair<pid_t, string> >::iterator it = retSet.begin(); it != retSet.end(); ++it ) {
            _qldbg("%s ",it->second.c_str());
        }
        _qdbg("");

        return retSet;
    }

    void ProcessManager::fetchDescendants_sub( set< pair<pid_t, string> > & procSet, ProcId procId) {
        ProcDescIter eit, eit_end;
        for(tie(eit, eit_end) = out_edges(procId, m_procTree); eit != eit_end; ++eit) {
            ProcId subProc = target(*eit, m_procTree);
            pair<pid_t, string> setEl( m_procTree[subProc].pid, m_procTree[subProc].name );
            procSet.insert(setEl);
            // Now recurse for children of subProc
            fetchDescendants_sub( procSet, subProc );
        }
    }

    const std::string & ProcessManager::fetchName( pid_t pid ) {
        static const std::string empty_string = "";
        if(0 == m_pidToId.count(pid)) {
            _dbg("NO SUCH PID %d! ", pid);
            return empty_string;
        }

        // The pid must exist (see count(key) above)
        ProcId procId = m_pidToId[pid];

        return m_procTree[procId].name;
    }
}

// vim:et:sw=4:ts=4:sts=4
