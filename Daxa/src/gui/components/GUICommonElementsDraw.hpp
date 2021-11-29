#pragma once

#include "GUIDraw.hpp"
namespace daxa {
	namespace gui {

		template<> inline Vec2 updateAndGetMinsize<Box>(Manager& manager, u32 id, Box& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			Vec2 minsize;
			if (self.child != INVALID_ELEMENT_ID) {
				minsize = updateAndGetMinsize(manager, self.child);
			}
			minsize += size(self.padding);
			minsize = max(minsize, self.minsize);
			return minsize;
		}
		template<> inline void onDraw<Box>(Manager& manager, Box& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			const Vec2 scaledSize = (self.bFillSpace ? context.size() : manager.minsizes[id] * context.scale);
			const Vec2 place = getPlace(scaledSize, context);

			//ImageSectionHandle& tex = static_cast<ImageSectionHandle&>(self.texture);
			auto [texMin, texMax] = self.bScreenTexture ?
				getScreenTextureMinMax(place, scaledSize, context.renderSpace, manager.coordSys) :
				//std::pair<Vec2, Vec2>{ tex.min, tex.max };

			if (static_cast<Vec4&>(self.color).w != 0.0f) {
				out.push_back(
					UISprite{
						.color = self.color,
						.position = Vec3{place, context.renderDepth},
						.scale = scaledSize,
						.imageIndex = tex.handle.getIndex(),
						.texMin = texMin,
						.texMax = texMax,
						.clipMin = context.clipMin,
						.clipMax = context.clipMax,
						.cornerRounding = self.cornerRounding * context.scale,
						.drawMode = RenderSpace2d::Pixel
					}
				);
			}

			if (self.bDragable) {
				if (manager.isCursorOver(place, scaledSize, context)) {
					manager.requestMouseEvent(id, context.root, context.renderDepth);
				}
			}

			if (self.child != INVALID_ELEMENT_ID) {
				auto childcontext = context;
				childcontext.xalign = self.xalign;
				childcontext.yalign = self.yalign;
				fit(childcontext, scaledSize, place);
				fit(childcontext, self.padding);
				draw(manager, manager.elements[self.child], self.child, childcontext, out);
			}
		}

		template<> inline Vec2 updateAndGetMinsize<_Button>(Manager& manager, u32 id, _Button& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			Vec2 minsize = self.size;
			if (self.child != INVALID_ELEMENT_ID) {
				minsize = max(minsize, updateAndGetMinsize(manager, self.child));
			}
			return minsize;
		}
		template<> inline void onDraw<_Button>(Manager& manager, _Button& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			auto scaledSize = manager.minsizes[id] * context.scale;
			auto place = getPlace(scaledSize, context);

			if (manager.isCursorOver(place, scaledSize, context)) {
				manager.requestMouseEvent(id, context.root, context.renderDepth);
			}
			else {
				self.bHold = false;
				self.bHover = false;
			}

			const Vec4 colorAddition = self.bHover ? Vec4{ 0.1,0.1,0.1,0 } : Vec4{};

			ImageSectionHandle& tex = static_cast<ImageSectionHandle&>(self.texture);
			auto [texMin, texMax] = self.bScreenTexture ?
				getScreenTextureMinMax(place, scaledSize, context.renderSpace, manager.coordSys) :
				std::pair<Vec2, Vec2>{ tex.min, tex.max };

			out.push_back(
				UISprite{
					.color = (self.bHold ? self.holdColor : self.color) + colorAddition,
					.position = Vec3{place, context.renderDepth },
					.scale = scaledSize,
					.imageIndex = tex.handle.getIndex(),
					.texMin = texMin,
					.texMax = texMax,
					.clipMin = context.clipMin,
					.clipMax = context.clipMax,
					.cornerRounding = 3.0f * context.scale,
					.drawMode = RenderSpace2d::Pixel
				}
			);


			if (self.child != INVALID_ELEMENT_ID) {
				const bool validChild = manager.isOneOf<Text, StaticText>(self.child);
				assert(validChild);
				auto childcontext = context;
				fit(childcontext, scaledSize, place);
				childcontext.xalign = XAlign::Center;
				childcontext.yalign = YAlign::Center;
				//childcontext.scale *= extraSizeScale;
				draw(manager, manager.elements[self.child], self.child, childcontext, out);
			}
		}

