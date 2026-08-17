#pragma once

// Production build: keep only runtime functionality used by the overlay.
#define IMGUI_DISABLE_DEBUG_TOOLS
#define IMGUI_DISABLE_DEMO_WINDOWS
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_TTY_FUNCTIONS
#define IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS

#ifndef IMGUI_PERF_SLIM_WIDGETS
#define IMGUI_PERF_SLIM_WIDGETS 1
#endif

// Features excluded from this overlay. Override with /D...=1 before including
// imgui.h when a future build needs one of them again.
#ifndef IMGUI_PERF_ENABLE_GAMEPAD
#define IMGUI_PERF_ENABLE_GAMEPAD 0
#endif

#ifndef IMGUI_PERF_ENABLE_DRAG_DROP
#define IMGUI_PERF_ENABLE_DRAG_DROP 0
#endif

#ifndef IMGUI_PERF_ENABLE_TOOLTIPS
#define IMGUI_PERF_ENABLE_TOOLTIPS 0
#endif

#ifndef IMGUI_PERF_ENABLE_IME
#define IMGUI_PERF_ENABLE_IME 0
#endif

#ifndef IMGUI_PERF_ENABLE_CLIPBOARD
#define IMGUI_PERF_ENABLE_CLIPBOARD 0
#endif

#ifndef IMGUI_PERF_ENABLE_INI_SETTINGS
#define IMGUI_PERF_ENABLE_INI_SETTINGS 0
#endif

#ifndef IMGUI_PERF_ENABLE_ERROR_RECOVERY
#define IMGUI_PERF_ENABLE_ERROR_RECOVERY 0
#endif

// Internal lifecycle hooks are unused by this overlay. Keep a switch for
// integrations which explicitly register ImGuiContextHook callbacks.
#ifndef IMGUI_PERF_ENABLE_CONTEXT_HOOKS
#define IMGUI_PERF_ENABLE_CONTEXT_HOOKS 0
#endif

// The hook never owns the game's OS cursor and does not request nav-driven
// mouse warping. Mouse position/buttons/wheel input remain fully enabled.
#ifndef IMGUI_PERF_ENABLE_MOUSE_CURSOR_CHANGE
#define IMGUI_PERF_ENABLE_MOUSE_CURSOR_CHANGE 0
#endif

#if !IMGUI_PERF_ENABLE_CLIPBOARD
#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
#endif

#if !IMGUI_PERF_ENABLE_IME
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#endif

// Release builds keep the return-on-error safety paths, but omit diagnostic
// logging/assert work from user-error checks.
#if defined(NDEBUG) && !IMGUI_PERF_ENABLE_ERROR_RECOVERY
#define IM_ASSERT_USER_ERROR(_EXPR, _MSG)            ((void)0)
#define IM_ASSERT_USER_ERROR_RET(_EXPR, _MSG)        do { if (!(_EXPR)) return; } while (0)
#define IM_ASSERT_USER_ERROR_RETV(_EXPR, _RETV, _MSG) do { if (!(_EXPR)) return (_RETV); } while (0)
#endif

#if !IMGUI_PERF_ENABLE_GAMEPAD
#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#endif

// Preload the fixed Latin/Cyrillic set, then disallow runtime glyph baking.
#ifndef IMGUI_PERF_STATIC_FONT_GLYPHS
#define IMGUI_PERF_STATIC_FONT_GLYPHS 1
#endif

// Keep peak-sized window/draw buffers instead of periodically compacting and
// reallocating them. This trades bounded retained memory for stable frame time.
#ifndef IMGUI_PERF_RETAIN_TRANSIENT_BUFFERS
#define IMGUI_PERF_RETAIN_TRANSIENT_BUFFERS 1
#endif

#ifndef IMGUI_PERF_ENABLE_SIMD_CRC
#define IMGUI_PERF_ENABLE_SIMD_CRC 1
#endif

// Strict hot-path optimizations preserve the stock pixels and input behavior.
// Disable this switch to build an A/B baseline from the same source tree.
#ifndef IMGUI_PERF_STRICT_OPTIMIZATIONS
#define IMGUI_PERF_STRICT_OPTIMIZATIONS 1
#endif

// Avoid first-use buffer reallocations for medium/large menus. These are
// capacities, not per-frame allocations (about 1.5 MiB with 16-bit indices).
#ifndef IMGUI_PERF_DX11_INITIAL_VERTEX_CAPACITY
#define IMGUI_PERF_DX11_INITIAL_VERTEX_CAPACITY 65536
#endif

#ifndef IMGUI_PERF_DX11_INITIAL_INDEX_CAPACITY
#define IMGUI_PERF_DX11_INITIAL_INDEX_CAPACITY 131072
#endif
