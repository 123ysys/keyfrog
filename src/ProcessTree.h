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

#ifndef PROCESSTREE_H
#define PROCESSTREE_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>

#include "ProcessProperties.h"
#include "ProcessMap.h"

namespace keyfrog {
    typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::directedS, ProcessProperties> ProcTree;
    typedef boost::graph_traits<ProcTree>::vertex_descriptor ProcId;
    typedef boost::graph_traits<ProcTree>::vertex_iterator ProcIter;
    typedef boost::graph_traits<ProcTree>::out_edge_iterator ProcDescIter;

    /**
      @author Sebastian Gniazdowski
      */
    class ProcessTree {
        private:
            /// Process graph
            ProcTree m_procTreeGraph;

            /// Maps pids to vertex indexes
            std::map<pid_t, ProcId> m_pidToIdMap;

            /// No concurent access allowed (read and write)
            mutable boost::recursive_mutex m_accessMutex;

            /// Adds process to tree without connecting to parent
            bool addProcess(pid_t pid, ProcessMap & procMap, ProcId & newProc);

            /// Used for recursion
            void fetchDescendants_sub( std::set< std::pair<pid_t, std::string> > & procSet, ProcId procId) const;

        public:
            /// Constructor
            ProcessTree();

            /// Destructor
            ~ProcessTree();

            /// Locks the process tree
            /// All the below methods start and end their
            /// tasks, which are complete, i.e. there is no
            /// actual thing to do with m_procTreeGraph after
            /// their end. Thus the method-scope locking is
            /// a correct solution. A not-complete method is
            /// addProcess()
            /// Here is an opportunity to manually lock the
            /// process tree when needed
            boost::recursive_mutex & mutex() { return m_accessMutex; }

            /// Adds given processes to the graph
            bool addConnectProcesses(ProcessMap & procMap);

            /// Adds given pid to the graph, together with
            /// its parents (and parents of parents, etc.)
            /// taken from procMap
            bool addConnectProcesses(const std::string & pidStr, ProcessMap & procMap);

            /// The same as above
            bool addConnectProcesses(pid_t pid, ProcessMap & procMap);

            /// Prints tree to stdout
            void dumpTree(std::string filename = "");

            /// Returns number of vertices
            int count() const;

            /// Clears the tree
            void clear() {
                /// Lock
                boost::recursive_mutex::scoped_lock lock(m_accessMutex);
                m_pidToIdMap.clear();
                m_procTreeGraph.clear();
            }

            /// Returns set of descendants of given process
            std::set< std::pair<pid_t, std::string> > fetchDescendants( pid_t pid ) const;

            /// Returns name of given pid
            const std::string & fetchName( pid_t pid ) const;
    };

}

#endif
