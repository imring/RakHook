#ifndef RAKHOOK_WRAPPERS_MINHOOK_HPP
#define RAKHOOK_WRAPPERS_MINHOOK_HPP

#include <bit>
#include <utility>

#include <MinHook.h>

namespace rakhook::wrappers {
struct minhook {
    static bool initialize() {
        if (!initialized_) {
            initialized_ = MH_Initialize() == MH_OK;
        }
        return initialized_;
    }

    static void uninitialize() {
        if (!initialized_) {
            return;
        }
        MH_Uninitialize();
        initialized_ = false;
    }

    template <typename Fn, auto Detour>
    struct adapter;

    template <auto Detour, typename R, typename... Args>
    struct adapter<R(__cdecl *)(Args...), Detour> {
        using fn_t                    = R(__cdecl *)(Args...);
        static inline fn_t trampoline = nullptr;

        static R __cdecl thunk(Args... args) {
            return Detour(trampoline, std::forward<Args>(args)...);
        }
    };

    template <auto Detour, typename R, typename... Args>
    struct adapter<R(__stdcall *)(Args...), Detour> {
        using fn_t                    = R(__stdcall *)(Args...);
        static inline fn_t trampoline = nullptr;

        static R __stdcall thunk(Args... args) {
            return Detour(trampoline, std::forward<Args>(args)...);
        }
    };

    template <auto Detour, typename R, typename This, typename... Args>
    struct adapter<R(__thiscall *)(This, Args...), Detour> {
        using fn_t                    = R(__thiscall *)(This, Args...);
        static inline fn_t trampoline = nullptr;

        static R __fastcall thunk(This self, void *, Args... args) {
            return Detour(trampoline, self, std::forward<Args>(args)...);
        }
    };

    /*
     * MinHook detours have to be declared with the same calling convention as
     * the target, while the hook contract passes the original function as the
     * first argument of a plain function. To bridge this, a static thunk is
     * generated for every supported convention:
     *   - cdecl   - target and detour conventions match, nothing to adapt;
     *   - thiscall - `R(__fastcall *)(This, void *, Args...)` is ABI-compatible
     *     with `R(__thiscall *)(This, Args...)`.
     */
    template <typename Fn, auto Detour>
    struct hook {
        using adapter_t = adapter<Fn, Detour>;
        using fn_t      = typename adapter_t::fn_t;

        hook(fn_t target) : target_{std::bit_cast<void *>(target)} {
            created_ = MH_CreateHook(target_, std::bit_cast<void *>(&adapter_t::thunk), std::bit_cast<void **>(&adapter_t::trampoline)) == MH_OK;
        }

        ~hook() {
            if (created_) {
                MH_RemoveHook(target_);
            }
        }

        bool install() {
            if (!created_ || enabled_) {
                return false;
            }
            if (MH_EnableHook(target_) != MH_OK) {
                return false;
            }
            enabled_ = true;
            return true;
        }

        bool uninstall() {
            if (!created_ || !enabled_) {
                return false;
            }
            if (MH_DisableHook(target_) != MH_OK) {
                return false;
            }
            enabled_ = false;
            return true;
        }

        fn_t get_original() const {
            return adapter_t::trampoline;
        }

    private:
        void *target_  = nullptr;
        bool  created_ = false;
        bool  enabled_ = false;
    };

private:
    static inline bool initialized_ = false;
};
} // namespace rakhook::wrappers

#endif // RAKHOOK_WRAPPERS_MINHOOK_HPP
