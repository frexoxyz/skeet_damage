#include "hook_library.h"
#include "minhook/include/MinHook.h"

hook_library::hook::hook(void* p_function, void* p_detour) {
	create(p_function, p_detour);
}

bool hook_library::hook::create(void* p_function, void* p_detour) {
	if (!p_function || !p_detour)
		return false;

	target = p_function;

	if (MH_CreateHook(target, p_detour, &original_pointer) != MH_OK) {
		target = nullptr;
		return false;
	}

	return enable();
}

bool hook_library::hook::enable() {
	if (!target)
		return false;
	return MH_EnableHook(target) == MH_OK;
}

bool hook_library::hook::disable() {
	if (!target)
		return false;
	return MH_DisableHook(target) == MH_OK;
}

bool hook_library::hook::remove() {
	if (!target)
		return false;

	disable();
	bool removed = MH_RemoveHook(target) == MH_OK;
	target = nullptr;
	original_pointer = nullptr;
	return removed;
}

bool hook_library::initialize() {
	return MH_Initialize() == MH_OK;
}

void hook_library::shut_down() {
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
}
