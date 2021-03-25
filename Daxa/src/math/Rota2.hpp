#pragma once

#include "Vec2.hpp"

namespace daxa {

	template<std::floating_point T>
	struct TRotation2 {
		constexpr TRotation2() = default;
		TRotation2(T angle) : cos{ std::cos(angle/RAD<T>::value) }, sin{ std::sin(angle/RAD<T>::value) }{}

		static constexpr TRotation2<T> fromUnitVec(TVec2<T> vec)
		{
			TRotation2<T> rota;
			rota.cos = vec.x;
			rota.sin = vec.y;
			return rota;
		}

		constexpr TRotation2<T> operator -()
		{
			TRotation2<T> rota;
			rota.cos = -this->sin;
			rota.sin = this->cos;
			return rota;
		}

		constexpr bool operator==(TRotation2<T> v)
		{
			return this->cos == v.cos && this->sin == v.sin;
		}

		constexpr TVec2<T> toUnitVec()
		{
			return { cos,sin };
		}

		T cos{ static_cast<T>(1.0) };
		T sin{ static_cast<T>(0.0) };
	};

	template<std::floating_point T>
	inline std::ostream& operator<<(std::ostream& os, TRotation2<T> rota)
	{
		return os << '(' << rota.cos << ',' << rota.sin << ')';
	}

	template<std::floating_point T>
	inline f32 angle(TRotation2<T> rota)
	{
		return std::atan2(rota.sin, rota.cos) * RAD_F32;
	}

	template<std::floating_point T>
	inline constexpr TRotation2<T> operator+(TRotation2<T> a, TRotation2<T> b)
	{
		TRotation2<T> rota;
		rota.cos = a.cos * b.cos - a.sin * b.sin;
		rota.sin = a.sin * b.cos + a.cos * b.sin;
		return rota;
	}

	template<std::floating_point T>
	inline constexpr TRotation2<T>& operator+=(TRotation2<T>& a, TRotation2<T> b)
	{
		return a = a + b;
	}

	template<std::floating_point T>
	inline constexpr TRotation2<T> operator-(TRotation2<T> a, TRotation2<T> b)
	{
		return a + -b;
	}

	template<std::floating_point T>
	inline constexpr TRotation2<T>& operator-=(TRotation2<T>& a, TRotation2<T> b)
	{
		return a = a - b;
	}

	template<std::floating_point T>
	inline constexpr Vec2 rotate(Vec2 vec, TRotation2<T> rotation)
	{
		return {
			vec.x * rotation.cos - vec.y * rotation.sin,
			vec.x * rotation.sin + vec.y * rotation.cos
		};
	}

	template<std::floating_point T>
	inline constexpr Vec2 rotateInverse(Vec2 vec, TRotation2<T> rotation)
	{
		return {
			vec.x * rotation.cos + vec.y * rotation.sin,
			-vec.x * rotation.sin + vec.y * rotation.cos
		};
	}

	template<std::floating_point T>
	inline constexpr Vec2 aabbBounds(Vec2 size, TRotation2<T> rotaVec)
	{
		const Vec2 halfsize = size * 0.5f;

		const Vec2 point1 = rotate(Vec2(halfsize.x, halfsize.y), rotaVec);
		const Vec2 point2 = rotate(Vec2(halfsize.x, -halfsize.y), rotaVec);
		const Vec2 point3 = rotate(Vec2(-halfsize.x, halfsize.y), rotaVec);
		const Vec2 point4 = rotate(Vec2(-halfsize.x, -halfsize.y), rotaVec);

		const Vec2 maxPos = max(max(point1, point2), max(point3, point4));
		const Vec2 minPos = min(min(point1, point2), min(point3, point4));

		return maxPos - minPos;
	}

	using Rotation2 = TRotation2<f32>;
}