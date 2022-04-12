
include(CheckCXXCompilerFlag)

function(cw_add_compile_flag TARGET VISIBILITY FLAG)
    set(CMAKE_REQUIRED_QUIET ON)
    check_cxx_compiler_flag(${FLAG} CW_COMPILE_FLAG${FLAG})
    if (CW_COMPILE_FLAG${FLAG})
        target_compile_options(${TARGET} ${VISIBILITY} ${FLAG})
    endif()
endfunction()

function(cw_string_literal VARNAME INFILE OUTFILE)
    file(READ ${INFILE} FILE_CONTENT)
    file(WRITE ${OUTFILE} "const std::string ")
    file(APPEND ${OUTFILE} "${VARNAME}")
    file(APPEND ${OUTFILE} " = R\"\"\"\"(")
    file(APPEND ${OUTFILE} "${FILE_CONTENT}")
    file(APPEND ${OUTFILE} ")\"\"\"\";")
endfunction()

function(cw_library NAME)
    set(multiValueArgs SRCS EPKGS ETARGETS)
    cmake_parse_Arguments(PARSE_ARGV 1 CW_LIBRARY
                          "" "" "${multiValueArgs}")

    string(TOUPPER "${NAME}" UPPERNAME)
    option(EXTERNAL_${UPPERNAME} "Use ${NAME} external to the project" ${EXTERNAL_LIBS})

    if (EXTERNAL_${UPPERNAME})
        find_package(${NAME} REQUIRED)
        target_compile_definitions(cw_all_targets INTERFACE EXTERNAL_${UPPERNAME})
        target_link_libraries(cw_all_targets INTERFACE ${NAME}::${NAME})
    else()
        foreach(E_PKG IN LISTS CW_LIBRARY_EPKGS)
            find_package(${E_PKG} REQUIRED)
        endforeach()
        foreach(E_TARGET IN LISTS CW_LIBRARY_ETARGETS)
            target_link_libraries(cw_all_targets INTERFACE ${E_TARGET})
        endforeach()
        target_include_directories(cw_all_targets INTERFACE
                                   ${CMAKE_SOURCE_DIR}/libs/${NAME})
        foreach(SRC_FILE IN LISTS CW_LIBRARY_SRCS)
            target_sources(cw_shared_src PRIVATE
                           ${CMAKE_SOURCE_DIR}/libs/${NAME}/${SRC_FILE})
        endforeach()
    endif()
endfunction()
