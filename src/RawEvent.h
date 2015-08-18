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

#ifndef KEYFROGRAWEVENT_H
#define KEYFROGRAWEVENT_H

#include <X11/Xlibint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "EventInternal.h"

// TODO: include native OSX events support

namespace keyfrog
{
    /**
     * @author Sebastian Gniazdowski
     */
    class RawEvent
    {
        XRecordDatum m_data;
        int m_time;

        public:
        RawEvent();
        ~RawEvent();

        unsigned char type() const { return m_data.type; }
        xEvent event() const { return m_data.event; }
        xResourceReq resourceReq() const { return m_data.req; }
        xGenericReply genericReply() const { return m_data.reply; }
        xError error() const { return m_data.error; }
        xConnSetupPrefix connSetupPrefix() const { return m_data.setup; }               
        int time() const { return m_time; }

        // Only one setter should be used generally (except for type and time)
        // because all this fields are in union

        void setType(unsigned char _type) { m_data.type = _type; }
        void setEvent(const xEvent& _event) { m_data.event = _event; }
        void setResourceReq(const xResourceReq& _req) { m_data.req = _req; }
        void setGenericReply(const xGenericReply& _reply) { m_data.reply = _reply; }
        void setError(const xError& _error) { m_data.error = _error; }
        void setConnSetupPrefix(const xConnSetupPrefix& _setup) { m_data.setup = _setup; }
        void setTime(int _time) { m_time = _time; }
    };
}

#endif
