#include "CoordSystem2d.hpp"

namespace daxa {

	RenderCoordSys::RenderCoordSys(const Window& window, const Camera2d& Camera2d) :
		windowWidth{ window.getSizeVec().x },
		windowHeight{ window.getSizeVec().y },
		camera2d{ Camera2d }
	{ }

	Vec2 RenderCoordSys::convertCoordSys(Vec2 coord, RenderSpace2d from, RenderSpace2d to) const {
		Vec2 corrdInWindowSpace;
		switch (from) {
		case RenderSpace2d::Pixel:
			corrdInWindowSpace = convertCoordSys<RenderSpace2d::Pixel, RenderSpace2d::Window>(coord);
			break;
		case RenderSpace2d::UniformWindow:
			corrdInWindowSpace = convertCoordSys<RenderSpace2d::UniformWindow, RenderSpace2d::Window>(coord);
			break;
		case RenderSpace2d::Window:
			corrdInWindowSpace = coord;
			break;
		case RenderSpace2d::Camera2d:
			corrdInWindowSpace = convertCoordSys<RenderSpace2d::Camera2d, RenderSpace2d::Window>(coord);
			break;
		}
		Vec2 retVal;
		switch (to) {
		case RenderSpace2d::Pixel:
			retVal = convertCoordSys<RenderSpace2d::Window, RenderSpace2d::Pixel>(corrdInWindowSpace);
			break;
		case RenderSpace2d::UniformWindow:
			retVal = convertCoordSys<RenderSpace2d::Window, RenderSpace2d::UniformWindow>(corrdInWindowSpace);
			break;
		case RenderSpace2d::Window:
			retVal = corrdInWindowSpace;
			break;
		case RenderSpace2d::Camera2d:
			retVal = convertCoordSys<RenderSpace2d::Window, RenderSpace2d::Camera2d>(corrdInWindowSpace);
			break;
		}
		return retVal;
	}


	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Pixel, RenderSpace2d::Window>(Vec2 coord) const {
		return {
			coord.x / windowWidth * 2.0f - 1.0f,
			coord.y / windowHeight * 2.0f - 1.0f
		};
	}

	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Camera2d, RenderSpace2d::Window>(Vec2 coord) const {
		const f32 uniformXBend = static_cast<f32>(windowHeight) / static_cast<f32>(windowWidth);
		auto res = (rotate(coord - camera2d.position, -camera2d.rotation) * camera2d.frustumBend * camera2d.zoom);
		res.x *= uniformXBend;
		return res;
	}

	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::UniformWindow, RenderSpace2d::Window>(Vec2 coord) const {
		const float xScale = (float)windowWidth / (float)windowHeight;
		coord.x /= xScale;
		return coord;
	}

	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Window, RenderSpace2d::Pixel>(Vec2 coord) const {
		return {
			(coord.x + 1.0f) / 2.0f * windowWidth,
			(coord.y + 1.0f) / 2.0f * windowHeight
		};
	}

	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Window, RenderSpace2d::Camera2d>(Vec2 coord) const {
		const f32 uniformXBend = static_cast<f32>(windowHeight) / static_cast<f32>(windowWidth);
		coord.x /= uniformXBend;
		return rotate(coord / camera2d.frustumBend / camera2d.zoom, camera2d.rotation) + camera2d.position;
	}

	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Window, RenderSpace2d::UniformWindow>(Vec2 coord) const {
		const float xScale = (float)windowWidth / (float)windowHeight;
		coord.x *= xScale;
		return coord;
	}

	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Camera2d, RenderSpace2d::Pixel>(Vec2 coord) const {
		return convertCoordSys<RenderSpace2d::Window, RenderSpace2d::Pixel>(
			convertCoordSys<RenderSpace2d::Camera2d, RenderSpace2d::Window>(coord));
	}

	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Pixel, RenderSpace2d::Camera2d>(Vec2 coord) const {
		return convertCoordSys<RenderSpace2d::Window, RenderSpace2d::Camera2d>(
			convertCoordSys<RenderSpace2d::Pixel, RenderSpace2d::Window>(coord));
	}

	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::UniformWindow, RenderSpace2d::Pixel>(Vec2 coord) const {
		return convertCoordSys<RenderSpace2d::Window, RenderSpace2d::Pixel>(
			convertCoordSys<RenderSpace2d::UniformWindow, RenderSpace2d::Window>(coord));
	}
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Pixel, RenderSpace2d::UniformWindow>(Vec2 coord) const {
		return convertCoordSys<RenderSpace2d::Window, RenderSpace2d::UniformWindow>(
			convertCoordSys<RenderSpace2d::Pixel, RenderSpace2d::Window>(coord));
	}

	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::UniformWindow, RenderSpace2d::Camera2d>(Vec2 coord) const {
		return convertCoordSys<RenderSpace2d::Window, RenderSpace2d::Camera2d>(
			convertCoordSys<RenderSpace2d::UniformWindow, RenderSpace2d::Window>(coord));
	}

	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Camera2d, RenderSpace2d::UniformWindow>(Vec2 coord) const {
		return convertCoordSys<RenderSpace2d::Window, RenderSpace2d::UniformWindow>(
			convertCoordSys<RenderSpace2d::Camera2d, RenderSpace2d::Window>(coord));
	}

}