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

#include "StorageSqlite.h"
#include "Debug.h"
#include <ctime>
#include <exception>

using namespace std;

namespace keyfrog {

    /** 
     * @brief Constructor
     */
    StorageSqlite::StorageSqlite() {
        // Cluster of time that keys will be group by
        m_clusterSize = 15*60;
    }

    /** 
     * @brief Destructor
     */
    StorageSqlite::~StorageSqlite() {
    }

    bool StorageSqlite::initDatabase() {
        int rc;
        char *zErrMsg = NULL;

        string sql = 
            "CREATE TABLE keypresses ( "
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "cluster_begin TIMESTAMP, "
            "cluster_end TIMESTAMP, "
            "count INTEGER, "
            "app_group INTEGER "
            "); "
            "CREATE UNIQUE INDEX cluster_index " 
            "ON keypresses ( cluster_begin, app_group ); ";

        rc = sqlite3_exec(m_db, sql.c_str(), NULL, NULL, &zErrMsg);
        if( rc != SQLITE_OK ) {
            _dbg("sqlite3_exec failed: `%s' (hint: if table already exists, this is not an actual error)", zErrMsg);
            // FIXME: if exists..
            //return false;
        }

        _dbg("Database initialized");
        return true;
    }

    /** 
     * @brief Prepares all needed statements
     * 
     * @return true/false
     */
    bool StorageSqlite::prepareStatements() {
        int rc;
        string stmt_str;

        _dbg("prepareStatements()");

        // Statement for adding key record
        stmt_str = "INSERT INTO keypresses ( cluster_begin, cluster_end, count, app_group ) "
            "VALUES ( ?, ?, ?, ? ) ";
        rc = sqlite3_prepare(m_db,
                stmt_str.c_str(),
                stmt_str.size(),
                &m_addKeyPress_insertStmt1, NULL);
        if(rc != SQLITE_OK) {
            _dbg("insert1 init failed");
            return false;
        }

        // Statement for updating existing key record
        stmt_str = "UPDATE keypresses SET count = count + ? WHERE cluster_begin = ? AND app_group = ?";
        rc = sqlite3_prepare(m_db,
                stmt_str.c_str(),
                stmt_str.size(),
                &m_addKeyPress_updateStmt1, NULL);
        if(rc != SQLITE_OK) {
            _dbg("update1 init failed");
            return false;
        }

        // Statement for updating existing key record
        stmt_str = "SELECT id FROM keypresses WHERE cluster_begin = ? LIMIT 1";
        rc = sqlite3_prepare(m_db,
                stmt_str.c_str(),
                stmt_str.size(),
                &m_addKeyPress_existsStmt1, NULL);
        if(rc != SQLITE_OK) {
            _dbg("exists1 init failed");
            return false;
        }

        return true;
    }

    /** 
     * @brief Returns start time boundary for given time stamp
     * @return time stamp
     */
    int StorageSqlite::getClusterStart(int timestamp) {
        return timestamp - (timestamp % m_clusterSize);
    }

    /** 
     * @brief Connects to given database
     * 
     * @param uri 
     */
    bool StorageSqlite::connect(std::string uri) {
        m_uri = uri;
        int rc = sqlite3_open(m_uri.c_str(), &m_db);
        if(rc != SQLITE_OK) {
            sqlite3_close(m_db);
            _dbg("sqlite3_open failed");
            return false;
        }
        if(!initDatabase()) {
            sqlite3_close(m_db);
            _dbg("initDatabase() failed");
            // XXX
            throw exception();
            return false;
        }
        if(!prepareStatements()) {
            sqlite3_close(m_db);
            _dbg("prepareStatements() failed");
            return false;
        }
        return true;
    }

    /** 
     * @brief Disconnects from database
     */
    void StorageSqlite::disconnect() {
        sqlite3_close(m_db);
    }

    /** 
     * @brief Records keypresses at actual time
     * 
     * @param count how many events
     */
    bool StorageSqlite::addKeyPress(int app_group, int count) {
        return addKeyPress(time(NULL), app_group, count);
    }

    /** 
     * @brief Records keypresses at given time
     * 
     * @param timestamp key press event time
     * @param count how many events
     */
    bool StorageSqlite::addKeyPress(int timestamp, int app_group, int count) {
        int rc;
        _dbg("addKeyPress -- UPDATE (timestamp=%d, app_group=0x%x, count=%d)", timestamp, app_group, count);

        // SQL Tip: "update keypresses SET count = count + ? WHERE cluster_begin = ? AND app_group = ?"
        // SQL Tip: "insert into keypresses ( cluster_begin, cluster_end, count, app_group )" 

        int cluster_begin = getClusterStart(timestamp);
        int cluster_end = cluster_begin + m_clusterSize;

        // First try update statement
        sqlite3_bind_int(m_addKeyPress_updateStmt1, 1, count);
        sqlite3_bind_int(m_addKeyPress_updateStmt1, 2, cluster_begin);
        sqlite3_bind_int(m_addKeyPress_updateStmt1, 3, app_group);
        rc = sqlite3_step(m_addKeyPress_updateStmt1);
        sqlite3_reset(m_addKeyPress_updateStmt1);
        if( rc != SQLITE_DONE || 0 == sqlite3_changes(m_db) ) {
            _dbg("addKeyPress -- INSERT (UPDATE FAIL rc=%d)", rc);
            // Try insert statement                 
            sqlite3_bind_int(m_addKeyPress_insertStmt1, 1, cluster_begin);
            sqlite3_bind_int(m_addKeyPress_insertStmt1, 2, cluster_end);
            sqlite3_bind_int(m_addKeyPress_insertStmt1, 3, count);
            sqlite3_bind_int(m_addKeyPress_insertStmt1, 4, app_group);
            rc = sqlite3_step(m_addKeyPress_insertStmt1);
            sqlite3_reset(m_addKeyPress_insertStmt1);
            if( rc != SQLITE_DONE || 0 == sqlite3_changes(m_db)) {
                _dbg("addKeyPress -- FAIL (rc=%d)", rc);
                return false;
            }
        }
        return true;
    }
}
