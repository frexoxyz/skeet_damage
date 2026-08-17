#include "hooks.h"
#include "../sdk/offsets/offsets.h"
#include "../menu/weapon_config.h"


bool cheat::hooks::initialize() {
	if (!hook_library::initialize()) {
		return false;
	}

	if (!ui::originals::wnd_proc.create(sdk::g_offsets.patterns.wnd_proc.get<void*>(), cheat::hooks::ui::wnd_proc) ||
		!ui::originals::present.create(sdk::g_offsets.patterns.dx11_present.get<void*>(), cheat::hooks::ui::present) ||
		!ui::originals::resize_buffers.create(sdk::g_offsets.patterns.dx11_resize_buffers.get<void*>(), cheat::hooks::ui::resize_buffers) || 
		!client::originals::mouse_input_enabled.create(memory::find_virtual(*sdk::g_offsets.patterns.input.resolve<void**>(), 23).get<void*>(), cheat::hooks::client::mouse_input_enabled) ||
		!client::originals::is_relative_mouse_mode.create(memory::find_virtual(sdk::g_offsets.interfaces.input_system, 76u).get<void*>(), cheat::hooks::client::is_relative_mouse_mode) ||
		!client::originals::on_render_start.create(memory::find_virtual(sdk::g_offsets.interfaces.view_render, 4).get<void*>(), cheat::hooks::client::on_render_start)) {
		shut_down();
		return false;
	}

	return true;
}

void cheat::hooks::shut_down() {
	cheat::weapon_cfg::release_binds();
    ui::originals::wnd_proc.remove();
    ui::originals::present.remove();
    ui::originals::resize_buffers.remove();

	client::originals::mouse_input_enabled.remove();
	client::originals::is_relative_mouse_mode.remove();
	client::originals::on_render_start.remove();

    hook_library::shut_down();
}
