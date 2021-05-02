#pragma once

#include "../base/GUIElement.hpp"
#include "../font/FontManager.hpp"

namespace daxa {
	namespace gui {

		struct StaticText : IElement {
			std::function<void(StaticText&, u32)> onUpdate;
			std::string_view value;
			f32 fontSize{ NAN };
			Vec4 color{ UNSET_COLOR };
			FontHandle font;
			ImageHandle fontTexture;
		};
		namespace {
			struct _StaticText : public StaticText {
				_StaticText(StaticText&& e) : StaticText{ e } {}
				Vec2 lastDrawMinSize{ 0,0 };
			};
		}

		struct Text : IElement {
			std::function<void(Text&, u32)> onUpdate;
			ValueOrPtr<std::string> value{ "" };
			f32 fontSize{ NAN };
			Vec4 color{ UNSET_COLOR };
			FontHandle font;
			ImageHandle fontTexture;
		};
		namespace {
			struct _Text : public Text {
				_Text(Text&& e) : Text{ e } {}
				Vec2 lastDrawMinSize{ 0,0 };
			};
		}

		struct TextInput : IElement {
			std::function<void(TextInput&, u32)> onUpdate;
			std::function<void(std::string)> onStore;
			Vec2 size{ 100,20 };
			XAlign xalign{ XAlign::Left };
			YAlign yalign{ YAlign::Top };
			f32 fontSize{ NAN };
			Vec4 color{ UNSET_COLOR };
			FontHandle font;
			ImageHandle fontTexture;
			bool bClearOnEnter{ false };
		};
		namespace {
			struct _TextInput : public TextInput {
				_TextInput(TextInput&& e) : TextInput{ e } {}
				std::string str;
			};
		}

		struct TextInputF64 : IElement {
			std::function<void(TextInputF64&, u32)> onUpdate;
			f64* value{ nullptr };
			Vec2 size{ 100,20 };
			f32 fontSize{ NAN };
			Vec4 color{ UNSET_COLOR };
			Vec4 colorFont{ UNSET_COLOR };
			Vec4 colorFontError{ UNSET_COLOR };
			Padding textPadding{ NAN,NAN,NAN,NAN };
			FontHandle font;
			ImageHandle fontTexture;
		};
		namespace {
			struct _TextInputF64 : public TextInputF64 {
				_TextInputF64(TextInputF64&& e) : TextInputF64{ e } {}
				std::string str;
				f64 lastKnownValue{ 0.0f };
			};
		}
	}
}