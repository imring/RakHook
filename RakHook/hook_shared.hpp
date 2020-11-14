#pragma once
#include "Yet-another-hook-library/hook.h"

#include <memory>

template<typename HookeeType>
using hook_t = hook<HookeeType,
	std::add_pointer_t<convert_to_default_cc_t<add_reference_to_args_t<remove_func_pointer_t<HookeeType>>>>
>;

template<typename HookeeType>
using hook_shared_t = std::shared_ptr<hook_t<HookeeType>>;