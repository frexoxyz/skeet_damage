#include "offsets.h"

bool sdk::c_offsets::c_patterns::initialize() {
	sdk::g_offsets.failed_pattern = false;

    auto client_dll = sdk::g_offsets.modules.client;
	sdk::g_offsets.init_pattern(client_dll, this->input, ("csgo_input"), ("48 8B 0D ? ? ? ? 4C 8B C6 8B 10 E8"));
    sdk::g_offsets.init_pattern(client_dll, this->view_render, ("screen_transform"), ("48 89 05 ?? ?? ?? ?? 48 8B C8 48 85 C0"));
    sdk::g_offsets.init_pattern(client_dll, this->entity_system, ("entity_system"), ("48 89 ? ? ? ? ? 4C 63 ? ? ? ? ? 44 3B ? ? ? ? ? 0F"));
    sdk::g_offsets.init_pattern(client_dll, this->get_base_entity, ("get_base_entity"), ("4C 8D 49 ? 81 FA"));
    sdk::g_offsets.init_pattern(client_dll, this->split_screen_view_pawn, ("split_screen_view_pawn"), ("48 83 EC ? 83 F9 ? 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 08 48 63 C1 4C 8D 05"));

    sdk::g_offsets.init_pattern(sdk::g_offsets.modules.input_system, this->wnd_proc, ("wnd_proc"), ("48 83 EC ? 4C 89 4C 24"));

    sdk::g_offsets.init_pattern(sdk::g_offsets.modules.game_overlay_renderer, this->dx11_present, ("dx11_present"), ("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC ? 41 8B F0 8B EA 4C 8B F9"));
    sdk::g_offsets.init_pattern(sdk::g_offsets.modules.game_overlay_renderer, this->dx11_resize_buffers, ("dx11_resize_buffers"), ("40 53 55 56 57 41 54 41 56 41 57 48 81 EC ? ? ? ? 8B EA"));

	return !sdk::g_offsets.failed_pattern;
}
