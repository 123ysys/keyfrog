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

#ifndef KEYFROGEVENTFILTER_H
#define KEYFROGEVENTFILTER_H


#include "ProcessManager.h"
#include "EventMonitorX11.h"
#include "FilterConfig.h"
#include "Event.h"
#include "KfWindowCache.h"

#include <list>

namespace keyfrog {
    /**
      @author Sebastian Gniazdowski
      */
    class EventFilter {
        /// Window properties cache - created outside
        KfWindowCache & m_wim;
        /// Process information cache - created outside
        ProcessManager & m_pm;
        /// Configuration according to EventFilter processes events
        FilterConfig m_filterConfig;
        /// Received and processed events
        std::list<Event> m_events;
        /// Source of events
        EventMonitorX11 m_eventMonitor;

        public:
        EventFilter(KfWindowCache & wim, ProcessManager & pm);

        ~EventFilter();

        void setFilterConfig(const FilterConfig& theValue) {
            m_filterConfig = theValue;
        }

        FilterConfig filterConfig() const {
            return m_filterConfig;
        }

        const EventMonitorX11 & eventMonitor() const {
            return m_eventMonitor;
        }

        /// Checks for events in EventMonitor queue, processes and requeues them locally
        void processEvents();

        /// Waits for pack of events, processes and requeues them locally
        void waitForEvents();

        /// Returns next queued event (FIFO), processes and queues new ones if needed
        Event nextEvent();

        /// Returns how many preprocessed events are waiting for fetch
        int numEvents() const { return m_events.size(); }

        bool connect(std::string displayName = ":0");
        void start();
        void stop();

        /// Matches and sets group id for given event
        void setGroupId(Event & Event, Window window);

        int matchWindowClass(std::string & className);

        int matchTermProc(pid_t pid);

        int matchProc(pid_t pid);
    };
}

#endif
