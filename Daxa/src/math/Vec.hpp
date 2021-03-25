#pragma once

#include "Vec4.hpp"

namespace daxa {
	template<size_t SIZE, std::floating_point T>
	requires requires() { SIZE >= 2; SIZE <= 4; }
	struct VecType {};
	template<> struct VecType<2, f32> { using type = TVec2<f32>; };
	template<> struct VecType<3, f32> { using type = TVec3<f32>; };
	template<> struct VecType<4, f32> { using type = TVec4<f32>; };
	template<> struct VecType<2, f64> { using type = TVec2<f64>; };
	template<> struct VecType<3, f64> { using type = TVec3<f64>; };
	template<> struct VecType<4, f64> { using type = TVec4<f64>; };
}
