cmake_minimum_required(VERSION 3.12)

project(crosswrench
        VERSION 0.0.0
        DESCRIPTION "Python wheel installer"
        HOMEPAGE_URL "http://github.com/niclasr/crosswrench"
        LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(CrossWrench)

add_executable(crosswrench
               src/config.cpp
               src/dirnames.cpp
               src/execute.cpp
               src/hashlib2botan.cpp
               src/isbase64nopad.cpp
               src/main.cpp
               src/license.cpp
               src/record.cpp
               src/wheel.cpp)

set_target_properties(crosswrench PROPERTIES
                      CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED ON
                      CXX_EXTENSIONS OFF)
target_include_directories(crosswrench PRIVATE ${CMAKE_BINARY_DIR})

cw_string_literal(cwLicense ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_BINARY_DIR}/license.inc)
cw_string_literal(cwLicenseLibs ${CMAKE_SOURCE_DIR}/LICENSE.libs ${CMAKE_BINARY_DIR}/license-libs.inc)

# add warnings
cw_add_compile_flag(crosswrench -Wall)
cw_add_compile_flag(crosswrench -Wextra)

# libs
find_package(PkgConfig REQUIRED)
pkg_check_modules(botan-2 REQUIRED IMPORTED_TARGET botan-2)
target_link_libraries(crosswrench PRIVATE PkgConfig::botan-2)
include(libs/csv2.cmake)
include(libs/cxxopts.cmake)
include(libs/libzippp.cmake)
include(libs/pystring.cmake)
include(libs/pstreams.cmake)

# install
include(GNUInstallDirs)

install(TARGETS crosswrench)
install(FILES LICENSE LICENSE.libs TYPE DOC)
