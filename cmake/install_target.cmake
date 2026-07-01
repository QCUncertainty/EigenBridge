include_guard()

function(install_library lib_name header_dir)
    install(TARGETS ${lib_name}
        EXPORT ${lib_name}Targets
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include
    )

    install(DIRECTORY "${header_dir}"
        DESTINATION include
        FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
    )

    install(EXPORT ${lib_name}Targets
        FILE ${lib_name}Targets.cmake
        NAMESPACE eigensolverapi::
        DESTINATION "lib/cmake/${lib_name}"
    )

    set(config_file "${CMAKE_CURRENT_BINARY_DIR}/${lib_name}Config.cmake")
    file(WRITE "${config_file}" "")
    file(APPEND
        "${config_file}"
        "include(CMakeFindDependencyMacro)

set(CMAKE_MODULE_PATH \${CMAKE_CURRENT_LIST_DIR} \${CMAKE_MODULE_PATH})
find_dependency(LAPACK)
find_dependency(LAPACKE)

include(\"\${CMAKE_CURRENT_LIST_DIR}/${lib_name}Targets.cmake\")
        "
    )
    install(FILES "${config_file}" "cmake/FindLAPACKE.cmake"
        DESTINATION "lib/cmake/${lib_name}"
    )


endfunction()
