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
#include "hashlib2botan.hpp"
#include "record.hpp"
#include "wheel.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("hashlib2botan", "[hashlib2botan]")
{
    crosswrench::hashlib2botan h2b{};

    REQUIRE_FALSE(h2b.available("fsasa"));
    CHECK(h2b.available("sha256"));
    REQUIRE_THROWS_AS(h2b.hashname("fsasa"), std::string);
    CHECK_NOTHROW(h2b.hashname("sha256"));
}

TEST_CASE("isbase64nopad", "[isbase64nopad]")
{
    REQUIRE_FALSE(crosswrench::isbase64nopad("="));
    REQUIRE(crosswrench::isbase64nopad(
      "47DEQpj8HBSa+/TImW+5JCeuQeRkm5NMpJWZG3hSuFU"));
}

TEST_CASE("isversionnumber", "[isversionnumber]")
{
    REQUIRE_FALSE(crosswrench::isversionnumber("3de.76"));
    REQUIRE(crosswrench::isversionnumber("1.2.3"));
}

TEST_CASE("iswheelfilenamevalid", "[iswheelfilenamvalid]")
{
    REQUIRE(crosswrench::iswheelfilenamevalid(
      "distribution-1.0-1-py27-none-any.whl"));
    REQUIRE(
      crosswrench::iswheelfilenamevalid("distribution-1.0-py27-none-any.whl"));
    REQUIRE_FALSE(
      crosswrench::iswheelfilenamevalid("distribution-sfe-py27-none-any.whl"));
    REQUIRE_FALSE(crosswrench::iswheelfilenamevalid("wheel.zip"));
    REQUIRE_FALSE(crosswrench::iswheelfilenamevalid("wheel.txt"));
    REQUIRE_FALSE(crosswrench::iswheelfilenamevalid("file.whl1"));
    REQUIRE_FALSE(crosswrench::iswheelfilenamevalid("file.wheel"));
}

TEST_CASE("wheel class", "[wheel]")
{
    REQUIRE_THROWS([&]() {
        std::string wheel_test1{ "not-a-valid-WHEEL: some value" };
        crosswrench::wheel wheel1{ wheel_test1 };
    }());
    REQUIRE_THROWS([&]() {
        std::string wheel_test2{ "Wheel-Version: 1.0\n" };
        crosswrench::wheel wheel2{ wheel_test2 };
    }());
    REQUIRE_THROWS([&]() {
        std::string wheel_test3{ "Wheel-Version: 1.0\nRoot-Is-Purelib: nhfg" };
        crosswrench::wheel wheel3{ wheel_test3 };
    }());
    REQUIRE_THROWS([&]() {
        std::string wheel_test4{
            "Wheel-Version: lhiui\nRoot-Is-Purelib: true"
        };
        crosswrench::wheel wheel4{ wheel_test4 };
    }());
    REQUIRE_NOTHROW([&]() {
        std::string wheel_test5{ "Wheel-Version: 1.0\nRoot-Is-Purelib: true" };
        crosswrench::wheel wheel5{ wheel_test5 };
    }());
    REQUIRE_NOTHROW([&]() {
        std::string wheel_test6{ "Wheel-Version: 1.0\nRoot-Is-Purelib: false" };
        crosswrench::wheel wheel6{ wheel_test6 };
    }());
}

TEST_CASE("record class", "[record]")
{
    std::map<std::string, std::string> sm;
    sm["wheel"] = "wheel-0.37.1-py2.py3-none-any.whl";
    crosswrench::config::instance()->setup(sm);
    REQUIRE_NOTHROW([&]() {
        std::string record_test1{
            "afile,sha256=iujzVdlXafvRsdXC6HMC/09grXvDF0Vl6PhKoHq4kLo,6"
        };
        crosswrench::record record1{ record_test1 };
    }());
    CHECK_NOTHROW([&]() {
        std::string record_test2{ "wheel-0.37.1.dist-info/RECORD,," };
        crosswrench::record record2{ record_test2 };
    }());
}
