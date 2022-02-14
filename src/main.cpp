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
#include "execute.hpp"
#include "license.hpp"

#include <cxxopts.hpp>

#include <cstdlib>
#include <iostream>
#include <list>
#include <string>
#include <vector>

bool check_options(cxxopts::ParseResult &pr);

int
main(int argc, char *argv[])
{
    cxxopts::Options options("crosswrench", "Python wheel installer");

    // clang-format off
    options.add_options()
        ("destdir", "destination root",
          cxxopts::value<std::string>()->implicit_value(""))
        ("prefix", "destination prefix",
          cxxopts::value<std::string>()->implicit_value(""))
        ("python", "path to python interpreter",
          cxxopts::value<std::string>()->implicit_value(""))
        ("wheel", "path to wheel file",
          cxxopts::value<std::string>()->implicit_value(""))
        ("license", "show license")
        ("license-libs", "show licenses of libraries used by crosswrench")
        ("help", "show help");
    // clang-format on

    options.allow_unrecognised_options();
    auto result = options.parse(argc, argv);

    if ((argc == 2 && result.count("help")) || argc == 1) {
        std::cout << options.help() << std::endl;
        return EXIT_SUCCESS;
    }

    if (argc == 2 && result.count("license")) {
        crosswrench::show_license();
        return EXIT_SUCCESS;
    }

    if (argc == 2 && result.count("license-libs")) {
        crosswrench::show_license_libs();
        return EXIT_SUCCESS;
    }

    if (!check_options(result)) {
        std::cerr << "use --help to see valid options" << std::endl;
        return EXIT_FAILURE;
    }

    if (!crosswrench::config::instance()->setup(result)) {
        return EXIT_FAILURE;
    }

    return crosswrench::execute();
}

bool
check_options(cxxopts::ParseResult &pr)
{
    bool areAllOptionsValid = true;

    std::list<std::string> unmatched_opts;
    for (auto opt : pr.unmatched()) {
        unmatched_opts.emplace_back(opt);
    }
    unmatched_opts.sort();
    unmatched_opts.unique();
    for (auto opt : unmatched_opts) {
        std::cerr << opt << " is an unrecognized option" << std::endl;
        areAllOptionsValid = false;
    }

    for (auto opt : pr.arguments()) {
        if (pr.count(opt.key()) != 1) {
            std::cerr << "--" << opt.key() << " must be used only once"
                      << std::endl;
            areAllOptionsValid = false;
        }
    }

    std::vector<std::string> run_opts{ "destdir", "prefix", "wheel", "python" };
    bool has_run_opts = false;
    for (auto opt : run_opts) {
        if (pr.count(opt)) {
            if (pr.count(opt) == 1 && pr[opt].as<std::string>() == "") {
                std::cerr << "--" << opt << " must be given an value"
                          << std::endl;
                areAllOptionsValid = false;
            }
            has_run_opts = true;
        }
    }
    if (has_run_opts) {
        for (auto opt : run_opts) {
            if (pr.count(opt) == 0) {
                std::cerr << "--" << opt << " must be provided" << std::endl;
                areAllOptionsValid = false;
            }
        }
    }
    std::vector<std::string> lone_options{ "help", "license", "license-libs" };
    for (auto opt : lone_options) {
        if (pr.count(opt)) {
            std::cerr << "--" << opt << " must be used alone" << std::endl;
        }
    }

    return areAllOptionsValid;
}
