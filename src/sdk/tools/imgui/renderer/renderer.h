#pragma once
#include <vector>

#undef small

namespace cheat {
    namespace overlay {
        enum render_flags : int {
            render_none        = 0,
            render_outline     = 1 << 0,
            render_drop_shadow = 1 << 1,
        };

        class renderer {
        private:
            bool process();
            bool create_render_target();
            ID3D11Device* m_device{};
            ID3D11DeviceContext* m_device_ctx{};
            ID3D11RenderTargetView* m_render_view{};
            IDXGISwapChain* m_swap_chain{};
            HWND m_hwnd{};
            bool m_initialized{};
        public:
			ImFont* indicator_fonts[2]{};

            bool process_frame(IDXGISwapChain* const swap_chain);
            void release_render_target();

            ID3D11DeviceContext* device_ctx() const;
            IDXGISwapChain* swap_chain() const;
            ID3D11RenderTargetView* render_view() const;
            ImDrawList* draw_list() const;

            void text(ImVec2 position, const char* text, ImFont* font, ImColor color, int flags = render_none);
    
        } inline g_renderer;

    }
}
