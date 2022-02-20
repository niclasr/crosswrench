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

#include <pstream.h>
#include <pystring.h>
#include <unistd.h>

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

const std::string pcode_start =
  " -c \"import sysconfig; print(sysconfig.get_path('";
const std::string pcode_end = "'));\"";
const std::array<std::string, 8> python_paths{ "data",        "include",
                                               "platinclude", "platlib",
                                               "platstdlib",  "purelib",
                                               "scripts",     "stdlib" };

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
    std::vector<std::string> config_opts{ "destdir",
                                          "installer",
                                          "prefix",
                                          "python",
                                          "wheel" };
    new_db.clear();

    if (!verify_python_interpreter(pr)) {
        return false;
    }

    for (auto opt : config_opts) {
        new_db[opt] = pr[opt].as<std::string>();
    }

    for (auto path : python_paths) {
        if (!set_python_value(path, pr)) {
            return false;
        }
    }

    std::swap(db, new_db);
    return true;
}

bool
config::set_python_value(std::string var, cxxopts::ParseResult &pr)
{

    std::string output;
    std::string errors;
    std::string cmd = pr["python"].as<std::string>();
    cmd += pcode_start;
    cmd += var;
    cmd += pcode_end;

    if (!get_cmd_output(cmd, output)) {
        return false;
    }

    new_db[var] = pystring::strip(output);

    return true;
}

bool
config::get_cmd_output(std::string &cmd, std::string &output)
{
    unsigned int exit_rounds = 0;
    redi::ipstream in(cmd);
    std::getline(in.out(), output);
    in.close();

    while (!in.rdbuf()->exited()) {
        if (exit_rounds > 4) {
            if (in.rdbuf()->kill() == nullptr) {
                in.rdbuf()->kill(SIGKILL);
            }
            std::cerr << "the command: " << in.command()
                      << " did not finish in time" << std::endl;
            return false;
        }
        usleep(250000);
        exit_rounds++;
    }

    if (in.rdbuf()->status() != 0) {
        std::cerr << "the command: " << in.command() << " did not exit with 0"
                  << std::endl;
        return false;
    }

    return true;
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
    std::string output;
    std::string cmd = pr["python"].as<std::string>();
    cmd += " --version";
    if (get_cmd_output(cmd, output)) {
        if (pystring::startswith(pystring::lower(output), "python ")) {
            return true;
        }
    }
    std::cerr << pr["python"].as<std::string>()
              << " is not a valid python interpreter" << std::endl;

    return false;
}

} // namespace crosswrench
