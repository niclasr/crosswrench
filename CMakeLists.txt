cmake_minimum_required(VERSION 3.12)

project(crosswrench
        VERSION 0.0.0
        DESCRIPTION "Python wheel installer"
        HOMEPAGE_URL "http://github.com/niclasr/crosswrench"
        LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(CrossWrench)

add_executable(crosswrench
               src/main.cpp
               src/license.cpp)

target_include_directories(crosswrench PRIVATE ${CMAKE_BINARY_DIR})

cw_string_literal(cwLicense ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_BINARY_DIR}/license.inc)
cw_string_literal(cwLicenseLibs ${CMAKE_SOURCE_DIR}/LICENSE.libs ${CMAKE_BINARY_DIR}/license-libs.inc)

# add warnings
cw_add_compile_flag(crosswrench -Wall)
cw_add_compile_flag(crosswrench -Wextra)

# libs
include(libs/cxxopts.cmake)

# install
include(GNUInstallDirs)

install(TARGETS crosswrench)
install(FILES LICENSE LICENSE.libs TYPE DOC)