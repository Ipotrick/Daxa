#pragma once

#include <vector>
#include <variant>
#include <deque>

#include "../util/StableVector.hpp"

#include "../rendering/CoordSystem2d.hpp"
#include "font/Font.hpp"

#include "base/GUIRootElement.hpp"
#include "base/GUIElement.hpp"
#include "components/GUICommonElements.hpp"
#include "components/GUITextElements.hpp"
#include "components/GUICommonElementsStyling.hpp"
#include "components/GUITextElementsStyling.hpp"
#include "components/GUIChildAccess.hpp"

namespace daxa {

	namespace gui {

		using ElementVariant = std::variant<
			std::monostate,
			Box,
			Group,
			_StaticText,
			_Text,
			_TextInput,
			_TextInputF64,
			_Button,
			_Checkbox,
			_Radiobox,
			_ScrollBox,
			SliderF64,
			DropBox,
			DragDroppable,
			HeadTail
		>;

		class Manager {
		public:
			struct RootHandle {
				bool valid() const { return index != 0xFFFFFFFF && version != 0xFFFFFFFF; }

				u32 index = 0xFFFFFFFF;
				u32 version = 0xFFFFFFFF;
			};

			Manager(/*ImageManager* tex,*/ FontManager* fonts);

			Manager(Manager&& rhs) = delete;

			Manager(const Manager& rhs) = delete;

			Manager& operator=(const Manager& rhs) = delete;

			Manager& operator=(const Manager&& rhs) = delete;

			RootHandle build(Root&& root);

			/**
			 * Creates a new element.
			 * Automaticly sets parent child relationships.
			 * The default Style is used to set any unset values here.
			 *
			 * \param element that should be constructed.
			 * \return the id of the element.
			 */
			template<CElement T>
			u32 build(T&& element) 		{
				return build<T>(defaultStyle, std::move(element));
			}

			/**
			 * Creates a new element.
			 * Automaticly sets parent child relationships.
			 *
			 * \param style the element should be styled after.
			 * The Style is overwritten by any manually set values in construction.
			 * \param element that should be constructed.
			 * \return the id of the element.
			 */
			template<CElement T>
			u32 build(const Style& style, T&& element) 		{
				applyStyle(element, style);
				u32 index;

				if (!freeElementIndices.empty()) {
					index = freeElementIndices.back();
					freeElementIndices.pop_back();
					elements[index] = std::move(element);
				}
				else {
					elements.emplaceBack(std::move(element));
					parents.emplaceBack(INVALID_ELEMENT_ID);
					minsizes.resize(elements.size(), Vec2{ 0.0f, 0.0f });
					index = static_cast<u32>(elements.size() - 1);
				}

				updateChildHierarchy(index);

				return index;
			}

			/**
			 * \param element to check the childcount for.
			 * \return if the given element contains and parent child relationships.
			 */
			bool hasChild(u32 element);

			std::vector<u32>* dynamicChildren(u32 element);

			std::pair<u32*, u32> staticChildren(u32 element);

			/**
			 * repositions the child in the list of children of the parent.
			 *
			 * \param child to change position of
			 * \param newPosition of the child.
			 */
			 //void changeChildPosition(u32 child, u32 newPosition);

			 /**
			  * Orphans the child from its parent.
			  * New parent adopts the child.
			  * All parent-child relations are properly changed.
			  *
			  * \param child that should change parents.
			  * \param newParent the new parent of the child.
			  * \param newParentPosition position in list of children of the new parent (ignored when parent is single parent), the palceholder value 0xFFFFFFFF means append to the end of the list.
			  */
			void changeParent(u32 child, u32 newParent, u32 newParentPosition = 0xFFFFFFFF);

			/**
			 * after adding children to an element manually, one has to update the child hierarchie.
			 *
			 * \param parent element whomst children were changed.
			 */
			void updateChildHierarchy(u32 parent);

			/**
			 * destroys a root and all its children.
			 *
			 * \param handle of the root that should be destroyed.
			 */
			void destroy(const RootHandle& handle);

			void destroy(const u32 elementid);

			/**
			 * asserts if root element that the handle refers too still exists.
			 *
			 * \param handle of the root.
			 */
			bool isHandleValid(const RootHandle& handle) const;

			/**
			 * Updates all elements.
			 * Makes all relevant callbacks.
			 * draws the ui to the set Layer in the given Renderer.
			 *
			 * \param renderer to render to.
			 * \param window that the renderer belongs to.
			 * \param deltaTime the time difference to the last draw call.
			 */
			void draw(const RenderCoordSys& coordSys, Window& window, float deltaTime);

			const std::vector<UISprite>& getSprites() const { return this->spritesOfLastDraw; }

			template<typename T>
			T& get(u32 id) {
				return std::get<T>(elements[id]);
			}

			template<typename T>
			const T& get(u32 id) const {
				return std::get<T>(elements[id]);
			}

			/**
			 * \return the amount of root and normal elements currently stored.
			 */
			size_t size() const;

			void printMemoryUtalisation();

			void setStyle(Style style) { this->defaultStyle = style; }

			Style& getStyle() { return this->defaultStyle; }
			const Style& getStyle() const { return this->defaultStyle; }

