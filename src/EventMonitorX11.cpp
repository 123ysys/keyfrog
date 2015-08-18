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

#include <unistd.h>
#include <exception>
using namespace std;

#include "TermCode.h"
#include "CallbackClosure.h"
#include "EventMonitorX11.h"
#include "EventInternal.h"
#include "XErrorUtil.h"
#include "Debug.h"

using namespace keyfrog::TermCodes;

namespace keyfrog {
    /**
     * Constructor which optionally takes display name
     */
    EventMonitorX11::EventMonitorX11() {
    }

    /**
     * Destructor musts unregister userData because
     * it contains pointer to object being destroyed
     * Virtual
     */
    EventMonitorX11::~ EventMonitorX11() {
        stop();
    }

    /**
     * Virtual
     */
    bool EventMonitorX11::connect(string displayName) {
        m_displayName = displayName;
        if (NULL == (userData.ctrlDisplay = XOpenDisplay(m_displayName.c_str())) ) {
            return false;
        }
        if (NULL == (userData.dataDisplay = XOpenDisplay(m_displayName.c_str())) ) {
            XCloseDisplay(userData.ctrlDisplay);
            userData.ctrlDisplay = NULL;
            return false;
        }

        // Set custom error handler
        XErrorUtil::initHandler(1);

        root = DefaultRootWindow(userData.ctrlDisplay);

        // Store pointer to this object so that static callback can use it
        userData.initialObject = (void *)this;

        setupRecordExtension();

        return true;
    }

    /**
     * Initializes record extension
     */
    void EventMonitorX11::setupRecordExtension() {
        XSynchronize(userData.ctrlDisplay, True);

        // Record extension exists?
        if (!XRecordQueryVersion (userData.ctrlDisplay, &recVer.first, &recVer.second)) {
            _inf(
                    "%sThere is no RECORD extension loaded into X server.\n"
                    "Try installing packages with 'xtst' in name (e.g. libxtst6, libxtst-dev for Ubuntu, xorg-libXtst for MacPorts)\n"
                    "and restarting your desktop. If you use older X.org server (unlikely), also add following line:\n"
                    "   Load  \"record\"\n"
                    "to /etc/X11/xorg.conf, in Module section.\n\n"

                    "On OS X the RECORD extension needs to be enabled first (restart X11 after\n"
                    "issuing one of the 'defaults' command):\n\n"

                    "# if you use MacPorts Xserver (Xquartz)\n"
                    "$ defaults write org.macports.X11 enable_test_extensions -boolean true\n\n"

                    "# if you use Apple's original Xquartz Xserver\n"
                    "$ defaults write org.x.X11 enable_test_extensions -boolean true\n\n"

                    "# if you use open source Xquartz Xserver\n"
                    "$ defaults write org.macosforge.xquartz.X11 enable_test_extensions -boolean true"

                    "%s"
                    , ccyan, creset);
            throw exception();
        }
        _dbg("Record extension v%d.%d", recVer.first, recVer.second);

        // Configure it
        recRanges[0] = XRecordAllocRange();
        recRanges[1] = XRecordAllocRange();
        if (!recRanges[0] || !recRanges[1]) {
            // "Could not alloc record range object!\n";
            throw exception();                      
        }
        // First of all - key presses are recorded
        recRanges[0]->delivered_events.first=KeyPress;
        recRanges[0]->delivered_events.last=KeyPress;

        // Also we must record DestroyNotify to keep
        // window information cache actual
        recRanges[1]->delivered_events.first=DestroyNotify;
        recRanges[1]->delivered_events.last=DestroyNotify;

        // Determine whether to workaround X server bug
        bool applyFix = true;
        char *tmp = ServerVendor(userData.ctrlDisplay);
        string xsv(tmp ? tmp : "Unknown");
        int xvr = VendorRelease(userData.ctrlDisplay);
        _dbg("X server: %s v%d.%d.%d (%d)", xsv.c_str(),
                xvr / 10000000,
                (xvr / 100000) % 100,
                (xvr / 1000) % 100,
                xvr
            );

        // The 70101000 is for old X.Org versioning convention, and
        // denotes a version with the bug fixed. The 50000000 is to
        // detect new X.org servers with the new versioning convention
        if( xsv.find("X.Org") && ( xvr >= 70101000 || xvr <= 50000000 ) )
            applyFix = false;

        if(applyFix) {
            _dbg("%s**%s Applying record extension workaround!", cboldRed, creset);
            for(int i=0; i<=1; i++) {
                recRanges[i]->errors.first = BadCursor;
                recRanges[i]->errors.last = BadCursor;
            }
        }

        recClientSpec = XRecordAllClients;

        // Get context with our configuration
        recContext = XRecordCreateContext (userData.ctrlDisplay, 0, 
                &recClientSpec, 1, recRanges, 2);
        if (!recContext) {
            // "Could not create a record context!\n"
            throw exception();                      
        }               
    }

    /**
     * Starts event capturing. 
     * Virtual
     */
    void EventMonitorX11::start() {
        if (!XRecordEnableContextAsync (userData.dataDisplay, recContext,
                    eventCallback, (XPointer) &userData)) {
            // "Could not enable the record context!\n"
            throw exception();
        }               
    }

    /**
     * Stops event capturing
     * Virtual
     */
    void EventMonitorX11::stop() {
        if(!XRecordDisableContext (userData.ctrlDisplay, recContext))
            throw exception();      
    }

    /**
     * Goes into Xserver, which listens for new event and
     * calls eventCallback(). 
     * Virtual
     */
    void EventMonitorX11::processEvents() {
        XRecordProcessReplies (userData.dataDisplay);
    }

    /**
     * Waits for new X11 event (TODO: thread)
     * Virtual
     *
     * @return RawEvent object
     */
    void EventMonitorX11::waitForEvents() {
        // Proper event will be saved in list
        // others will be skipped
        while ( events.size() == 0 ) {
            // Wait for some event
            processEvents();
            usleep(75000);
        }
    }

    /**
     * Event is removed from internal queue
     * TODO: should not be blocking
     * Virtual
     */
    RawEvent EventMonitorX11::nextEvent() {
        if ( numEvents() == 0 )
            waitForEvents();
        RawEvent re = events.front();
        events.pop_front();
        return re;
    }

    /**
     * Called from Xserver when new event occurs. Prepares
     * RawEvent object and stores it into EventMonitorX11
     * events attribute.
     */
    void EventMonitorX11::eventCallback(XPointer priv, XRecordInterceptData *hook) {
        static int pc = 0;
        /* FIXME: need use XQueryPointer to get the first location */
        if (hook->category != XRecordFromServer) {
            XRecordFreeData (hook);
            return;
        }

        CallbackClosure *userData = (CallbackClosure *)priv;
        XRecordDatum *data = (XRecordDatum*) hook->data;
        if(data->event.u.u.type == KeyPress) { 
            int c = data->event.u.u.detail;
            if(c == pc) {
                return;
            } else
                pc = c;
        }

        // Create new RawEvent that will be remembered
        RawEvent newRawEvent;           

        // It is done in such not so pretty way
        // to make interface more generic
        newRawEvent.setType(data->type);
        newRawEvent.setConnSetupPrefix(data->setup);
        newRawEvent.setError(data->error);
        newRawEvent.setGenericReply(data->reply);
        newRawEvent.setResourceReq(data->req);
        newRawEvent.setEvent(data->event);
        newRawEvent.setTime(hook->server_time);
        XRecordFreeData (hook);

        // Append newly received RawEvent to list
        ((EventMonitorX11 *)userData->initialObject)->events.push_back(newRawEvent);
    }
}
