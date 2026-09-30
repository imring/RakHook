#include "RakHook/wrappers/cyanide.hpp"

#include "wrapper_tests.hpp"

TEST_CASE("cyanide hook wrapper", "[wrapper][cyanide]") {
    rakhook::tests::run_wrapper_tests<rakhook::wrappers::cyanide>();
}
