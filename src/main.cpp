#include "sdk/offsets/offsets.h"
#include "hooks/hooks.h"
#include "menu/configs/config.h"

void initializate(HMODULE instance) {
	if (!sdk::g_offsets.initialize()) {
		MessageBoxA(nullptr, "failed to intialize", "damage", MB_OK);
	}

	if (!cheat::hooks::initialize()) {
		MessageBoxA(nullptr, "failed to initialize hooks", "damage", MB_OK);
	}
}

int __stdcall DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    if (reason != DLL_PROCESS_ATTACH)
        return 0;

    DisableThreadLibraryCalls(instance);

	if (HANDLE thread = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(initializate), reinterpret_cast<LPVOID>(instance), NULL, nullptr))
		CloseHandle(thread);

    return 1;
}
