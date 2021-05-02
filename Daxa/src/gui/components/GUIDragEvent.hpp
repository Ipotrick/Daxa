#pragma once

#include "../GUIManager.hpp"

namespace gui {
	template<typename T>
	inline void onDragEvent(Manager& manager, T& self, u32 id, u32 rootid) { }

	template<> void onDragEvent<Box>(Manager& manager, Box& self, u32 id, u32 rootindex);
	
	inline void onDragEvent(Manager& manager, ElementVariant& var, u32 id, u32 rootid)
	{
		std::visit([&](auto&& element) { onDragEvent(manager, element, id, rootid); }, var);
	}

	  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
     ////////////////////////////////////////////////// TYPE SPECIFIC IMPLEMENTATIONS: ////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	template<> inline void onDragEvent(Manager& manager, Box& self, u32 id, u32 rootindex)
	{
		Root& root = manager.rootElements[rootindex].element;
		Vec2 cursorPos = manager.coordSys.convertCoordSys<RenderSpace::Window, RenderSpace::Pixel>(manager.window->getCursorPos());
		Vec2 cursorPrevPos = manager.coordSys.convertCoordSys<RenderSpace::Window, RenderSpace::Pixel>(manager.window->getPrevCursorPos());

		DrawContext windowContext;
		windowContext.scale = manager.globalScaling;
		windowContext.topleft = Vec2{ 0.0f, manager.window->getSizeVec().y };
		windowContext.bottomright = Vec2{ manager.window->getSizeVec().x, 0.0f };

		if (self.onDrag == OnDrag::Move) {
			root.placing.move(cursorPos - cursorPrevPos, windowContext, root.sizing);
		}
		else /* self.onDrag == OnDrag::Resize */{
			root.sizing.changeBy(cursorPos - cursorPrevPos, windowContext, root.placing);
		}
	}
}
