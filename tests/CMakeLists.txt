include(FetchContent)

if (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG cde26de803964197e7d5e988a17a8dad0cd3721e
    )
    FetchContent_MakeAvailable(Catch2)

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)

    add_executable(rakhook_tests_core
        "core.cpp"
    )
    target_link_libraries(rakhook_tests_core PRIVATE
        rakhook::rakhook
        Catch2::Catch2WithMain
    )
    catch_discover_tests(rakhook_tests_core)

    function(rakhook_add_wrapper_tests wrapper)
        set(target rakhook_tests_${wrapper})

        add_executable(${target}
            "common.cpp"
            "${wrapper}.cpp"
        )

        target_link_libraries(${target} PRIVATE
            rakhook::rakhook
            rakhook::${wrapper}
            Catch2::Catch2WithMain
        )

        catch_discover_tests(${target})
    endfunction()

    if (RAKHOOK_WRAPPER_CYANIDE)
        rakhook_add_wrapper_tests(cyanide)
    endif ()

    if (RAKHOOK_WRAPPER_MINHOOK)
        rakhook_add_wrapper_tests(minhook)
    endif ()

    if (RAKHOOK_WRAPPER_KTHOOK)
        rakhook_add_wrapper_tests(kthook)
    endif ()
else ()
    message(STATUS "RakHook wrapper tests are supported only on 32-bit MSVC")
endif ()
