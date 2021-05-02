#include "GUIDrawContext.hpp"

namespace daxa {
	namespace gui {

		void Sizing::changeBy(Vec2 diff, const DrawContext& context, Placing& place) 	{
			Vec2 scaledDiff = diff / context.scale;
			switch (xmode) {
			case Mode::Absolute:
				x += scaledDiff.x;
				break;
			case Mode::Relative:
				x += scaledDiff.x / context.size().x; break;
			default:
				break;
			}
			switch (ymode) {
			case Mode::Absolute:
				y -= scaledDiff.y;
				break;
			case Mode::Relative:
				y += scaledDiff.y / context.size().y;
				break;
			default:
				break;
			}

			if (place.xmode == Placing::XMode::AbsoluteRight) place.move(Vec2{ diff.x,0 }, context, *this);
			if (place.ymode == Placing::YMode::AbsoluteBottom) place.move(Vec2{ 0, diff.y }, context, *this);
		}

		Placing Placing::absDistLeft(float dist) const 	{
			auto ret = *this;
			ret.xmode = XMode::AbsoluteLeft;
			ret.x = dist;
			return ret;
		}

		Placing Placing::absDistRight(float dist) const 	{
			auto ret = *this;
			ret.xmode = XMode::AbsoluteRight;
			ret.x = dist;
			return ret;
		}

		Placing Placing::absDistTop(float dist) const 	{
			auto ret = *this;
			ret.ymode = YMode::AbsoluteTop;
			ret.y = dist;
			return ret;
		}

		Placing Placing::absDistBottom(float dist) const 	{
			auto ret = *this;
			ret.ymode = YMode::AbsoluteBottom;
			ret.y = dist;
			return ret;
		}

		Placing Placing::relDistLeft(float dist) const 	{
			auto ret = *this;
			ret.xmode = XMode::RelativeLeft;
			ret.x = dist;
			return ret;
		}

		Placing Placing::relDistRight(float dist) const 	{
			auto ret = *this;
			ret.xmode = XMode::AbsoluteRight;
			ret.x = dist;
			return ret;
		}

		Placing Placing::relDistTop(float dist) const 	{
			auto ret = *this;
			ret.ymode = YMode::RelativeTop;
			ret.y = dist;
			return ret;
		}

		Placing Placing::relDistBottom(float dist) const 	{
			auto ret = *this;
			ret.ymode = YMode::RelativeBottom;
			ret.y = dist;
			return ret;
		}

		void Placing::move(Vec2 dist, const DrawContext& context, const Sizing& sizing) 	{
			dist /= context.scale;
			Vec2 contextSize = context.size() / context.scale;
			switch (xmode) {
			case XMode::AbsoluteLeft:
				x += dist.x;
				break;
			case XMode::AbsoluteRight:
				x -= dist.x;
				break;
			case XMode::RelativeLeft:
				x += dist.x / contextSize.x;
				break;
			case XMode::RelativeRight:
				x -= dist.x / contextSize.x;
				break;
			default: break;
			}
			switch (ymode) {
			case YMode::AbsoluteTop:
				y -= dist.y;
				break;
			case YMode::AbsoluteBottom:
				y += dist.y;
				break;
			case YMode::RelativeTop:
				y -= dist.y / contextSize.y;
				break;
			case YMode::RelativeBottom:
				y += dist.y / contextSize.y;
				break;
			default: break;
			}
		}

		Sizing Sizing::absX(float size) const 	{
			auto ret = *this;
			ret.xmode = Mode::Absolute;
			ret.x = size;
			return ret;
		}
		Sizing Sizing::absY(float size) const 	{
			auto ret = *this;
			ret.ymode = Mode::Absolute;
			ret.y = size;
			return ret;
		}
		Sizing Sizing::relX(float size) const 	{
			auto ret = *this;
			ret.xmode = Mode::Relative;
			ret.x = size;
			return ret;
		}
		Sizing Sizing::relY(float size) const 	{
			auto ret = *this;
			ret.ymode = Mode::Relative;
			ret.y = size;
			return ret;
		}
		Padding Padding::absTop(float p) const 	{
			Padding pad = *this;
			pad.topmode = Mode::Absolute;
			pad.top = p;
			return pad;
		}
		Padding Padding::absBottom(float p) const 	{
			Padding pad = *this;
			pad.bottommode = Mode::Absolute;
			pad.bottom = p;
			return pad;
		}
		Padding Padding::absLeft(float p) const 	{
			Padding pad = *this;
			pad.leftmode = Mode::Absolute;
			pad.left = p;
			return pad;
		}
		Padding Padding::absRight(float p) const 	{
			Padding pad = *this;
			pad.rightmode = Mode::Absolute;
			pad.right = p;
			return pad;
		}
		Padding Padding::absY(float p) const 	{
			Padding pad = *this;
			pad.topmode = Mode::Absolute;
			pad.top = p;
			pad.bottommode = Mode::Absolute;
			pad.bottom = p;
			return pad;
		}
		Padding Padding::absX(float p) const 	{
			Padding pad = *this;
			pad.leftmode = Mode::Absolute;
			pad.left = p;
			pad.rightmode = Mode::Absolute;
			pad.right = p;
			return pad;
		}
		Vec2 size(const Padding& p) 	{
			return Vec2{ p.left + p.right, p.top + p.bottom };
		}
		bool hasNANS(const Padding& p) 	{
			return std::isnan(p.left) || std::isnan(p.right) || std::isnan(p.top) || std::isnan(p.bottom);
		}
		void fit(DrawContext& context, Vec2 scaledSize, Vec2 place) 	{
			context.topleft = place + Vec2{ -scaledSize.x, scaledSize.y } * 0.5f;
			context.bottomright = place - Vec2{ -scaledSize.x, scaledSize.y } *0.5f;
		}
		void fit(DrawContext& context, Padding const& padding) 	{
			switch (padding.topmode) {
			case Mode::Absolute: context.cutTop(padding.top * context.scale); break;
			case Mode::Relative: context.cutTop(padding.top * context.size().y); break;
			default: break;
			}
			switch (padding.bottommode) {
			case Mode::Absolute: context.cutBottom(padding.bottom * context.scale); break;
			case Mode::Relative: context.cutBottom(padding.bottom * context.size().y); break;
			default: break;
			}
			switch (padding.leftmode) {
			case Mode::Absolute: context.cutLeft(padding.left * context.scale); break;
			case Mode::Relative: context.cutLeft(padding.left * context.size().x); break;
			default: break;
			}
			switch (padding.rightmode) {
			case Mode::Absolute: context.cutRight(padding.right * context.scale); break;
			case Mode::Relative: context.cutRight(padding.right * context.size().x); break;
			default: break;
			}
		}

		void DrawContext::grow(Vec2 scaledSize) 	{
			topleft.x -= scaledSize.x * 0.5f * scale;
			bottomright.x += scaledSize.x * 0.5f * scale;
			topleft.y += scaledSize.y * 0.5f * scale;
			bottomright.y -= scaledSize.y * 0.5f * scale;
		}
	}
}