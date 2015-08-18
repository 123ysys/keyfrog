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

#ifndef KEYFROGCONFIGURATION_H
#define KEYFROGCONFIGURATION_H

#include "FilterConfig.h"
#include "Options.h"
#include <string>

namespace keyfrog {
    /**
     * @author Sebastian Gniazdowski
     */
    class Configuration {
        std::string m_sqliteDbFilePath;
        std::string m_homeDir;
        std::string m_configPath;

        // Configuration packs
        FilterConfig m_filterConfig;
        Options m_options;

        public:
        Configuration();
        ~Configuration();

        // Options pack
        Options & options() {
            return m_options;
        }
        // Filter config pack
        FilterConfig & filterConfig() {
            return m_filterConfig;
        }
        // Sqlite db path
        std::string & sqliteDbFilePath() {
            return m_sqliteDbFilePath;
        }
        // Home dir
        std::string & homeDir() {
            return m_homeDir;
        }
        // Configuration path 
        std::string & configPath() {
            return m_configPath;
        }



        //protected:
        // Home dir
        void setHomeDir(std::string & theVal) {
            m_homeDir = theVal;
        }
        // Configuration path 
        void setConfigPath(std::string & theVal) {
            m_configPath = theVal;
        }
        // Sqlite db path
        void setSqliteDbFilePath(const std::string & sqliteDbFilePath) {
            m_sqliteDbFilePath = sqliteDbFilePath;
        }
        // Filter config pack
        void setFilterConfig(const FilterConfig & theValue) {
            m_filterConfig = theValue;
        }
        // Options pack
        void setOptions(const Options & theValue) {
            m_options = theValue;
        }
    };
}
#endif
