#ifndef RAKHOOK_WRAPPERS_CYANIDE_HPP
#define RAKHOOK_WRAPPERS_CYANIDE_HPP

#include <bit>
#include <memory>

#include <cyanide/hook_impl_polyhook.hpp>

namespace rakhook::wrappers {
struct cyanide {
    static bool initialize() {
        return true;
    }

    static void uninitialize() {}

    template <typename Fn, auto Detour>
    class hook {
        using impl_t = ::cyanide::polyhook_x86<Fn, decltype(Detour)>;

    public:
        hook(Fn target) : target_{target} {}

        hook(const hook &)            = delete;
        hook &operator=(const hook &) = delete;

        bool install() {
            if (impl_) {
                return false;
            }
            impl_ = std::make_unique<impl_t>(Fn{target_}, static_cast<decltype(Detour)>(Detour));
            impl_->install();
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
            return std::bit_cast<Fn>(impl_->get_trampoline());
        }

    private:
        Fn                      target_ = nullptr;
        std::unique_ptr<impl_t> impl_;
    };
};
} // namespace rakhook::wrappers

#endif // RAKHOOK_WRAPPERS_CYANIDE_HPP
