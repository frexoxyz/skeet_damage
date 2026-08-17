#include "weapon_config.h"
#include "keybind.h"
#include <atomic>
#include <cstdio>

namespace {
	using namespace cheat::weapon_cfg;
	std::atomic<std::uint16_t> active_item{};
	std::atomic_bool game_is_active{};
	bool force_state{};
	bool damage_state{};
	enum class active_action { none, force_lethal, damage_override };
	active_action last_action = active_action::none;
	std::vector<int> held_output_keys;
	bool binds_initialized{};
	bool binds_enabled{};
	bool output_reassert_pending{};
	std::vector<int> pending_pulse_release;
	constexpr ULONG_PTR injected_input_tag = static_cast<ULONG_PTR>(0xF1A7E55u);

	bool contains(std::uint16_t item, std::initializer_list<std::uint16_t> values) {
		return std::find(values.begin(), values.end(), item) != values.end();
	}

	void initialize_binds() {
		if (binds_initialized)
			return;

		keybind::add_bind("force_lethal", &force_lethal_key.get(), &force_lethal_mode.get(), &force_state, "Force Lethal");
		keybind::add_bind("minimum_damage_override", &damage_override_key.get(), &damage_override_mode.get(), &damage_state, "Minimum Damage Override");
		binds_initialized = true;
	}

	void send_key(int key, bool down) {
		if (key <= 0 || key >= 256)
			return;

		INPUT input{};
		if (key >= VK_LBUTTON && key <= VK_XBUTTON2) {
			input.type = INPUT_MOUSE;
			input.mi.dwExtraInfo = injected_input_tag;
			switch (key) {
			case VK_LBUTTON: input.mi.dwFlags = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP; break;
			case VK_RBUTTON: input.mi.dwFlags = down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP; break;
			case VK_MBUTTON: input.mi.dwFlags = down ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP; break;
			default: input.mi.dwFlags = down ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP; input.mi.mouseData = key == VK_XBUTTON1 ? XBUTTON1 : XBUTTON2; break;
			}
		} 
		else {
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = static_cast<WORD>(key);
			input.ki.dwFlags = down ? 0 : KEYEVENTF_KEYUP;
			input.ki.dwExtraInfo = injected_input_tag;
		}
		::SendInput(1, &input, sizeof(input));
	}

	void pulse_key_combo(const std::vector<int>& keys) {
		if (keys.empty())
			return;

		for (auto it = pending_pulse_release.rbegin(); it != pending_pulse_release.rend(); ++it)
			send_key(*it, false);
		pending_pulse_release.clear();
		for (int key : keys)
			send_key(key, true);
		pending_pulse_release = keys;
	}

	void deactivate_external_bind(const std::vector<int>& keys, int mode) {
		if (keys.empty())
			return;

		if (mode == keybind::toggle) {
			pulse_key_combo(keys);
			return;
		}

		for (auto it = keys.rbegin(); it != keys.rend(); ++it)
			send_key(*it, false);
	}

	void set_held_output(const std::vector<int>& keys) {
		if (held_output_keys != keys) {
			for (auto it = held_output_keys.rbegin(); it != held_output_keys.rend(); ++it) 
				send_key(*it, false);
			held_output_keys = keys;
			for (int key : held_output_keys) 
				send_key(key, true);
			output_reassert_pending = false;
			return;
		}
		if (output_reassert_pending) {
			for (int key : held_output_keys) 
				send_key(key, true);
			output_reassert_pending = false;
		}
	}

	void normalize_active_action() {
		force_state = keybind::is_active(&force_lethal_key.get());
		damage_state = keybind::is_active(&damage_override_key.get());
		if (last_action == active_action::force_lethal && !force_state)
			last_action = damage_state ? active_action::damage_override : active_action::none;
		else if (last_action == active_action::damage_override && !damage_state)
			last_action = force_state ? active_action::force_lethal : active_action::none;
		else if (last_action == active_action::none) {
			if (damage_state) 
				last_action = active_action::damage_override;
			else if (force_state) 
				last_action = active_action::force_lethal;
		}
	}

	void track_physical_output_release(UINT msg, WPARAM wparam) {
		if (::GetMessageExtraInfo() == static_cast<LPARAM>(injected_input_tag)) return;

		int key{};
		switch (msg) {
		case WM_KEYUP: case WM_SYSKEYUP: key = static_cast<int>(wparam); break;
		case WM_LBUTTONUP: key = VK_LBUTTON; break; case WM_RBUTTONUP: key = VK_RBUTTON; break;
		case WM_MBUTTONUP: key = VK_MBUTTON; break;
		case WM_XBUTTONUP: key = GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2; break;
		default: return;
		}

		if (std::find(held_output_keys.begin(), held_output_keys.end(), key) != held_output_keys.end())
			output_reassert_pending = true;
	}
}

void cheat::weapon_cfg::set_active_weapon(std::uint16_t index) { active_item.store(index, std::memory_order_relaxed); }

void cheat::weapon_cfg::set_game_active(bool active) { game_is_active.store(active, std::memory_order_relaxed); }

bool cheat::weapon_cfg::game_active() { return game_is_active.load(std::memory_order_relaxed); }

