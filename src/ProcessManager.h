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

#ifndef KEYFROG_PROCESSMANAGER_H
#define KEYFROG_PROCESSMANAGER_H

// For process tree
#include <boost/graph/adjacency_list.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/filesystem/path.hpp>
#include <string>
#include <map>
#include <set>
#include <sys/types.h>
#include <unistd.h>

namespace keyfrog {
    namespace proc {
        struct ProcProperties {
            // Set in addProcess()
            pid_t pid;
            std::string pidStr;
            // Set in setProcessProperties()
            pid_t ppid;
            std::string ppidStr;
            std::string name;
        };
        typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::directedS, ProcProperties> ProcTree;
        typedef boost::graph_traits<ProcTree>::vertex_descriptor ProcId;
        typedef boost::graph_traits<ProcTree>::vertex_iterator ProcIter;
        typedef boost::graph_traits<ProcTree>::out_edge_iterator ProcDescIter;
    }

    /**
     * Tool for process related tasks (ie. child retrieval)
     *
     * Three methods are virtual, to provide different OS support:
     * - setProcessProperties()
     * - createProcTree()
     * - processExists()
     */
    class ProcessManager {  
        protected:

            /// Process graph
            proc::ProcTree m_procTreeGraph;

            /// Maps pids to vertex indexes
            std::map<pid_t, proc::ProcId> m_pidToIdMap;

            /// Path to proc filesystem
            boost::filesystem::path m_procBase;

            /// No concurent access allowed (read and write)
            boost::recursive_mutex m_accessMutex;

            /// Adds process to tree without connecting to parent
            bool addProcess(const std::string & pidStr, proc::ProcId & newProc);

            /// Sets parent, name, etc. parameters 
            virtual bool setProcessProperties(const proc::ProcId & newProc) = 0;

            /// Used for recursion
            void fetchDescendants_sub( std::set< std::pair<pid_t, std::string> > & procSet, proc::ProcId procId);

        public:

            /// Public constructor
            ProcessManager();

            /// Public destructor
            virtual ~ProcessManager();

            /// Creates complete, initial process tree
            virtual void createProcTree() = 0;

            /// Adds process and connects it to parent
            bool addConnectProcess(const std::string & pidStr);
            bool addConnectProcess(pid_t pid);

            /// Refresh process subtree
            void updateProcess(pid_t pid);

            /// Remove process, update children 
            void removeProcess(proc::ProcId procId);

            /// Checks if process exists
            virtual bool processExists(const std::string & pidStr) = 0;

            /// Prints tree to stdout
            void dumpTree(std::string filename = "");

            /// Returns number of vertices
            int procCount();

            /// Returns set of descendants of given process
            std::set< std::pair<pid_t, std::string> > fetchDescendants(pid_t pid);

            /// Returns name of given pid
            const std::string & fetchName( pid_t pid );
    };
}
#endif
