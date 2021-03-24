#pragma once

#include "../DaxaCore.hpp"

namespace daxa {

	static inline constexpr f64 PI_F64 = 3.1415926535897932384626433832795028841971693993751058209749445923078164062;
	static inline constexpr f32 PI_F32 = 3.1415926535897932384626433832795028841971693993751058209749445923078164062f;

	template<std::floating_point T>
	struct PI {
		T value = PI_F64;
	};

	template<std::floating_point T>
	struct RAD {
		T value = RAD_F64;
	};

	static inline constexpr f64 RAD_F64 = 180.0 / PI_F64;
	static inline constexpr f32 RAD_F32 = 180.0f / PI_F32;
}