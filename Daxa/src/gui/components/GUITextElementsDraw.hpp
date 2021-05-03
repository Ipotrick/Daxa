#pragma once

#include <charconv>

#include "GUICommonElementsDraw.hpp"
#include "../../platform/Window.hpp"
namespace daxa {
	namespace gui {

		template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, _TextInput& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			return self.size;
		}
		template<> inline void onDraw(Manager& manager, _TextInput& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			const auto scaledCompleteSize = manager.minsizes[id] * context.scale;
			const Vec2 centerPlace = getPlace(scaledCompleteSize, context);

			if (manager.isCursorOver(centerPlace, scaledCompleteSize, context))
				manager.requestMouseEvent(id, context.root, context.renderDepth);

			Vec4 color = self.color;
			const bool focused = manager.focusedTextInput.first == id;
			if (focused) {
				// we are the focused text input
				color += Vec4{ 0.1,0.1,0.1,0.1 };
				if (manager.window->keyJustPressed(Scancode::ESCAPE)) {
					manager.focusedTextInput = { INVALID_ELEMENT_ID, {} };
				}

				auto keyEvents = manager.window->getKeyEventsInOrder();

				bool shift = manager.window->keyPressed(Scancode::LSHIFT);

				for (auto e : keyEvents) {
					if (e.type == KeyEvent::Type::JustPressed || e.type == KeyEvent::Type::Repeat) {
						if (e.scancode == Scancode::BACKSPACE) {
							if (self.str.size() > 0) self.str.pop_back();
						}
						else if (e.scancode == Scancode::RETURN) {
							if (shift) {
								self.str += '\n';
							}
							else {
								manager.focusedTextInput = { INVALID_ELEMENT_ID, {} };
								self.onStore(self.str);
								if (self.bClearOnEnter) self.str.clear();
							}
						}
						else if (e.scancode == Scancode::DELETE) {
							self.str.clear();
						}
						else {
							auto opt = scanCodeToChar(e.scancode, shift);
							if (opt.has_value()) {
								self.str += opt.value();
							}
						}
						manager.window->hideKey(e.scancode);
					}
				}
			}
			out.push_back(UISprite{
				.color = color,
				.position = Vec3{centerPlace, context.renderDepth},
				.scale = scaledCompleteSize,
				.clipMin = context.clipMin,
				.clipMax = context.clipMax,
				.cornerRounding = 3.0f * context.scale,
				.drawMode = RenderSpace2d::Pixel
				}
			);

			TextContext textContext{ context };
			textContext.xalign = self.xalign;
			textContext.yalign = self.yalign;
			textContext.fontSize = self.fontSize;
			textContext.color = Vec4{ 1,1,1,1 };
			textContext.tex = self.fontTexture;
			textContext.font = &*self.font.get();
			fit(textContext, scaledCompleteSize, centerPlace);
			auto [charsDrawn, size] = drawFontText(self.str.data(), textContext, out);

			if (self.str.size() > charsDrawn) /* we stopped char processing cause the string is too long, we need to remove the last few chars */ {
				self.str = self.str.substr(0, charsDrawn);
			}
		}

		template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, _StaticText& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			return self.lastDrawMinSize;
		}
		template<> inline void onDraw(Manager& manager, _StaticText& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			TextContext textContext{ context };
			textContext.fontSize = self.fontSize;
			textContext.color = self.color;
			textContext.tex = self.fontTexture;
			textContext.font = &*self.font.get();
			auto [charsprocessed, size] = drawFontText(self.value.data(), textContext, out);
			self.lastDrawMinSize = size;
		}

		template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, _Text& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			return self.lastDrawMinSize;
		}
		template<> inline void onDraw(Manager& manager, _Text& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			TextContext textContext{ context };
			textContext.fontSize = self.fontSize;
			textContext.color = self.color;
			textContext.tex = self.fontTexture;
			textContext.font = &*self.font.get();
			auto [charsprocessed, unscaledMinSize] = drawFontText(std::string(self.value).data(), textContext, out);
			self.lastDrawMinSize = unscaledMinSize;
		}

		template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, _TextInputF64& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			return self.size;
		}
		template<> inline void onDraw(Manager& manager, _TextInputF64& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			const Vec2 scaledCompleteSize = manager.minsizes[id] * context.scale;
			const Vec2 centerPlace = getPlace(scaledCompleteSize, context);

			if (manager.isCursorOver(centerPlace, scaledCompleteSize, context)) {
				manager.requestMouseEvent(id, context.root, context.renderDepth);
			}

			Vec4 color = self.color;
			bool focused = manager.focusedTextInput.first == id;
			if (focused) {
				// we are the focused text input
				if (manager.window->keyJustPressed(Scancode::ESCAPE)) {
					manager.focusedTextInput = { INVALID_ELEMENT_ID, {} };
				}

				auto keyEvents = manager.window->getKeyEventsInOrder();

				const bool shift = manager.window->keyPressed(Scancode::LSHIFT);

				for (auto e : keyEvents) {
					if (e.type == KeyEvent::Type::JustPressed || e.type == KeyEvent::Type::Repeat) {
						if (e.scancode == Scancode::BACKSPACE) {
							if (self.str.size() > 0) self.str.pop_back();
						}
						else if (e.scancode == Scancode::RETURN) {
							manager.focusedTextInput = { INVALID_ELEMENT_ID, {} };
							focused = false;
							break;
						}
						else if (e.scancode == Scancode::DELETE) {
							self.str.clear();
						}
						else {
							auto opt = scanCodeToChar(e.scancode, shift);
							if (opt.has_value()) {
								self.str += opt.value();
							}
						}
						manager.window->hideKey(e.scancode);
					}
				}
			}

			char* p = nullptr;
			f64 value = std::strtod(self.str.data(), &p);
			const bool bTextValid = p == self.str.data() + self.str.size() && self.str.size() > 0;
			if (!focused) /* this cannot be the else of the upper if statement as the variable 'focused' could change in the upper if statement */ {
				color.x *= 0.8f;
				color.y *= 0.8f;
				color.z *= 0.8f;
				if (self.value) {
					if (*self.value != self.lastKnownValue) /* value changed from outside, we take that value into the text */ {
						self.str = std::to_string(*self.value);
					}
					else if (bTextValid && value != *self.value) /* we changed the text so we write it to the value */ {
						*self.value = value;
					}
					else /* set text to the value (makes formating with .0's prettier) */ {
						self.str = std::to_string(*self.value);
					}
					self.lastKnownValue = *self.value;
				}
			}

			out.push_back(UISprite{
				.color = bTextValid ? color : self.colorFontError,
				.position = Vec3{centerPlace, context.renderDepth},
				.scale = scaledCompleteSize,
				.clipMin = context.clipMin,
				.clipMax = context.clipMax,
				.cornerRounding = 3.0f * context.scale,
				.drawMode = RenderSpace2d::Pixel
				}
			);

			TextContext textContext{ context };
			textContext.xalign = XAlign::Center;
			textContext.yalign = YAlign::Center;
			textContext.fontSize = self.fontSize;
			textContext.color = self.colorFont;
			textContext.tex = self.fontTexture;
			textContext.font = &*self.font.get();
			fit(textContext, scaledCompleteSize, centerPlace);
			fit(textContext, self.textPadding);
			auto [charsProcessed, size] = drawFontText(self.str.c_str(), textContext, out);

			if (self.str.size() > charsProcessed) /* we stopped char processing cause the string is too long, we need to remove the last few chars */ {
				self.str = self.str.substr(0, charsProcessed);
			}
		}
	}
}