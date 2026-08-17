#include "menu.h"
#include "weapon_config.h"
#include "keybind.h"
#include "../sdk/tools/imgui/renderer/renderer.h"

namespace {
	void damage_slider(const char* label, int& value) {
		char display[16]{};
		const char* format = "%d";
		if (value > 100) {
			std::snprintf(display, sizeof(display), "HP+%d", value - 100);
			format = display;
		}
		ImGui::SliderInt(label, &value, 1, 125, format);
	}

	ImVec2 indicator_position(ImFont* font, const char* text) {
		constexpr float horizontal_offset = 5.f;
		constexpr float vertical_offset = 3.f;
		const ImVec2 screen = ImGui::GetIO().DisplaySize;
		const ImVec2 size = font->CalcTextSizeA(font->LegacySize, FLT_MAX, 0.f, text);
		return ImVec2(std::floor(screen.x * .5f + horizontal_offset), std::floor(screen.y * .5f - size.y - vertical_offset));
	}

	void bind_widget(const char* label, std::vector<int>& keys, int* mode = nullptr) {
		ImGui::TextUnformatted(label);
		ImGui::SameLine(230.f);
		ImGui::PushID(label);
		const bool capturing = keybind::is_capturing(&keys);
		std::string display = keybind::combo_name(capturing && !keybind::capture_buffer().empty() ? keybind::capture_buffer() : keys);
		if (capturing && keybind::capture_buffer().empty()) display = "Press keys...";
		if (ImGui::Button(display.c_str(), ImVec2(160.f, 0.f))) {
			if (capturing) keybind::stop_capture(); else keybind::start_capture(&keys);
		}
		if (ImGui::IsItemHovered()) keybind::set_hovered_block(&keys);
		if (mode && ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			if (capturing) keybind::stop_capture();
			ImGui::OpenPopup("bind_mode");
		}
		if (mode && ImGui::BeginPopup("bind_mode")) {
			if (ImGui::Selectable("Hold", *mode == cheat::weapon_cfg::hold))
				*mode = cheat::weapon_cfg::hold;
			if (ImGui::Selectable("Toggle", *mode == cheat::weapon_cfg::toggle))
				*mode = cheat::weapon_cfg::toggle;
			if (ImGui::Selectable("Always On", *mode == cheat::weapon_cfg::always))
				*mode = cheat::weapon_cfg::always;
			ImGui::EndPopup();
		}
		ImGui::PopID();
	}
}

namespace cheat {
	void style_set() {
		static bool initialized = false;
		if (initialized) return;
		auto& style = ImGui::GetStyle();
		style.WindowRounding = 0.f;
		style.FrameRounding = 0.f;
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.063f, 0.067f, 0.071f, 0.96f);
		initialized = true;
	}

	void c_menu::on_render() {
		style_set();
		if (!is_open) return;

		keybind::reset_hovered_block();
		ImGui::SetNextWindowSize(ImVec2(450.f, 360.f), ImGuiCond_Always);
		constexpr auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
		if (!ImGui::Begin("Weapon configuration", nullptr, flags)) { ImGui::End(); return; }

		if (ImGui::BeginTabBar("main_tabs")) {
			if (ImGui::BeginTabItem("Weapons")) {
				static int selected = 0;
				ImGui::Combo("##weapon_profile", &selected, weapon_cfg::profile_names.data(), static_cast<int>(weapon_cfg::profile_names.size()));
				auto& cfg = *weapon_cfg::profiles[static_cast<std::size_t>(selected)];
				if (selected != 0) ImGui::Checkbox("Enabled", &cfg.enabled.get());

				damage_slider("Min. damage", cfg.minimum_damage.get());
				damage_slider("Min. damage override", cfg.override_damage.get());
				bind_widget("Min. Damage Override", weapon_cfg::damage_override_key.get(), &weapon_cfg::damage_override_mode.get());
				bind_widget("Force Lethal", weapon_cfg::force_lethal_key.get(), &weapon_cfg::force_lethal_mode.get());
				bind_widget("Force Lethal Skeet Bind", weapon_cfg::force_lethal_skeet_key.get());

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Indicator")) {
				ImGui::Checkbox("Show indicator", &weapon_cfg::show_indicator.get());
				if (weapon_cfg::show_indicator.get())
					ImGui::Checkbox("Show if active", &weapon_cfg::show_indicator_if_active.get());
				static constexpr const char* fonts[] = { "Small", "Verdana" };
				ImGui::Combo("Indicator font", &weapon_cfg::indicator_font.get(), fonts, IM_ARRAYSIZE(fonts));
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Config")) {
				static char save_name[64] = "default";
				static std::string selected_config = "default";
				const auto configs = config::g_registry.list_configs();
				const auto selected_it = std::find(configs.begin(), configs.end(), selected_config);
				if (!configs.empty() && selected_it == configs.end())
					selected_config = configs.front();

				ImGui::TextUnformatted("New config name");
				ImGui::SetNextItemWidth(-1.f);
				ImGui::InputText("##save_name", save_name, sizeof(save_name));

				ImGui::TextUnformatted("Existing configs");
				if (ImGui::BeginChild("##config_list", ImVec2(0.f, 155.f), ImGuiChildFlags_Borders)) {
					for (const auto& name : configs) {
						const bool selected = name == selected_config;
						if (ImGui::Selectable(name.c_str(), selected))
							selected_config = name;
						if (selected)
							ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndChild();


				if (ImGui::Button("Create", ImVec2(90.f, 0.f))) {
					if (config::g_registry.create_config(save_name)) {
						selected_config = config::c_registry::sanitize_name(save_name);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Save", ImVec2(90.f, 0.f))) {
					config::g_registry.save_to_file(config::c_registry::file_for(selected_config));
				}
				ImGui::SameLine();
				if (ImGui::Button("Load", ImVec2(90.f, 0.f))) {
					config::g_registry.load_from_file(config::c_registry::file_for(selected_config));
				}
				ImGui::SameLine();
				if (ImGui::Button("Open folder", ImVec2(110.f, 0.f)))
					config::g_registry.open_folder();

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();

	}

	void c_menu::on_overlay() {
		weapon_cfg::update_binds();

		if (!weapon_cfg::game_active())
			return;

		if (!weapon_cfg::show_indicator.get())
			return;

		if (weapon_cfg::show_indicator_if_active.get() && !weapon_cfg::force_lethal_active() && !weapon_cfg::override_active())
			return;

		const char* text = weapon_cfg::damage_text();
		const int font_index = std::clamp(weapon_cfg::indicator_font.get(), 0, 1);
		auto* font = overlay::g_renderer.indicator_fonts[font_index];

		const int text_flags = font_index == 1 ? overlay::render_drop_shadow : overlay::render_outline;
		overlay::g_renderer.text(indicator_position(font, weapon_cfg::damage_text()), weapon_cfg::damage_text(), font,ImColor(1.f, 1.f, 1.f, 1.f), text_flags);
	}
}
