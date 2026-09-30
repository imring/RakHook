set(RAKHOOK_EXAMPLE_WRAPPER "auto" CACHE STRING "Hook wrapper used by the example")
set_property(CACHE RAKHOOK_EXAMPLE_WRAPPER PROPERTY STRINGS "auto" "cyanide" "minhook" "kthook")

if (RAKHOOK_EXAMPLE_WRAPPER STREQUAL "auto")
    if (RAKHOOK_WRAPPER_CYANIDE)
        set(RAKHOOK_EXAMPLE_WRAPPER "cyanide")
    elseif (RAKHOOK_WRAPPER_MINHOOK)
        set(RAKHOOK_EXAMPLE_WRAPPER "minhook")
    elseif (RAKHOOK_WRAPPER_KTHOOK)
        set(RAKHOOK_EXAMPLE_WRAPPER "kthook")
    endif ()
endif ()

string(TOUPPER "${RAKHOOK_EXAMPLE_WRAPPER}" RAKHOOK_EXAMPLE_WRAPPER_UPPER)

if (NOT RAKHOOK_WRAPPER_${RAKHOOK_EXAMPLE_WRAPPER_UPPER})
    message(FATAL_ERROR
        "RAKHOOK_EXAMPLE_WRAPPER is '${RAKHOOK_EXAMPLE_WRAPPER}', "
        "but the RAKHOOK_WRAPPER_${RAKHOOK_EXAMPLE_WRAPPER_UPPER} option is not enabled")
endif ()

add_library(example SHARED)
target_sources(example PRIVATE "main.cpp")
target_link_libraries(example PRIVATE rakhook::rakhook rakhook::${RAKHOOK_EXAMPLE_WRAPPER})
target_compile_definitions(example PRIVATE RAKHOOK_EXAMPLE_WRAPPER_${RAKHOOK_EXAMPLE_WRAPPER_UPPER})
set_target_properties(example PROPERTIES SUFFIX ".asi")
