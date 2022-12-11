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

#include <pystring.h>

#include <array>
#include <cstdlib>
#include <iostream>
#include <string>

namespace crosswrench {

namespace {
config *config_instance = nullptr;

void
delete_config_instance()
{
    delete config_instance;
}
} // namespace

const std::string algo_python =
  " -c \"import hashlib; print(hashlib.algorithms_guaranteed);\"";

const std::string framework_python =
  " -c \"import sys; print(sys._framework);\"";

const std::string pcode_start =
  " -c \"import sysconfig;[print(i, sysconfig.get_path(i, '";
const std::string pcode_end = "')) for i in sysconfig.get_path_names()]\"";

const std::array<std::string, 7> python_paths{
    "data", "include", "platlib", "platstdlib", "purelib", "scripts", "stdlib"
};

config::config()
  : dotdatakeydir2config_map{ { "data", "data" },
                              { "headers", "include" },
                              { "platlib", "platlib" },
                              { "purelib", "purelib" },
                              { "scripts", "scripts" } }
{}

config *
config::instance()
{
    if (config_instance == nullptr) {
        config_instance = new config();
        std::atexit(delete_config_instance);
    }

    return config_instance;
}

bool
config::setup(cxxopts::ParseResult &pr)
{
    std::vector<std::string> config_opts{ "destdir",       "installer",
                                          "python",        "script-prefix",
                                          "script-suffix", "wheel" };
    std::vector<std::string> directurl_opts{ "direct-url",
                                             "direct-url-archive" };
    std::vector<std::string> path_opts{ "destdir", "python", "wheel" };
    new_db.clear();

    if (!verify_python_interpreter(pr)) {
        return false;
    }

    for (auto opt : config_opts) {
        if (strvec_contains(path_opts, opt)) {
            new_db[opt] = expandhome(getoptorenv(pr, opt));
        }
        else {
            new_db[opt] = getoptorenv(pr, opt);
        }
    }

    if (pr["verbose"].as<bool>()) {
        new_db["verbose"] = "true";
    }
    else {
        new_db["verbose"] = "false";
    }

    for (auto opt : directurl_opts) {
        if (pr.count(opt)) {
            new_db[opt] = pr[opt].as<std::string>();
        }
        else {
            new_db[opt] = "";
        }
    }

    if (!get_framework(pr)) {
        return false;
    }

    if (!get_python_paths(pr)) {
        return false;
    }

    if (!get_algos(pr)) {
        return false;
    }

    std::swap(db, new_db);
    return true;
}

bool
config::setup(std::map<std::string, std::string> &input)
{
    new_db.clear();
    new_db = input;
    std::swap(db, new_db);
    return true;
}

bool
config::get_python_paths(cxxopts::ParseResult &pr)
{
    std::vector<std::string> output;
    std::string cmd = getoptorenv(pr, "python");
    std::string scheme = getoptorenv(pr, "scheme");

    cmd += pcode_start;
    cmd += get_scheme(scheme);
    cmd += pcode_end;

    if (!get_cmd_output(cmd, output, "")) {
        return false;
    }

    std::vector<std::string> presult;
    std::string sep = " ";
    for (auto line : output) {
        pystring::partition(line, sep, presult);
        if (!presult[1].empty()) {
            new_db[pystring::strip(presult[0])] = pystring::strip(presult[2]);
        }
    }

    bool hasallvars = true;
    for (auto key : python_paths) {
        if (new_db.count(key) == 0) {
            std::cerr << getoptorenv(pr, "python");
            std::cerr << " is missing the path to \"";
            std::cerr << key << "\", config failed";
            std::cerr << std::endl;
            hasallvars = false;
        }
    }

    return hasallvars;
}

std::string
config::get_value(std::string key)
{
    return db.at(key);
}

void
config::print_all()
{
    for (auto i : db) {
        std::cout << i.first << ": " << i.second << std::endl;
    }
}

bool
config::verify_python_interpreter(cxxopts::ParseResult &pr)
{
    std::vector<std::string> output;
    std::string cmd = getoptorenv(pr, "python");
    cmd += " --version";
    if (get_cmd_output(cmd, output, "")) {
        if (pystring::startswith(pystring::lower(output.at(0)), "python ")) {
            return true;
        }
    }
    std::cerr << getoptorenv(pr, "python")
              << " is not a valid python interpreter" << std::endl;

    return false;
}

std::string
config::dotdatakeydir2config(std::string &keydir)
{
    if (dotdatakeydir2config_map.count(keydir) == 0) {
        return std::string{};
    }

    return dotdatakeydir2config_map.at(keydir);
}

bool
config::get_algos(cxxopts::ParseResult &pr)
{
    std::vector<std::string> output;
    std::string cmd = getoptorenv(pr, "python");
    cmd += algo_python;

    if (!get_cmd_output(cmd, output, "")) {
        return false;
    }

    new_db["algorithms"] = pystring::strip(output.at(0));

    return true;
}

bool
config::get_framework(cxxopts::ParseResult &pr)
{
    if (!isosdarwin()) {
        _framework = false;
        return true;
    }

    std::vector<std::string> output;
    std::string cmd;

    cmd += getoptorenv(pr, "python");
    cmd += framework_python;

    if (!get_cmd_output(cmd, output, "")) {
        return false;
    }

    _framework = !output.at(0).empty();

    return true;
}

std::string
config::get_scheme(std::string &key)
{

    if ("user" == key) {
        return _framework ? "osx_framework_user" : "posix_user";
    }
    else if ("prefix" == key) {
        return "posix_prefix";
    }
    else {
        std::cerr << "This should not happen, ";
        std::cerr << "config::get_scheme called with key=";
        std::cerr << key << std::endl;
        std::abort();
    }

    return "";
}

} // namespace crosswrench
