include(FetchContent)
include(GNUInstallDirs)

add_library(rakhook_raknet STATIC)
add_library(rakhook::raknet ALIAS rakhook_raknet)

target_include_directories(rakhook_raknet PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(rakhook_raknet PUBLIC cxx_std_20)

target_sources(rakhook_raknet PRIVATE
    "RakNet/BitStream.cpp"
    "RakNet/DS_HuffmanEncodingTree.cpp"
    "RakNet/PluginInterface.cpp"
    "RakNet/StringCompressor.cpp"
)

add_library(rakhook INTERFACE)
add_library(rakhook::rakhook ALIAS rakhook)

target_include_directories(rakhook INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(rakhook INTERFACE cxx_std_20)
target_link_libraries(rakhook INTERFACE rakhook_raknet)

if (RAKHOOK_WRAPPER_CYANIDE AND RAKHOOK_WRAPPER_KTHOOK)
    message(FATAL_ERROR
        "RAKHOOK_WRAPPER_CYANIDE and RAKHOOK_WRAPPER_KTHOOK cannot be enabled "
        "at the same time: both of them provide the xbyak target")
endif ()

if (RAKHOOK_WRAPPER_CYANIDE)
    set(CYANIDE_INSTALL ${RAKHOOK_INSTALL})

    FetchContent_Declare(
        cyanide
        GIT_REPOSITORY https://github.com/imring/cyanide.git
        GIT_TAG 959fd05d56a779b584e7020b7243ff600799fc46
    )
    FetchContent_MakeAvailable(cyanide)

    add_library(rakhook_cyanide INTERFACE)
    add_library(rakhook::cyanide ALIAS rakhook_cyanide)
    target_link_libraries(rakhook_cyanide INTERFACE rakhook cyanide::cyanide)
endif ()

if (RAKHOOK_WRAPPER_MINHOOK)
    FetchContent_Declare(
        minhook
        GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
        GIT_TAG v1.3.4
    )
    FetchContent_MakeAvailable(minhook)

    add_library(rakhook_minhook INTERFACE)
    add_library(rakhook::minhook ALIAS rakhook_minhook)
    target_link_libraries(rakhook_minhook INTERFACE rakhook minhook)
endif ()

if (RAKHOOK_WRAPPER_KTHOOK)
    FetchContent_Declare(
        kthook
        GIT_REPOSITORY https://github.com/kin4stat/kthook.git
        GIT_TAG 4331456543f408c42122ced22144cfb698031266
        GIT_SUBMODULES "ktsignal;xbyak"
    )
    FetchContent_MakeAvailable(kthook)

    add_library(rakhook_kthook INTERFACE)
    add_library(rakhook::kthook ALIAS rakhook_kthook)
    target_link_libraries(rakhook_kthook INTERFACE rakhook kthook)
endif ()
