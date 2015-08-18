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

#if HAVE_CONFIG_H       /* HAVE_CONFIG_H */
#include <config.h>
#else                           /* HAVE_CONFIG_H */
#include <FallbackConfigH.h>
#endif                          /* HAVE_CONFIG_H */

#ifdef HOST_IS_OSX

#include <unistd.h>
#include <exception>
using namespace std;

#include "TermCode.h"
#include "EventMonitorMac.h"
#include "EventInternal.h"
#include "XErrorUtil.h"
#include "Debug.h"

using namespace keyfrog::TermCodes;

namespace keyfrog {
    /**
     * Constructor which optionally takes display name
     */
    EventMonitorMac::EventMonitorMac() : m_eventTap(0), m_runLoopSource(0) {
    }

    /**
     * Destructor musts unregister userData because
     * it contains pointer to object being destroyed
     * Virtual
     */
    EventMonitorMac::~ EventMonitorMac() {
        stop();
        CFRelease(m_eventTap);
        CFRelease(m_runLoopSource);
    }

    /**
     * Virtual
     */
    bool EventMonitorMac::connect(string unused) {
        return setupEventTap();
    }

    /**
     * Attaches to OSX event system
     */
    bool EventMonitorMac::setupEventTap() {
        CGEventMask mask = CGEventMaskBit(kCGEventKeyUp) | CGEventMaskBit(kCGEventKeyDown);

        m_eventTap = CGEventTapCreate( kCGHIDEventTap, kCGTailAppendEventTap, kCGEventTapOptionDefault, mask, eventCallback, NULL );

        if( !m_eventTap ) {
            // FIXME
            fprintf( stderr, "Couldn't create event tap!" );
            return false;
        }

        m_runLoopSource = CFMachPortCreateRunLoopSource( kCFAllocatorDefault, m_eventTap, 0 );

        CFRunLoopAddSource( CFRunLoopGetMain(), m_runLoopSource, kCFRunLoopCommonModes );

        return true;
    }

    /**
     * Starts event capturing
     * Virtual
     */
    void EventMonitorMac::start() {
        CGEventTapEnable( m_eventTap, true );
    }

    /**
     * Stops event capturing
     * Virtual
     */
    void EventMonitorMac::stop() {
        CGEventTapEnable( m_eventTap, false );
    }

    /**
     * Give time to CF Loop?
     * Virtual
     */
    void EventMonitorMac::processEvents() {
    }

    /**
     * Waits for new OSX event (TODO: thread)
     * Virtual
     *
     * @return RawEvent object
     */
    void EventMonitorMac::waitForEvents() {
        int i = 10;
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
    RawEvent EventMonitorMac::nextEvent() {
        if ( numEvents() == 0 )
            waitForEvents();
        RawEvent re = events.front();
        events.pop_front();
        return re;
    }

    /**
     * Called from Xserver when new event occurs. Prepares
     * RawEvent object and stores it into EventMonitorMac
     * events attribute.
     */
    CGEventRef EventMonitorMac::eventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {

        // TODO: update for OSX
        RawEvent newRawEvent;           
    }
}

#endif

