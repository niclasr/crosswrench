
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
