#include "../hooks.h"
#include "../../sdk/classes/types.h"
#include "../../sdk/offsets/offsets.h"
#include "../../sdk/classes/interfaces/engine.h"
#include "../../sdk/classes/interfaces/entity_system.h"
#include "../../sdk/classes/classes.h"
#include "../../menu/menu.h"
#include "../../menu/weapon_config.h"


bool cheat::hooks::client::mouse_input_enabled(void* rcx) {
	const auto original = originals::mouse_input_enabled.original<bool(*)(void*)>();

	if (cheat::g_menu.is_open)
		return false;

	return original(rcx);
}

void* cheat::hooks::client::is_relative_mouse_mode(void* a1, bool active) {
	const auto original = originals::is_relative_mouse_mode.original<void*(*)(void* a1, bool active)>();
	cheat::g_menu.input_active = active;
	return original(a1, cheat::g_menu.is_open ? false : active);
}

void cheat::hooks::client::on_render_start(void* a1) {
	const auto original = originals::on_render_start.original<void* (*)(void* a1)>();
	original(a1);

	if (!sdk::g_offsets.interfaces.engine->in_game() || !sdk::g_offsets.interfaces.engine->is_connected())
	{
		cheat::weapon_cfg::set_active_weapon(0);
		return;
	}

	auto local_pawn = sdk::g_offsets.interfaces.entity_system->split_screen_view_pawn(0);
	if (!local_pawn)
	{
		cheat::weapon_cfg::set_active_weapon(0);
		return;
	}

	auto active_weapon = local_pawn->active_weapon();
	cheat::weapon_cfg::set_active_weapon(active_weapon ? active_weapon->attribute_manager().item().item_definition_index() : 0);
}
