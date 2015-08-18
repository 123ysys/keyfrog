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

#ifndef KEYFROGSTORAGESQLITE_H
#define KEYFROGSTORAGESQLITE_H

#include "Storage.h"
#include <sqlite3.h>
#include <string>

namespace keyfrog {

    /**
     * @author Sebastian Gniazdowski <srnt at users dot sf dot net>
     *
     * @warning FIXME: add setter for cluster size
     *
     * Front interface for storing and reading event data.
     */
    class StorageSqlite : public Storage {
        std::string m_uri;
        sqlite3 *m_db;
        sqlite3_stmt *m_addKeyPress_insertStmt1;
        sqlite3_stmt *m_addKeyPress_updateStmt1;
        sqlite3_stmt *m_addKeyPress_existsStmt1;

        int m_clusterSize;

        bool prepareStatements();
        bool initDatabase();

        public:
        StorageSqlite();
        ~StorageSqlite();

        /** 
         * @brief Connects to given database
         */
        virtual bool connect(std::string uri);

        /** 
         * @brief Disconnects from database
         */
        virtual void disconnect();

        /** 
         * @brief Gets begining of  cluster timestamp for given timestamp
         */
        virtual int getClusterStart(int timestamp);

        /** 
         * @brief Records keypresses at actual time
         */
        virtual bool addKeyPress(int app_group, int count = 1);

        /** 
         * @brief Records keypresses at given time
         */
        virtual bool addKeyPress(int app_group, int timestamp, int count = 1);
    };
}

#endif
