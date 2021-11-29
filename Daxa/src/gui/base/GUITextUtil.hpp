#pragma once
#include "../../rendering/UISprite.hpp"

#include "../font/FontManager.hpp"

#include "GUIDrawContext.hpp"

namespace daxa {
	namespace gui {

		struct TextContext : public DrawContext {
			TextContext(DrawContext const& super) : DrawContext{ super } {}
			f32 fontSize;
			Vec4 color{ 0,0,0,1 };
			//ImageHandle tex;
			Font const* font{ nullptr };
			bool bPixelPerfectAlignment{ false };
		};

		inline Vec2 minsizeFontText(const char* str, TextContext const& context) 	{
			const Font& FONT = *context.font;
			f32 SCALED_FONT_SIZE = context.fontSize * context.scale;

			f32 currentRowWidth{ 0.0f };
			f32 maxRowWidth{ 0.0f };
			f32 rowCount{ 1.0f };
			for (const char* iter = str; *iter != 0x00; ++iter) {
				if (*iter == '\n') {
					currentRowWidth = 0.0f;
					rowCount += 1;
				}
				else {
					const Glyph scaledGlyph = scaleGlyph(FONT.codepointToGlyph.get(*iter), 4096, 4096);
					currentRowWidth += scaledGlyph.advance * SCALED_FONT_SIZE;
					if (context.bPixelPerfectAlignment) {
						currentRowWidth = std::ceil(maxRowWidth - 0.1f);
					}
					maxRowWidth = std::max(maxRowWidth, currentRowWidth);
				}
			}
			return { maxRowWidth, rowCount * SCALED_FONT_SIZE };
		}

		/**
		 * \param str string in UTF-8 coding (CURRENTLY ONLY TAKING ASCII (TODO ADD UTF-8 SUPPORT)) to be drawn
		 * \param context containing all relevant drawing information refering to the context
		 * \param out buffer to write sprites to
		 * \return first: number of codepoints processed in the string; second: unscaled bounding size of the drawn text.
		 */
		inline std::pair<u32, Vec2> drawFontText(const char* str, TextContext const& context, std::vector<UISprite>& out) 	{
			if (str == nullptr || str[0] == 0x00) return { 0,{} };

			const Font& FONT = *context.font;

			f32 SCALED_FONT_SIZE = context.fontSize * context.scale;
			if (context.bPixelPerfectAlignment) {
				SCALED_FONT_SIZE = std::round(SCALED_FONT_SIZE);
			}

			const f32 MAX_X = context.bottomright.x + 0.2f;
			const f32 MIN_Y = context.bottomright.y - 0.2f;

			const Glyph DUMMY;

			const f32 TEXT_UPLIFT = SCALED_FONT_SIZE / 4.0f;

			static std::vector<f32> rowWidths;
			rowWidths.clear();
			rowWidths.push_back(0);
			f32 maxRowWidth{ 0.0f };
			rowWidths.reserve(10);
			for (const char* iter = str; *iter != 0x00; ++iter) {
				if (*iter == '\n') {
					rowWidths.push_back(0);
				}
				else {
					const Glyph scaledGlyph = scaleGlyph(FONT.codepointToGlyph.get(*iter), 4096, 4096);
					if (rowWidths.back() + scaledGlyph.advance * SCALED_FONT_SIZE < MAX_X) {
						rowWidths.back() += scaledGlyph.advance * SCALED_FONT_SIZE;
						if (context.bPixelPerfectAlignment) {
							rowWidths.back() = std::ceil(rowWidths.back() - 0.1f);
						}
					}
					maxRowWidth = std::max(maxRowWidth, rowWidths.back());
				}
			}
			const f32 SCALED_COMPLETE_HEIGHT = rowWidths.size() * SCALED_FONT_SIZE;

			const f32 REMAINING_Y_SIZE = std::max(context.size().y - SCALED_COMPLETE_HEIGHT, 0.0f);
			f32 CURSOR_STARTING_Y = context.topleft.y - SCALED_FONT_SIZE;
			if (context.yalign != YAlign::Top) {
				CURSOR_STARTING_Y -= REMAINING_Y_SIZE * (context.yalign == YAlign::Center ? 0.5f : 1.0f);
			}

			auto getStartingCursor = [&](u32 rowIndex) {
				const f32 REMAINING_X_SIZE_IN_ROW = std::max(context.size().x - rowWidths[rowIndex], 0.0f);
				f32 CURSOR_STARTING_X = context.topleft.x;
				if (context.xalign != XAlign::Left) {
					CURSOR_STARTING_X += REMAINING_X_SIZE_IN_ROW * (context.xalign == XAlign::Center ? 0.5f : 1.0f);
				}

				return Vec2{ CURSOR_STARTING_X, CURSOR_STARTING_Y - rowIndex * SCALED_FONT_SIZE };
			};

			Vec2 cursor = getStartingCursor(0);
			u32 colIndex{ 0 };
			u32 rowIndex{ 0 };
			u32 charsProcessed{ 0 };
			auto attemptLinebreak = [&]() -> bool {
				if (cursor.y - SCALED_FONT_SIZE > MIN_Y) {
					rowIndex += 1;
					if (rowIndex >= rowWidths.size()) return false;
					cursor = getStartingCursor(rowIndex);
					return true;
				}
				return false;
			};
			for (const char* iter = str; *iter != 0x00; ++iter) {
				if (context.bPixelPerfectAlignment) {
					cursor = ceil(cursor - Vec2{ 0.1f,0.1f });
				}
				if (*iter == '\n') {
					if (!attemptLinebreak()) break;
					colIndex = 0;
				}
				else {
					colIndex++;
					const Glyph scaledGlyph = scaleGlyph(FONT.codepointToGlyph.get(*iter), 4096, 4096);

					if (scaledGlyph.advance * SCALED_FONT_SIZE + cursor.x < MAX_X) {

						Vec2 size = SCALED_FONT_SIZE *
							Vec2{
								scaledGlyph.planeBounds.right - scaledGlyph.planeBounds.left,
								scaledGlyph.planeBounds.top - scaledGlyph.planeBounds.bottom
						};
						Vec2 place = cursor + Vec2{ 0, TEXT_UPLIFT }/* move text up by 25% */ +
							Vec2{
								SCALED_FONT_SIZE * scaledGlyph.planeBounds.left + size.x * 0.5f,
								SCALED_FONT_SIZE * scaledGlyph.planeBounds.bottom + size.y * 0.5f
						};
						out.push_back(
							UISprite{
								.color = context.color,
								.position = Vec3{ place, context.renderDepth },
								.scale = size,
								//.imageIndex = context.tex.getIndex(),
								.texMin = Vec2{scaledGlyph.atlasBounds.left, scaledGlyph.atlasBounds.bottom},
								.texMax = Vec2{scaledGlyph.atlasBounds.right, scaledGlyph.atlasBounds.top},
								.clipMin = context.clipMin,
								.clipMax = context.clipMax,
								.isMSDF = true,
								.drawMode = RenderSpace2d::Pixel,
							}
						);

						cursor.x += SCALED_FONT_SIZE * scaledGlyph.advance;
					}
					else {
						if (!attemptLinebreak()) break;
					}
				}
				charsProcessed += 1;
			}
			return { charsProcessed, minsizeFontText(str, context) / context.scale };
		}
	}
}