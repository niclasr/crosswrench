option(EXTERNAL_PYSTRING "Use pystring external to the project" OFF)

if (EXTERNAL_PYSTRING)
    find_package(pystring REQUIRED)
    target_link_libraries(crosswrench PRIVATE pystring::pystring)
else()
    target_include_directories(crosswrench PRIVATE
                               ${CMAKE_SOURCE_DIR}/libs/pystring)
    target_sources(crosswrench PRIVATE libs/pystring/pystring.cpp)
endif()
