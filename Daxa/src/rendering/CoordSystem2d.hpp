#pragma once

#include <cassert>

#include <iostream>

#include "../math/Vec2.hpp"

#include "../platform/Window.hpp"
#include "Camera2d.hpp"

namespace daxa {

	enum class RenderSpace2d : char {
		/* world coordinates, (0,0) is world's (0,0) */
		Camera2d,
		/* window (-1 to 1 in x and y) cooordinates, (0,0) is middle of the window */
		Window,
		/*
		* window coordinates that ignore aspect ratio,
		* eg y coordinates are the same as in window space,
		* but the x coordinates are scaled so that they stride
		* is the same of the y axis
		*/
		UniformWindow,
		/* coordinates are pixels, (0,0) is lower left corner */
		Pixel
	};

	inline std::string renderSpaceToStr(RenderSpace2d rs) {
		switch (rs) {
		case RenderSpace2d::Camera2d:
			return "WorldSpace";
		case RenderSpace2d::Window:
			return "WindowSpace";
		case RenderSpace2d::UniformWindow:
			return "UniformWindowSpace";
		case RenderSpace2d::Pixel:
			return "PixelSpace";
		default:
			assert(false);
			return "";
		}
	}

	inline std::ostream& operator<<(std::ostream& ostream, RenderSpace2d rs) {
		ostream << renderSpaceToStr(rs);
		return ostream;
	}

	class RenderCoordSys {
	public:
		RenderCoordSys(const Window& window, const Camera2d& Camera2d);
		RenderCoordSys() = default;
		/**
		 * compile time convertion of coordinate system of vector.
		 *
		 * \param From RenderSpace the coord is in
		 * \param To RenderSpace the corrd should be converted to
		 * \param vec vector to convert
		 * \return converted vector
		 */
		template<RenderSpace2d From, RenderSpace2d To>
		Vec2 convertCoordSys(Vec2 vec) const;

		/**
		 * run time convertion of coordinate system of vector.
		 *
		 * \param vec vector to convert
		 * \param from RenderSpace the coord is in
		 * \param to RenderSpace the corrd should be converted to
		 * \return converted vector
		 */
		Vec2 convertCoordSys(Vec2 vec, RenderSpace2d from, RenderSpace2d to) const;
	private:
		f32 windowWidth{ 1.0f };
		f32 windowHeight{ 1.0f };
		Camera2d camera2d;
	};

	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Pixel, RenderSpace2d::Window>(Vec2 coord) const;
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Camera2d, RenderSpace2d::Window>(Vec2 coord) const;
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::UniformWindow, RenderSpace2d::Window>(Vec2 coord) const;
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Window, RenderSpace2d::Pixel>(Vec2 coord) const;
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Window, RenderSpace2d::Camera2d>(Vec2 coord) const;
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Window, RenderSpace2d::UniformWindow>(Vec2 coord) const;
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Camera2d, RenderSpace2d::Pixel>(Vec2 coord) const;
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Pixel, RenderSpace2d::Camera2d>(Vec2 coord) const;
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::UniformWindow, RenderSpace2d::Pixel>(Vec2 coord) const;
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Pixel, RenderSpace2d::UniformWindow>(Vec2 coord) const;
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::UniformWindow, RenderSpace2d::Camera2d>(Vec2 coord) const;
	template<> Vec2 RenderCoordSys::convertCoordSys<RenderSpace2d::Camera2d, RenderSpace2d::UniformWindow>(Vec2 coord) const;

}