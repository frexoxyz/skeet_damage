#pragma once

#undef small

#define IMGUI_DISABLE_DEBUG_TOOLS
#define IMGUI_DISABLE_WIN32_FUNCTIONS
#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#define IMGUI_IMPL_WIN32_DISABLE_LINKING_XINPUT
#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
#define IMGUI_DEFINE_MATH_OPERATORS

#include <windows.h>
#include <mutex>
#include <winternl.h>
#include <filesystem>
#include <random>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <optional>
#include <array>
#include <fstream>
#include <sstream>
#include <ostream>
#include <iostream>
#include <bitset>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <cstdint>
#include <minwindef.h>
#include <string>
#include <unordered_set>
#include <cstddef>

// DirectX
#include <d3d11.h>

#include "sdk/tools/imgui/impl/imgui_impl_dx11.h"
#include "sdk/tools/imgui/impl/imgui_impl_win32.h"
#include "sdk/tools/imgui/imgui.h"

#include "sdk/tools/fnv1a.h"
