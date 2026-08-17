#include "entity_system.h"
#include "../../offsets/offsets.h"

sdk::classes::base_entity* sdk::interfaces::entity_system::get_base_entity(const int index) {
	const auto fn = sdk::g_offsets.patterns.get_base_entity.get<sdk::classes::base_entity * (__thiscall*)(entity_system*, const int)>();
	return fn(this, index);
}

sdk::classes::cs_player_pawn* sdk::interfaces::entity_system::split_screen_view_pawn(const int index) {
	const auto fn = sdk::g_offsets.patterns.split_screen_view_pawn.get<sdk::classes::cs_player_pawn * (__thiscall*)(const int)>();
	return fn(index);
}