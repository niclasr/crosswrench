option(EXTERNAL_CSV2 "Use csv2 external to the project" OFF)

if (EXTERNAL_CSV2)
    find_package(csv2 REQUIRED)
    target_compile_definitions(crossrench PRIVATE EXTERNAL_CSV2)
    target_link_libraries(crosswrench PRIVATE csv2:csv2)
else()
    target_include_directories(crosswrench PRIVATE
                               ${CMAKE_SOURCE_DIR}/libs/csv2)
endif()
