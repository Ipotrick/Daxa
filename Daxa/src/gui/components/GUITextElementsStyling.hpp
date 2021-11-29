#pragma once

#include "GUITextElements.hpp"
#include "../base/GUIStyle.hpp"
namespace daxa {
	namespace gui {

		template<> inline void applyStyle(TextInputF64& self, const Style& style) 	{
			if (hasNANS(self.color)) self.color = Vec4(1, 1, 1, 1);
			if (hasNANS(self.colorFont)) self.colorFont = style.fontColor2;
			if (hasNANS(self.colorFontError)) self.colorFontError = style.negative;
			if (hasNANS(self.textPadding)) self.textPadding = Padding{ 0.0f, 0.0f, style.padding.left * 0.5f, style.padding.right * 0.5f };
			if (!self.font.holdsValue()) self.font = style.font;
			//if (!self.fontTexture.holdsValue()) self.fontTexture = style.fontTexture;
			if (std::isnan(self.fontSize)) self.fontSize = style.fontSize;
		}

		template<> inline void applyStyle(StaticText& self, const Style& style) {
			if (hasNANS(self.color)) self.color = style.fontColor1;
			if (!self.font.holdsValue()) self.font = style.font;
			//if (!self.fontTexture.holdsValue()) self.fontTexture = style.fontTexture;
			if (std::isnan(self.fontSize)) self.fontSize = style.fontSize;
		}

		template<> inline void applyStyle(Text& self, const Style& style) {
			if (hasNANS(self.color)) self.color = style.fontColor1;
			if (!self.font.holdsValue()) self.font = style.font;
			//if (!self.fontTexture.holdsValue()) self.fontTexture = style.fontTexture;
			if (std::isnan(self.fontSize)) self.fontSize = style.fontSize;
		}

		template<> inline void applyStyle(TextInput& self, const Style& style) {
			if (hasNANS(self.color)) self.color = style.fill2;
			if (!self.font.holdsValue()) self.font = style.font;
			//if (!self.fontTexture.holdsValue()) self.fontTexture = style.fontTexture;
			if (std::isnan(self.fontSize)) self.fontSize = style.fontSize;
		}
	}
}