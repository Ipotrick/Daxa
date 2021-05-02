#pragma once

#include <any>

#include "../base/GUIElement.hpp"
#include "../components/GUITextElements.hpp"
namespace daxa {
	namespace gui {

		struct Group;

		struct Column : IElement {
			std::function<void(Group&, u32)> onUpdate;
			ValueOrPtr<XAlign> xalign{ XAlign::Left };
			ValueOrPtr<YAlign> yalign{ YAlign::Top };
			ValueOrPtr<Padding> padding{ Padding{NAN,NAN,NAN,NAN} };
			ValueOrPtr<Packing> packing{ Packing::Tight };
			ValueOrPtr<f32> spacing{ NAN };
			std::vector<u32> children;
		};



		struct Row : IElement {
			std::function<void(Group&, u32)> onUpdate;
			ValueOrPtr<XAlign>xalign{ XAlign::Left };
			ValueOrPtr<YAlign>yalign{ YAlign::Center };
			ValueOrPtr<Padding>padding{ Padding{NAN,NAN,NAN,NAN} };
			ValueOrPtr<Packing>packing{ Packing::Tight };
			ValueOrPtr<f32>spacing{ NAN };
			std::vector<u32> children;
		};



		struct Group : IElement {
			Group(Column&& r) :
				onUpdate{ std::move(r.onUpdate) },
				xalign{ std::move(r.xalign) },
				yalign{ std::move(r.yalign) },
				padding{ std::move(r.padding) },
				packing{ std::move(r.packing) },
				spacing{ std::move(r.spacing) },
				bVertical{ true },
				children{ std::move(r.children) }
			{}
			Group(Row&& r) :
				onUpdate{ std::move(r.onUpdate) },
				xalign{ std::move(r.xalign) },
				yalign{ std::move(r.yalign) },
				padding{ std::move(r.padding) },
				packing{ std::move(r.packing) },
				spacing{ std::move(r.spacing) },
				bVertical{ false },
				children{ std::move(r.children) }
			{}

			std::function<void(Group&, u32)> onUpdate;
			ValueOrPtr<XAlign> xalign{ XAlign::Left };
			ValueOrPtr<YAlign> yalign{ YAlign::Top };
			ValueOrPtr<Padding> padding{ Padding{NAN,NAN,NAN,NAN} };
			ValueOrPtr<Packing> packing{ Packing::Tight };
			ValueOrPtr<f32> spacing{ NAN };
			ValueOrPtr<bool> bVertical{ true };
			std::vector<u32> children;
		};



		enum class OnDrag { Move, Resize };
		struct Box : IElement {
			std::function<void(Box&, u32)> onUpdate;
			ValueOrPtr<Vec2> minsize{ Vec2{} };
			ValueOrPtr<bool> bFillSpace{ false };
			ValueOrPtr<bool> bDragable{ false };
			ValueOrPtr<bool> bScreenTexture{ false };
			ValueOrPtr<ImageSectionHandle> texture{ ImageSectionHandle{} };
			ValueOrPtr<Vec4> color{ UNSET_COLOR };
			ValueOrPtr<Padding> padding{ Padding{NAN, NAN, NAN, NAN} };
			f32 cornerRounding{ NAN };
			ValueOrPtr<XAlign> xalign{ XAlign::Left };
			ValueOrPtr<YAlign> yalign{ YAlign::Top };
			ValueOrPtr<OnDrag> onDrag{ OnDrag::Move };
			u32 child{ INVALID_ELEMENT_ID };
		};



		struct HeadTail : IElement {
			std::function<void(HeadTail&, u32)> onUpdate;
			ValueOrPtr<Mode> mode{ Mode::Absolute };
			ValueOrPtr<f32> size{ 30 };
			ValueOrPtr<f32> spacing{ NAN };
			std::array<u32, 2> children{ INVALID_ELEMENT_ID ,INVALID_ELEMENT_ID };
		};

