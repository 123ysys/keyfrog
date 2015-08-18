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

#ifndef KEYFROGDEBUG_H
#define KEYFROGDEBUG_H

// FIXME
//#define _KF_DEBUG 1

#ifdef _KF_DEBUG

#include <cstdio>
#include <cstdarg>

#ifdef __OPTIMIZE__
#define __OPT__ 1
#else
#define __OPT__ 0
#endif

#ifdef __OPTIMIZE_SIZE__
#define __OPT_SIZE__ 1
#else
#define __OPT_SIZE__ 0
#endif

#endif /* _KF_DEBUG */

namespace keyfrog {

    namespace Debug {
#ifdef _KF_DEBUG
        void __dbg(const char *f, int l, int flag, const char *format, ...);
        enum Flags {
            quiet = 1,
            nonewline = 2,
            info = 4,
        };
        // Full debug
#define _dbg(format, args...) Debug::__dbg(__FILE__, __LINE__, 0, format , ## args)
        // Quiet 
#define _qdbg(format, args...) Debug::__dbg(NULL, 0, Debug::quiet, format, ## args)
        // No new line
#define _ldbg(format, args...) Debug::__dbg(__FILE__, __LINE__, Debug::nonewline, format, ## args)
        // No new line and quiet
#define _qldbg(format, args...) Debug::__dbg(NULL, 0, Debug::quiet | Debug::nonewline, format , ## args)

        // Error function
        void __err(int flags, const char *file, const char *function, int line, const char *date, const char *version,
                int opt, int opt_size, const char *txt=NULL, ...);
#define _err(args...) Debug::__err(0, __FILE__, __FUNCTION__, __LINE__, __DATE__ " " __TIME__, __VERSION__, __OPT__, __OPT_SIZE__, ##args)
#define _inf(args...) Debug::__err(Debug::info,__FILE__, __FUNCTION__, __LINE__, __DATE__ " " __TIME__, __VERSION__, __OPT__, __OPT_SIZE__, ##args)

#else /* _KF_DEBUG */

#define _dbg(format, args...)
#define _qdbg(format, args...)
#define _ldbg(format, args...)
#define _qldbg(format, args...)

#define _err(format, args...)
#define _inf(format, args...)

#endif /* _KF_DEBUG */

    }
}
#endif
