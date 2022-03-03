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

#include "record.hpp"
#include "wheel.hpp"

#include <cstdlib>
#include <iostream>

namespace crosswrench {

int
execute()
{
    // testing exceptions, just for testing will be removed
    // std::string wheel_test{ "not-a-valid-WHEEL: some value" };
    // std::string wheel_test{ "Wheel-Version: 1.0\n" };
    // std::string wheel_test{ "Wheel-Version: 1.0\nRoot-Is-Purelib: nhfg" };
    // std::string wheel_test{ "Wheel-Version: lhiui\nRoot-Is-Purelib: true" };
    std::string wheel_test{ "Wheel-Version: 1.0\nRoot-Is-Purelib: true" };

    std::string record_test{
        "afile,sha256=iujzVdlXafvRsdXC6HMC/09grXvDF0Vl6PhKoHq4kLo,6"
    };
    //std::string record_test{ "wheel-1.0.dist-info/RECORD,," };

    try {
        wheel wheel_obj{ wheel_test };
        record record_obj{ record_test };
    }
    catch (std::string s) {
        std::cerr << s << std::endl;
        return EXIT_FAILURE;
    }
    // end of testing

    // the main runner of the program (to be witten)
    return EXIT_SUCCESS;
}

} // namespace crosswrench
