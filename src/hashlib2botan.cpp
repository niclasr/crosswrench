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

#include "hashlib2botan.hpp"

#include "config.hpp"
#include "functions.hpp"

#include <botan/hash.h>
#include <pystring.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace crosswrench {

hashlib2botan::hashlib2botan()
  : algorithms_by_strength{ "sha3_512", "sha512", "sm3", "sha256" }
{
#if defined(BOTAN_HAS_SHA2_32)
    conv_table["sha256"] = "SHA-256";
#endif
#if defined(BOTAN_HAS_SHA2_64)
    conv_table["sha384"] = "SHA-384";
    conv_table["sha512"] = "SHA-512";
    conv_table["sha512_256"] = "SHA-512-256";
#endif
#if defined(BOTAN_HAS_SHA3)
    conv_table["sha3_224"] = "SHA-3(224)";
    conv_table["sha3_256"] = "SHA-3(256)";
    conv_table["sha3_384"] = "SHA-3(384)";
    conv_table["sha3_512"] = "SHA-3(512)";
#endif
#if defined(BOTAN_HAS_SM3)
    conv_table["sm3"] = "SM3";
#endif
#if defined(BOTAN_HAS_WHIRLPOOL)
    conv_table["whirlpool"] = "Whirlpool";
#endif

    std::vector<std::string> algo_g;
    std::string ag_str = config::instance()->get_value("algorithms");
    pystring::split(ag_str, algo_g, ",");

    for (auto &str : algo_g) {
        algorithms_guaranteed.push_back(pystring::strip(str, "{}' "));
    }
}

bool
hashlib2botan::available(std::string hashlib_algo)
{
    return conv_table.count(hashlib_algo) == 1;
}

std::string
hashlib2botan::hashname(std::string hashlib_algo)
{
    if (!available(hashlib_algo)) {
        throw std::string(
          "hashlib2botan::hashname() called with an unavalable algorithm");
    }

    return conv_table[hashlib_algo];
}

std::string
hashlib2botan::strongest_algorithm_hashlib()
{
    if (best_algo.empty()) {
        for (auto &str : algorithms_by_strength) {
            if (available(str) && strvec_contains(algorithms_guaranteed, str)) {
                best_algo = str;
                break;
            }
        }
    }

    return best_algo;
}

std::string
hashlib2botan::strongest_algorithm_botan()
{
    return hashname(strongest_algorithm_hashlib());
}

void
hashlib2botan::print_guaranteed()
{
    for (auto &str : algorithms_guaranteed) {
        std::cout << str << std::endl;
    }
}

} // namespace crosswrench
