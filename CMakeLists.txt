cmake_minimum_required(VERSION 3.12)

project(crosswrench
        VERSION 0.0.0
        DESCRIPTION "Python wheel installer"
        HOMEPAGE_URL "http://github.com/niclasr/crosswrench"
        LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(CrossWrench)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cw_all_targets INTERFACE)

add_library(cw_shared_src OBJECT
            src/config.cpp
            src/functions.cpp
            src/hashlib2botan.cpp
            src/record.cpp
            src/spread.cpp
            src/wheel.cpp)
target_link_libraries(cw_shared_src cw_all_targets)

add_executable(crosswrench
               src/execute.cpp
               src/main.cpp
               src/license.cpp)

target_include_directories(crosswrench PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(crosswrench cw_all_targets cw_shared_src)

cw_string_literal(cwLicense
                  ${CMAKE_SOURCE_DIR}/LICENSE
                  ${CMAKE_BINARY_DIR}/license.inc)
cw_string_literal(cwLicenseLibs
                  ${CMAKE_SOURCE_DIR}/LICENSE.libs
                  ${CMAKE_BINARY_DIR}/license-libs.inc)

# add warnings
cw_add_compile_flag(cw_all_targets INTERFACE -Wall)
cw_add_compile_flag(cw_all_targets INTERFACE -Wextra)

# libs
option(EXTERNAL_LIBS "Use external libs by default" OFF)
find_package(Boost REQUIRED COMPONENTS "filesystem")
target_link_libraries(cw_all_targets INTERFACE Boost::filesystem)
target_compile_definitions(cw_all_targets INTERFACE BOOST_FILESYSTEM_VERSION=4)
find_package(PkgConfig REQUIRED)
pkg_check_modules(botan-2 REQUIRED IMPORTED_TARGET botan-2)
target_link_libraries(cw_all_targets INTERFACE PkgConfig::botan-2)
cw_library(csv2)
cw_library(cxxopts)
cw_library(libzippp SRCS libzippp.cpp EPKGS libzip ETARGETS libzip::zip)
cw_library(pystring SRCS pystring.cpp)
cw_library(pstreams)

#tests
option(ENABLE_TESTS "Build catch2 test cases" OFF)
if(ENABLE_TESTS)
    find_package(Catch2 REQUIRED)
    add_executable(cwtests src/tests.cpp)
    target_link_libraries(cwtests cw_all_targets)
    target_link_libraries(cwtests cw_shared_src)
    target_link_libraries(cwtests Catch2::Catch2)
    include(CTest)
    include(Catch)
    catch_discover_tests(cwtests)
endif()

# install
include(GNUInstallDirs)

install(TARGETS crosswrench)
install(FILES LICENSE LICENSE.libs TYPE DOC)
install(FILES man/crosswrench.1.html TYPE DOC)
install(FILES man/crosswrench.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/)

# release
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
    ${PROJECT_NAME}-${PROJECT_VERSION})
set(CPACK_SOURCE_IGNORE_FILES
    \\.git/
    .gitignore)
set(CPACK_VERBATIM_VARIABLES ON)
include(CPack)
