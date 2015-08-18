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

#include <boost/program_options.hpp>
#include "Daemon.h"
#include "Debug.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Shell.h>

#include <cstdlib>
#include <exception>
#include <iostream>

using namespace std;
using namespace keyfrog;
namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "display help message")
        ("nb", "don't run in background")
        ("display", po::value<string>(), "X display name, eg. :0")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        return EXIT_SUCCESS;
    }

    bool daemonMode = true;
    if (vm.count("nb")) {
        daemonMode = false;
    }

    Daemon daemon(daemonMode);
    if(vm.count("display")) {
        // FIXME
        daemon.connectXserver(vm["display"].as<string>());
    }
    daemon.run();

    return EXIT_SUCCESS;    
}
