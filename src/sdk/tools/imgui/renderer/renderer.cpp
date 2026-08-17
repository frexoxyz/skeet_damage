#include "renderer.h"
#include "gdi_font.h"
#include "../../../offsets/offsets.h"

#include <limits>

#define RELEASE_D3D( ptr ) \
    if ( ptr ) {           \
        ptr->Release( );   \
        ptr = nullptr;     \
    }

namespace {
	constexpr ImWchar indicator_ranges[] = { 0x0020, 0x00FF, 0x0400, 0x052F, 0 };

    ImVec4 get_clip_rect(const ImDrawList* const draw_list) {
        if (draw_list && draw_list->_ClipRectStack.Size > 0)
            return draw_list->_ClipRectStack.back();

        return ImVec4(-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX);
    }

    void render_text_fast(ImDrawList* const draw_list,ImFont* const font,const float font_size,const ImVec2& position,const ImColor color,const char* const text) {
        if (!draw_list || !font || font_size <= 0.f || !text || text[0] == '\0')
            return;

        font->RenderText(draw_list, font_size, position, color, get_clip_rect(draw_list), text, text + std::strlen(text));
    }
}

bool cheat::overlay::renderer::process() {
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

	auto& io = ImGui::GetIO();
	io.FontDefault = io.Fonts->AddFontDefault();

	AddFontResourceExA("C:\\Windows\\Fonts\\SMALLER.FON", FR_PRIVATE, nullptr);
	indicator_fonts[0] = add_gdi_font(9.f, "Small Fonts", FW_NORMAL, NONANTIALIASED_QUALITY);
	indicator_fonts[1] = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdanab.ttf", 11.f, nullptr, indicator_ranges);

    if (!ImGui_ImplWin32_Init(m_hwnd)) {
        ImGui::DestroyContext();
        return false;
    }
    if (!ImGui_ImplDX11_Init(m_device, m_device_ctx)) {
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    return true;
}

bool cheat::overlay::renderer::create_render_target() {
    if (m_render_view)
        return true;
    if (!m_swap_chain || !m_device)
        return false;

    ID3D11Texture2D* buffer{};
    if (FAILED(m_swap_chain->GetBuffer(0u, IID_PPV_ARGS(&buffer))))
        return false;

    const HRESULT result = m_device->CreateRenderTargetView(buffer, nullptr, &m_render_view);
    buffer->Release();
    return SUCCEEDED(result);
}

bool cheat::overlay::renderer::process_frame(IDXGISwapChain* const swap_chain) {
    if (!swap_chain)
        return false;

    if (m_initialized) {
        if (swap_chain != m_swap_chain)
            return false;
        if (m_render_view)
            return true;

        const bool created = create_render_target();
        return created;
    }

    DXGI_SWAP_CHAIN_DESC desc{};
    if (FAILED(swap_chain->GetDesc(&desc)) || !desc.OutputWindow)
        return false;

    ID3D11Device* device{};
    if (FAILED(swap_chain->GetDevice(IID_PPV_ARGS(&device))))
        return false;

    ID3D11DeviceContext* device_ctx{};
    device->GetImmediateContext(&device_ctx);
    if (!device_ctx) {
        device->Release();
        return false;
    }

    m_hwnd = desc.OutputWindow;
    m_swap_chain = swap_chain;
    m_device = device;
    m_device_ctx = device_ctx;

    if (!create_render_target() || !process()) {
        release_render_target();
        RELEASE_D3D(m_device_ctx);
        RELEASE_D3D(m_device);
        m_swap_chain = nullptr;
        m_hwnd = nullptr;
        return false;
    }

    m_initialized = true;
    return true;
}

void cheat::overlay::renderer::release_render_target() {
    if (m_device_ctx)
        m_device_ctx->OMSetRenderTargets(0u, nullptr, nullptr);
    RELEASE_D3D(m_render_view);
}

ID3D11DeviceContext* cheat::overlay::renderer::device_ctx() const {
    return m_device_ctx;
}

IDXGISwapChain* cheat::overlay::renderer::swap_chain() const {
    return m_swap_chain;
}

ID3D11RenderTargetView* cheat::overlay::renderer::render_view() const {
    return m_render_view;
}

ImDrawList* cheat::overlay::renderer::draw_list() const {
    return ImGui::GetBackgroundDrawList();
}

void cheat::overlay::renderer::text(const ImVec2 position, const char* const text, ImFont* const font, const ImColor color, const int flags) {
    const auto list = draw_list();
    if (!list || !font || !text || text[0] == '\0')
        return;

    const float size = font->LegacySize;

    const ImVec2 pos = position;

    const bool push = font->OwnerAtlas != nullptr;
    if (push)
        list->PushTexture(font->OwnerAtlas->TexRef);

    if (flags & (render_outline | render_drop_shadow)) {
        const ImColor shadow(0.f, 0.f, 0.f, color.Value.w);
        if (flags & render_outline) {
            for (int dx = -1; dx <= 1; ++dx)
                for (int dy = -1; dy <= 1; ++dy)
                    if (dx || dy)
                        render_text_fast(list, font, size, ImVec2(pos.x + (float)dx, pos.y + (float)dy), shadow, text);
        }
        else {
            render_text_fast(list, font, size, ImVec2(pos.x + 1.f, pos.y + 1.f), shadow, text);
        }
    }

    render_text_fast(list, font, size, pos, color, text);

    if (push)
        list->PopTexture();
}
