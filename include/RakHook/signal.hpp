#ifndef RAKHOOK_SIGNAL_HPP
#define RAKHOOK_SIGNAL_HPP

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include "RakHook/detail.hpp"

namespace rakhook {
template <typename Func>
class signal {
public:
    signal() = default;

    using function_type = Func;
    using return_type   = detail::function_return_t<function_type>;

    template <typename... Args>
    return_type call(Args &&...args) {
        static_assert(std::is_invocable_v<function_type, Args...>, "Function signature mismatch in signal::call");
        static_assert(std::is_same_v<return_type, void> || std::is_same_v<return_type, bool>, "Return type must be void or bool");

        constexpr bool is_bool = std::is_same_v<return_type, bool>;

        for (auto it = functions_.begin(); it != functions_.end();) {
            if (auto func = *it) {
                if constexpr (is_bool) {
                    if (!func(std::forward<Args>(args)...)) {
                        return false;
                    }
                } else {
                    func(std::forward<Args>(args)...);
                }
                it++;
            } else {
                it = functions_.erase(it);
            }
        }

        if constexpr (is_bool) {
            return true;
        }
    }

    void push_back(std::function<function_type> function) {
        functions_.emplace_back(std::move(function));
    }

    signal &operator+=(std::function<function_type> function) {
        push_back(std::move(function));
        return *this;
    }

private:
    std::vector<std::function<function_type>> functions_;
};
} // namespace rakhook

#endif // RAKHOOK_SIGNAL_HPP
