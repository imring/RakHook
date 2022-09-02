add_library(example SHARED)
target_sources(example PRIVATE "main.cpp")
target_link_libraries(example PRIVATE rakhook cyanide::cyanide)
set_property(TARGET example PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")