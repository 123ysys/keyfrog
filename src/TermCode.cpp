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

#include "TermCode.h"

#define RED "\e[0;31m"
#define BOLDRED "\e[1;31m"
#define GREEN "\e[0;32m"
#define BOLDGREEN "\e[1;32m"
#define YELLOW "\e[0;33m"
#define BOLDYELLOW "\e[1;33m"
#define PURPLE "\e[0;35m"
#define BOLDPURPLE "\e[1;35m"
#define BLUE "\e[0;34m"
#define BOLDBLUE "\e[1;34m"
#define CYAN "\e[0;36m"
#define BOLDCYAN "\e[1;36m"
#define RESET "\e[0;0m"

namespace keyfrog {
    TermCode::TermCode(Codes code) {
        switch(code) {
            case red:
                m_code = RED;
                break;
            case boldRed:
                m_code = BOLDRED;
                break;
            case green:
                m_code = GREEN;
                break;
            case boldGreen:
                m_code = BOLDGREEN;
                break;
            case yellow:
                m_code = YELLOW;
                break;
            case boldYellow:
                m_code = BOLDYELLOW;
                break;
            case purple:
                m_code = PURPLE;
                break;
            case boldPurple:
                m_code = BOLDPURPLE;
                break;
            case blue:
                m_code = BLUE;
                break;
            case boldBlue:
                m_code = BOLDBLUE;
                break;
            case cyan:
                m_code = CYAN;
                break;
            case boldCyan:
                m_code = BOLDCYAN;
                break;
            case reset:
                m_code = RESET;
                break;
            case null:
                m_code = "";
                break;
        }
    }

    const char *TermCode::ccode() {
        return m_code.c_str();
    }

    TermCode::~TermCode() {
    }

    namespace TermCodes {
        TermCode red(TermCode::red);
        TermCode boldRed(TermCode::boldRed);
        TermCode green(TermCode::green);
        TermCode boldGreen(TermCode::boldGreen);
        TermCode blue(TermCode::blue);
        TermCode boldBlue(TermCode::boldBlue);
        TermCode yellow(TermCode::yellow);
        TermCode boldYellow(TermCode::boldYellow);
        TermCode cyan(TermCode::cyan);
        TermCode boldCyan(TermCode::boldCyan);
        TermCode reset(TermCode::reset);

        const char * cred = RED;
        const char * cboldRed = BOLDRED;
        const char * cgreen = GREEN;
        const char * cboldGreen = BOLDGREEN;
        const char * cblue = BLUE;
        const char * cboldBlue = BOLDBLUE;
        const char * cyellow = YELLOW;
        const char * cboldYellow = BOLDYELLOW;
        const char * ccyan = CYAN;
        const char * cboldCyan = BOLDCYAN;
        const char * creset = RESET;

        void nullOutput() {
            red = boldRed = green = boldGreen = 
                blue = boldBlue = yellow = boldYellow =
                cyan = boldCyan = reset = TermCode(TermCode::null);

            cred = cboldRed = cgreen = cboldGreen = 
                cblue = cboldBlue = cyellow = cboldYellow =
                ccyan = cboldCyan = creset = "";
        }
    }
}
