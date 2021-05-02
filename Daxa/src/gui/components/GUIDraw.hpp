#pragma once


#include "../GUIManager.hpp"
#include "../base/GUITextUtil.hpp"
namespace daxa {
	namespace gui {
		template<typename T>
		void onDraw(Manager& manager, T& element, u32 id, DrawContext const& context, std::vector<Sprite>& out) { static_assert(false); }

		template<> void onDraw(Manager& manager, std::monostate& self, u32 id, DrawContext const& context, std::vector<Sprite>& out) {}
		template<> void onDraw(Manager& manager, Box& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, Group& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, _Button& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, _StaticText& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, _Text& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, _TextInput& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, _Checkbox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, _Radiobox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, SliderF64& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, DragDroppable& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, DropBox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, _TextInputF64& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, HeadTail& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
		template<> void onDraw(Manager& manager, _ScrollBox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);

		inline void draw(Manager& manager, ElementVariant& var, u32 id, DrawContext const& context, std::vector<Sprite>& out) 	{
			std::visit([&](auto&& element) { onDraw(manager, element, id, context, out); }, var);
		}

		template<typename T>
		/**
		 * shoud allways be called on child first and then one should update oneself.
		 * \return UNSCALED minimum size the element needs.
		 */
		Vec2 updateAndGetMinsize(Manager& manager, u32 id, T& element) { static_assert(false); return {}; }
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, std::monostate& self) { return {}; }
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, Box& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, Group& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, _Button& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, _StaticText& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, _Text& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, _TextInput& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, _Checkbox& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, _Radiobox& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, SliderF64& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, DragDroppable& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, DropBox& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, _TextInputF64& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, HeadTail& self);
		template<> Vec2 updateAndGetMinsize(Manager& manager, u32 id, _ScrollBox& self);


		/**
		 * shoud allways be called on child first and then one should update oneself.
		 * \return UNSCALED minimum size the element needs.
		 */
		inline Vec2 updateAndGetMinsize(Manager& manager, u32 id) 	{
			std::visit([&](auto&& element) { manager.minsizes[id] = updateAndGetMinsize(manager, id, element); }, manager.elements[id]);
			return manager.minsizes[id];
		}

		inline void drawRoot(Manager& manager, u32 id, std::vector<Sprite>& out) 	{
			Root& self = manager.rootElements[id].element;

			if (self.onUpdate) self.onUpdate(self, id);

			if (self.child != INVALID_ELEMENT_ID) {
				DrawContext myContext;
				myContext.scale = manager.globalScaling;
				myContext.topleft = Vec2{ 0, cast<f32>(manager.window->getHeight()) };
				myContext.bottomright = Vec2{ cast<f32>(manager.window->getWidth()), 0 };

				auto scaledSize = getSize(self.sizing, myContext);
				auto place = getPlace(scaledSize, self.placing, myContext);
				DrawContext childContext;
				childContext.scale *= manager.globalScaling;
				childContext.root = id;
				fit(childContext, scaledSize, place);
				updateAndGetMinsize(manager, self.child);
				draw(manager, manager.elements[self.child], self.child, childContext, out);
			}
		}
	}
}