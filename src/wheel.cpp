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

#include "wheel.hpp"

#include "config.hpp"

#include <pystring.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace crosswrench {

namespace {
const unsigned long SupportedWheelVersionMajor = 1;
const unsigned long SupportedWheelVersionMinor = 0;

// helper function
uint8_t mandatory = 0b1;
uint8_t only_once = 0b10;

void
find_values_in_wheel(std::vector<std::string> &lines,
                     std::vector<std::string> &result,
                     std::string key,
                     uint8_t flags)
{
    result.clear();
    auto key_l = pystring::lower(key);

    for (auto &line : lines) {
        if (pystring::startswith(pystring::strip(line), key_l)) {
            std::vector<std::string> split_result;
            pystring::split(line, split_result, ":");
            if (split_result.size() != 2) {
                throw std::string("WHEEL metadata is malformed, " + key +
                                  " value line contains an extra :");
            }
            result.push_back(pystring::strip(split_result.at(1)));
        }
    }

    if ((flags & mandatory) && (result.size() == 0)) {
        throw std::string("WHEEL metadata is malformed, " + key +
                          " is missing");
    }

    if ((flags & only_once) && (result.size() > 1)) {
        throw std::string("WHEEL metadata is malformed, " + key +
                          " is allowed only once");
    }
}
} // namespace

wheel::wheel(std::string wheel_meta)
{
    std::vector<std::string> wheel_lines;
    std::vector<std::string> values;
    std::vector<std::string> split_result;

    auto wheel_meta_lower = pystring::lower(wheel_meta);
    pystring::splitlines(wheel_meta_lower, wheel_lines);

    // Wheel-Version
    find_values_in_wheel(wheel_lines,
                         values,
                         "Wheel-Version",
                         mandatory | only_once);
    pystring::split(values.at(0), split_result, ".");
    if (split_result.size() == 2) {
        if (!pystring::isdigit(split_result.at(0)) ||
            !pystring::isdigit(split_result.at(1)))
        {
            throw std::string("WHEEL metadata is malformed, Wheel-Version "
                              "value line format invalid");
        }
    }
    else {
        throw std::string("WHEEL metadata is malformed, Wheel-Version value "
                          "line format invalid");
    }
    WheelVersionMajor = std::stoul(split_result.at(0));
    WheelVersionMinor = std::stoul(split_result.at(1));

    // Root-Is-Purelib
    find_values_in_wheel(wheel_lines,
                         values,
                         "Root-Is-Purelib",
                         mandatory | only_once);
    auto value = values.at(0);
    if (!(value == "true" || value == "false")) {
        throw std::string("WHEEL metadata is malformed, Root-Is-Purelib value "
                          "line format invalid");
    }
    RootIsPurelib = (value == "true");
}

bool
wheel::root_is_purelib()
{
    return RootIsPurelib;
}

unsigned long
wheel::version_major()
{
    return WheelVersionMajor;
}

unsigned long
wheel::version_minor()
{
    return WheelVersionMinor;
}

bool
wheel::wheel_version_unsupported()
{
    if (WheelVersionMajor > SupportedWheelVersionMajor) {
        std::cerr << config::instance()->get_value("wheel") << " Wheel version "
                  << WheelVersionMajor << "." << WheelVersionMinor
                  << " is to new to be supported by this version of crosswrench"
                  << std::endl;
        return true;
    }

    if (WheelVersionMinor > SupportedWheelVersionMinor) {
        std::cout << config::instance()->get_value("wheel") << " Wheel version "
                  << WheelVersionMajor << "." << WheelVersionMinor
                  << "is greater than crosswrench supported Whell version "
                  << SupportedWheelVersionMajor << "."
                  << SupportedWheelVersionMinor << " the wheel file might not"
                  << " be installed correctly" << std::endl;
    }

    return false;
}

} // namespace crosswrench
