#pragma once

#include "../GUIManager.hpp"
namespace daxa {
	namespace gui {

		template<typename T>
		void onMouseEvent(Manager& manager, T& self, u32 id, u32 rootid) {}

		template<> void onMouseEvent(Manager& manager, _Button& self, u32 id, u32 rootid);
		template<> void onMouseEvent(Manager& manager, Box& self, u32 id, u32 rootid);
		template<> void onMouseEvent(Manager& manager, _Checkbox& self, u32 id, u32 rootid);
		template<> void onMouseEvent(Manager& manager, _Radiobox& self, u32 id, u32 rootid);
		template<> void onMouseEvent(Manager& manager, SliderF64& self, u32 id, u32 rootid);
		template<> void onMouseEvent(Manager& manager, DragDroppable& self, u32 id, u32 rootid);
		template<> void onMouseEvent(Manager& manager, _ScrollBox& self, u32 id, u32 rootid);

		inline void onMouseEvent(Manager& manager, ElementVariant& var, u32 id, u32 rootid) 	{
			std::visit([&](auto&& element) { onMouseEvent(manager, element, id, rootid); }, var);
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	   ////////////////////////////////////////////////// TYPE SPECIFIC IMPLEMENTATIONS: ////////////////////////////////////////////////////////
	  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		template<> inline void onMouseEvent<_Button>(Manager& manager, _Button& self, u32 id, u32 rootid) 	{
			self.bHover = true;

			if (self.bHold && manager.window->buttonJustReleased(MouseButton::Left)) {
				if (self.onRelease) self.onRelease(self);
			}
			else if (self.bHold && manager.window->buttonPressed(MouseButton::Left)) {
				if (self.onHold) self.onHold(self);
			}
			else if (manager.window->buttonJustPressed(MouseButton::Left)) {
				if (self.onPress) self.onPress(self);
				self.bHold = true;
			}
			else if (self.bHold && !manager.window->buttonPressed(MouseButton::Left)) {
				self.bHold = false;
			}
			manager.window->hideButton(MouseButton::Left);
		}

		template<> inline void onMouseEvent(Manager& manager, _TextInput& self, u32 id, u32 rootid) 	{
			if (manager.window->buttonJustPressedAndHide(MouseButton::Left)) {
				manager.focusedTextInput = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
			}
		}

		template<> inline void onMouseEvent(Manager& manager, _TextInputF64& self, u32 id, u32 rootid) 	{
			if (manager.window->buttonJustPressedAndHide(MouseButton::Left)) {
				manager.focusedTextInput = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
				self.str = "";
			}
		}

		template<> inline void onMouseEvent<Box>(Manager& manager, Box& self, u32 id, u32 rootid) 	{
			if (manager.window->buttonJustPressedAndHide(MouseButton::Left)) {
				manager.draggedElement = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
			}
		}

		template<> inline void onMouseEvent<_Checkbox>(Manager& manager, _Checkbox& self, u32 id, u32 rootid) 	{
			self.bHover = true;
			if (self.bHold && manager.window->buttonJustReleased(MouseButton::Left)) {
				if (self.value) *self.value = !*self.value;
			}
			else if (manager.window->buttonJustPressed(MouseButton::Left)) {
				self.bHold = true;
			}
			else if (self.bHold && !manager.window->buttonPressed(MouseButton::Left)) {
				self.bHold = false;
			}
			manager.window->hideButton(MouseButton::Left);
		}

		template<> inline void onMouseEvent<SliderF64>(Manager& manager, SliderF64& self, u32 id, u32 rootid) 	{
			if (manager.window->buttonJustPressedAndHide(MouseButton::Left) && self.max > self.min) {
				manager.draggedElement = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
			}
		}

		template<> inline void onMouseEvent<_ScrollBox>(Manager& manager, _ScrollBox& self, u32 id, u32 rootid) 	{
			bool clicked{ false };
			switch (self.mouseEventType) {
			case _ScrollBox::MouseEventType::ClampScrollerToCursor:
			{
				if (manager.window->buttonPressedAndHide(MouseButton::Left)) {
					self.viewOffset = self.viewOffsetIfCursorClamp;
					manager.draggedElement = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
					clicked = true;
				}
				break;
			}
			case _ScrollBox::MouseEventType::DragScroller:
			{
				if (manager.window->buttonPressedAndHide(MouseButton::Left)) {
					manager.draggedElement = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
					clicked = true;
				}
				break;
			}
			default:
				break;
			}

			if (!clicked) {
				if (f32 scroll = manager.window->scrollYAndHide(); scroll != 0.0f) {
					self.viewOffset -= scroll * self.scrollSpeed / self.lastDrawScale;
				}
			}
		}

		template<> inline void onMouseEvent(Manager& manager, DragDroppable& self, u32 id, u32 rootid) 	{
			if (manager.window->buttonJustPressedAndHide(MouseButton::Left)) {
				manager.draggedElement = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
			}
		}

		template<> inline void onMouseEvent(Manager& manager, DropBox& self, u32 id, u32 rootid) 	{
			if (self.child == INVALID_ELEMENT_ID) {
				if (manager.window->buttonJustReleasedAndHide(MouseButton::Left) && manager.droppedElement && manager.droppedElement->bCatchable) {
					if (!self.onCatch || self.onCatch(self, *manager.droppedElement)) {
						manager.changeParent(manager.droppedElementId, id);
					}
				}
			}
		}

		template<> inline void onMouseEvent(Manager& manager, _Radiobox& self, u32 id, u32 rootid) 	{
			self.bHover = true;
			if (self.bHold && manager.window->buttonJustReleased(MouseButton::Left)) {
				if (self.value) *self.value = self.index;
			}
			else if (manager.window->buttonJustPressed(MouseButton::Left)) {
				self.bHold = true;
			}
			else if (self.bHold && !manager.window->buttonPressed(MouseButton::Left)) {
				self.bHold = false;
			}
			manager.window->hideButton(MouseButton::Left);
		}
	}
}