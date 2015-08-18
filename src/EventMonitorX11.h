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

#ifndef KEYFROG_DAEMONKEYMONITORX11_H
#define KEYFROG_DAEMONKEYMONITORX11_H

#include <string>
#include <utility>
#include <list>

#include <X11/Xlibint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Shell.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/extensions/record.h>
#include <X11/extensions/XTest.h>

#include "EventMonitor.h"
#include "CallbackClosure.h"
#include "RawEvent.h"

namespace keyfrog {

    /// Allows to catch every keypress in X11
    class EventMonitorX11 : public EventMonitor {
        private:
            /// X display address
            std::string m_displayName;
            /// Used to pass user data in record extension callback
            CallbackClosure userData;
            /// Root window
            Window root;
            /// Received events
            std::list<RawEvent> events;

            XRecordRange *recRanges[2];
            XRecordClientSpec recClientSpec;
            XRecordContext recContext;
            /// Record extension version
            std::pair<int,int> recVer;

            void setupRecordExtension();
            // TODO: hide implementation?
            static void eventCallback(XPointer priv, XRecordInterceptData *hook);
        public:
            /// Tries to connect to Xserver
            virtual bool connect(std::string displayName = ":0");

            /// Starts event capturing
            virtual void start();

            /// Stops event capturing
            virtual void stop();

            /// Checks for events in X11 queue, processes and requeues them locally
            virtual void processEvents();

            /// Waits for pack of events, processes and requeues them locally
            virtual void waitForEvents();

            /// Returns next event
            virtual RawEvent nextEvent();

            /// Returns how many processed events are waiting in local queue for fetch
            virtual int numEvents() const { return events.size(); }

            /// Returns control display
            Display *ctrlDisplay() const { return userData.ctrlDisplay; }

            /// Returns data display
            Display *dataDisplay() const { return userData.dataDisplay; }

            EventMonitorX11();
            virtual ~EventMonitorX11();
    };

}

#endif
