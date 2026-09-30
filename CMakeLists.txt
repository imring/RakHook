cmake_minimum_required(VERSION 3.20)
project(RakHook
    VERSION 1.0
    LANGUAGES CXX C
)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(RAKHOOK_MASTER_PROJECT ON)
endif ()

option(RAKHOOK_INSTALL "Install targets" ${RAKHOOK_MASTER_PROJECT})
option(RAKHOOK_EXAMPLE "Build an example" ${RAKHOOK_MASTER_PROJECT})
option(RAKHOOK_TESTS "Build unit tests for hook wrappers" ${RAKHOOK_MASTER_PROJECT})

option(RAKHOOK_WRAPPER_CYANIDE "Build cyanide hook wrapper" OFF)
option(RAKHOOK_WRAPPER_MINHOOK "Build MinHook hook wrapper" OFF)
option(RAKHOOK_WRAPPER_KTHOOK "Build kthook hook wrapper" ${RAKHOOK_MASTER_PROJECT})

add_subdirectory(src)

if (RAKHOOK_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(
        TARGETS rakhook_raknet rakhook
        EXPORT ${PROJECT_NAME}Targets

        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        DIRECTORY include/RakHook include/RakNet
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    set(config_file ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake)
    set(version_file ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
    set(config_install_destination lib/cmake/${PROJECT_NAME})

    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
        ${config_file}
        INSTALL_DESTINATION ${config_install_destination}
    )

    write_basic_package_version_file(
        ${version_file}
        COMPATIBILITY SameMajorVersion
    )

    install(
        FILES ${config_file} ${version_file}
        DESTINATION ${config_install_destination}
    )

    install(
        EXPORT ${PROJECT_NAME}Targets
        NAMESPACE rakhook::
        DESTINATION ${config_install_destination}
    )
endif ()

if (RAKHOOK_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (RAKHOOK_EXAMPLE)
    if (NOT RAKHOOK_WRAPPER_CYANIDE AND NOT RAKHOOK_WRAPPER_MINHOOK AND NOT RAKHOOK_WRAPPER_KTHOOK)
        message(FATAL_ERROR "RAKHOOK_EXAMPLE requires at least one of the hook wrappers to be enabled")
    endif ()
    add_subdirectory(example)
endif ()