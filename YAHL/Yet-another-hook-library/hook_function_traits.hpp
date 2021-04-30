#pragma once
#include <tuple>

//STRUCT TEMPLATE get_calling_convention
enum class cc : uint8_t {
	cdecl_,
	stdcall_,
	thiscall_,
	unknown_
};

namespace detail {

template<typename>
struct get_calling_convention_helper {
	static constexpr cc value = cc::unknown_;
};

#define _GET_NON_MEMBER_CALLING_CONVENTION_HELPER_IMPL(CALL_OPT, VAL)			\
	template <typename Ret, typename... Args>									\
	struct get_calling_convention_helper<Ret CALL_OPT(Args...)> {				\
		static constexpr cc value = VAL;										\
	};

_GET_NON_MEMBER_CALLING_CONVENTION_HELPER_IMPL(__cdecl, cc::cdecl_)
_GET_NON_MEMBER_CALLING_CONVENTION_HELPER_IMPL(__stdcall, cc::stdcall_)

#define _GET_MEMBER_CALLING_CONVENTION_HELPER_IMPL(CALL_OPT, VAL)				\
	template <typename Ret, typename Class, typename... Args>					\
	struct get_calling_convention_helper<Ret(CALL_OPT Class::*)(Args...)> {		\
		static constexpr cc value = VAL;										\
	};

_GET_MEMBER_CALLING_CONVENTION_HELPER_IMPL(__cdecl, cc::cdecl_)
_GET_MEMBER_CALLING_CONVENTION_HELPER_IMPL(__stdcall, cc::stdcall_)
_GET_MEMBER_CALLING_CONVENTION_HELPER_IMPL(__thiscall, cc::thiscall_)

} //namespace detail


template<typename Func>
struct get_calling_convention : detail::get_calling_convention_helper<std::remove_pointer_t<Func>>, 
				std::enable_if_t<std::is_function_v<std::remove_pointer_t<Func>> || std::is_member_pointer_v<std::remove_pointer_t<Func>>> {};

template<typename Ret, typename... Args>
struct get_calling_convention<Ret(__thiscall*)(Args...)> { // non-member thiscall function type can be only by pointer
	static constexpr cc value = cc::thiscall_;
};

template<typename Ret, typename Class,  typename... Args>
struct get_calling_convention<Ret(__thiscall Class::*)(Args...)> { 
	static constexpr cc value = cc::thiscall_;
};


template<typename Func>
inline constexpr cc get_calling_convention_v = get_calling_convention<Func>::value;

template<typename Func>
struct move_to_args {
	using type = Func;
};

template<typename Ret, typename Class, typename... Args>
struct move_to_args<Ret(__thiscall Class::*)(Args...)> {
	using type = Ret(__thiscall*)(Class*, Args...);
};

template<typename T>
using move_to_args_t = typename move_to_args<T>::type;


// STRUCT TEMPLATE add_reference_no_ref_collapsing
/*
	convert T to T&, T& to T*&...
*/
template<typename T>
struct add_reference_no_collapsing {
	using type = std::add_lvalue_reference_t<T>;
};

template<typename T>
struct add_reference_no_collapsing<T&> {
	using type = std::add_lvalue_reference_t<std::add_pointer_t<T>>;
};

template<typename T>
using add_reference_no_collapsing_t = typename add_reference_no_collapsing<T>::type;



// STRUCT TEMPLATE add_reference_to_params
/*
	convert Ret(ParamType1, ParamType2, ParamType3&, ...) to Ret(ParamType1&, ParamType2&, ParamType3*&, ...) 
*/
template<typename>
struct add_reference_to_args {};

template<typename Ret, typename... Args>
struct add_reference_to_args<Ret(__cdecl)(Args...)> {
	using type = Ret(__cdecl)(add_reference_no_collapsing_t<Args>...);
};

template<typename Ret, typename... Args>
struct add_reference_to_args<Ret(__stdcall)(Args...)> {
	using type = Ret(__stdcall)(add_reference_no_collapsing_t<Args>...);
};

template<typename Ret, typename... Args>
struct add_reference_to_args<Ret(__thiscall*)(Args...)> {
	using type = Ret(__thiscall*)(add_reference_no_collapsing_t<Args>...);
};

template<typename Func>
using add_reference_to_args_t = typename add_reference_to_args<Func>::type;



//STRUCT TEMPLATE convert_member_to_free_func
template<typename>
struct convert_member_to_free_func {};

template<typename Ret, typename Class, typename... Args>
struct convert_member_to_free_func<Ret(Class::*)(Args...)> {
	using type = Ret(Class*, Args...);
};

template<typename Ret, typename... Args>
struct convert_member_to_free_func<Ret(__thiscall*)(Args...)> {
	using type = Ret(Args...);
};


template<typename Func>
using convert_member_to_free_func_t = typename convert_member_to_free_func<Func>::type;



//STRUCT TEMPLATE add_argument_front
template<typename, typename>
struct add_argument_front {};

template<typename... AdditionalArg, typename Ret, typename... Args>
struct add_argument_front<Ret(Args...), AdditionalArg...> {
	using type = Ret(AdditionalArg..., Args...);
};

