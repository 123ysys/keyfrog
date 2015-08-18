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

#ifndef KEYFROG_DAEMONKEYMONITORMAC_H
#define KEYFROG_DAEMONKEYMONITORMAC_H

#include <string>
#include <utility>
#include <list>

// The OSX framework that lets intercepting events
extern "C" {
#undef CR
#define __MACHINEEXCEPTIONS_USE_OLD_CR_FIELD_NAME__ 1
#include <Carbon/Carbon.h>
}

#include "EventMonitor.h"
#include "RawEvent.h"

namespace keyfrog {

    /// Allows to catch every keypress in X11
    class EventMonitorMac : public EventMonitor {
        private:
            /// Received events. TODO: CGEventRef
            std::list<RawEvent> events;

            // Attaches to OSX event system
            bool setupEventTap();

            // TODO: hide implementation?
            CFMachPortRef m_eventTap;
            CFRunLoopSourceRef m_runLoopSource;
            static CGEventRef eventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon);
        public:
            /// Tries to insert the event tap
            virtual bool connect(std::string unused = "");

            /// Starts event capturing
            virtual void start();

            /// Stops event capturing
            virtual void stop();

            /// Unused in Mac Monitor
            virtual void processEvents();

            /// Waits for pack of events
            virtual void waitForEvents();

            /// Returns next event (TODO: RawEvent)
            virtual RawEvent nextEvent();

            /// Returns how many processed events are waiting in local queue for fetch
            virtual int numEvents() const { return events.size(); }

            EventMonitorMac();
            virtual ~EventMonitorMac();
    };

}

#endif
