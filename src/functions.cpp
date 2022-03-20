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

#include <libzippp.h>
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
isbase64urlsafenopad(const std::string &str)
{
    auto pred = [](unsigned char c) {
        return (std::isalpha(c) || std::isdigit(c) || c == '-' || c == '_');
    };

    return std::all_of(str.cbegin(), str.cend(), pred);
}

bool
isversionnumber(const std::string &str)
{
    std::vector<std::string> splited;
    pystring::split(str, splited, ".");

    return std::all_of(splited.begin(), splited.end(), pystring::isdigit);
}

bool
iswheelfilenamevalid(const std::string &filepath)
{
    std::filesystem::path wheelpath{ filepath };
    std::string wheelname = wheelpath.filename().string();

    if (!pystring::endswith(wheelname, ".whl")) {
        return false;
    }

    std::vector<std::string> splited;
    pystring::split(wheelname, splited, "-");

    if (!(splited.size() == 6 || splited.size() == 5)) {
        return false;
    }

    if (!isversionnumber(splited.at(1))) {
        return false;
    }

    return true;
}

bool
miniumdistinfofiles(libzippp::ZipArchive &ar)
{
    std::array<std::string, 3> reqfiles{ "/METADATA", "/RECORD", "/WHEEL" };
    auto pred = [&](std::string &filename) {
        return ar.hasEntry(dotdistinfodir() + filename);
    };
    return std::all_of(reqfiles.begin(), reqfiles.end(), pred);
}

std::string
base64urlsafenopad(std::string b64str)
{
    auto usnpb64 = pystring::rstrip(b64str, "=");
    std::replace(usnpb64.begin(), usnpb64.end(), '+', '-');
    std::replace(usnpb64.begin(), usnpb64.end(), '/', '_');
    return usnpb64;
}

} // namespace crosswrench