template<typename... AdditionalArg, typename Ret, typename... Args>
struct add_argument_front<Ret(*)(Args...), AdditionalArg...> {
	using type = Ret(*)(AdditionalArg..., Args...);
};

template<typename Func, typename... Arg>
using add_argument_front_t = typename add_argument_front<Func, Arg...>::type;



//STRUCT TEMPLATE function_decompose
template<typename>
struct function_decompose {};

template<typename Ret, typename... Args>
struct function_decompose<Ret(__cdecl)(Args...)> {
	using return_value = Ret;
	using arguments = std::tuple<Args...>;
};

template<typename Ret, typename... Args>
struct function_decompose<Ret(__stdcall)(Args...)> {
	using return_value = Ret;
	using arguments = std::tuple<Args...>;
};

template<typename Ret, typename Class, typename... Args>
struct function_decompose<Ret(Class:: *)(Args...)> {
	using return_value = Ret;
	using arguments = std::tuple<Args...>;
	using class_type = Class; //suggest better name :p
};

template<typename Ret, typename... Args>
struct function_decompose<Ret(__thiscall *)(Args...)> {
	using return_value = Ret;
	using arguments = std::tuple<Args...>;
	using class_type = std::remove_pointer_t<std::tuple_element_t<0, arguments>>; 
};

template<typename Func>
using get_function_args_t = typename function_decompose<Func>::arguments;

template<typename Func>
using get_function_return_type_t = typename function_decompose<Func>::return_value;

template<typename Func>
using get_function_class_type_t = typename function_decompose<Func>::class_type;



//STRUCT TEMPLATE convert_to_default_cc
/*
	change calling convention of function type to default used in compiler
*/
template<typename>
struct convert_to_default_cc {};

template<typename Ret, typename... Args>
struct convert_to_default_cc<Ret(__stdcall)(Args...)> {
	using type = Ret(Args...);
};

template<typename Ret, typename... Args>
struct convert_to_default_cc<Ret(__cdecl)(Args...)> {
	using type = Ret(Args...);
};

template<typename Ret, typename... Args>
struct convert_to_default_cc<Ret(__thiscall*)(Args...)> {
	using type = Ret(Args...);
};

template<typename Ret, typename Class, typename... Args>
struct convert_to_default_cc<Ret(Class:: *)(Args...)> {
	using type = Ret(Class*, Args...);
};

template<typename Func>
using convert_to_default_cc_t = typename convert_to_default_cc<Func>::type;													


//STRUCT TEMPLATE remove_func_pointer
template<typename Func>
struct remove_func_pointer {
	using type = std::remove_pointer_t<Func>;
};

// non-member thiscall function type can be only by pointer
template<typename Ret, typename... Args>
struct remove_func_pointer<Ret(__thiscall*)(Args...)> {
	using type = Ret(__thiscall*)(Args...);
};

template<typename Func>
using remove_func_pointer_t = typename remove_func_pointer<Func>::type;



//STRUCT TEMPLATE params_size_as_array
namespace detail {

	template<typename Arg>
	constexpr static auto get_size_rounded() {
		//suppose that references use pointers in implement
		constexpr auto REFERENCE_SIZE = sizeof(uintptr_t);
		//round by stack's push argument size 
		constexpr auto ROUNDING_FACTOR = 4;

		if constexpr (std::is_reference_v<Arg>) {
			return REFERENCE_SIZE;
		}
		
		const auto remainder = sizeof(Arg) % ROUNDING_FACTOR;
		
		return remainder == 0 ? sizeof(Arg) : sizeof(Arg) + ROUNDING_FACTOR - remainder;
	}

	template<typename... Args>
	constexpr static auto get_size_as_array() -> std::array<size_t, sizeof...(Args)> {
		return { get_size_rounded<Args>()... };
	}

}

template<typename>
struct params_size_as_array {};

template<typename Ret, typename... Args>
struct params_size_as_array<Ret(__cdecl)(Args...)> {
	static constexpr auto value = detail::get_size_as_array<Args...>();
};

template<typename Ret, typename... Args>
struct params_size_as_array<Ret(__stdcall)(Args...)> {
	static constexpr auto value = detail::get_size_as_array<Args...>();
};

template<typename Ret, typename Class, typename... Args>
struct params_size_as_array<Ret(__thiscall*)(Class, Args...)> {
	static constexpr auto value = detail::get_size_as_array<Args...>();
};

template<typename Ret, typename Class, typename... Args>
struct params_size_as_array<Ret(Class::*)(Args...)> {
	static constexpr auto value = detail::get_size_as_array<Args...>();
};

template<typename Func>
constexpr auto params_size_as_array_v = params_size_as_array<Func>::value;



//STRUCT TEMPLATE params_size
namespace detail {
	template<typename T>
	constexpr auto sum(T& container) -> typename T::value_type {
		typename T::value_type sum = 0;

		for (auto it = container.cbegin(); it != container.cend(); it++) {
			sum += *it;
		}

		return sum;
	}
}

template<typename Func>
constexpr size_t params_size_v = detail::sum(params_size_as_array_v<Func>);


//STRUCT TEMPLATE params_count
template<typename Func>
constexpr size_t params_count_v = params_size_as_array_v<Func>.size();