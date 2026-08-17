#pragma once

namespace cheat {
	class c_menu {
	public:
		bool is_open{};
		bool input_active{};

		void on_render();
		void on_overlay();
	} inline g_menu;
}
