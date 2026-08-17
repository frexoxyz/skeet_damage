#pragma once
#include "../tools/memory/memory.h"

namespace sdk {
	namespace interfaces {
		class schema_system;
		class entity_system;
		class engine;
	}

	class c_offsets
	{
	public:
		bool failed_pattern{};
		void init_pattern(HMODULE module, memory::address_t& out, const char* name, const char* pattern);

		class c_modules
		{
		public:
			HMODULE client{};
			HMODULE engine{};
			HMODULE schema_system{};
			HMODULE game_overlay_renderer{};
			HMODULE input_system{};

			bool initialize();
		} modules;

		class c_patterns {
		public:
			memory::address_t wnd_proc{};
			memory::address_t dx11_present{};
			memory::address_t dx11_resize_buffers{};
			memory::address_t input;
			memory::address_t view_render;
			memory::address_t entity_system;
			memory::address_t get_base_entity;
			memory::address_t split_screen_view_pawn;

			bool initialize();
		} patterns;

		class c_interfaces {
		public:
			sdk::interfaces::entity_system* entity_system;
			sdk::interfaces::schema_system* schema_system;
			sdk::interfaces::engine* engine;
			void* input_system;
			void* view_render;

			bool initialize();
		} interfaces;

		bool initialize();
	} inline g_offsets;
}
