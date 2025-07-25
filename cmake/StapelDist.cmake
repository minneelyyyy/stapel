
set(DISTMAN "${CMAKE_BINARY_DIR}/DistManifest.txt")
set(DISTMANIN "${DISTMAN}.in")

function(stapel_dist_init)
    file(REMOVE "${DISTMAN}")
    file(REMOVE "${DISTMANIN}")
endfunction()

function(stapel_dist_generate_manifest)
    file(READ "${DISTMANIN}" content)

    file(
        GENERATE
        OUTPUT "${DISTMAN}"
        CONTENT "${content}"
    )
endfunction()

function(stapel_dist_install_manifest)
    install(
        FILES "${DISTMAN}"
        DESTINATION .
    )
endfunction()

function(stapel_dist_add_file)
    set(oneValueArgs FILE DESTINATION)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    if(NOT DEFINED arg_FILE)
        message(SEND_FATAL "no file specified")
    endif()

    if(NOT DEFINED arg_DESTINATION)
        set(arg_DESTINATION "${arg_FILE}")
    endif()

    file(APPEND "${DISTMANIN}" "${arg_FILE}\t${arg_DESTINATION}\n")
endfunction()

function(stapel_dist_add_target)
    set(oneValueArgs TARGET RENAME DIST_DESTINATION INSTALL_DESTINATION)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    if(NOT DEFINED arg_TARGET)
        message(SEND_FATAL "no target specified")
    endif()

    if(NOT DEFINED arg_INSTALL_DESTINATION)
        message(SEND_FATAL "INSTALL_DESTINATION must be defined")
    endif()

    set(tgt_prefix "$<TARGET_FILE_PREFIX:${arg_TARGET}>")
    set(tgt_suffix "$<TARGET_FILE_SUFFIX:${arg_TARGET}>")
    set(tgt_basename "$<TARGET_FILE_BASE_NAME:${arg_TARGET}>")

    if(NOT DEFINED arg_RENAME)
        set(arg_RENAME "${tgt_basename}")
    endif()

    set(tgt_file "$<TARGET_FILE_NAME:${arg_TARGET}>")
    set(dest_file "${tgt_prefix}${arg_RENAME}${tgt_suffix}")

    if(NOT DEFINED arg_DIST_DESTINATION)
        set(arg_DIST_DESTINATION "${arg_INSTALL_DESTINATION}")
    endif()

    set(tgt_file_path "${arg_INSTALL_DESTINATION}/${tgt_file}")
    set(dest_file_path "${arg_DIST_DESTINATION}/${dest_file}")

    file(APPEND "${DISTMANIN}" "${tgt_file_path}\t${dest_file_path}\n")
endfunction()
