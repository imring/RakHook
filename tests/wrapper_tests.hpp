#ifndef RAKHOOK_TESTS_WRAPPER_TESTS_HPP
#define RAKHOOK_TESTS_WRAPPER_TESTS_HPP

#include <catch2/catch_test_macros.hpp>

#include "RakHook/rakhook.hpp"

#include "test_targets.hpp"

namespace rakhook::tests {
struct cdecl_probe {
    static inline int calls  = 0;
    static inline int last_a = 0;
    static inline int last_b = 0;
};

inline int __cdecl detour_add(int(__cdecl *orig)(int, int), int a, int b) {
    ++cdecl_probe::calls;
    cdecl_probe::last_a = a;
    cdecl_probe::last_b = b;
    return orig(a, b) + 10;
}

inline int __cdecl detour_mul(int(__cdecl *orig)(int, int), int a, int b) {
    return orig(a, b) + 100;
}

struct thiscall_probe {
    static inline int              calls     = 0;
    static inline thiscall_target *last_self = nullptr;
    static inline int              last_x    = 0;
};

inline int __cdecl detour_method(thiscall_method_fn orig, thiscall_target *self, int x) {
    ++thiscall_probe::calls;
    thiscall_probe::last_self = self;
    thiscall_probe::last_x    = x;
    return orig(self, x) + self->value;
}

struct stdcall_probe {
    static inline int calls = 0;
};

inline int __cdecl detour_sub(int(__stdcall *orig)(int, int), int a, int b) {
    ++stdcall_probe::calls;
    return orig(a, b) + 1000;
}

struct void_probe {
    static inline int calls = 0;
};

inline void __cdecl detour_void(void(__cdecl *orig)(int *), int *out) {
    ++void_probe::calls;
    *out = 100;
    orig(out);
}

template <typename Backend>
void run_wrapper_tests() {
    REQUIRE(Backend::initialize());

    SECTION("cdecl: arguments, return value and original function") {
        using fn_t   = int(__cdecl *)(int, int);
        using hook_t = typename Backend::template hook<fn_t, &detour_add>;

        cdecl_probe::calls = 0;

        REQUIRE(test_add(2, 3) == 5);

        hook_t hook{&test_add};
        REQUIRE(hook.install());
        REQUIRE(hook.get_original()(2, 3) == 5);

        REQUIRE(test_add(2, 3) == 15);
        REQUIRE(cdecl_probe::calls == 1);
        REQUIRE(cdecl_probe::last_a == 2);
        REQUIRE(cdecl_probe::last_b == 3);

        REQUIRE(hook.uninstall());
        REQUIRE(test_add(2, 3) == 5);
        REQUIRE(cdecl_probe::calls == 1);
    }

    SECTION("cdecl: hook can be installed again after uninstall") {
        using fn_t   = int(__cdecl *)(int, int);
        using hook_t = typename Backend::template hook<fn_t, &detour_add>;

        hook_t hook{&test_add};

        REQUIRE(hook.install());
        REQUIRE(test_add(2, 3) == 15);
        REQUIRE(hook.uninstall());
        REQUIRE(test_add(2, 3) == 5);

        REQUIRE(hook.install());
        REQUIRE(test_add(2, 3) == 15);
        REQUIRE(hook.uninstall());
        REQUIRE(test_add(2, 3) == 5);
    }

    SECTION("stdcall: arguments, return value and original function") {
        using fn_t   = int(__stdcall *)(int, int);
        using hook_t = typename Backend::template hook<fn_t, &detour_sub>;

        stdcall_probe::calls = 0;

        REQUIRE(test_sub(7, 3) == 4);

        hook_t hook{&test_sub};
        REQUIRE(hook.install());
        REQUIRE(hook.get_original()(7, 3) == 4);

        REQUIRE(test_sub(7, 3) == 1004);
        REQUIRE(stdcall_probe::calls == 1);

        REQUIRE(hook.uninstall());
        REQUIRE(test_sub(7, 3) == 4);
    }

    SECTION("thiscall: this pointer, arguments and return value") {
        using hook_t = typename Backend::template hook<thiscall_method_fn, &detour_method>;

        thiscall_target target{4};
        thiscall_probe::calls = 0;

        REQUIRE(thiscall_target::test_method(&target, 3) == 7);

        hook_t hook{&thiscall_target::test_method};
        REQUIRE(hook.install());

        REQUIRE(thiscall_target::test_method(&target, 3) == 11);
        REQUIRE(thiscall_probe::calls == 1);
        REQUIRE(thiscall_probe::last_self == &target);
        REQUIRE(thiscall_probe::last_x == 3);
        REQUIRE(hook.get_original()(&target, 3) == 7);

        REQUIRE(hook.uninstall());
        REQUIRE(thiscall_target::test_method(&target, 3) == 7);
    }

    SECTION("two hooks on different targets at the same time") {
        using fn_t       = int(__cdecl *)(int, int);
        using add_hook_t = typename Backend::template hook<fn_t, &detour_add>;
        using mul_hook_t = typename Backend::template hook<fn_t, &detour_mul>;

        add_hook_t add_hook{&test_add};
        mul_hook_t mul_hook{&test_mul};

        REQUIRE(add_hook.install());
        REQUIRE(mul_hook.install());

        REQUIRE(test_add(2, 3) == 15);
        REQUIRE(test_mul(2, 3) == 106);

        REQUIRE(add_hook.uninstall());
        REQUIRE(test_add(2, 3) == 5);
        REQUIRE(test_mul(2, 3) == 106);

        REQUIRE(mul_hook.uninstall());
        REQUIRE(test_mul(2, 3) == 6);
    }

    SECTION("void return value") {
        using fn_t   = void(__cdecl *)(int *);
        using hook_t = typename Backend::template hook<fn_t, &detour_void>;

        void_probe::calls = 0;
        int out           = 1;

        hook_t hook{&test_void};
        REQUIRE(hook.install());

        test_void(&out);
        REQUIRE(out == 101);
        REQUIRE(void_probe::calls == 1);

        REQUIRE(hook.uninstall());

        out = 1;
        test_void(&out);
        REQUIRE(out == 2);
    }

    SECTION("hooker<Backend> is usable and does nothing without samp.dll") {
        using hooker_t = rakhook::hooker<Backend>;

        static bool called = false;
        called             = false;

        hooker_t::on_send_packet += [](RakNet::BitStream *, PacketPriority &, PacketReliability &, char &) -> bool {
            called = true;
            return true;
        };

        RakNet::BitStream bs;
        PacketPriority    priority    = HIGH_PRIORITY;
        PacketReliability reliability = RELIABLE;
        char              channel     = 0;

        REQUIRE(hooker_t::on_send_packet.call(&bs, priority, reliability, channel));
        REQUIRE(called);

        REQUIRE_FALSE(hooker_t::initialize());
        REQUIRE_FALSE(hooker_t::initialized);
    }

    Backend::uninitialize();
}
} // namespace rakhook::tests

#endif // RAKHOOK_TESTS_WRAPPER_TESTS_HPP
