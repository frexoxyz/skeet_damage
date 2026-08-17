#pragma once
#include "../imgui.h"

namespace cheat::overlay {
	ImFont* add_gdi_font(float size, const char* face_name, int weight, int quality);
}
