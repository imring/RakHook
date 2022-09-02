include(FetchContent)
include(GNUInstallDirs)

set(CYANIDE_INSTALL ${RAKHOOK_INSTALL})

FetchContent_Declare(
    cyanide
    GIT_REPOSITORY https://github.com/imring/cyanide.git
    GIT_TAG 959fd05d56a779b584e7020b7243ff600799fc46
)
FetchContent_MakeAvailable(cyanide)

add_library(rakhook STATIC)
add_library(rakhook::rakhook ALIAS rakhook)

target_include_directories(rakhook PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(rakhook PUBLIC cxx_std_20)
target_link_libraries(rakhook PUBLIC cyanide::cyanide)

target_sources(rakhook PRIVATE
    "RakHook/offsets.cpp"
    "RakHook/rakhook.cpp"
    "RakHook/samp.cpp"

    "RakNet/BitStream.cpp"
    "RakNet/DS_HuffmanEncodingTree.cpp"
    "RakNet/PluginInterface.cpp"
    "RakNet/StringCompressor.cpp"
)

set_property(TARGET rakhook PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")