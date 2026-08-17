#include "gdi_font.h"
#include "../imgui_internal.h"

namespace {
	constexpr ImWchar k_text_glyph_ranges[] = {
		0x0020, 0x00FF, 0x0100, 0x017F, 0x0180, 0x024F, 0x0370, 0x03FF,
		0x0400, 0x052F, 0x0E00, 0x0E7F, 0x1100, 0x11FF, 0x1E00, 0x1EFF,
		0x2000, 0x206F, 0x2DE0, 0x2DFF, 0x3000, 0x30FF, 0x3130, 0x318F,
		0x31F0, 0x31FF, 0x3400, 0x4DBF, 0x4E00, 0x9FFF, 0xA640, 0xA69F,
		0xAC00, 0xD7A3, 0xFF00, 0xFFEF, 0xFFFD, 0xFFFD, 0
	};

	struct gdi_font_src_t { char face_name[LF_FACESIZE]{}; int weight = FW_NORMAL; int quality = NONANTIALIASED_QUALITY; };
	struct gdi_font_baked_t {
		HDC hdc{}; HBITMAP bitmap{}, old_bitmap{}; HFONT font{}, old_font{};
		unsigned char* bits{}; int width = 64, height = 64, pitch = 256; TEXTMETRICW tm{};
	};

	bool src_init(ImFontAtlas*, ImFontConfig* src) {
		if (!src->FontData || src->FontDataSize != sizeof(gdi_font_src_t)) return false;
		src->FontLoaderData = IM_NEW(gdi_font_src_t)(*static_cast<const gdi_font_src_t*>(src->FontData));
		return true;
	}
	void src_destroy(ImFontAtlas*, ImFontConfig* src) { IM_DELETE(static_cast<gdi_font_src_t*>(src->FontLoaderData)); src->FontLoaderData = nullptr; }
	bool contains_glyph(ImFontAtlas*, ImFontConfig*, ImWchar cp) { return cp >= 32; }

