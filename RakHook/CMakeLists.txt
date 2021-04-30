project(RakHook)
cmake_minimum_required(VERSION 3.10)

add_library(RakHook 
    offsets.cpp
    rakhook.cpp
    hook_shared.hpp
    hooked_rakclient_interface.hpp
    offsets.cpp
    offsets.hpp
    rakhook.cpp
    rakhook.hpp
)
target_link_libraries(RakHook PUBLIC RakNet YAHL)
target_compile_features(RakHook PRIVATE cxx_std_17)