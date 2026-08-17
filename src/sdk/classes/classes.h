#pragma once
#include "types.h"
#include "interfaces/schema/schema.h"

namespace sdk::classes {
	class entity_instance {};
	class base_entity : public entity_instance {};

	class econ_item_view {
	public:
		VALVE_SCHEMA_FIELD(std::uint16_t, item_definition_index, "C_EconItemView", "m_iItemDefinitionIndex");
	};

	class attribute_container {
	public:
		VALVE_SCHEMA_FIELD(econ_item_view, item, "C_AttributeContainer", "m_Item");
	};

	class econ_entity : public base_entity {
	public:
		VALVE_SCHEMA_FIELD(attribute_container, attribute_manager, "C_EconEntity", "m_AttributeManager");
	};

	class cs_weapon_base : public econ_entity {};

	class player_weapon_services {
	public:
		VALVE_SCHEMA_FIELD(sdk::types::base_handle, active_weapon, "CPlayer_WeaponServices", "m_hActiveWeapon");
	};

	class cs_player_pawn : public base_entity {
	public:
		VALVE_SCHEMA_FIELD(player_weapon_services*, weapon_services, "C_BasePlayerPawn", "m_pWeaponServices");
		cs_weapon_base* active_weapon();
	};
}
