
find_path(PYSTRING_INCLUDE_DIR
          NAMES pystring.h
          PATH_SUFFIXES pystring)

find_library(PYSTRING_LIBRARY
             NAMES pystring)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pystring
                                  FOUND_VAR PYSTRING_FOUND
                                  REQUIRED_VARS
                                  PYSTRING_INCLUDE_DIR
                                  PYSTRING_LIBRARY)

if (PYSTRING_FOUND)
  set(PYSTRING_FOUND true)
  add_library(pystring::pystring UNKNOWN IMPORTED)
  set_target_properties(pystring::pystring PROPERTIES
    IMPORTED_LOCATION "${PYSTRING_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${PYSTRING_INCLUDE_DIR}"
  )
endif()