		template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, _Checkbox& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			return self.size;
		}
		template<> inline void onDraw<_Checkbox>(Manager& manager, _Checkbox& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			Vec2 scaledSize = self.size * context.scale;
			Vec2 place = getPlace(scaledSize, context);

			f32 HOVER_SCALING{ 0.9f };

			if (self.bHover) {
				scaledSize *= HOVER_SCALING;
			}

			out.push_back(
				UISprite{
					.color = self.color,
					.position = Vec3{place, context.renderDepth},
					.scale = scaledSize,
					.clipMin = context.clipMin,
					.clipMax = context.clipMax,
					.cornerRounding = 2.0f * context.scale,
					.drawMode = RenderSpace2d::Pixel
				}
			);

			if (self.value) {
				if (manager.isCursorOver(place, scaledSize, context)) {
					manager.requestMouseEvent(id, context.root, context.renderDepth);
				}
				else {
					self.bHold = false;
					self.bHover = false;
				}

				f32 smallerDiemnsion = std::min(scaledSize.x, scaledSize.y);

				smallerDiemnsion *= 0.25f;
				scaledSize -= Vec2{ smallerDiemnsion ,smallerDiemnsion };

				out.push_back(
					UISprite{
						.color = (*self.value ? self.colorEnabled : self.colorDisabled),
						.position = Vec3{place, context.renderDepth },
						.scale = scaledSize,
						.clipMin = context.clipMin,
						.clipMax = context.clipMax,
						.cornerRounding = 1.5f * context.scale,
						.drawMode = RenderSpace2d::Pixel
					}
				);
			}
		}

