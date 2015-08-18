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

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "EventFilter.h"
#include "RawEvent.h"
#include "Event.h"
#include "Common.h"
#include "TermCode.h"
#include <sys/types.h>
#include "Debug.h"

#include <list>

using namespace std;
using namespace keyfrog::TermCodes;
using namespace boost;

namespace keyfrog {
    /** 
     * Takes and saves window cache as reference,
     * no new object is being created.
     *
     * @param wim Window cache, as reference
     */
    EventFilter::EventFilter(WindowInformationManager & wim, ProcessManager & pm) : m_wim(wim), m_pm(pm) {
        m_wim.setDisplay(m_eventMonitor.ctrlDisplay());
    }

    /** 
    */
    EventFilter::~EventFilter() {
    }

    bool EventFilter::connect(string displayName) {
        return m_eventMonitor.connect(displayName);
    }
    /** 
     * All subsystems are started, ie. EventMonitor.
     */
    void EventFilter::start() {
        m_eventMonitor.start();
    }

    /** 
     * All key catching subsystems are started.
     */
    void EventFilter::stop() {
        m_eventMonitor.stop();
    }

    /**
     * Fetches queued raw events from EventMonitor,
     * converts them to higher level events.
     */
    void EventFilter::processEvents() {
        // Get only preprocessed raw events,
        // do not start processing of new ones
        while(m_eventMonitor.numEvents()) {
            RawEvent rawEvent = m_eventMonitor.nextEvent();
            Event event;
            Window window;
            // Fields required to set: time, type, groupId
            event.setTime(rawEvent.time());
            switch(rawEvent.type()) {
                case KeyPress:
                    event.setType(kfKeyPress);
                    window = rawEvent.event().u.keyButtonPointer.event;
                    event.setDestWin(window);
                    setGroupId(event, window);
                    break;
                case DestroyNotify:
                    event.setType(kfDestroyNotify);
                    window = rawEvent.event().u.destroyNotify.window;
                    event.setDestWin(window);
                    break;
                case FocusIn:
                    event.setType(kfFocusIn);
                    break;
                default:
                    // This should not happen
                    _err("Unrecognized event type [%d]", rawEvent.type());
#ifdef _KF_DEBUG
                    throw std::exception();
#endif
            }
            m_events.push_back(event);
        }
    }

    /**
     * Receives and adds new event into internal queue
     */
    void EventFilter::waitForEvents() {
        while(m_events.size() == 0) {
            m_eventMonitor.waitForEvents();
            processEvents();
        }
    }

    /**
     * Event is removed from internal queue
     * @return Next buffered event
     */
    Event EventFilter::nextEvent() {
        if(numEvents() == 0) 
            waitForEvents();
        Event event = m_events.front();
        m_events.pop_front();
        return event;
    }

    /**
     * Traverses all groups looking for a match
     */
    void EventFilter::setGroupId(Event & event, Window window) {
        int gid = -1;
        event.setGroupId(-1);

        m_wim.setDisplay(m_eventMonitor.ctrlDisplay());
        if(!m_wim.findAndUseWindow(window)) {
            return;
        }

        string className = m_wim.fetchClassName();

        // Detect terminal, then match against child unix processes
        // TODO: database (external?) of uncommon terminals
        int pid = -1;
        if ( iends_with( className, "term" ) || iends_with( className, "rxvt" )
                || iends_with( className, "konsole" ) || iends_with( className, "xterminal" )
           ) {
            // It's terminal, so we want to check sub-sub-process (TODO: any odd cases?)
            _dbg("Terminal window!");
            pid = m_wim.fetchClientPid();
            if(pid != -1 && pid != 0)
                gid = matchTermProc(pid);
        }

        // Match against process name that owns the Window
        // TODO: use against OSX native windows
        if( gid == -1 ) {
            pid = m_wim.fetchClientPid();
            if(pid != -1 && pid != 0)
                gid = matchProc(pid);
        }

        // Third check: window class name
        // TODO: handle empty class name (e.g. OSX native window -- or better to simulate X11 class name?)
        // TODO: skip match against terminals? should be configurable
        if( gid == -1 ) {
            // Does class name match any group?
            gid = matchWindowClass(className);
        }

        if( gid != -1 )
            _dbg( "Match group (%s%d%s)", cboldRed, gid, creset );
        else 
            _dbg( "No match (%s)(pid:%d)", className.c_str(), pid );

        event.setGroupId( gid );
    }

    int EventFilter::matchWindowClass(string & className) {
        int gid = -1;
        list<Group> & groups = m_filterConfig.groups();

        // Walk through ALL groups (and then iterate over class names, which may not exist)
        bool matched = false;
        for(list<Group>::iterator grp = groups.begin(); grp != groups.end(); ++grp) {
            const list<string> & wndClasses = grp->windowClasses();
            // Walk through windowClasses
            for ( list<string>::const_iterator clp = wndClasses.begin(); clp != wndClasses.end(); ++clp ) {
                // Case insensitive compare
                if( string_eq_ci( (*clp), className ) ) {
                    matched = true;
                    // Set matched group
                    gid = grp->id();
                    break;
                }
            }
            if(matched)
                break;
        }
        return gid;
    }

    int EventFilter::matchTermProc(pid_t pid) {
        int gid = -1;
        const list<Group> & groups = m_filterConfig.groups();

        // Fetch the set of descendant processes
        set< pair<pid_t, string> > dprocs = m_pm.fetchDescendants(pid);

        // Walk through ALL groups (and then iterate over proc names, which may not exist)
        bool matched = false;
        for ( list<Group>::const_iterator grp = groups.begin(); grp != groups.end(); ++grp ) {
            const list<string> & termProcs = grp->termProcs();

            // Iterate through term proc names
            // TODO: correct term proc match must be at leaf process?
            for( list<string>::const_iterator tpit = termProcs.begin(); tpit != termProcs.end(); ++tpit ) {

                // Compare proc names in group with fetched sub processes of terminal
                _ldbg("TermProc Compare");
                for(set< pair<pid_t, string> >::iterator it = dprocs.begin(); it != dprocs.end(); ++it) {
                    _qldbg(" %s == %s,", tpit->c_str(), it->second.c_str());
                    if( string_eq_ci( (*tpit), it->second ) ) {
                        matched = true;
                        gid = grp->id();
                        break;
                    }
                }
                _qdbg("");
                if(matched)
                    break;
            }
            if(matched)
                break;
        }

        return gid;
    }

    int EventFilter::matchProc(pid_t pid) {
        int gid = -1;
        const list<Group> & groups = m_filterConfig.groups();
        const std::string & procName = m_pm.fetchName( pid );

        // Walk through ALL groups (and then iterate over proc names, which may not exist)
        bool matched = false;
        for ( list<Group>::const_iterator grp = groups.begin(); grp != groups.end(); ++grp ) {
            const list<string> & procs = grp->procs();

            // Iterate through proc names
            for ( list<string>::const_iterator pit = procs.begin(); pit != procs.end(); ++pit ) {
                // Compare proc names in group with proc name of given pid
                _ldbg( "ProcCompare %s == %s", pit->c_str(), procName.c_str() );
                if( string_eq_ci( (*pit), procName ) ) {
                    matched = true;
                    gid = grp->id();
                    break;
                }
            }
            if(matched)
                break;
        }

        return gid;
    }
}

