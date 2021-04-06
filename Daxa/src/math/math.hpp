#pragma once

#include "../DaxaCore.hpp"

namespace daxa {

	static inline constexpr f64 PI_F64 = 3.1415926535897932384626433832795028841971693993751058209749445923078164062;
	static inline constexpr f32 PI_F32 = 3.1415926535897932384626433832795028841971693993751058209749445923078164062f;

	template<std::floating_point T>
	struct PI {
		inline static const T value = static_cast<T>(PI_F64);
	};

	static inline constexpr f64 RAD_F64 = 180.0 / PI_F64;
	static inline constexpr f32 RAD_F32 = 180.0f / PI_F32;

	template<std::floating_point T>
	struct RAD {
		inline static const T value = static_cast<T>(RAD_F64);
	};

	template<std::floating_point T>
	struct INV_RAD {
		inline static const T value = static_cast<T>(1) / static_cast<T>(RAD_F64);
	};

	template<std::floating_point T>
	inline constexpr T radians(T deg)
	{
		return deg * INV_RAD<T>::value;
	}

	template<std::floating_point T>
	inline constexpr T degree(T rad)
	{
		return rad * RAD<T>::value;
	}


	// credit: https://en.wikipedia.org/wiki/Fast_inverse_square_root
	inline constexpr f32 invSqrt(f32 number)
	{
		const f32 x2 = number * 0.5f;
		const f32 threehalfs = 1.5f;

		union {
			f32 f;
			u32 i;
		} conv = { .f = number };
		conv.i = 0x5f3759df - (conv.i >> 1);
		conv.f *= threehalfs - (x2 * conv.f * conv.f);
		return conv.f;
	}
}