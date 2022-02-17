
#include "wheel.hpp"

#include <cstdlib>
#include <iostream>

namespace crosswrench {

int
execute()
{
    // testing exceptions, just for testing will be removed
    // std::string wheel_test{ "not-a-valid-WHEEL: some value" };
    // std::string wheel_test{ "Wheel-Version: 1.0\n" };
    // std::string wheel_test{ "Wheel-Version: 1.0\nRoot-Is-Purelib: nhfg" };
    // std::string wheel_test{ "Wheel-Version: lhiui\nRoot-Is-Purelib: true" };
    std::string wheel_test{ "Wheel-Version: 1.0\nRoot-Is-Purelib: true" };

    try {
        wheel wheel_obj{ wheel_test };
    }
    catch (std::string s) {
        std::cerr << s << std::endl;
        return EXIT_FAILURE;
    }

    // the main runner of the program (to be witten)
    return EXIT_SUCCESS;
}

} // namespace crosswrench