	bool baked_init(ImFontAtlas*, ImFontConfig* src, ImFontBaked* baked, void* data) {
		auto* cfg = static_cast<gdi_font_src_t*>(src->FontLoaderData);
		auto* bd = static_cast<gdi_font_baked_t*>(data);
		IM_PLACEMENT_NEW(bd) gdi_font_baked_t();
		BITMAPINFO bmi{}; bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); bmi.bmiHeader.biWidth = bd->width;
		bmi.bmiHeader.biHeight = -bd->height; bmi.bmiHeader.biPlanes = 1; bmi.bmiHeader.biBitCount = 32; bmi.bmiHeader.biCompression = BI_RGB;
		bd->hdc = CreateCompatibleDC(nullptr);
		if (!bd->hdc) return false;
		bd->bitmap = CreateDIBSection(bd->hdc, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&bd->bits), nullptr, 0);
		if (!bd->bitmap || !bd->bits) return false;
		wchar_t face[LF_FACESIZE]{}; MultiByteToWideChar(CP_ACP, 0, cfg->face_name, -1, face, LF_FACESIZE);
		bd->font = CreateFontW((std::max)(1, static_cast<int>(baked->Size + .5f)), 0, 0, 0, cfg->weight, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, cfg->quality, DEFAULT_PITCH | FF_DONTCARE, face);
		if (!bd->font) return false;
		bd->old_bitmap = static_cast<HBITMAP>(SelectObject(bd->hdc, bd->bitmap));
		bd->old_font = static_cast<HFONT>(SelectObject(bd->hdc, bd->font));
		SetBkMode(bd->hdc, OPAQUE); SetBkColor(bd->hdc, RGB(0, 0, 0)); SetTextColor(bd->hdc, RGB(255, 255, 255));
		GetTextMetricsW(bd->hdc, &bd->tm); baked->Ascent = static_cast<float>(bd->tm.tmAscent); baked->Descent = -static_cast<float>(bd->tm.tmDescent);
		return true;
	}

	void baked_destroy(ImFontAtlas*, ImFontConfig*, ImFontBaked*, void* data) {
		auto* bd = static_cast<gdi_font_baked_t*>(data);
		if (bd->hdc) { if (bd->old_font) SelectObject(bd->hdc, bd->old_font); if (bd->old_bitmap) SelectObject(bd->hdc, bd->old_bitmap); }
		if (bd->font) DeleteObject(bd->font); if (bd->bitmap) DeleteObject(bd->bitmap); if (bd->hdc) DeleteDC(bd->hdc); bd->~gdi_font_baked_t();
	}

	bool load_glyph(ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void* data, ImWchar cp, ImFontGlyph* glyph, float* advance_x) {
		auto* bd = static_cast<gdi_font_baked_t*>(data); if (!bd || !bd->hdc || cp < 32) return false;
		const wchar_t ch = static_cast<wchar_t>(cp); ABC abc{}; SIZE sz{};
		const bool has_abc = GetCharABCWidthsW(bd->hdc, cp, cp, &abc) != FALSE; GetTextExtentPoint32W(bd->hdc, &ch, 1, &sz);
		const int advance = has_abc ? abc.abcA + static_cast<int>(abc.abcB) + abc.abcC : sz.cx;
		if (advance_x) { *advance_x = static_cast<float>((std::max)(advance, 0)); return true; }
		if (!glyph) return false;
		RECT rc{ 0, 0, bd->width, bd->height }; FillRect(bd->hdc, &rc, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH))); TextOutW(bd->hdc, 0, 0, &ch, 1); GdiFlush();
		int min_x = bd->width, min_y = bd->height, max_x = -1, max_y = -1;
		for (int y = 0; y < bd->height; ++y) for (int x = 0; x < bd->width; ++x) if (bd->bits[y * bd->pitch + x * 4]) {
			min_x = (std::min)(min_x, x); min_y = (std::min)(min_y, y); max_x = (std::max)(max_x, x); max_y = (std::max)(max_y, y);
		}
		glyph->Codepoint = cp; glyph->AdvanceX = static_cast<float>((std::max)(advance, 0)); if (max_x < min_x || max_y < min_y) return true;
		const int w = max_x - min_x + 1, h = max_y - min_y + 1; const auto id = ImFontAtlasPackAddRect(atlas, w, h); if (id == ImFontAtlasRectId_Invalid) return false;
		auto* rect = ImFontAtlasPackGetRect(atlas, id); atlas->Builder->TempBuffer.resize(w * h); auto* alpha = atlas->Builder->TempBuffer.Data;
		for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) alpha[y * w + x] = bd->bits[(min_y + y) * bd->pitch + (min_x + x) * 4];
		glyph->X0 = static_cast<float>(min_x); glyph->Y0 = static_cast<float>(min_y); glyph->X1 = static_cast<float>(min_x + w); glyph->Y1 = static_cast<float>(min_y + h);
		glyph->Visible = true; glyph->PackId = id; ImFontAtlasBakedSetFontGlyphBitmap(atlas, baked, src, glyph, rect, alpha, ImTextureFormat_Alpha8, w); return true;
	}

	const ImFontLoader* loader() {
		static ImFontLoader value; static bool init{}; if (!init) { value.Name = "gdi_bitmap"; value.FontSrcInit = src_init; value.FontSrcDestroy = src_destroy;
			value.FontSrcContainsGlyph = contains_glyph; value.FontBakedInit = baked_init; value.FontBakedDestroy = baked_destroy; value.FontBakedLoadGlyph = load_glyph;
			value.FontBakedSrcLoaderDataSize = sizeof(gdi_font_baked_t); init = true; } return &value;
	}
}

ImFont* cheat::overlay::add_gdi_font(float size, const char* face_name, int weight, int quality) {
	ImFontConfig cfg{}; cfg.SizePixels = size; cfg.FontLoader = loader(); cfg.PixelSnapH = true; cfg.OversampleH = cfg.OversampleV = 1; cfg.GlyphRanges = k_text_glyph_ranges;
	auto* source = static_cast<gdi_font_src_t*>(IM_ALLOC(sizeof(gdi_font_src_t))); IM_PLACEMENT_NEW(source) gdi_font_src_t{};
	strncpy_s(source->face_name, face_name, _TRUNCATE); source->weight = weight; source->quality = quality;
	cfg.FontData = source; cfg.FontDataSize = sizeof(gdi_font_src_t); cfg.FontDataOwnedByAtlas = true;
	return ImGui::GetIO().Fonts->AddFont(&cfg);
}
