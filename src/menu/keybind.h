#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <algorithm>

namespace keybind {
	enum mode_e : int { hold = 0, toggle = 1, always = 2 };

	struct setup {
		std::vector<int>* keys_ptr{};
		int* mode_ptr{};
		std::shared_ptr<bool> is_active_ptr;
		bool* mirror_value{};
		std::string display_name;
		std::string location;
	};

	inline std::unordered_map<std::string, setup>& binds() { static std::unordered_map<std::string, setup> value; return value; }
	inline std::unordered_map<std::vector<int>*, setup>& binds_by_key() { static std::unordered_map<std::vector<int>*, setup> value; return value; }
	inline std::vector<std::string>& bind_names() { static std::vector<std::string> value; return value; }
	inline std::vector<int>*& capture_target() { static std::vector<int>* value{}; return value; }
	inline std::vector<int>& capture_buffer() { static std::vector<int> value; return value; }
	inline std::unordered_set<int>& keys_down() { static std::unordered_set<int> value; return value; }
	inline std::vector<int>*& hovered_block() { static std::vector<int>* value{}; return value; }

	inline void add_bind(const std::string& name, std::vector<int>* keys, int* mode = nullptr,
		bool* mirror = nullptr, const std::string& display = "", const std::string& location = "") {
		auto active = std::make_shared<bool>(false);
		if (mode && *mode == always) { *active = true; if (mirror) *mirror = true; }
		else if (mirror) *active = *mirror;
		setup value{ keys, mode, active, mirror, display.empty() ? name : display, location };
		binds()[name] = value;
		if (keys) binds_by_key()[keys] = value;
		bind_names().push_back(name);
	}

	inline bool is_active(std::vector<int>* keys) {
		auto it = binds_by_key().find(keys);
		return it != binds_by_key().end() && *it->second.is_active_ptr;
	}
	inline void set_active(std::vector<int>* keys, bool active) {
		auto it = binds_by_key().find(keys);
		if (it == binds_by_key().end()) return;
		*it->second.is_active_ptr = active;
		if (it->second.mirror_value) *it->second.mirror_value = active;
	}
	inline setup* find_by_key(std::vector<int>* keys) {
		auto it = binds_by_key().find(keys); return it == binds_by_key().end() ? nullptr : &it->second;
	}
	inline void start_capture(std::vector<int>* target) { capture_target() = target; capture_buffer().clear(); }
	inline bool is_capturing(std::vector<int>* target) { return capture_target() == target; }
	inline bool is_any_capturing() { return capture_target() != nullptr; }
	inline void stop_capture() { capture_target() = nullptr; capture_buffer().clear(); }
	inline void set_hovered_block(std::vector<int>* keys) { hovered_block() = keys; }
	inline void reset_hovered_block() { hovered_block() = nullptr; }

