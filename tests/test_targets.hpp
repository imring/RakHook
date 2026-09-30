#ifndef RAKHOOK_TESTS_TEST_TARGETS_HPP
#define RAKHOOK_TESTS_TEST_TARGETS_HPP

#ifdef _MSC_VER
#define RAKHOOK_TEST_NOINLINE __declspec(noinline)
#else
#define RAKHOOK_TEST_NOINLINE
#endif

/*
 * In Release the compiler is able to generate a function body that is too
 * short to be detoured (less than a relative jump), so the body is enlarged
 * artificially.
 */
#define RAKHOOK_TEST_ENLARGE()                                             \
    do {                                                                   \
        static volatile unsigned long long rakhook_test_size_enlarger = 5; \
        switch (rakhook_test_size_enlarger) {                              \
        case 1:                                                            \
            rakhook_test_size_enlarger = 0;                                \
            break;                                                         \
        case 2:                                                            \
            rakhook_test_size_enlarger = 4;                                \
            break;                                                         \
        case 3:                                                            \
            rakhook_test_size_enlarger = 5;                                \
            break;                                                         \
        default:                                                           \
            break;                                                         \
        }                                                                  \
    } while (false)

namespace rakhook::tests {
RAKHOOK_TEST_NOINLINE int __cdecl   test_add(int a, int b);
RAKHOOK_TEST_NOINLINE int __cdecl   test_mul(int a, int b);
RAKHOOK_TEST_NOINLINE int __stdcall test_sub(int a, int b);
RAKHOOK_TEST_NOINLINE void __cdecl  test_void(int *out);

struct thiscall_target {
    int value = 0;

    RAKHOOK_TEST_NOINLINE static int __thiscall test_method(thiscall_target *self, int x);
};

using thiscall_method_fn = int(__thiscall *)(thiscall_target *, int);
} // namespace rakhook::tests

#endif // RAKHOOK_TESTS_TEST_TARGETS_HPP
