#ifndef RAKHOOK_WRAPPERS_KTHOOK_HPP
#define RAKHOOK_WRAPPERS_KTHOOK_HPP

#include <bit>
#include <memory>
#include <type_traits>
#include <utility>

#include <kthook/kthook.hpp>

#include "RakHook/detail.hpp"

namespace rakhook::wrappers {
struct kthook {
    static bool initialize() {
        return true;
    }

    static void uninitialize() {}

    template <typename Fn, auto Detour>
    class hook {
        using result_t = detail::function_return_t<Fn>;
        using impl_t   = ::kthook::kthook_simple<Fn>;

    public:
        hook(Fn target) : target_{target} {}

        hook(const hook &)            = delete;
        hook &operator=(const hook &) = delete;

        bool install() {
            if (impl_) {
                return false;
            }

            impl_ = std::make_unique<impl_t>(
                std::bit_cast<std::uintptr_t>(target_),
                [](const impl_t &hk, auto &&...args) -> result_t {
                    if constexpr (std::is_void_v<result_t>) {
                        Detour(hk.get_trampoline(), std::forward<decltype(args)>(args)...);
                    } else {
                        return Detour(hk.get_trampoline(), std::forward<decltype(args)>(args)...);
                    }
                },
                false);

            if (!impl_->install()) {
                impl_.reset();
                return false;
            }
            return true;
        }

        bool uninstall() {
            if (!impl_) {
                return false;
            }
            impl_.reset();
            return true;
        }

        Fn get_original() const {
            if (!impl_) {
                return nullptr;
            }
            return impl_->get_trampoline();
        }

    private:
        Fn                      target_ = nullptr;
        std::unique_ptr<impl_t> impl_;
    };
};
} // namespace rakhook::wrappers

#endif // RAKHOOK_WRAPPERS_KTHOOK_HPP
