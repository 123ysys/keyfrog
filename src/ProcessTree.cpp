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

#include "Debug.h"
#include "ProcessTree.h"
#include "sys/types.h"

#include <boost/lexical_cast.hpp>
#include <boost/graph/graph_utility.hpp>

using namespace std;

namespace keyfrog {

    ProcessTree::ProcessTree()
    {
    }

    ProcessTree::~ProcessTree()
    {
    }

    /**
     * Adds to graph all processes given in map
     */
    bool ProcessTree::addConnectProcesses( ProcessMap & procMap ) {
        /// Lock
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        bool retval = true;
        for( ProcessMap::iterator it = procMap.begin(); it != procMap.end(); ++ it ) {
            pid_t pid = it->first;
            retval = retval && addConnectProcesses( pid, procMap );
        }

        return retval;
    }

    /**
     * Converts pid_t into string
     *
     * Thread safe
     */
    bool ProcessTree::addConnectProcesses( const string & pidStr, ProcessMap & procMap ) {
        /// Lock
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        pid_t pid;
        try {
            pid = boost::lexical_cast< pid_t >( pidStr );
        } catch( const std::exception & ex ) {
            return false;
        }
        return addConnectProcesses(pid, procMap);
    }

    /**
     * Complete add process method. Adds tree node, sets
     * properties, adds edge to parent.
     */
    bool ProcessTree::addConnectProcesses( pid_t pid, ProcessMap & procMap ) {
        /// Lock
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        // Add this process
        ProcId newProc;

        // Invalid PID?
        if( !addProcess( pid, procMap, newProc ) ) {
            return false;
        }

        // For special pid 0 it is all done
        // Don't add any edge connected to any parent
        if( 0 == m_procTreeGraph[newProc].pid ) {
            return true;
        }

        // If parent does not exist in graph, add him first
        pid_t ppid = m_procTreeGraph[ newProc ].ppid;
        if( 0 == m_pidToIdMap.count(ppid) ) {
            addConnectProcesses(ppid, procMap);
        }

        // Add edge parent -> newProc
        add_edge(m_pidToIdMap[ppid], newProc, m_procTreeGraph);
        return true;
    }

    /**
     * Adds process vertex to graph. Sets it's two properties: pid and pidStr.
     */
    bool ProcessTree::addProcess(pid_t pid, ProcessMap & procMap, ProcId & newProc) {
        /// Lock
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        try {
            // Vertex of given process already exists in graph?
            // (check if the mapping pid - vertex ID exists)
            if(0 != m_pidToIdMap.count(pid)) {
                return true;
            }

            // Check if the process exists in the input map
            // then store it into graph
            map<pid_t, ProcessProperties>::const_iterator iter = procMap.find( pid );
            if (iter != procMap.end()) {
                ProcessProperties newProcProp = iter->second;

                newProc = add_vertex( m_procTreeGraph );
                m_procTreeGraph[ newProc ] = newProcProp;
                m_pidToIdMap[ pid ] = newProc;
            } else {
                // TODO handle error;
                return false;
            }
        } catch( const std::exception & ex ) {
            // FIXME
            return false;
        }
        return true;
    }

    /**
     * Draft look of graph
     */
    void ProcessTree::dumpTree(string filename) {
        /// Lock
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        print_graph(m_procTreeGraph, get(&ProcessProperties::pid, m_procTreeGraph));
    }

    int ProcessTree::count() const {
        /// Lock
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        return num_vertices(m_procTreeGraph);
    }

    set< pair<pid_t, std::string> > ProcessTree::fetchDescendants(pid_t pid) const {
        /// Lock
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        set< pair<pid_t, string> > retSet;
        if(0 == m_pidToIdMap.count(pid)) {
            _dbg("NO SUCH PID %d! ", pid);
            return retSet;
        }

        // The pid must exist (see count(key) above)
        ProcId procId;
        map<pid_t, ProcId>::const_iterator iter = m_pidToIdMap.find( pid );
        if (iter != m_pidToIdMap.end()) {
            procId = iter->second;
        } else {
            // TODO handle error;
        }

        fetchDescendants_sub(retSet, procId);

        _qldbg("-----:::: ");
        for( set< pair<pid_t, string> >::iterator it = retSet.begin(); it != retSet.end(); ++it ) {
            _qldbg("%s ",it->second.c_str());
        }
        _qdbg("");

        return retSet;
    }

    void ProcessTree::fetchDescendants_sub( set< pair<pid_t, string> > & procSet, ProcId procId) const {
        /// Lock
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        ProcDescIter eit, eit_end;
        _dbg("m_procTreeGraph size at fetchDescendants_sub: %d", num_vertices(m_procTreeGraph));
        for(tie(eit, eit_end) = out_edges(procId, m_procTreeGraph); eit != eit_end; ++eit) {
            ProcId subProc = target(*eit, m_procTreeGraph);
            pair<pid_t, string> setEl( m_procTreeGraph[subProc].pid, m_procTreeGraph[subProc].name );
            procSet.insert(setEl);
            // Now recurse for children of subProc
            fetchDescendants_sub( procSet, subProc );
        }
    }

    const std::string & ProcessTree::fetchName( pid_t pid ) const {
        /// Lock
        boost::recursive_mutex::scoped_lock lock(m_accessMutex);

        static const std::string empty_string = "";
        if(0 == m_pidToIdMap.count(pid)) {
            _dbg("NO SUCH PID %d! ", pid);
            return empty_string;
        }

        ProcId procId;
        map<pid_t, ProcId>::const_iterator iter = m_pidToIdMap.find( pid );
        if (iter != m_pidToIdMap.end()) {
            procId = iter->second;
        } else {
            // TODO handle error;
        }

        return m_procTreeGraph[procId].name;
    }

}