			float globalScaling{ 1.0f };
		private:
			friend Vec2 updateAndGetMinsize(Manager&, u32);
			template<typename T>
			friend Vec2 updateAndGetMinsize(Manager& manager, u32 id, T& self);
			friend void drawRoot(Manager& manager, u32 id, std::vector<UISprite>& out);
			template<typename T>
			friend void onDraw(Manager&, T&, u32, DrawContext const&, std::vector<UISprite>&);
			template<typename T>
			friend void onMouseEvent(Manager& manager, T& element, u32 id, u32 rootid);
			template<typename T>
			friend void onDragEvent(Manager& manager, T& self, u32 id, u32 rootid);

			struct { bool bIsOver; Vec2 cursor; operator bool() { return bIsOver; } } isCursorOver(Vec2 place, Vec2 size, DrawContext const& context) 		{
				const Vec2 windowSpaceCursor = this->window->getCursorPositionVec();
				const Vec2 cursor = this->coordSys.convertCoordSys(windowSpaceCursor, RenderSpace2d::Window, context.renderSpace);
				const bool cursorOverElement = isPointInAABB(cursor, place, size);
				const bool cursorInClip = isPointInRange(windowSpaceCursor, context.clipMin, context.clipMax);

				return { cursorOverElement && cursorInClip, cursor };
			}

			template<typename ... T>
			bool isOneOf(u32 elementid) 		{
				//ElementVariant& element = elements[elementid];
				//return (std::holds_alternative<T>(element) || ...);
				return true;
			}

			/**
			 * Removes the child and parent relationship in the manager.
			 * Should only be used to transfer relationships, NOT to store elements without a root.
			 *
			 * \param child to remove the relationship of
			 */
			void orphanChild(u32 child);

			/**
			 * Adds a parent -child relationship inside the manager.
			 * Should only be used to transfer relationships, NOT to store elements without a root.
			 *
			 * \param child to adopt
			 * \param parent that adopts the child
			 * \param position the child should be placed in the children list (is ignored for single parents)
			 */
			void adoptChild(u32 child, u32 parent, u32 position = 0xFFFFFFFF);

			void requestMouseEvent(u32 elementid, u32 rootid, float depth);

			/// out factored functions called in draw:			///
			void updateDraggedElement();
			void updateFocusedTextInput();

			/// Temporary variables with lifetime of the draw:  ///
			//ImageHandle blurTextureHandle{};
			Window* window{ nullptr };
			RenderCoordSys coordSys;
			float deltaTime{ 0.0f };
			// Mouse Event Listener:
			struct MouseEvent {
				u32 element{ INVALID_ELEMENT_ID };
				u32 root{ INVALID_ELEMENT_ID };
				f32 depth{ NAN };
			};
			std::vector<MouseEvent> mouseEvenetQueue;
			// Drag Drop:
			u32 droppedElementId{ INVALID_ELEMENT_ID };
			DragDroppable* droppedElement{ nullptr };
			/// ----------------------------------------------- ///


			std::vector<UISprite> spritesOfLastDraw;
			//ImageManager* tex{ nullptr };
			FontManager* fonts{ nullptr };

			static inline const std::filesystem::path DEFAULT_FONT_TEXTURE{ "ressources/fonts/Consolas_font.png" };
			static inline const std::filesystem::path DEFAULT_FONT{ "ressources/fonts/Consolas_font.csv" };
			Style defaultStyle{
				.fill0 = Vec4::From255(0x15, 0x15, 0x16, 0xFF),
				.fill1 = Vec4::From255(0x24, 0x24, 0x25, 0xFF),
				.fill2 = Vec4::From255(0x104, 0x104, 0x110, 0xFF),
				.accent0 = Vec4{0.37,0.37,0.37, 1},
				//.positive = Vec4::From255(30, 100, 40, 0xFF),
				//.negative = Vec4::From255(100, 30, 40, 0xFF),
				.positive = Vec4{0.2,0.6,0.2,1.0},
				.negative = Vec4{0.6,0.2,0.2,1.0},
				.fontSize = 12.0f,
				.fontColor1 = Vec4{ 1, 1, 1, 1 },
				.fontColor2 = Vec4{ 0, 0, 0, 1 },
				.padding = Padding{5,5,5,5},
				.spacing = 5.0f,
				.cornerRounding = 3.0f,
				.scrollerWidth = 10.0f,
			};
			struct RootElementMetaInfo {
				Root element;
				u32 version = 0;
				bool containsElement = false;
			};
			StableVector<RootElementMetaInfo> rootElements;
			StableVector<u32> freeRootElementIndices;
			StableVector<u32> destroylist;						// used as buffer for element ids when destroying elements and their children

			StableVector<ElementVariant>		elements;
			StableVector<u32>				parents;			// stores parent of each element
			StableVector<Vec2>				minsizes;			// used to cache min sizes of elements, to minimise redundant calculations
			std::deque<u32>					freeElementIndices;

			struct DraggedElementInfo {
				u32 elementId;
				u32 parentId;
				RootHandle root;
			};


			std::pair<u32 /*element id*/, RootHandle> draggedElement{ INVALID_ELEMENT_ID, {} };
			std::pair<u32 /* element id*/, RootHandle> focusedTextInput{ INVALID_ELEMENT_ID, {} };
		};
	}
}