		template<> inline Vec2 updateAndGetMinsize<SliderF64>(Manager& manager, u32 id, SliderF64& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			if (self.child != INVALID_ELEMENT_ID) updateAndGetMinsize(manager, self.child);
			return self.size;
		}
		template<> inline void onDraw<SliderF64>(Manager& manager, SliderF64& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			const Vec2 responsiveArea = self.size * context.scale;
			const Vec2 place = getPlace(responsiveArea, context);
			const Vec2 scaledBarSize = responsiveArea * (self.bThin ? (self.bVertical ? Vec2{ 0.225f, 1.0f } : Vec2{ 1.0f, 0.225f }) : Vec2{ 1,1 });
			const f32 barThiccness = std::min(scaledBarSize.x, scaledBarSize.y);

			const f32 biggerInidcatorSize = std::min(responsiveArea.x, responsiveArea.y);
			const f32 smallerInidcatorSize = biggerInidcatorSize * (self.bThin ? 1.0f : 0.2f);
			const Vec2 indicatorSize2 = (self.bVertical ? Vec2{ biggerInidcatorSize, smallerInidcatorSize } : Vec2{ smallerInidcatorSize, biggerInidcatorSize });

			const bool rangeOK{ self.max > self.min };
			// Draw bar:
			out.push_back(
				UISprite{
					.color = rangeOK ? self.colorBar : self.colorError,
					.position = Vec3{place, context.renderDepth},
					.scale = scaledBarSize,
					.clipMin = context.clipMin,
					.clipMax = context.clipMax,
					.cornerRounding = biggerInidcatorSize * 0.2f * 0.5f,
					.drawMode = RenderSpace2d::Pixel
				}
			);

			const auto [isCursorOver, cursor] = manager.isCursorOver(place, responsiveArea, context);

			//const Vec2 cursor = manager.coordSys.convertCoordSys(manager.window->getCursorPos(), RenderSpace2d::Window, RenderSpace2d::Pixel);
			if (isCursorOver) {
				manager.requestMouseEvent(id, context.root, context.renderDepth);
			}

			auto drawInvalidSlider = [&]() {
				out.push_back(
					UISprite{
						.color = rangeOK ? self.colorBar : self.colorError,
						.position = Vec3{place, context.renderDepth},
						.scale = indicatorSize2,
						.clipMin = context.clipMin,
						.clipMax = context.clipMax,
						.cornerRounding = smallerInidcatorSize * 0.5f,
						.drawMode = RenderSpace2d::Pixel
					}
				);
			};

			if (self.value) {
				const f32 sliderMinPosMainAxis = (self.bVertical ? place.y - scaledBarSize.y * 0.5f : place.x - scaledBarSize.x * 0.5f) + smallerInidcatorSize * 0.5f;
				const f32 sliderMaxPosMainAxis = (self.bVertical ? place.y + scaledBarSize.y * 0.5f : place.x + scaledBarSize.x * 0.5f) - smallerInidcatorSize * 0.5f;
				const f32 clampedCursorPos = std::clamp((self.bVertical ? cursor.y : cursor.x), sliderMinPosMainAxis, sliderMaxPosMainAxis);
				const f32 relativeCursorPos = (clampedCursorPos - sliderMinPosMainAxis) / (sliderMaxPosMainAxis - sliderMinPosMainAxis);;

				const bool bDragged = manager.draggedElement.first == id;
				if (bDragged) {
					*self.value = self.min + (self.max - self.min) * relativeCursorPos;
				}
				if (*self.value > self.max || *self.value < self.min /* does the value escape the min-max-range */) {
					drawInvalidSlider();
				}
				else {
					const Vec2 sliderMinPos = (self.bVertical ? Vec2{ place.x, sliderMinPosMainAxis } : Vec2{ sliderMinPosMainAxis, place.y });
					const Vec2 sliderMaxPos = (self.bVertical ? Vec2{ place.x, sliderMaxPosMainAxis } : Vec2{ sliderMaxPosMainAxis, place.y });
					const f32 relativeSliderValue = (*self.value - self.min) / (self.max - self.min);
					const Vec2 sliderPos = sliderMinPos + (sliderMaxPos - sliderMinPos) * relativeSliderValue;

					// Draw Slider indexer:
					out.push_back(
						UISprite{
							.color = self.colorSlider * (bDragged ? 0.9f : 1.0f),
							.position = Vec3{sliderPos, context.renderDepth},
							.scale = indicatorSize2,
							.clipMin = context.clipMin,
							.clipMax = context.clipMax,
							.cornerRounding = smallerInidcatorSize * 0.5f,
							.drawMode = RenderSpace2d::Pixel
						}
					);

					if (self.child != INVALID_ELEMENT_ID) {
						const bool validChild = manager.isOneOf<Text, StaticText>(self.child);
						assert(validChild && !self.bVertical && !self.bThin);
						auto childcontext = context;
						fit(childcontext, scaledBarSize, place);
						childcontext.cutLeft(smallerInidcatorSize);
						childcontext.cutRight(smallerInidcatorSize);
						childcontext.xalign = XAlign::Center;
						childcontext.yalign = YAlign::Center;
						draw(manager, manager.elements[self.child], self.child, childcontext, out);
					}
				}
			}
			else {
				drawInvalidSlider();
			}
		}

