
include(CheckCXXCompilerFlag)

function(cw_add_compile_flag TARGET FLAG)
    set(CMAKE_REQUIRED_QUIET ON)
    check_cxx_compiler_flag(${FLAG} CW_COMPILE_FLAG${FLAG})
    if (CW_COMPILE_FLAG${FLAG})
        target_compile_options(${TARGET} PRIVATE ${FLAG})
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