option(EXTERNAL_PSTREAMS "Use pstreams external to the project" OFF)

if (EXTERNAL_PSTREAMS)
    find_package(pstreams REQUIRED)
    target_link_libraries(crosswrench PRIVATE pstreams::pstreams)
else()
    target_include_directories(crosswrench PRIVATE
                               ${CMAKE_SOURCE_DIR}/libs/pstreams)
endif()
