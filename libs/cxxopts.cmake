option(EXTERNAL_CXXOPTS "Use cxxopts external to the project" OFF)

if (EXTERNAL_CXXOPTS)
    find_package(cxxopts REQUIRED)
    target_link_libraries(crosswrench PRIVATE cxxopts::cxxopts)
else()
    target_include_directories(crosswrench PRIVATE
                               ${CMAKE_SOURCE_DIR}/libs/cxxopts)
endif()
