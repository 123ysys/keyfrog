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

#include "StorageManager.h"
#include "Debug.h"
#include <sstream>
#include <ctime>
#include <vector>
#include <boost/version.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#if BOOST_VERSION >= 105000
#define THE_TIME_UTC boost::TIME_UTC_
#else
#define THE_TIME_UTC boost::TIME_UTC
#endif

// TODO:
// - reduce dependencies (sstream? boost/string? vector?)
// - choose one way to do things

using namespace std;

namespace keyfrog {

    // Commiter (works in thread)
    void StorageManager::StorageManagerCommiter::operator()() {
        while(1) {
            xtime_get(&m_xt, THE_TIME_UTC);
            m_xt.sec += 5;
            boost::thread::sleep(m_xt);

            map<string,int> localcache;
            // Get actual cache snapshot
            {
                boost::mutex::scoped_lock lock(m_owner->m_cache_mutex);
                localcache = m_owner->m_cache;
                m_owner->m_cache.clear();
            }

            // Iterate through all cached key presses and send them to real storage
            for( map<string,int>::iterator it = localcache.begin(); it != localcache.end(); it++) {
                // Convert string "{app_group}_{timestamp}" to two numbers
                vector<string> result;
                split(result, it->first, boost::is_any_of("_"));

                int cluster_begin = boost::lexical_cast<int>(result[0]);
                int app_group = boost::lexical_cast<int>(result[1]);

                // Now propagate those parameters to real storage
                m_owner->m_backend->addKeyPress(app_group, cluster_begin, it->second);
            }

            _dbg("Committed");
        }
    }

    StorageManager::StorageManager(Storage *backend) {
        // Object that will do real writes
        m_backend = backend;

        // Create commiter
        m_commiter = new StorageManagerCommiter(this);

        // Create thread
        m_commiterThread = new boost::thread(*m_commiter);
    }


    StorageManager::~StorageManager() {
    }

    bool StorageManager::connect(std::string uri) {
        m_backend->connect(uri);
        return true;
    }

    void StorageManager::disconnect() {
        m_backend->disconnect();
    }

    // FIXME: cluster calculation should be outside Storage interface?
    int StorageManager::getClusterStart(int timestamp) {
        return m_backend->getClusterStart(timestamp);
    }

    bool StorageManager::addKeyPress(int app_group, int count) {
        addKeyPress(app_group, time(NULL), count);
        return true;
    }

    /** 
     * Fake addKeyPress method. It only caches given key press.
     */
    bool StorageManager::addKeyPress(int app_group, int timestamp, int count) {
        stringstream stream;
        int begin_cluster = m_backend->getClusterStart(timestamp);
        stream << app_group << "_" << begin_cluster;
        boost::mutex::scoped_lock lock(m_cache_mutex);
        // Add count
        m_cache[stream.str()]  += count;
        _dbg("+=%d, m_cache(%s) is now: %d", count, stream.str().c_str(), m_cache[stream.str()]);
        return true;
    }
}
