project(RakHook)
cmake_minimum_required(VERSION 3.10)

add_library(YAHL Yet-another-hook-library/hde32/src/hde32.cpp)
target_include_directories(YAHL PUBLIC .)
target_compile_features(YAHL PUBLIC cxx_std_17)