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
#include "functions.hpp"
#include "license.hpp"

#include <cxxopts.hpp>

#include <cstdlib>
#include <iostream>
#include <list>
#include <string>
#include <vector>

bool check_options(cxxopts::ParseResult &);

int
main(int argc, char *argv[])
{
    try {
        cxxopts::Options options("crosswrench", "Python wheel installer");

        // clang-format off
        options.add_options()
            ("destdir",
              "destination root" + crosswrench::envdescmsg("destdir"),
              cxxopts::value<std::string>()->implicit_value(""))
            ("direct-url", "url part of direct url",
              cxxopts::value<std::string>()->implicit_value(""))
            ("direct-url-archive", "file to base the direct url hash on",
              cxxopts::value<std::string>()->implicit_value(""))
            ("installer", "installer name",
              cxxopts::value<std::string>()->
              implicit_value("")->
              default_value("crosswrench"))
            ("python",
              "path to python interpreter" + crosswrench::envdescmsg("python"),
              cxxopts::value<std::string>()->implicit_value(""))
            ("script-prefix", "prefix for script filenames",
              cxxopts::value<std::string>()->
              implicit_value("")->
              default_value(""))
            ("script-suffix", "suffix for script filenames",
              cxxopts::value<std::string>()->
              implicit_value("")->
              default_value(""))
            ("scheme", "install scheme (prefix user)",
              cxxopts::value<std::string>()->
              implicit_value("")->
              default_value("prefix"))
            ("verbose", "print files that are installed",
              cxxopts::value<bool>()->default_value("false"))
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
            return (argc == 2 ? EXIT_SUCCESS : EXIT_FAILURE);
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
    }
    catch (cxxopts::argument_incorrect_type &e) {
        std::cerr << e.what() << ", use --help to see valid options"
                  << std::endl;
        return EXIT_FAILURE;
    }
    catch (cxxopts::OptionException &e) {
        std::cerr << e.what() << ", this shuld never happen in crosswrench"
                  << std::endl;
        return EXIT_FAILURE;
    }

    return crosswrench::execute();
}

bool
check_options(cxxopts::ParseResult &pr)
{
    bool areAllOptionsValid = true;

    std::list<std::string> unmatched_opts;
    for (auto &opt : pr.unmatched()) {
        unmatched_opts.emplace_back(opt);
    }
    unmatched_opts.sort();
    unmatched_opts.unique();
    for (auto &opt : unmatched_opts) {
        std::cerr << opt << " is an unrecognized option" << std::endl;
        areAllOptionsValid = false;
    }

    for (auto &opt : pr.arguments()) {
        if (pr.count(opt.key()) != 1) {
            std::cerr << "--" << opt.key() << " must be used only once"
                      << std::endl;
            areAllOptionsValid = false;
        }
    }

    std::vector<std::string> run_opts{ "destdir", "wheel", "python" };
    std::vector<std::string> optional_run_opts{
        "direct-url",    "direct-url-archive", "installer",
        "script-prefix", "script-suffix",      "scheme",
        "verbose"
    };
    std::vector<std::string> direct_url_opts{ "direct-url",
                                              "direct-url-archive" };
    std::vector<std::string> valid_scheme_values{ "prefix", "user" };

    bool has_run_opts = false;
    for (auto &opt : run_opts) {
        if (crosswrench::countoptorenv(pr, opt)) {
            if (crosswrench::countoptorenv(pr, opt) == 1 &&
                crosswrench::getoptorenv(pr, opt) == "")
            {
                std::cerr << "--" << opt << crosswrench::envormsg(opt)
                          << " must be given an value" << std::endl;
                areAllOptionsValid = false;
            }
            has_run_opts = true;
        }
    }

    bool all_directurl_opts_unset = true;
    bool all_directurl_opts_set = true;
    for (auto &opt : direct_url_opts) {
        if (pr.count(opt)) {
            all_directurl_opts_unset = false;
        }
        else {
            all_directurl_opts_set = false;
        }
    }
    if (!(all_directurl_opts_set ^ all_directurl_opts_unset)) {
        std::cerr << "the options ";
        for (auto &opt : direct_url_opts) {
            std::cerr << "--" << opt << " ";
        }
        std::cerr << "can only be used together" << std::endl;
        areAllOptionsValid = false;
    }

    if (has_run_opts) {
        for (auto &opt : run_opts) {
            if (crosswrench::countoptorenv(pr, opt) == 0) {
                std::cerr << "--" << opt << crosswrench::envormsg(opt)
                          << " must be provided" << std::endl;
                areAllOptionsValid = false;
            }
        }
        if (pr.count("installer")) {
            if (pr["installer"].as<std::string>() == "") {
                std::cerr << "--installer must be given a value or not used"
                          << std::endl;
                areAllOptionsValid = false;
            }
        }
        if (pr.count("scheme")) {
            std::string schemearg = pr["scheme"].as<std::string>();
            if (!crosswrench::strvec_contains(valid_scheme_values, schemearg)) {
                std::cerr << "--scheme can only be given the value prefix "
                          << "or user" << std::endl;
                areAllOptionsValid = false;
            }
        }
    }
    else {
        for (auto &opt : optional_run_opts) {
            if (pr.count(opt)) {
                std::cerr << "--" << opt << " can only be used with ";
                for (auto &ropt : run_opts) {
                    std::cerr << "--" << ropt << crosswrench::envormsg(opt)
                              << " ";
                }
                std::cerr << std::endl;
                areAllOptionsValid = false;
            }
        }
    }
    std::vector<std::string> lone_options{ "help", "license", "license-libs" };
    for (auto &opt : lone_options) {
        if (pr.count(opt)) {
            std::cerr << "--" << opt << " must be used alone" << std::endl;
        }
    }

    return areAllOptionsValid;
}