		struct ScrollBox : IElement {
			std::function<void(ScrollBox&, u32)> onUpdate;
			ValueOrPtr<Vec2> minsize{ Vec2{} };
			ValueOrPtr<bool> bFillSpace{ true };
			ValueOrPtr<bool> bScreenTextureView{ false };
			ValueOrPtr<ImageSectionHandle> textureView{ ImageSectionHandle{} };
			ValueOrPtr<Vec4> colorView{ UNSET_COLOR };
			ValueOrPtr<Vec4> colorScroller{ UNSET_COLOR };
			ValueOrPtr<Vec4> colorScrollBar{ UNSET_COLOR };
			ValueOrPtr<XAlign> xalign{ XAlign::Left };
			ValueOrPtr<YAlign> yalign{ YAlign::Top };
			ValueOrPtr<Padding> padding{ Padding{0, 0, 0, 0} };
			f32 scrollerWidth{ NAN };
			f32 scrollSpeed{ 10.0f };
			u32 child{ INVALID_ELEMENT_ID };
		};
		struct _ScrollBox : ScrollBox {
			_ScrollBox(ScrollBox&& sb) : ScrollBox{ sb } {}
			f32 viewOffset{ 0.0f };
			f32 viewOffsetIfCursorClamp{ 0.0f };
			f32 lastDrawScale{ 1.0f };
			enum class MouseEventType {
				ClampScrollerToCursor,
				DragScroller,
				BackgroundScrolling
			};
			MouseEventType mouseEventType{ MouseEventType::ClampScrollerToCursor };
		};



		struct Button : IElement {
			std::function<void(Button&, u32)> onUpdate;
			Vec2 size{ Vec2{15,15} };
			Vec4 color{ UNSET_COLOR };
			Vec4 holdColor{ UNSET_COLOR };
			bool bScreenTexture{ false };
			ImageSectionHandle texture{ ImageSectionHandle{} };
			std::function<void(Button& self)> onPress;
			std::function<void(Button& self)> onHold;
			std::function<void(Button& self)> onRelease;
			u32 child{ INVALID_ELEMENT_ID };
		};
		namespace {
			struct _Button : public Button {
				_Button(Button&& b) : Button{ b } {}
				bool bHold{ false };
				bool bHover{ false };
			};
		}



		struct Checkbox : IElement {
			std::function<void(Checkbox&, u32)> onUpdate;
			bool* value{ nullptr };
			ValueOrPtr<Vec2> size{ Vec2{15,15} };
			ValueOrPtr<Vec4> color{ UNSET_COLOR };
			ValueOrPtr<Vec4> colorEnabled{ UNSET_COLOR };
			ValueOrPtr<Vec4> colorDisabled{ UNSET_COLOR };
		};
		namespace {
			struct _Checkbox : public Checkbox {
				_Checkbox(Checkbox&& e) : Checkbox{ e } {}
				bool bHold{ false };
				bool bHover{ false };
			};
		}



		struct Radiobox : IElement {
			std::function<void(Radiobox&, u32)> onUpdate;
			u32* value{ nullptr };
			u32 index{ 0 };
			ValueOrPtr<Vec2> size{ Vec2{15,15} };
			ValueOrPtr<Vec4> color{ UNSET_COLOR };
			ValueOrPtr<Vec4> colorEnabled{ UNSET_COLOR };
			ValueOrPtr<Vec4> colorDisabled{ UNSET_COLOR };
		};
		namespace {
			struct _Radiobox : public Radiobox {
				_Radiobox(Radiobox&& e) : Radiobox{ e } {}
				bool bHold{ false };
				bool bHover{ false };
			};
		}



		struct SliderF64 : IElement {
			std::function<void(SliderF64&, u32)> onUpdate;
			f64* value{ nullptr };
			ValueOrPtr<Vec2> size{ Vec2{150,25} };
			ValueOrPtr<f64> min{ 0.0 };
			ValueOrPtr<f64> max{ 1.0 };
			bool bVertical{ false };
			bool bThin{ true };
			ValueOrPtr<Vec4> colorBar{ UNSET_COLOR };
			ValueOrPtr<Vec4> colorSlider{ UNSET_COLOR };
			ValueOrPtr<Vec4> colorError{ UNSET_COLOR };
			u32 child{ INVALID_ELEMENT_ID };
		};


		struct DragDroppable : IElement {
			std::function<void(DragDroppable&, u32)> onUpdate;
			std::any data;
			ValueOrPtr<bool> bCatchable{ true };
			u32 child{ INVALID_ELEMENT_ID };
		};



		struct DropBox : IElement {
			std::function<void(DropBox&, u32)> onUpdate;
			// gets executed in a catch event
			// the u32 represents the id of the catched element
			// return true when we want to catch the element and make it our child
			std::function<bool(DropBox&, DragDroppable&)> onCatch;
			std::any data;
			ValueOrPtr<Vec2> minsize{ Vec2{ 50,50 } };
			ValueOrPtr<Vec4> color{ UNSET_COLOR };
			// true: mouse input will be catched by the element.
			// false: mouse input will fall throu to the element below.*/
			bool bCatchMouseInput{ true };
			u32 child{ INVALID_ELEMENT_ID };
		};
	}
}