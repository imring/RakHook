#include "RakHook/wrappers/minhook.hpp"

#include "wrapper_tests.hpp"

TEST_CASE("minhook hook wrapper", "[wrapper][minhook]") {
    rakhook::tests::run_wrapper_tests<rakhook::wrappers::minhook>();
}

TEST_CASE("minhook reports failure on a non-executable target", "[wrapper][minhook]") {
    using fn_t   = int(__cdecl *)(int, int);
    using hook_t = rakhook::wrappers::minhook::hook<fn_t, &rakhook::tests::detour_add>;

    static int not_executable = 0;

    hook_t hook{reinterpret_cast<fn_t>(&not_executable)};
    REQUIRE_FALSE(hook.install());
}
