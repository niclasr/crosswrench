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

#include <pystring.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace crosswrench {

namespace {
// helper function
std::string
find_line_in_wheel(std::vector<std::string> &lines, std::string key)
{
    auto find_line = [](std::string v) {
        return [&](std::string l) {
            return pystring::startswith(pystring::strip(l), v);
        };
    };

    auto line =
      std::find_if(lines.begin(), lines.end(), find_line(pystring::lower(key)));
    if (line == lines.end()) {
        throw std::string("WHEEL metadata is malformed, " + key +
                          " is missing");
    }
    std::vector<std::string> split_result;
    pystring::split(*line, split_result, ":");
    if (split_result.size() != 2) {
        throw std::string("WHEEL metadata is malformed, " + key +
                          " value line contains an extra :");
    }

    return pystring::strip(split_result.at(1));
}
} // namespace

wheel::wheel(std::string wheel_meta)
{
    auto wheel_meta_lower = pystring::lower(wheel_meta);
    std::vector<std::string> wheel_lines;
    pystring::splitlines(wheel_meta_lower, wheel_lines);
    std::vector<std::string> split_result;
    std::string value;
    // Wheel-Version
    value = find_line_in_wheel(wheel_lines, "Wheel-Version");
    pystring::split(value, split_result, ".");
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
    value = find_line_in_wheel(wheel_lines, "Root-Is-Purelib");
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

} // namespace crosswrench
