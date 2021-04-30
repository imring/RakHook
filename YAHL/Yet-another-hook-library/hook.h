#pragma once
#include <array>
#include <vector>
#include <memory>
#include <iterator>

#include "xbyak/xbyak.h"
#include "hde32/include/hde32.h"
#include "hook_function_traits.hpp"

namespace detail {

constexpr auto JUMP_OPCODE_SIZE = 5;
constexpr auto PARAM_SIZE = 4;

/*
	By unknown reason, C++ does not support converting a pointer to a member function to an arbitrary pointer.
	So, this "semantic sledgehammer" used for fix that
*/
template<typename Out, typename In>
constexpr Out force_cast(In in)
{
	union
	{
		In  in;
		Out out;
	}
	u = { in };

	return u.out;
};

template<typename Func1, typename Func2>
constexpr bool is_same_custom_v = std::is_same_v<get_function_args_t<Func1>, get_function_args_t<Func2>> &&
	(std::is_same_v<get_function_return_type_t<Func1>, get_function_return_type_t<Func2>> 
		|| std::is_void_v<get_function_return_type_t<Func1>>);


namespace memory_protect {
	enum class protection {
		R,
		RW,
		RE,
		RWE
	};

	auto constexpr get_system_protection_constant(protection mode) noexcept {
#if defined(_WIN32)
		constexpr auto DEFAULT_CONSTANT = PAGE_EXECUTE_READWRITE;

		if (mode == protection::R)
			return PAGE_READONLY;
		else if (mode == protection::RW)
			return PAGE_READWRITE;
		else if (mode == protection::RE)
			return PAGE_EXECUTE_READ;
		else if (mode == protection::RWE)
			return PAGE_EXECUTE_READWRITE;
		else
			return DEFAULT_CONSTANT;
#elif defined(__linux__)
		auto DEFAULT_CONSTANT = PROT_READ | PROT_WRITE | PROT_EXEC;

		if (mode == protection::R)
			return PROT_READ;
		else if (mode == protection::RW)
			return PROT_READ | PROT_WRITE;
		else if (mode == protection::RE)
			return PROT_READ | PROT_EXEC;
		else if (mode == protection::RWE)
			return PROT_READ | PROT_WRITE | PROT_EXEC;
		else
			return DEFAULT_CONSTANT;
#endif
	}

	template<typename T>
	protection constexpr convert_system_protection_constant(T constant) noexcept {
		const protection DEFAULT_PROTECTION = protection::RWE;
#if defined(_WIN32)
		if (constant == PAGE_READONLY)
			return protection::R;
		else if (constant == PAGE_READWRITE)
			return protection::RW;
		else if (constant == PAGE_EXECUTE_READ)
			return protection::RE;
		else if (constant == PAGE_EXECUTE_READWRITE)
			return protection::RWE;
		else
			return DEFAULT_PROTECTION;
#elif defined(__linux__)
		if (constant == PROT_READ)
			return protection::R;
		else if (constant == PROT_READ | PROT_WRITE)
			return protection::RW;
		else if (constant == PROT_READ | PROT_EXEC)
			return protection::RE;
		else if (constant == PROT_READ | PROT_WRITE | PROT_EXEC)
			return protection::RWE;
		else
			return DEFAULT_PROTECTION;
#endif
	}

	template<typename AddrType>
	protection set_protection(AddrType address, size_t size, protection mode) {
#if defined(_WIN32)
		const DWORD prot = get_system_protection_constant(mode);
		DWORD prev_prot;
		
		VirtualProtect(force_cast<LPVOID>(address), size, prot, &prev_prot);

		return convert_system_protection_constant(prev_prot);
#elif defined(__linux__)
		// linux unprotect implementation... where?

		/*
			Linux doesn't have any syscall for retrieve protection of memory region
			So, at default, we use READ WRITE EXECUTE
		*/
		return convert_system_protection_constant(PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
	}

	template<typename AddrType>
	class unprotect_guard {
	public:
		unprotect_guard(AddrType address, size_t size): address(address), size(size) {
			prev_protection = set_protection(address, size, protection::RWE);
		}

		~unprotect_guard() {
			set_protection(address, size, prev_protection);
		}

	private:
		AddrType address;
		size_t size;
		protection prev_protection;
	};
};

class trampouline_generator {
public:
	trampouline_generator() : code(std::make_shared<Xbyak::CodeGenerator>()) {};

