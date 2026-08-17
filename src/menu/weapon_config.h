#pragma once
#include "configs/vars.h"

namespace cheat::weapon_cfg {
	void set_active_weapon(std::uint16_t item_definition_index);
	void set_game_active(bool active);
	bool game_active();
	void update_binds();
	void set_binds_enabled(bool enabled);
	void process_key_message(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
	void release_binds();
	profile_id active_profile();
	profile_vars& effective_profile();
	bool override_active();
	bool force_lethal_active();
	bool lethal_active();
	int effective_damage();
	const char* damage_text();
}