		template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, Group& self) 	{
			if (self.onUpdate) { self.onUpdate(self, id); }
			Vec2 minsize;
			for (u32 child : self.children) {
				auto childMinSize = updateAndGetMinsize(manager, child);
				if (self.bVertical) {
					minsize.x = std::max(minsize.x, childMinSize.x);
					minsize.y += childMinSize.y;
				}
				else {
					minsize.y = std::max(minsize.y, childMinSize.y);
					minsize.x += childMinSize.x;
				}
			}

			if (self.bVertical) {
				minsize.y += (self.children.size() - 1) * self.spacing;
			}
			else {
				minsize.x += (self.children.size() - 1) * self.spacing;
			}
			minsize += size(self.padding);
			return minsize;
		}
		template<> inline void onDraw<Group>(Manager& manager, Group& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			const Vec2 scaledWholeSize = context.size();
			const Vec2 scaledHalfSize = scaledWholeSize * 0.5f;
			const Vec2 place = getPlace(scaledWholeSize, context);
			const f32 scaledSpaceing = self.spacing * context.scale;

			f32 mainAxisAllChildSize{ 0.0f };
			for (auto& child : self.children) {
				mainAxisAllChildSize += (self.bVertical ? manager.minsizes[child].y : manager.minsizes[child].x);
			}
			mainAxisAllChildSize *= context.scale;
			const f32 childCount = static_cast<f32>(self.children.size());
			const f32 allSpacing{ (childCount - 1) * scaledSpaceing };

			const f32 mainAxisLeftSpace = (self.bVertical ? scaledWholeSize.y : scaledWholeSize.x) - scaledSpaceing * (childCount - 1.0f) - mainAxisAllChildSize;
			const f32 perChildExtraSpace = mainAxisLeftSpace / childCount;

			DrawContext childContext = context;
			childContext.topleft = Vec2{ place.x - scaledHalfSize.x, place.y + scaledHalfSize.y };
			childContext.bottomright = Vec2{ place.x + scaledHalfSize.x, place.y - scaledHalfSize.y };
			childContext.xalign = self.xalign;
			childContext.yalign = self.yalign;
			fit(childContext, self.padding);

			f32 uniformSpacePerChild = ((self.bVertical ? childContext.size().y : childContext.size().x) - scaledSpaceing * (childCount - 1.0f)) / childCount;
			f32 offset{ 0.0f };	// the offset to the top or to the left of the context, that next child is getting its context shifted by
			if (self.packing == Packing::Tight) {
				const f32 mainAxisContextSize = (self.bVertical ? scaledWholeSize.y : scaledWholeSize.x);
				if (self.bVertical) {
					if (self.yalign == YAlign::Center)
						offset += (mainAxisContextSize - mainAxisAllChildSize - allSpacing) * 0.5f;
					if (self.yalign == YAlign::Bottom)
						offset += mainAxisContextSize - mainAxisAllChildSize - allSpacing;
				}
				else {
					if (self.xalign == XAlign::Center)
						offset += (mainAxisContextSize - mainAxisAllChildSize - allSpacing) * 0.5f;
					if (self.xalign == XAlign::Right)
						offset += mainAxisContextSize - mainAxisAllChildSize - allSpacing;
				}
			}

			for (auto& childid : self.children) {
				ElementVariant& childvar = manager.elements[childid];
				const auto scaledSize = manager.minsizes[childid] * context.scale;

				DrawContext myContext = childContext;
				if (self.bVertical) {
					if (self.packing == Packing::Tight) {
						myContext.cutTop(offset);
						myContext.cutBottom(childContext.size().y - scaledSize.y - offset);
						offset += scaledSize.y + scaledSpaceing;
					}
					else if (self.packing == Packing::Spread) {
						myContext.cutTop(offset);
						myContext.cutBottom(childContext.size().y - scaledSize.y - perChildExtraSpace - offset);
						offset += scaledSize.y + scaledSpaceing + perChildExtraSpace;
					}
					else /* self.packing == Packing::Uniform */ {
						myContext.cutTop(offset);
						myContext.cutBottom(childContext.size().y - uniformSpacePerChild - offset);
						offset += scaledSpaceing + uniformSpacePerChild;
					}
				}
				else {
					if (self.packing == Packing::Tight) {
						myContext.cutLeft(offset);
						myContext.cutRight(childContext.size().x - scaledSize.x - offset);
						offset += scaledSize.x + scaledSpaceing;
					}
					else if (self.packing == Packing::Spread) {
						myContext.cutLeft(offset);
						myContext.cutRight(childContext.size().x - scaledSize.x - perChildExtraSpace - offset);
						offset += scaledSize.x + scaledSpaceing + perChildExtraSpace;
					}
					else /* self.packing == Packing::Uniform */ {
						myContext.cutLeft(offset);
						myContext.cutRight(childContext.size().x - uniformSpacePerChild - offset);
						offset += scaledSpaceing + uniformSpacePerChild;
					}
				}
				draw(manager, childvar, childid, myContext, out);
			}
		}

		template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, DragDroppable& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			Vec2 minsize;
			if (self.child != INVALID_ELEMENT_ID) {
				minsize = updateAndGetMinsize(manager, self.child);
			}
			return minsize;
		}
		template<> inline void onDraw<DragDroppable>(Manager& manager, DragDroppable& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			const Vec2 scaledSize = manager.minsizes[id] * context.scale;
			Vec2 place = getPlace(scaledSize, context);

			const auto [isCursorOver, cursor] = manager.isCursorOver(place, scaledSize, context);

			if (isCursorOver) {
				manager.requestMouseEvent(id, context.root, context.renderDepth);
			}

			const bool bDragged = manager.draggedElement.first == id;
			if (bDragged) {
				place = cursor;
			}

			const bool wasIDropped = manager.droppedElementId == id;
			/* when we are getting dropped, a potential catch could accur AFTER this draw, so the element will jump for one frame. To avoid that we do not draw when we are dropped */
			if (!wasIDropped) {
				DrawContext childContext = context;
				if (bDragged) {
					fit(childContext, scaledSize, place);
					childContext.renderDepth -= 0.001f;
				}

				if (self.child != INVALID_ELEMENT_ID) {
					draw(manager, manager.elements[self.child], self.child, childContext, out);
				}
			}
		}

		template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, DropBox& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			Vec2 minsize;
			if (self.child != INVALID_ELEMENT_ID) {
				minsize = updateAndGetMinsize(manager, self.child);
			}
			minsize = max(minsize, self.minsize);
			return minsize;
		}
		template<> inline void onDraw<DropBox>(Manager& manager, DropBox& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			const Vec2 scaledSize = manager.minsizes[id] * context.scale;
			const Vec2 place = getPlace(scaledSize, context);

			if (manager.isCursorOver(place, scaledSize, context)) {
				if (manager.window->buttonJustReleased(MouseButton::Left) || self.bCatchMouseInput) {
					manager.requestMouseEvent(id, context.root, context.renderDepth);
				}
			}

			if (static_cast<Vec4&>(self.color).w != 0.0f) {
				out.push_back(
					UISprite{
						.color = self.color,
						.position = Vec3{place, context.renderDepth},
						.scale = scaledSize,
						.clipMin = context.clipMin,
						.clipMax = context.clipMax,
						.cornerRounding = 3.0f * context.scale,
						.drawMode = RenderSpace2d::Pixel
					}
				);
			}

			if (self.child != INVALID_ELEMENT_ID) {
				auto childcontext = context;
				childcontext.xalign = XAlign::Center;
				childcontext.yalign = YAlign::Center;
				fit(childcontext, scaledSize, place);
				draw(manager, manager.elements[self.child], self.child, childcontext, out);
			}
		}

		template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, HeadTail& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			Vec2 minsize;
			for (u32 child : self.children) {
				auto childMinSize = updateAndGetMinsize(manager, child);
				minsize.x = std::max(minsize.x, childMinSize.x);
				minsize.y += childMinSize.y;
			}
			minsize.y += self.spacing;
			return minsize;
		}
		template<> inline void onDraw<HeadTail>(Manager& manager, HeadTail& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			const f32 scaledFooterSize = self.size * context.scale;
			const f32 scaledSpacing = self.spacing * context.scale;
			const f32 scaledHeadderSize = context.size().y - scaledFooterSize - scaledSpacing;

			if (self.children.size() >= 1) {
				const u32 headChild = self.children[0];
				auto headContext = context;
				headContext.cutBottom(scaledSpacing + scaledFooterSize);

				draw(manager, manager.elements[headChild], headChild, headContext, out);

				if (self.children.size() >= 2) {
					const u32 footerChild = self.children[1];
					auto footerContext = context;
					footerContext.cutTop(scaledHeadderSize + scaledSpacing);

					draw(manager, manager.elements[footerChild], footerChild, footerContext, out);
				}
			}
			if (self.children.size() != 2) {
				std::cerr << "WARNING: footer (id: " << id << ") element does not have two children!\n";
			}
		}

		template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, _Radiobox& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			return self.size;
		}
		template<> inline void onDraw<_Radiobox>(Manager& manager, _Radiobox& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			Vec2 scaledSize = self.size * context.scale;
			Vec2 place = getPlace(scaledSize, context);

			//const Vec2 cursor = manager.coordSys.convertCoordSys(manager.window->getCursorPos(), RenderSpace2d::Window, RenderSpace2d::Pixel);
			//const bool cursorOverElement = isPointInAABB(cursor, place, scaledSize);

			f32 HOVER_SCALING{ 0.9f };

			if (self.bHover) {
				scaledSize *= HOVER_SCALING;
			}

			const f32 smallerScale = std::min(scaledSize.x, scaledSize.y);

			out.push_back(
				UISprite{
					.color = self.color,
					.position = Vec3{place, context.renderDepth},
					.scale = scaledSize,
					.clipMin = context.clipMin,
					.clipMax = context.clipMax,
					.cornerRounding = smallerScale * 0.5f,
					.drawMode = RenderSpace2d::Pixel
				}
			);

			if (self.value) {
				if (manager.isCursorOver(place, scaledSize, context)) {
					manager.requestMouseEvent(id, context.root, context.renderDepth);
				}
				else {
					self.bHold = false;
					self.bHover = false;
				}

				const bool enabled = *self.value == self.index;

				out.push_back(
					UISprite{
						.color = (enabled ? self.colorEnabled : self.colorDisabled),
						.position = Vec3{place, context.renderDepth },
						.scale = scaledSize * 0.75f,
						.clipMin = context.clipMin,
						.clipMax = context.clipMax,
						.cornerRounding = smallerScale * 0.75f * 0.5f,
						.drawMode = RenderSpace2d::Pixel
					}
				);
			}
		}

		template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, _ScrollBox& self) 	{
			if (self.onUpdate) self.onUpdate(self, id);
			updateAndGetMinsize(manager, self.child);
			Vec2 minsize = size(self.padding) + self.minsize;
			return minsize;
		}
		template<> inline void onDraw<_ScrollBox>(Manager& manager, _ScrollBox& self, u32 id, DrawContext const& context, std::vector<UISprite>& out) 	{
			const Vec2 scaledSize = (self.bFillSpace ? context.size() : manager.minsizes[id] * context.scale);
			const Vec2 place = getPlace(scaledSize, context);


			ImageSectionHandle& tex = static_cast<ImageSectionHandle&>(self.textureView);
			auto [texMin, texMax] = self.bScreenTextureView ?
				getScreenTextureMinMax(place, scaledSize, context.renderSpace, manager.coordSys) :
				std::pair<Vec2, Vec2>{ tex.min, tex.max };


			out.push_back(
				UISprite{
					.color = self.colorView,
					.position = Vec3{place, context.renderDepth},
					.scale = scaledSize,
					.imageIndex = tex.handle.getIndex(),
					.texMin = texMin,
					.texMax = texMax,
					.clipMin = context.clipMin,
					.clipMax = context.clipMax,
					.drawMode = RenderSpace2d::Pixel
				}
			);

			if (self.child != INVALID_ELEMENT_ID) {
				const Vec2 cursor = manager.coordSys.convertCoordSys(manager.window->getCursorPositionVec(), RenderSpace2d::Window, RenderSpace2d::Pixel);
				const Vec2 scaledChildMinSize = (manager.minsizes[self.child] + size(self.padding)) * context.scale;
				const f32 scaledScrollerWidth = self.scrollerWidth * context.scale;
				Vec2 scaledViewSize = scaledSize;
				Vec2 scaledViewPlace = place;

				Vec2 childContextPlace;
				Vec2 childContextSize;
				bool requestMouseEvent{ false };
				if (scaledViewSize.y < scaledChildMinSize.y * 0.999f) {
					// we need to enable the scroller in Y dir:
					scaledViewSize.x -= scaledScrollerWidth;
					scaledViewPlace.x -= scaledScrollerWidth * 0.5f;
					const f32 viewScrollableRange = scaledChildMinSize.y - scaledViewSize.y;

					Vec2 scrollBarPlace = scaledViewPlace + Vec2{ (scaledViewSize.x + scaledScrollerWidth) * 0.5f, 0 };
					Vec2 scrollBarSize = Vec2{ scaledScrollerWidth, scaledViewSize.y };
					const f32 scrollerHeight = std::max(scaledViewSize.y * (scaledViewSize.y / scaledChildMinSize.y), scaledScrollerWidth);
					out.push_back(UISprite{
						.color = self.colorScrollBar,
						.position = Vec3{scrollBarPlace, context.renderDepth},
						.scale = scrollBarSize,
						.clipMin = context.clipMin,
						.clipMax = context.clipMax,
						.drawMode = RenderSpace2d::Pixel
						});

					const f32 scrollerPlaceMax = scrollBarPlace.y + (scrollBarSize.y - scrollerHeight) * 0.5f;
					const f32 scrollerPlaceMin = scrollBarPlace.y - (scrollBarSize.y - scrollerHeight) * 0.5f;
					const f32 scrollerCoordRange = scrollerPlaceMax - scrollerPlaceMin;
					self.viewOffset = std::clamp(self.viewOffset, 0.0f, viewScrollableRange / context.scale);
					const f32 scrollerRelativePosition = self.viewOffset / (viewScrollableRange / context.scale);

					const auto scrollerSize = Vec2{
						scaledScrollerWidth,
						scrollerHeight
					};
					const auto scrollerPlace = Vec2{
						scrollBarPlace.x,
						scrollerPlaceMax - scrollerCoordRange * scrollerRelativePosition
					};

					childContextPlace = scaledViewPlace - Vec2{ 0, viewScrollableRange * 0.5f - self.viewOffset * context.scale };
					childContextSize.x = scaledViewSize.x;
					childContextSize.y = scaledChildMinSize.y;

					out.push_back(UISprite{
						.color = self.colorScroller,
						.position = Vec3{scrollerPlace, context.renderDepth},
						.scale = scrollerSize,
						.clipMin = context.clipMin,
						.clipMax = context.clipMax,
						.drawMode = RenderSpace2d::Pixel
						});

					if (manager.draggedElement.first == id) {
						const Vec2 prevCursor = manager.coordSys.convertCoordSys(manager.window->getPrevCursorPositionVec(), RenderSpace2d::Window, RenderSpace2d::Pixel);
						const f32 cursorYDistTraveled = cursor.y - prevCursor.y;
						const f32 relativeScrollerPosChange = cursorYDistTraveled / scrollerCoordRange;
						self.viewOffset -= (relativeScrollerPosChange * viewScrollableRange) / context.scale;
					}
					else {
						if (manager.isCursorOver(scrollerPlace, scrollerSize, context)) {
							self.mouseEventType = _ScrollBox::MouseEventType::DragScroller;
							self.lastDrawScale = context.scale;
							requestMouseEvent = true;
						}
						else if (manager.isCursorOver(scrollBarPlace, scrollBarSize, context)) {
							self.mouseEventType = _ScrollBox::MouseEventType::ClampScrollerToCursor;
							const f32 clampedcursor = std::clamp(cursor.y, scrollerPlaceMin, scrollerPlaceMax);
							const f32 relativeScrollerPosition = 1.0f - (clampedcursor - scrollerPlaceMin) / (scrollerPlaceMax - scrollerPlaceMin);
							self.viewOffsetIfCursorClamp = relativeScrollerPosition * viewScrollableRange / context.scale;
							self.lastDrawScale = context.scale;
							requestMouseEvent = true;
						}
						else if (manager.isCursorOver(place, scaledSize, context)) {
							self.mouseEventType = _ScrollBox::MouseEventType::BackgroundScrolling;
							manager.requestMouseEvent(id, context.root, context.renderDepth);
						}
					}
				}
				else {
					childContextPlace = scaledViewPlace;
					childContextSize = scaledSize;
				}

				auto childcontext = context;
				childcontext.xalign = self.xalign;
				childcontext.yalign = self.yalign;
				const Vec2 viewClipMin = scaledViewPlace - scaledViewSize * 0.5f + Vec2{ static_cast<Padding>(self.padding).left, static_cast<Padding>(self.padding).bottom } *context.scale;
				const Vec2 viewClipMax = scaledViewPlace + scaledViewSize * 0.5f - Vec2{ static_cast<Padding>(self.padding).right, static_cast<Padding>(self.padding).top } *context.scale;
				childcontext.clipMin = manager.coordSys.convertCoordSys(viewClipMin, context.renderSpace, RenderSpace2d::Window);
				childcontext.clipMax = manager.coordSys.convertCoordSys(viewClipMax, context.renderSpace, RenderSpace2d::Window);
				fit(childcontext, childContextSize, childContextPlace);
				fit(childcontext, self.padding);

				draw(manager, manager.elements[self.child], self.child, childcontext, out);

				if (requestMouseEvent) {
					manager.requestMouseEvent(id, context.root, context.renderDepth);
				}
			}
		}
	}
}