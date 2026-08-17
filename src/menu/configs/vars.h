#pragma once
#include "config.h"
#include <array>

namespace cheat::weapon_cfg {
	enum bind_mode : int { hold = 0, toggle = 1, always = 2 };
	enum class profile_id : std::size_t {
		global, auto_sniper, ssg08, awp, r8_revolver, desert_eagle,
		pistol, rifle, shotgun, smg, machine_gun, count
	};

	inline constexpr std::array<const char*, static_cast<std::size_t>(profile_id::count)> profile_names{
		"Global", "Auto sniper", "SSG 08", "AWP", "R8 Revolver", "Desert Eagle",
		"Pistol", "Rifle", "Shotgun", "SMG", "Machine gun"
	};

	struct profile_vars {
		config::c_var<bool> enabled;
		config::c_var<int> minimum_damage;
		config::c_var<int> override_damage;

		profile_vars(const char* prefix, bool is_enabled = false)
			: enabled((std::string(prefix) + ".enabled").c_str(), is_enabled),
			minimum_damage((std::string(prefix) + ".minimum_damage").c_str(), 5),
			override_damage((std::string(prefix) + ".override_damage").c_str(), 50) {}
	};

	inline profile_vars global{ "weapon.global", true };
	inline profile_vars auto_sniper{ "weapon.auto_sniper" };
	inline profile_vars ssg08{ "weapon.ssg08" };
	inline profile_vars awp{ "weapon.awp" };
	inline profile_vars r8_revolver{ "weapon.r8_revolver" };
	inline profile_vars desert_eagle{ "weapon.desert_eagle" };
	inline profile_vars pistol{ "weapon.pistol" };
	inline profile_vars rifle{ "weapon.rifle" };
	inline profile_vars shotgun{ "weapon.shotgun" };
	inline profile_vars smg{ "weapon.smg" };
	inline profile_vars machine_gun{ "weapon.machine_gun" };
	inline config::c_var<std::vector<int>> force_lethal_skeet_key{ "binds.force_lethal_skeet", {} };
	inline config::c_var<std::vector<int>> force_lethal_key{ "binds.force_lethal", {} };
	inline config::c_var<int> force_lethal_mode{ "binds.force_lethal_mode", bind_mode::toggle };
	inline config::c_var<std::vector<int>> damage_override_key{ "binds.minimum_damage_override", {} };
	inline config::c_var<int> damage_override_mode{ "binds.minimum_damage_override_mode", bind_mode::hold };
	inline config::c_var<bool> show_indicator{ "indicator.show", true };
	inline config::c_var<bool> show_indicator_if_active{ "indicator.show_if_active", false };
	inline config::c_var<int> indicator_font{ "indicator.font", 0 };

	inline std::array<profile_vars*, static_cast<std::size_t>(profile_id::count)> profiles{
		&global, &auto_sniper, &ssg08, &awp, &r8_revolver, &desert_eagle,
		&pistol, &rifle, &shotgun, &smg, &machine_gun
	};
}
