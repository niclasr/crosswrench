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

#include "functions.hpp"

#include "config.hpp"

#include <pystring.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <vector>

namespace crosswrench {

namespace {
std::string
distdashversion()
{
    std::vector<std::string> result;
    std::string wheelarg = config::instance()->get_value("wheel");
    std::filesystem::path wheelpath{ wheelarg };
    std::string wheelname = wheelpath.filename().string();
    pystring::split(wheelname, result, "-");
    result.erase(result.begin() + 2, result.end());

    return pystring::join("-", result);
}

} // namespace

std::string
dotdistinfodir()
{
    return distdashversion() + ".dist-info";
}

std::string
dotdatadir()
{
    return distdashversion() + ".data";
}

bool
isbase64nopad(const std::string &str)
{
    auto pred = [](unsigned char c) {
        return (std::isalpha(c) || std::isdigit(c) || c == '+' || c == '/');
    };

    return std::all_of(str.cbegin(), str.cend(), pred);
}
} // namespace crosswrench
