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

#include <cstring>
#include "WindowInformationManager.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "TermCode.h"
#include "Debug.h"

using namespace std;
using namespace keyfrog::TermCodes;

namespace keyfrog {
    WindowInformationManager::WindowInformationManager() : m_display(NULL) {
        _dbg("WindowInformationManager constructor: no display given");

    }

    WindowInformationManager::WindowInformationManager(Display *display) : m_display(display) {
        _dbg("WindowInformationManager constructor: with display");
    }

    WindowInformationManager::~WindowInformationManager() {
    }

    /**
     * Searches for given window, sets it's as current window
     *
     * return Was given window successfully found (always successful?)
     */
    bool WindowInformationManager::findAndUseWindow(Window window) {
        X11WindowInfoCache_it tmpIterator = m_cache.find(window);
        // No such window in cache?
        if( m_cache.end() == tmpIterator ) {
            // Add it
            m_cache[window] = WindowInformation();
            tmpIterator = m_cache.find(window);
        }
        m_currentCacheEntry = tmpIterator;
        return true;
    }

    const std::string & WindowInformationManager::fetchClassName() {
        static const std::string empty_string = "";
        if( m_cache.end() == m_currentCacheEntry )
            return empty_string;

        WindowInformation & winInfo = m_currentCacheEntry->second;

        // WM_CLASS already fetched and valid?
        if(winInfo.m_classNameOk)
            return winInfo.m_className;

        // No - fetch it
        Window winId = m_currentCacheEntry->first;
        winInfo.m_className = this->resolveClassName(winId);
        // FIXME
        winInfo.m_classNameOk = true;
        return winInfo.m_className;
    }

    pid_t WindowInformationManager::fetchClientPid() {
        if( m_cache.end() == m_currentCacheEntry )
            return 0;

        WindowInformation & winInfo = m_currentCacheEntry->second;
        // PID in cache?
        if(winInfo.m_clientPidOk)
            return winInfo.m_clientPid;

        Window window = m_currentCacheEntry->first;

        // No - try to fetch it

        pid_t pid = resolveClientPid(window);

        if( pid ) {
            winInfo.m_clientPid = pid;
            winInfo.m_clientPidOk = true;
        }

        return pid;
    }

    /**
     * Moves up to the root in window tree to find first window with CLASS property
     *
     * @warning Explicit error handling needed ?
     *
     * @return Class name or empty string if not found 
     */
    string WindowInformationManager::resolveClassName(Window winId) {
        XClassHint hint;
        std::string className = "";
        Window root;

        bool stop = false;
        while(!stop) {
            _dbg("-- Loop -- (0x%x)",winId);
            if(!XGetClassHint(m_display, winId, &hint)) {
                // Empty class name in case of error
                if(!getWindowParent(winId, root)) {
                    stop = true;
                } else {
                    if(winId == root || winId == 0x0) {
                        _dbg("Root window (no hint)! id=0x%x",winId);
                        stop = true;
                    }
                }
            } else {
                if( hint.res_class && strlen( hint.res_class ) > 0 ) {
                    // Try using CLASS first -- if it's not empty
                    className = hint.res_class;
                } else if ( hint.res_name && strlen( hint.res_name ) > 0 ) {
                    // Next try NAME
                    className = hint.res_name;
                } else {
                    // No information -- set error name
                    className = "<unknown>";
                }
                XFree(hint.res_name);
                XFree(hint.res_class);
                stop = true;
            }
        }
        return className;
    }

    pid_t WindowInformationManager::resolveClientPid(Window winId) {
        // Bunch of vars for XGetWindowProperty
        Atom actualType;
        int actualFormat;
        unsigned long nitems, bytesLeft;
        pid_t *p_pid, pid = 0;
        Window root;
        XClassHint hint;

        Atom _NET_WM_PID = XInternAtom(m_display, "_NET_WM_PID", False);
        if(_NET_WM_PID == None)
            return 0;

        bool stop = false;
        while(!stop) {
            _dbg("-- PidLoop -- (0x%x)(pid:%d)",winId,pid);

            if(!XGetClassHint(m_display, winId, &hint)) {
                // Get upper window
                if(!getWindowParent(winId, root) || winId == root || winId == 0x0)
                    stop = true;
            } else {
                // Window has properties - so now try to fetch _NET_WM_PID !
                Status result = XGetWindowProperty(
                        m_display, winId, _NET_WM_PID, 0, 1, False,
                        XA_CARDINAL, &actualType, &actualFormat, &nitems, &bytesLeft,
                        reinterpret_cast<unsigned char**>(&p_pid));
                if(result == Success && nitems && p_pid) {
                    pid = *(reinterpret_cast<pid_t*>(p_pid));
                    XFree(p_pid);
                }

                // Stop - no sense to check other windows
                stop = true;
            }
        }
        return pid;
    }

    bool WindowInformationManager::getWindowParent(Window & winId, Window & _root) {
        Window root, parent, *children = NULL;
        unsigned int num_children;

        if(!XQueryTree(m_display, winId, &root, &parent, &children, &num_children))
            return false;

        if (children)
            XFree((char *)children);

        winId = parent;
        _root = root;
        return true;
    }

    void WindowInformationManager::invalidateEntry() {
        if( m_cache.end() != m_currentCacheEntry ) {
            _dbg("INVALIDATE: 0x%x", m_currentCacheEntry->first);
            m_cache.erase(m_currentCacheEntry);
            m_currentCacheEntry = m_cache.end();
        }
    }
}