cheat::weapon_cfg::profile_id cheat::weapon_cfg::active_profile() {
	const auto item = active_item.load(std::memory_order_relaxed);

	if (contains(item, { 11, 38 }))
		return profile_id::auto_sniper;
	if (item == 40)
		return profile_id::ssg08;
	if (item == 9)
		return profile_id::awp;
	if (item == 64)
		return profile_id::r8_revolver;
	if (item == 1)
		return profile_id::desert_eagle;
	if (contains(item, { 2, 3, 4, 30, 32, 36, 61, 63 }))
		return profile_id::pistol;
	if (contains(item, { 7, 8, 10, 13, 16, 39, 60 }))
		return profile_id::rifle;
	if (contains(item, { 25, 27, 29, 35 }))
		return profile_id::shotgun;
	if (contains(item, { 17, 19, 23, 24, 26, 33, 34 }))
		return profile_id::smg;
	if (contains(item, { 14, 28 }))
		return profile_id::machine_gun;

	return profile_id::global;
}

cheat::weapon_cfg::profile_vars& cheat::weapon_cfg::effective_profile() {
	const auto id = active_profile();
	auto* selected = profiles[static_cast<std::size_t>(id)];
	return id != profile_id::global && selected->enabled.get() ? *selected : global;
}

void cheat::weapon_cfg::update_binds() {
	initialize_binds();
	if (!pending_pulse_release.empty()) {
		for (auto it = pending_pulse_release.rbegin(); it != pending_pulse_release.rend(); ++it)
			send_key(*it, false);
		pending_pulse_release.clear();
	}
	if (!binds_enabled) {
		set_held_output({});
		return;
	}
	if (force_lethal_mode.get() == keybind::always) 
		keybind::set_active(&force_lethal_key.get(), true);

	if (damage_override_mode.get() == keybind::always) 
		keybind::set_active(&damage_override_key.get(), true);

	normalize_active_action();
	set_held_output(last_action != active_action::none ? force_lethal_skeet_key.get() : std::vector<int>{});
}

void cheat::weapon_cfg::set_binds_enabled(bool enabled) {
	initialize_binds();
	if (binds_enabled == enabled)
		return;

	binds_enabled = enabled;
	keybind::keys_down().clear();
	if (enabled)
		return;

	for (auto& [_, bind] : keybind::binds()) {
		if (bind.mode_ptr && *bind.mode_ptr != keybind::hold)
			continue;
		*bind.is_active_ptr = false;
		if (bind.mirror_value)
			*bind.mirror_value = false;
	}
	set_held_output({});
	normalize_active_action();
}

void cheat::weapon_cfg::process_key_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	initialize_binds();
	(void)hwnd;
	track_physical_output_release(msg, wparam);
	if (::GetMessageExtraInfo() == static_cast<LPARAM>(injected_input_tag))
		return;

	const bool force_before = keybind::is_active(&force_lethal_key.get());
	const bool damage_before = keybind::is_active(&damage_override_key.get());
	LPARAM bind_lparam = lparam;
	if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) {
		const int key = static_cast<int>(wparam);
		if (std::find(held_output_keys.begin(), held_output_keys.end(), key) != held_output_keys.end())
			bind_lparam &= ~(static_cast<LPARAM>(1) << 30);
	}

	keybind::process_msg(msg, wparam, bind_lparam, binds_enabled);
	force_state = keybind::is_active(&force_lethal_key.get());
	damage_state = keybind::is_active(&damage_override_key.get());

	if (!force_before && force_state) {
		last_action = active_action::force_lethal;
		keybind::set_active(&damage_override_key.get(), false);
		damage_state = false;
		if (damage_before)
			deactivate_external_bind(damage_override_key.get(), damage_override_mode.get());
	}
	if (!damage_before && damage_state) {
		last_action = active_action::damage_override;
		keybind::set_active(&force_lethal_key.get(), false);
		force_state = false;
	}
	normalize_active_action();
}

void cheat::weapon_cfg::release_binds() {
	for (auto it = pending_pulse_release.rbegin(); it != pending_pulse_release.rend(); ++it)
		send_key(*it, false);
	pending_pulse_release.clear();
	set_held_output({});
	keybind::keys_down().clear();
}

bool cheat::weapon_cfg::override_active() { 
	initialize_binds();
	normalize_active_action(); 
	return last_action == active_action::damage_override; 
}

bool cheat::weapon_cfg::force_lethal_active() { 
	initialize_binds();
	normalize_active_action(); 
	return last_action == active_action::force_lethal; 
}

bool cheat::weapon_cfg::lethal_active() {
	const auto& off_hotkey = force_lethal_skeet_key.get();
	if (off_hotkey.empty() || force_lethal_active() || override_active()) 
		return false;

	const bool off_hotkey_down = std::all_of(off_hotkey.begin(), off_hotkey.end(), [](int key) {
		return key > 0 && (::GetAsyncKeyState(key) & 0x8000) != 0;
	});

	return !off_hotkey_down;
}

int cheat::weapon_cfg::effective_damage() {
	auto& cfg = effective_profile();
	if (override_active())
		return std::clamp(cfg.override_damage.get(), 1, 125);

	if (lethal_active())
		return 100;

	return std::clamp(cfg.minimum_damage.get(), 1, 125);
}

const char* cheat::weapon_cfg::damage_text() {
	static char text[32];
	if (lethal_active())
		return "LETHAL";

	const int damage = effective_damage();
	if (damage > 100)
		std::snprintf(text, sizeof(text), "HP+%d", damage - 100);
	else
		std::snprintf(text, sizeof(text), "%d", damage);

	return text;
}
