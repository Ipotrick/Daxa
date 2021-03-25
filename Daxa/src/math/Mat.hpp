#pragma once

#include <ostream>
#include <iostream>
#include <array>

#include "../DaxaCore.hpp"

#include "math.hpp"

#include "Vec4.hpp"

namespace daxa {
	template<size_t M, std::floating_point T>
	struct Mat {

		std::array<T, M * M>& linear()
		{
			return static_cast<std::array<T, M* M>>(values);
		}
		
		const std::array<T, M * M>& linear() const
		{
			return static_cast<std::array<T, M* M>>(values);
		}

		T* data()
		{
			return &values[0][0];
		}

		std::array<T, M>& operator[](u32 index)
		{
			return values[index];
		}

		const std::array<T, M>& operator[](u32 index) const
		{
			return values[index];
		}

		std::array<std::array<T, M>, M> values{ 0 };
	};

	template<size_t M, std::floating_point T>
	inline constexpr Mat<M,T> operator+(const Mat<M, T>& a, const Mat<M, T>& b)
	{
		Mat<M,  T> ret;
		auto& rLinear = ret.linear();
		auto& aLinear = a.linear();
		auto& bLinear = b.linear();
		for (u32 i = 0; i < M * M; i++) {
			rLinear[i] = aLinear[i] + bLinear[i];
		}
		return ret;
	}

	template<size_t M, std::floating_point T>
	inline constexpr Mat<M,  T>& operator+=(Mat<M, T>& a, const Mat<M, T>& b)
	{
		auto& aLinear = a.linear();
		auto& bLinear = b.linear();
		for (u32 i = 0; i < M * M; i++) {
			aLinear[i] = aLinear[i] + bLinear[i];
		}
		return a;
	}

	template<size_t M, std::floating_point T>
	inline constexpr Mat<M,  T> operator-(const Mat<M, T>& a, const Mat<M, T>& b)
	{
		Mat<M,  T> ret;
		auto& rLinear = ret.linear();
		auto& aLinear = a.linear();
		auto& bLinear = b.linear();
		for (u32 i = 0; i < M * M; i++) {
			rLinear[i] = aLinear[i] - bLinear[i];
		}
		return ret;
	}

	template<size_t M, std::floating_point T>
	inline constexpr Mat<M,  T>& operator-=(Mat<M, T>& a, const Mat<M, T>& b)
	{
		auto& aLinear = a.linear();
		auto& bLinear = b.linear();
		for (u32 i = 0; i < M * M; i++) {
			aLinear[i] = aLinear[i] - bLinear[i];
		}
		return a;
	}

	template<size_t M, std::floating_point T>
	inline constexpr Mat<M, T> operator*(const Mat<M, T>& a, const Mat<M, T>& b)
	{
		Mat<M, T> ret;

		for (u32 y = 0; y < M; y++) {
			for (u32 x = 0; x < M; x++) {
				ret[y][x] = 0;
				for (u32 i = 0; i < M; i++) {
					ret[y][x] += a.values[y][i] * b.values[i][x];
				}
			}
		}
		return ret;
	}

	template<size_t M, std::floating_point T>
	inline constexpr Mat<M, T>& operator*=(Mat<M, T>& a, const Mat<M, T>& b)
	{
		return a = a * b;
	}

	template<size_t M, std::floating_point T>
	inline constexpr std::ostream& operator<<(std::ostream& os, const Mat<M, T>& mat)
	{
		os << "(";
		for (u32 y = 0; y < M; y++) {
			os << '(';
			for (u32 x = 0; x < M; x++) {
				os << mat[y][x];
				if (x < M - 1) {
					os << ',';
				}
			}
			os << ')';
			if (y < M - 1) {
				os << ',';
			}
		}
		os << ")";
		return os;
	}

	template<std::floating_point T>
	inline constexpr T det2x2(const Mat<2, T>& mat)
	{
		return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
	}

	template<size_t M, std::floating_point T>
	inline constexpr T det(const Mat<M, T>& mat)
	{
		if constexpr (M == 2) {
			return det2x2(mat);
		}
		else {
			T ret = 0;
			for (u32 i = 0; i < M; i++) {
				constexpr T signs[2] = { static_cast<T>(-1),static_cast<T>(1) };
				const T sign = signs[i & 1];
				const T factor = mat[0][i];
				Mat<M - 1, T> subMat;
				for (u32 y = 1; y < M; y++) {
					u32 py = y - 1;
					u32 px{ 0 };
					for (u32 x = 0; x < M; x++) {
						if (x == i) continue;
						subMat[py][px] = mat[y][x];
						px++;
					}
				}

				ret += sign * factor * det(subMat);
			}
			return ret;
		}
	}

	template<size_t M, std::floating_point T>
	inline constexpr T cofactor(const Mat<M, T>& mat, u32 i, u32 j)
	{
		u32 py{ 0 };
		Mat<M - 1, T> subMat;
		for (u32 y = 0; y < M; y++) {
			if (y == i) continue;
			u32 px{ 0 };
			for (u32 x = 0; x < M; x++) {
				if (x == j) continue;
				subMat[py][px] = mat[y][x];
				px++;
			}
			py++;
		}
		constexpr T signs[2] = { static_cast<T>(-1),static_cast<T>(1) };
		const T sign = signs[(i+j) & 1];
		return sign * det(subMat);
	}

	template<size_t M, std::floating_point T>
	inline constexpr Mat<M, T> transpose(const Mat<M,T>& mat)
	{
		Mat<M, T> ret;
		for (u32 y = 0; y < M; y++) {
			for (u32 x = 0; x < M; x++) {
				ret[y][x] = mat[x][y];
			}
		}
		return ret;
	}

	template<size_t M, std::floating_point T>
	inline constexpr Mat<M, T> inverse(const Mat<M, T>& mat)
	{
		Mat<M, T> ret;
		const T invDeterminant = static_cast<T>(1.0) / det(mat);
		for (u32 y = 0; y < M; y++) {
			for (u32 x = 0; x < M; x++) {
				const T cofac = cofactor(mat, y, x);
				ret[x][y] = invDeterminant * cofac;
			}
		}
		return ret;
	}

	using Mat2 = Mat<2, f32>;
	using Mat3 = Mat<3, f32>;
	using Mat4 = Mat<4, f32>;
}
