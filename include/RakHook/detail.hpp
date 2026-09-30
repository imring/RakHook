#ifndef RAKHOOK_DETAIL_HPP
#define RAKHOOK_DETAIL_HPP

namespace rakhook::detail {
template <typename>
struct function_traits;

template <typename Ret, typename... Args>
struct function_traits<Ret(Args...)> {
    using return_type = Ret;
};

template <typename Ret, typename... Args>
struct function_traits<Ret (*)(Args...)> : function_traits<Ret(Args...)> {};

template <typename Ret, typename... Args>
struct function_traits<Ret(__stdcall *)(Args...)> : function_traits<Ret(Args...)> {};

template <typename Ret, typename... Args>
struct function_traits<Ret(__thiscall *)(Args...)> : function_traits<Ret(Args...)> {};

template <typename Ret, typename... Args>
struct function_traits<Ret(__fastcall *)(Args...)> : function_traits<Ret(Args...)> {};

template <typename Func>
using function_return_t = typename function_traits<Func>::return_type;
} // namespace rakhook::detail

#endif // RAKHOOK_DETAIL_HPP
