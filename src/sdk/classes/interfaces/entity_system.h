#pragma once
namespace sdk {
	namespace classes {
		class cs_player_pawn;
		class base_entity;
	}
}
namespace sdk::interfaces {
	class entity_system {
	public:
		sdk::classes::base_entity* get_base_entity(const int index);
		static sdk::classes::cs_player_pawn* split_screen_view_pawn(const int index);
	};
}