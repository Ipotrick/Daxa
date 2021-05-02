#pragma once

#include "../../rendering/ImageManager.hpp"

#include "../font/FontManager.hpp"

#include "GUIElement.hpp"

namespace daxa {
	namespace gui {

		struct Style {
			Vec4 fill0;
			Vec4 fill1;
			Vec4 fill2;
			Vec4 accent0;
			Vec4 positive;
			Vec4 negative;
			f32 fontSize;
			Vec4 fontColor1;
			Vec4 fontColor2;
			Padding padding;
			f32 spacing;
			f32 cornerRounding;
			f32 scrollerWidth;
			FontHandle font;
			ImageHandle fontTexture;
		};

		template<CElement T>
		void applyStyle(T& element, const Style& style) {}
	}
}
