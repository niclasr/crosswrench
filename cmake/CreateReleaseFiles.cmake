#
# Creates an anonymous ustar archive
# and compresses it with different
# compression utilities.
# CMakes tar packers create tar files
# with user and group info in them.
#

execute_process(COMMAND
                bsdtar -c -f ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar
                --no-acl --gid 0 --uid 0 --numeric-owner -o
                ${CPACK_SOURCE_PACKAGE_FILE_NAME}
                WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY})

execute_process(COMMAND gzip -9 -k ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar
                WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY})

execute_process(COMMAND bzip2 -6 -k ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar
                WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY})

execute_process(COMMAND xz -k ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar
                WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY})

execute_process(COMMAND lzip -k ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar
                WORKING_DIRECTORY ${CPACK_TOPLEVEL_DIRECTORY})

set(CPACK_EXTERNAL_BUILT_PACKAGES
    ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.gz
    ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2
    ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.xz
    ${CPACK_TOPLEVEL_DIRECTORY}/${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.lz)
