#pragma once
#include "../sdk/tools/hook_library/hook_library.h"
#include "../sdk/classes/types.h"

namespace cheat {
	namespace hooks {
		namespace ui {
			namespace originals {
				inline hook_library::hook wnd_proc;
				inline hook_library::hook present;
				inline hook_library::hook resize_buffers;
			}
			LRESULT __stdcall wnd_proc(const HWND hwnd, const UINT msg, const WPARAM wparam, const LPARAM lparam);
			HRESULT __stdcall present(IDXGISwapChain* const swapChain, const std::uint32_t syncInterval, const std::uint32_t flags);
			HRESULT __stdcall resize_buffers(IDXGISwapChain* const swapChain, const std::uint32_t count, const std::uint32_t width, const std::uint32_t height, DXGI_FORMAT const format, const std::uint32_t flags, void* n1, void* n2);
		}
		namespace client {
			namespace originals {
				inline hook_library::hook mouse_input_enabled;
				inline hook_library::hook is_relative_mouse_mode;
				inline hook_library::hook on_render_start;

			}
			bool mouse_input_enabled(void* rcx);
			void* is_relative_mouse_mode(void* a1, bool active);
			void on_render_start(void* a1);

		}
		bool initialize();
		void shut_down();
	}
}
