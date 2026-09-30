#include "RakHook/wrappers/kthook.hpp"

#include "wrapper_tests.hpp"

TEST_CASE("kthook hook wrapper", "[wrapper][kthook]") {
    rakhook::tests::run_wrapper_tests<rakhook::wrappers::kthook>();
}

TEST_CASE("kthook reports failure on a non-executable target", "[wrapper][kthook]") {
    using fn_t   = int(__cdecl *)(int, int);
    using hook_t = rakhook::wrappers::kthook::hook<fn_t, &rakhook::tests::detour_add>;

    static int not_executable = 0;

    hook_t hook{reinterpret_cast<fn_t>(&not_executable)};
    REQUIRE_FALSE(hook.install());
}
