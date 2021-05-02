#pragma once

#include "GUICommonElements.hpp"
namespace daxa {
	namespace gui {

		template<typename T>
		std::pair<u32*, u32> getStaticChildren(T&) { return { nullptr, 0 }; }

		template<> inline std::pair<u32*, u32> getStaticChildren(Box& self) { return { &self.child, self.child != INVALID_ELEMENT_ID }; }
		template<> inline std::pair<u32*, u32> getStaticChildren(DropBox& self) { return { &self.child, self.child != INVALID_ELEMENT_ID }; }
		template<> inline std::pair<u32*, u32> getStaticChildren(DragDroppable& self) { return { &self.child, self.child != INVALID_ELEMENT_ID }; }
		template<> inline std::pair<u32*, u32> getStaticChildren(_ScrollBox& self) { return { &self.child, self.child != INVALID_ELEMENT_ID }; }
		template<> inline std::pair<u32*, u32> getStaticChildren(HeadTail& self) { return { &self.children[0], 2 }; }

		template<typename T>
		std::vector<u32>* getDynamicChildrenIf(T&) { return nullptr; }

		template<> inline std::vector<u32>* getDynamicChildrenIf(Group& self) { return &self.children; }
	}
}