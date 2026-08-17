#include "../hooks.h"
#include "../../menu/menu.h"
#include "../../menu/weapon_config.h"
#include "../../menu/keybind.h"
#include "../../sdk/tools/imgui/renderer/renderer.h"
#include <cmath>
#include <sdk/offsets/offsets.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

LRESULT __stdcall cheat::hooks::ui::wnd_proc(const HWND hwnd, const UINT msg, const WPARAM wparam, const LPARAM lparam)
{
	const bool bind_captured = keybind::is_any_capturing();
	cheat::weapon_cfg::process_key_message(hwnd, msg, wparam, lparam);
    if (msg == WM_KEYUP && wparam == VK_HOME) {
        cheat::g_menu.is_open ^= 1;
        const auto original = hooks::client::originals::is_relative_mouse_mode.original<void* (*)(void* a1, bool active)>();
        if (original)
            original(sdk::g_offsets.interfaces.input_system, cheat::g_menu.is_open ? false : cheat::g_menu.input_active);
    }

    ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
	if (bind_captured && cheat::g_menu.is_open)
		return true;

    if (cheat::g_menu.is_open && (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_RBUTTONUP || msg == WM_RBUTTONDOWN))
        return false;

    return originals::wnd_proc.original<LRESULT(*)(const HWND, const UINT, const WPARAM, const LPARAM)>()(hwnd, msg, wparam, lparam);
}

HRESULT __stdcall cheat::hooks::ui::present(IDXGISwapChain* const swapChain, const std::uint32_t syncInterval, const std::uint32_t flags) 
{
    const auto original_present = originals::present.original<HRESULT(*)(IDXGISwapChain* const, const std::uint32_t, const std::uint32_t)>();
    if ((flags & DXGI_PRESENT_TEST) != 0)
        return original_present(swapChain, syncInterval, flags);

    auto& renderer = cheat::overlay::g_renderer;
    if (!renderer.process_frame(swapChain))
        return original_present(swapChain, syncInterval, flags);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();
    const ImGuiIO& io = ImGui::GetIO();

    const bool valid_display = std::isfinite(io.DisplaySize.x) && std::isfinite(io.DisplaySize.y) && io.DisplaySize.x >= 12.f && io.DisplaySize.y >= 7.f;

    if (valid_display) {
        cheat::g_menu.on_overlay();
        cheat::g_menu.on_render();
    }


    ImGui::Render();

    ImDrawData* const draw_data = ImGui::GetDrawData();
    if (draw_data && draw_data->CmdListsCount > 0 && draw_data->TotalVtxCount > 0) {
        ID3D11DeviceContext* const device_ctx = renderer.device_ctx();
        ID3D11RenderTargetView* const render_view = renderer.render_view();
        if (device_ctx && render_view) {
            device_ctx->OMSetRenderTargets(1u, &render_view, nullptr);
            ImGui_ImplDX11_RenderDrawData(draw_data);
        }
    }

    return original_present(swapChain, syncInterval, flags);
}

HRESULT __stdcall cheat::hooks::ui::resize_buffers(IDXGISwapChain* const swapChain, const std::uint32_t count, const std::uint32_t width, const std::uint32_t height, DXGI_FORMAT const format, const std::uint32_t flags, void* n1, void* n2) 
{
    const auto original_resize = originals::resize_buffers.original<HRESULT(*)(IDXGISwapChain* const, const std::uint32_t, const std::uint32_t, const std::uint32_t, DXGI_FORMAT const, const std::uint32_t, void*, void*)>();
    auto& renderer = cheat::overlay::g_renderer;
    if (renderer.swap_chain() != swapChain)
        return original_resize(swapChain, count, width, height, format, flags, n1, n2);

    renderer.release_render_target();
    const HRESULT result = original_resize(swapChain, count, width, height, format, flags, n1, n2);
    if (SUCCEEDED(result))
        renderer.process_frame(swapChain);
    return result;
}