	inline std::string key_name(int vk) {
		if (!vk) return "None";
		switch (vk) {
		case VK_LBUTTON: return "LMB"; case VK_RBUTTON: return "RMB"; case VK_MBUTTON: return "MMB";
		case VK_XBUTTON1: return "Mouse4"; case VK_XBUTTON2: return "Mouse5";
		}
		if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z')) return std::string(1, static_cast<char>(vk));
		if (vk >= VK_F1 && vk <= VK_F24) { char b[8]; sprintf_s(b, "F%d", vk - VK_F1 + 1); return b; }
		if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) { char b[8]; sprintf_s(b, "Num%d", vk - VK_NUMPAD0); return b; }
		switch (vk) {
		case VK_SPACE: return "Space"; case VK_RETURN: return "Enter"; case VK_TAB: return "Tab"; case VK_ESCAPE: return "Esc";
		case VK_BACK: return "Backspace"; case VK_CAPITAL: return "Caps"; case VK_SHIFT: case VK_LSHIFT: return "Shift";
		case VK_RSHIFT: return "R Shift"; case VK_CONTROL: case VK_LCONTROL: return "Ctrl"; case VK_RCONTROL: return "R Ctrl";
		case VK_MENU: case VK_LMENU: return "Alt"; case VK_RMENU: return "R Alt"; case VK_LWIN: case VK_RWIN: return "Win";
		case VK_APPS: return "Menu"; case VK_LEFT: return "Left"; case VK_RIGHT: return "Right"; case VK_UP: return "Up";
		case VK_DOWN: return "Down"; case VK_PRIOR: return "PageUp"; case VK_NEXT: return "PageDown"; case VK_HOME: return "Home";
		case VK_END: return "End"; case VK_INSERT: return "Insert"; case VK_DELETE: return "Delete"; case VK_ADD: return "Num+";
		case VK_SUBTRACT: return "Num-"; case VK_MULTIPLY: return "Num*"; case VK_DIVIDE: return "Num/"; case VK_DECIMAL: return "Num.";
		case VK_NUMLOCK: return "NumLock"; case VK_SCROLL: return "ScrollLock"; case VK_OEM_1: return ";"; case VK_OEM_PLUS: return "=";
		case VK_OEM_COMMA: return ","; case VK_OEM_MINUS: return "-"; case VK_OEM_PERIOD: return "."; case VK_OEM_2: return "/";
		case VK_OEM_3: return "`"; case VK_OEM_4: return "["; case VK_OEM_5: return "\\"; case VK_OEM_6: return "]"; case VK_OEM_7: return "'";
		}
		char b[16]; sprintf_s(b, "Key%d", vk); return b;
	}

	inline std::string combo_name(const std::vector<int>& keys) {
		std::string out;
		for (int key : keys) if (key) { if (!out.empty()) out += "+"; out += key_name(key); }
		return out.empty() ? "None" : out;
	}

	inline bool process_msg(UINT msg, WPARAM wparam, LPARAM lparam) {
		if (msg == WM_KILLFOCUS) {
			keys_down().clear();
			for (auto& [_, bind] : binds()) if ((!bind.mode_ptr || *bind.mode_ptr == hold)) {
				*bind.is_active_ptr = false; if (bind.mirror_value) *bind.mirror_value = false;
			}
			return false;
		}
		int vk{}; bool down{}, mouse{};
		switch (msg) {
		case WM_KEYDOWN: case WM_SYSKEYDOWN: vk = static_cast<int>(wparam); down = true; break;
		case WM_KEYUP: case WM_SYSKEYUP: vk = static_cast<int>(wparam); break;
		case WM_LBUTTONDOWN: vk = VK_LBUTTON; down = mouse = true; break; case WM_LBUTTONUP: vk = VK_LBUTTON; mouse = true; break;
		case WM_RBUTTONDOWN: vk = VK_RBUTTON; down = mouse = true; break; case WM_RBUTTONUP: vk = VK_RBUTTON; mouse = true; break;
		case WM_MBUTTONDOWN: vk = VK_MBUTTON; down = mouse = true; break; case WM_MBUTTONUP: vk = VK_MBUTTON; mouse = true; break;
		case WM_XBUTTONDOWN: vk = GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2; down = mouse = true; break;
		case WM_XBUTTONUP: vk = GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2; mouse = true; break;
		default: return false;
		}
		const bool repeat = !mouse && down && ((lparam >> 30) & 1);
		if (down) keys_down().insert(vk); else keys_down().erase(vk);
		auto* captured = capture_target(); bool consumed = false;
		if (capture_target()) {
			if (down) {
				if (vk == VK_ESCAPE) {
					auto* target = capture_target(); target->clear(); stop_capture();
					if (auto it = binds_by_key().find(target); it != binds_by_key().end()) {
						*it->second.is_active_ptr = false; if (it->second.mirror_value) *it->second.mirror_value = false;
					}
					return true;
				}
				if (std::find(capture_buffer().begin(), capture_buffer().end(), vk) == capture_buffer().end()) capture_buffer().push_back(vk);
			} else if (!capture_buffer().empty() && std::find(capture_buffer().begin(), capture_buffer().end(), vk) != capture_buffer().end()) {
				*capture_target() = capture_buffer(); stop_capture();
			}
			consumed = true;
		}
		if (repeat) return consumed;
		for (auto& [_, bind] : binds()) {
			if ((captured && bind.keys_ptr == captured) || (hovered_block() && bind.keys_ptr == hovered_block())) continue;
			const int mode = bind.mode_ptr ? *bind.mode_ptr : toggle;
			if (mode == always) { *bind.is_active_ptr = true; if (bind.mirror_value) *bind.mirror_value = true; continue; }
			if (!bind.keys_ptr || bind.keys_ptr->empty() || std::find(bind.keys_ptr->begin(), bind.keys_ptr->end(), vk) == bind.keys_ptr->end()) continue;
			if (down) {
				bool all = true; for (int key : *bind.keys_ptr) if (key && !keys_down().contains(key)) { all = false; break; }
				if (!all) continue;
				if (mode == hold) *bind.is_active_ptr = true; else if (mode == toggle) *bind.is_active_ptr = !*bind.is_active_ptr;
				if (bind.mirror_value) *bind.mirror_value = *bind.is_active_ptr;
			} else if (mode == hold) { *bind.is_active_ptr = false; if (bind.mirror_value) *bind.mirror_value = false; }
		}
		return consumed;
	}
}
