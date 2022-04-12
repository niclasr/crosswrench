
include(CheckCXXSourceCompiles)

find_path(PSTREAMS_INCLUDE_DIR
          NAMES pstream.h
          PATH_SUFFIXES pstreams)


if (PSTREAMS_INCLUDE_DIR)
  set(CMAKE_REQUIRED_QUIET true)
  set(CMAKE_REQUIRED_INCLUDES ${PSTREAMS_INCLUDE_DIR})
  check_cxx_source_compiles("#include <pstream.h>
                             #if PSTREAMS_VERSION < 0x0101
                             #error
                             #endif
                             int main(){return 0;}"
                             PV)
  if(PV)
  add_library(pstreams::pstreams INTERFACE IMPORTED)
  set_target_properties(pstreams::pstreams PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${PSTREAMS_INCLUDE_DIR}"
  )
  else()
    message(FATAL_ERROR "pstreams was found but is of a version below 1.0.1")
  endif()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(pstreams
                                  FOUND_VAR PSTREAMS_FOUND
                                  REQUIRED_VARS PSTREAMS_INCLUDE_DIR)