	template<size_t Size>
	void push_references_to_params_in_stack(const std::array<size_t, Size>& params_size) {
		using namespace Xbyak::util;

		auto params_count = Size;

		for (size_t i = 0; i < params_count; i++) {
			auto push_offset = sum_first_until_n(params_size, params_count - (i+1));
			push_offset += (i+1) * PARAM_SIZE;

			code->lea(eax, dword[esp + push_offset]);
			code->push(eax);
		}
	}

	template<size_t Size>
	void push_references_to_params_and_ecx_in_stack(const std::array<size_t, Size>& params_size) {
		using namespace Xbyak::util;

		code->push(ecx);

		constexpr auto params_count = Size;

		for (size_t i = 0; i < params_count; i++) {
			auto push_offset = sum_first_until_n(params_size, params_count - (i + 1));
			push_offset += (i + 2) * PARAM_SIZE;

			code->lea(eax, dword[esp + push_offset]);
			code->push(eax);
		}

		code->lea(eax, dword[esp + params_size.size() * PARAM_SIZE]);
		code->push(eax);
	}
	
	void copy_params_in_stack(size_t params_size) {
		using namespace Xbyak::util;

		for (size_t i = 0; i < params_size; i += PARAM_SIZE) {
			code->push(dword [esp + params_size]);
		}
	}

	template<typename Func>
	void call_function(Func function) {
		using namespace Xbyak::util;

		code->call(force_cast<void *>(function));
		
		if constexpr (get_calling_convention_v<Func> == cc::cdecl_) {
			code->add(esp, params_size_v<std::remove_pointer_t<Func>>);
		}
	}

	template<typename Func, typename Obj>
	void call_method(Func method, Obj object) {
		using namespace Xbyak::util;

		code->mov(ecx, force_cast<uint32_t>(object));
		code->call(force_cast<void *>(method));
	}

	template<typename Func> 
	void return_from() {
		if constexpr (get_calling_convention_v<Func> == cc::stdcall_ || 
				get_calling_convention_v<Func> == cc::thiscall_) {
			code->ret(params_size_v<Func>);
		} 
		else {
			code->ret();
		}
	}

	void save_registers() {
		code->pushad();
	}

	void restore_registers() {
		code->popad();
	}

	void push_ecx_by_reference() {
		using namespace Xbyak::util;

		code->lea(eax, dword [esp - PARAM_SIZE*2]);
		code->push(eax);
		code->push(ecx);
		code->add(esp, PARAM_SIZE);
	}

	void pop_ecx() {
		using namespace Xbyak::util;

		code->pop(ecx);
	}

	std::shared_ptr<Xbyak::CodeGenerator> get_code() {
		code->ready();

		return code;
	}

private:
	template<typename T>
	static constexpr auto sum_first_until_n(T& container, size_t n) {
		typename T::value_type sum = 0;
		for (auto it = container.cbegin(); it != container.cbegin() + n; it++) {
			sum += *it;
		}

		return sum;
	}

	std::shared_ptr<Xbyak::CodeGenerator> code;
};

}

template<typename HookeeType, typename HookType>
class hook {
public:
	hook(HookeeType source, HookType destination) : hookee_address(source), hook_address(destination),
		original_instructions(
			get_full_instructions_at_least_n_bytes(detail::force_cast<uint8_t*>(hookee_address), detail::JUMP_OPCODE_SIZE)) {
		assert_check_function_types();

		generate_trampouline();
		set_jump_to_trampouline();
	};

	~hook() {
		restore_original_instructions();
	}

	hook(const hook&) = delete;
	hook(hook&&) = delete;
	hook& operator=(const hook&) = delete;
	hook& operator=(hook&&) = delete;

	template<typename ... Args>
	get_function_return_type_t<remove_func_pointer_t<HookeeType>>
	call_original(Args ... args) {
		temp_disable_hook disable(this);

		return detail::force_cast<move_to_args_t<HookeeType>>(hookee_address)(args...);
	}

private:
	class temp_disable_hook {
	public:
		temp_disable_hook(hook<HookeeType, HookType>* h) : _hook(h) {
			_hook->restore_original_instructions();
		}

		~temp_disable_hook() {
			_hook->set_jump_to_trampouline();
		}

	private:
		hook<HookeeType, HookType>* _hook;
	};

