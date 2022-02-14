option(EXTERNAL_LIBZIPPP "Use libzippp external to the project" OFF)

if (EXTERNAL_LIBZIPP)
    find_package(libzippp REQUIRED)
    target_link_libraries(crosswrench PRIVATE libzippp::libzippp)
else()
    find_package(libzip REQUIRED)
    target_link_libraries(crosswrench PRIVATE libzip::zip)
    target_include_directories(crosswrench PRIVATE
                               ${CMAKE_SOURCE_DIR}/libs/libzippp)
    target_sources(crosswrench PRIVATE libs/libzippp/libzippp.cpp)
endif()
