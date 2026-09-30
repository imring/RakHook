#include "test_targets.hpp"

// The library is header-only, so every test executable links at least two
// translation units that include its headers.
#include "RakHook/rakhook.hpp"

namespace rakhook::tests {
int __cdecl test_add(int a, int b) {
    RAKHOOK_TEST_ENLARGE();
    return a + b;
}

int __cdecl test_mul(int a, int b) {
    RAKHOOK_TEST_ENLARGE();
    return a * b;
}

int __stdcall test_sub(int a, int b) {
    RAKHOOK_TEST_ENLARGE();
    return a - b;
}

void __cdecl test_void(int *out) {
    RAKHOOK_TEST_ENLARGE();
    *out += 1;
}

int __thiscall thiscall_target::test_method(thiscall_target *self, int x) {
    RAKHOOK_TEST_ENLARGE();
    return self->value + x;
}
} // namespace rakhook::tests
