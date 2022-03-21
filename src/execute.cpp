/*
Copyright (c) 2022 Niclas Rosenvik

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "config.hpp"
#include "functions.hpp"
#include "record.hpp"
#include "spread.hpp"
#include "wheel.hpp"

#include <cstdlib>
#include <iostream>

namespace crosswrench {

int
execute()
{
    // the main runner of the program (to be finished)

    if (!iswheelfilenamevalid(config::instance()->get_value("wheel"))) {
        std::cerr << config::instance()->get_value("wheel")
                  << " is not a wheelfile based on its filename" << std::endl;
        return EXIT_FAILURE;
    }

    libzippp::ZipArchive wheelfile{ config::instance()->get_value("wheel") };

    if (!wheelfile.open(libzippp::ZipArchive::ReadOnly, true)) {
        std::cerr << config::instance()->get_value("wheel")
                  << " could not be opened or is an invalid wheelfile"
                  << std::endl;
        return EXIT_FAILURE;
    }

    if (!minimumdistinfofiles(wheelfile)) {
        std::cerr
          << config::instance()->get_value("wheel")
          << " is an invalid wheelfile since it is missing required files"
          << std::endl;
        return EXIT_FAILURE;
    }

    try {
        wheel wheel_obj{
            wheelfile.getEntry(dotdistinfodir() + "/WHEEL").readAsText()
        };
        record record_obj{
            wheelfile.getEntry(dotdistinfodir() + "/RECORD").readAsText()
        };

        if (!record_obj.verify(wheelfile)) {
            std::cerr << config::instance()->get_value("wheel")
                      << " is an invalid wheel file since the files failed "
                      << "verification against RECORD" << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << config::instance()->get_value("wheel")
                  << " is a valid wheel file as verified against RECORD"
                  << std::endl;
        std::cout << "installing!" << std::endl;

        spread installer{ wheelfile, wheel_obj.root_is_purelib() };
        installer.install();
    }
    catch (std::string s) {
        std::cerr << s << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

} // namespace crosswrench
