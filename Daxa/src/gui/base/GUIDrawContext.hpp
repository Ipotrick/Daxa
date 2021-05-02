#pragma once

#include "../../DaxaCore.hpp"

#include "../../math/Vec.hpp"
#include "../../rendering/CoordSystem2d.hpp"
namespace daxa {

	namespace gui {
		enum class Mode {
			Relative,
			Absolute
		};

		enum class XAlign {
			/**
			 * elements are packed to the left.
			 */
			Left,
			/**
			 * elements are packed to the right.
			 */
			 Right,
			 /**
			 * elements are packed to the center.
			 */
			 Center
		};

		enum class YAlign {
			/**
			 * elements are packed to the top.
			 */
			Top,
			/**
			 * elements are packed to the bottom.
			 */
			 Bottom,
			 /**
			  * elements are packed to the center.
			  */
			  Center
		};

		enum class Packing {
			Tight,
			Spread,
			Uniform
		};

		struct Padding {
			Padding absTop(float p) const;
			Padding absBottom(float p) const;
			Padding absLeft(float p) const;
			Padding absRight(float p) const;
			Padding absY(float p) const;
			Padding absX(float p) const;

			float top{ 0.0f };
			float bottom{ 0.0f };
			float left{ 0.0f };
			float right{ 0.0f };
			Mode topmode{ Mode::Absolute };
			Mode bottommode{ Mode::Absolute };
			Mode leftmode{ Mode::Absolute };
			Mode rightmode{ Mode::Absolute };
		};

		Vec2 size(const Padding&);

		bool hasNANS(const Padding&);

		struct DrawContext {
			/**
			 * \return scaled size of the context.
			 */
			Vec2 size() const { return Vec2{ bottomright.x - topleft.x, topleft.y - bottomright.y }; }

			Vec2 centerpos() const { return 0.5f * (topleft + bottomright); }

			float left() const { return topleft.x; }

			float right() const { return bottomright.x; }

			float top() const { return topleft.y; }

			float bottom() const { return bottomright.y; }

			/**
			 * \param dist SCALED absolute size that should be cut from the left.
			 */
			void cutLeft(float dist) { topleft.x += dist; }

			/**
			 * \param dist SCALED absolute size that should be cut from the right.
			 */
			void cutRight(float dist) { bottomright.x -= dist; }

			/**
			 * \param dist SCALED absolute size that should be cut from the top.
			 */
			void cutTop(float dist) { topleft.y -= dist; }

			/**
			 * \param dist SCALED absolute size that should be cut from the bottom.
			 */
			void cutBottom(float dist) { bottomright.y += dist; }

			/**
			 * \param amount SCALED size to grow the context around its center.
			 */
			void grow(Vec2 scaledSize);

			void assertState() const 		{
				assert(topleft.x <= bottomright.x && topleft.y >= bottomright.y);
			}
			XAlign xalign{ XAlign::Left };
			YAlign yalign{ YAlign::Top };
			Vec2 topleft{ 0.0f, 0.0f };
			Vec2 bottomright{ 0.0f, 0.0f };
			Vec2 clipMin{ -1.0f, -1.0f };
			Vec2 clipMax{ 1.0f,  1.0f };
			RenderSpace2d renderSpace{ RenderSpace2d::Pixel };
			float renderDepth{ 0.0f };
			float scale{ 1.0f };
			u32 root{ 0xFFFFFFFF };
			/**
			 * Tells flexible elements to either fill up all available space or to size themselfs the smallest size possible in the horizontal direction.
			 */
			bool bFlexFillX{ true };
			/**
			 * Tells flexible elements to either fill up all available space or to size themselfs the smallest size possible in vertical direction.
			 */
			bool bFlexFillY{ true };
		};

		struct Placing;

		struct Sizing {

			Sizing absX(float size) const;

			Sizing absY(float size) const;

			Sizing relX(float size) const;

			Sizing relY(float size) const;

			float x{ 1.0f };
			float y{ 1.0f };
			Mode xmode{ Mode::Absolute };
			Mode ymode{ Mode::Absolute };

			void changeBy(Vec2 diff, const DrawContext& context, Placing& place);
		};

		/**
		 * \param sizeing defines the size paramters relative to the given context parameter.
		 * \param context is the relative frame ans scaling used to determine the size.
		 * \return size SCALED by the context.scale.
		 */
		static constexpr Vec2 getSize(const Sizing& sizeing, const DrawContext& context) 	{
			return Vec2{
				sizeing.xmode == Mode::Absolute ? sizeing.x * context.scale : sizeing.x * context.size().x,
				sizeing.ymode == Mode::Absolute ? sizeing.y * context.scale : sizeing.y * context.size().y,
			};
		}

		struct Placing {
			enum class XMode {
				RelativeLeft,
				RelativeRight,
				AbsoluteLeft,
				AbsoluteRight
			};

			enum class YMode {
				RelativeTop,
				RelativeBottom,
				AbsoluteTop,
				AbsoluteBottom
			};

			Placing absDistLeft(float dist) const;

			Placing absDistRight(float dist) const;

			Placing absDistTop(float dist) const;

			Placing absDistBottom(float dist) const;

			Placing relDistLeft(float dist) const;

			Placing relDistRight(float dist) const;

			Placing relDistTop(float dist) const;

			Placing relDistBottom(float dist) const;

			void move(Vec2 dist, const DrawContext& context, const Sizing& sizing);

			float x{ 0.5 };
			float y{ 0.5 };
			XMode xmode{ XMode::RelativeLeft };
			YMode ymode{ YMode::RelativeTop };
		};

		void fit(DrawContext& context, Vec2 scaledSize, Vec2 place);

		void fit(DrawContext& context, Padding const& padding);

	}
}