	static void constexpr assert_check_function_types() {
		static_assert(get_calling_convention_v<HookeeType> != cc::unknown_ ||
					  get_calling_convention_v<HookType> != cc::unknown_,
			"Unknown calling convention");
		
		if constexpr (get_calling_convention_v<HookeeType> == cc::thiscall_) {
			static_assert(detail::is_same_custom_v<remove_func_pointer_t<HookType>,
				add_reference_to_args_t<
					convert_member_to_free_func_t<HookeeType>>>,
				"For member function, hook function must get params by reference and have a first argument \"*this\"");
		}
		else {
			static_assert(detail::is_same_custom_v<
										remove_func_pointer_t<HookType>,
				add_reference_to_args_t<remove_func_pointer_t<HookeeType>>>,
				"Hook function must have same signature and get params by reference");
		}
	};

	static std::vector<uint8_t>
	get_full_instructions_at_least_n_bytes(const uint8_t* function, size_t n) {
		std::vector<uint8_t> instructions;

		while (instructions.size() < n) {
			hde32s instruction;

			hde32_disasm(function, &instruction);
			std::copy(function, function + instruction.len, std::back_inserter(instructions));
			function += instruction.len;
		}

		return instructions;
	}

	void generate_trampouline() {
		detail::trampouline_generator code_gen;

		using rp_HookeeType = remove_func_pointer_t<HookeeType>;
		using rp_HookType = remove_func_pointer_t<HookType>;

		constexpr auto is_thiscall = get_calling_convention_v<rp_HookeeType> == cc::thiscall_;
		// constexpr auto call_original = std::is_void_v<get_function_return_type_t<rp_HookType>>;
		constexpr auto hookee_params_array = params_size_as_array_v<rp_HookeeType>;
		constexpr auto hookee_params_size = params_size_v<rp_HookeeType>;

		if constexpr (is_thiscall) {
			code_gen.push_references_to_params_and_ecx_in_stack(hookee_params_array);
		}
		else {
			code_gen.push_references_to_params_in_stack(hookee_params_array);
		}
		code_gen.call_function(hook_address);
		if constexpr (is_thiscall) {
			code_gen.pop_ecx();
		}
		
		/*if constexpr (call_original) {
			code_gen.save_registers();
			code_gen.call_method(&hook<HookeeType, HookType>::restore_original_instructions, this);
			code_gen.restore_registers();

			code_gen.copy_params_in_stack(hookee_params_size);
			code_gen.call_function(hookee_address);

			code_gen.save_registers();
			code_gen.call_method(&hook<HookeeType, HookType>::set_jump_to_trampouline, this);
			code_gen.restore_registers();
		}*/

		code_gen.return_from<rp_HookeeType>();

		trampouline_code = code_gen.get_code();
	}

	void set_jump_to_trampouline() {
		using namespace detail;
		memory_protect::unprotect_guard guard(hookee_address, 5);

		constexpr unsigned char JMP_REL32_BYTECODE = 0xE9;
		constexpr unsigned char NOP_BYTECODE = 0x90;
		std::array<uint8_t, JUMP_OPCODE_SIZE> jump_opcode;
		
		jump_opcode[0] = JMP_REL32_BYTECODE;

		uint32_t jump_offset = force_cast<uint32_t>(trampouline_code->getCode()) - 
			force_cast<uint32_t>(hookee_address) - JUMP_OPCODE_SIZE;
		*(uint32_t*)&jump_opcode[1] = jump_offset;

		std::copy(jump_opcode.begin(), jump_opcode.end(),
			force_cast<uint8_t*>(hookee_address));
			
		if (original_instructions.size() > JUMP_OPCODE_SIZE) {
			std::fill(force_cast<uint8_t*>(hookee_address) + JUMP_OPCODE_SIZE,
				force_cast<uint8_t*>(hookee_address) + original_instructions.size(), NOP_BYTECODE);
		}
	}

	void restore_original_instructions() {
		detail::memory_protect::unprotect_guard guard(hookee_address, 5);

		std::copy(original_instructions.begin(), original_instructions.end(), 
			detail::force_cast<uint8_t*>(hookee_address));
	}

	HookeeType hookee_address;
	HookType hook_address;
	std::vector<uint8_t> original_instructions;
	std::shared_ptr<Xbyak::CodeGenerator> trampouline_code;
};