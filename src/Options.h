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

#ifndef KEYFROGOPTIONS_H
#define KEYFROGOPTIONS_H

#include <string>

namespace keyfrog {

    /**
     * @author Sebastian Gniazdowski <srnt at users dot sf dot net>
     */
    class Options{
        // Debug options
        bool m_debugState;
        bool m_debugUseLogFile;
        bool m_debugUseStdErr;
        bool m_daemonMode;
        std::string m_debugLogFile;

        // Cluster options
        int m_clusterSize;

        // General options
        std::string m_userHomeDir;

        public:
        void setDebugState(bool theVal) { m_debugState = theVal; }
        bool debugState() { return m_debugState; }

        void setDebugUseStdErr(bool theVal) { m_debugUseStdErr = theVal; }
        bool debugUseStdErr() { return m_debugUseStdErr; }

        void setDebugUseLogFile(bool theVal) { m_debugUseLogFile = theVal; }
        bool debugUseLogFile() { return m_debugUseLogFile; }

        void setDebugLogFile(std::string & theVal) { m_debugLogFile = theVal; }
        std::string & debugDebugLogFile() { return m_debugLogFile; }

        void setClusterSize(int theVal) { m_clusterSize = theVal; }
        int clusterSize() { return m_clusterSize; }

        void setDaemonMode(bool theVal) { m_daemonMode = theVal; }
        int daemonMode() { return m_daemonMode; }

        Options();
        ~Options();
    };
}

#endif
