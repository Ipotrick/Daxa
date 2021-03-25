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

	// credit: https://en.wikipedia.org/wiki/Fast_inverse_square_root
	float invSqrt(f32 number)
	{
		const float x2 = number * 0.5f;
		const float threehalfs = 1.5f;

		union {
			float f;
			uint32_t i;
		} conv = { .f = number };
		conv.i = 0x5f3759df - (conv.i >> 1);
		conv.f *= threehalfs - (x2 * conv.f * conv.f);
		return conv.f;
	}
}