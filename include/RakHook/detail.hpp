#ifndef RAKHOOK_DETAIL_HPP
#define RAKHOOK_DETAIL_HPP

#include <version>
#include <type_traits>
#ifdef __cpp_lib_to_underlying
#include <utility>
#endif

namespace rakhook::detail {
template <typename Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
#ifdef __cpp_lib_to_underlying
    return std::to_underlying(e);
#else
    return static_cast<std::underlying_type_t<Enum>>(e);
#endif
}
} // namespace rakhook::detail

#endif // RAKHOOK_DETAIL_HPP