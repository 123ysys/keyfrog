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

#ifndef KEYFROGTERMUTIL_H
#define KEYFROGTERMUTIL_H

#include <string>
#include <iostream>

namespace keyfrog {
    class TermCode;

    /**
     * @author Sebastian Gniazdowski <srnt at users dot sf dot net>
     */
    class TermCode {
        std::string m_code;
        public:
        enum Codes {
            red,
            boldRed,
            green,
            boldGreen,
            blue,
            boldBlue,
            yellow,
            boldYellow,
            purple,
            boldPurple,
            cyan,
            boldCyan,
            reset,
            null
        };
        TermCode(Codes code);
        ~TermCode();

        const char *ccode();

        template <class charT, class Traits>
            friend std::basic_ostream<charT, Traits> &
            operator <<(std::basic_ostream<charT, Traits> &os, TermCode & termCode);
    };

    template <class charT, class Traits>
        std::basic_ostream<charT, Traits> &
        operator <<(std::basic_ostream<charT, Traits> &os, TermCode & termCode) {
            os << termCode.m_code;
            return os;
        }

    namespace TermCodes {
        extern TermCode red;
        extern TermCode boldRed;
        extern TermCode green;
        extern TermCode boldGreen;
        extern TermCode blue;
        extern TermCode boldBlue;
        extern TermCode yellow;
        extern TermCode boldYellow;
        extern TermCode cyan;
        extern TermCode boldCyan;
        extern TermCode reset;
        extern const char * cred;
        extern const char * cboldRed;
        extern const char * cgreen;
        extern const char * cboldGreen;
        extern const char * cblue;
        extern const char * cboldBlue;
        extern const char * cyellow;
        extern const char * cboldYellow;
        extern const char * ccyan;
        extern const char * cboldCyan;
        extern const char * creset;

        void nullOutput();
    }
}

#endif
