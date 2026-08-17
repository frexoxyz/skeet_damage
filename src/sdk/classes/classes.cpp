#include "classes.h"
#include "../offsets/offsets.h"
#include "interfaces/entity_system.h"

sdk::classes::cs_weapon_base* sdk::classes::cs_player_pawn::active_weapon() {
	if (!this) 
		return nullptr;

	auto* services = weapon_services();
	if (!services) 
		return nullptr;

	const auto handle = services->active_weapon();
	if (!handle.is_valid())
		return nullptr;

	return reinterpret_cast<cs_weapon_base*>(sdk::g_offsets.interfaces.entity_system->get_base_entity(handle.get_entry_index()));
}
