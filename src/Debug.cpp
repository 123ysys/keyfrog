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
#include <cstdlib>
#include <cstring>
#include "Debug.h"
#include "TermCode.h"

#ifdef _KF_DEBUG

#define DEBUG_FLAG_SET(flag, x) (x&flag)
#define DEBUG_FLAG_NSET(flag, x) (!(x&flag))

#endif /* _KF_DEBUG */

using namespace keyfrog::TermCodes;

namespace keyfrog {
    namespace Debug {

#ifdef _KF_DEBUG
        // Debug configuration
        const char * logFilePath = NULL;
        bool useLogFile = true;
        bool useStdErr = true;

        /** 
         * @brief Main debug function
         * 
         * @param format Format string like in printf etc.
         */
        void __dbg(const char *f, int l, int flag, const char *format, ...) {
            va_list argp;
            FILE *log;

            if (logFilePath == NULL) {
                const char *home = getenv("HOME");
                logFilePath = (const char *) new char[ strlen(home)+64 ];
                if ( logFilePath == NULL ) {
                    logFilePath = "/tmp/keyfrog.log";
                } else {
                    sprintf( (char *) logFilePath, "%s/.keyfrog/keyfrog.log", home);
                }
            }

            if (format == NULL)
                return;

            // Log to file?
            if(useLogFile) {
                log = fopen(logFilePath, "a");
                if (log != NULL) {
                    if(DEBUG_FLAG_NSET(quiet, flag)) {
                        fprintf(log, "%s%s:%s%d ", f, ccyan, creset, l);
                    }

                    va_start(argp, format);
                    vfprintf(log, format, argp);
                    va_end(argp);

                    if(DEBUG_FLAG_NSET(nonewline, flag)) {
                        fputs("\n",log);
                    }

                    fclose(log);
                }
            }

            // Output to stderr?
            if(useStdErr) {
                if(DEBUG_FLAG_NSET(quiet, flag)) {
                    fprintf(stderr, "%s%s:%s%d ", f, ccyan, creset, l);
                }

                va_start(argp, format);
                vfprintf(stderr, format, argp);
                va_end(argp);

                if(DEBUG_FLAG_NSET(nonewline, flag)) {
                    fputs("\n",stderr);
                }
            }
        }

        void __err(int flags, const char *file, const char *function, int line, const char *date, const char *version,
                int opt, int opt_size, const char *txt, ...)
        {
            va_list argp;

            const char *cmyColor;
            if(DEBUG_FLAG_NSET(info, flags)) {
                cmyColor = cboldRed;
                printf("%s**%s ERROR! %s**%s\n", cmyColor, creset, cmyColor, creset);
            } else {
                cmyColor = cboldCyan;
                printf("%s**%s INFORMATION %s**%s\n", cmyColor, creset, cmyColor, creset);
            }
            printf("File:                \t %s\n",file);
            printf("Function             \t %s\n",function);
            printf("Line:                \t %d\n",line);
            printf("Compilation date:    \t %s\n",date);
            printf("Compiler:            \t %s\n",version);
            printf("Optimization:        \t %s\n",opt==1 ? "Yes" : "No");
            printf("Size optimization:   \t %s\n",opt_size==1 ? "Yes" : "No");

            if (txt == NULL)
                return;
            printf("%s**%s Description: %s**%s\n", cmyColor, creset, cmyColor, creset);
            va_start(argp, txt);
            vprintf(txt, argp);
            va_end(argp);
            printf("\n%s**%s%s**%s\n", cmyColor, creset, cmyColor, creset);
        }

#endif
    }
}
