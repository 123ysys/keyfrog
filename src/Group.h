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

#ifndef KEYFROGGROUP_H
#define KEYFROGGROUP_H

#include <string>
#include <list>

namespace keyfrog {     
    /**
     * @author Sebastian Gniazdowski
     */
    class Group {
        int m_id;
        std::list<std::string> m_windowClasses;
        std::list<std::string> m_terminalProcesses;
        std::list<std::string> m_processes;
        public:
        Group();
        ~Group();

        void addWindowClass( const std::string & windowClass ) {
            m_windowClasses.push_back( windowClass );
        }

        void addTerminalProcess( const std::string & termProc ) {
            m_terminalProcesses.push_back( termProc );
        }

        void addProcess( const std::string & proc ) {
            m_processes.push_back( proc );
        }

        const std::list<std::string> & windowClasses() const { return m_windowClasses; }
        const std::list<std::string> & termProcs() const { return m_terminalProcesses; }
        const std::list<std::string> & procs() const { return m_processes; }

        void setId(int id) {
            m_id = id;
        }

        int id() const {
            return m_id;
        }
    };
}

#endif
