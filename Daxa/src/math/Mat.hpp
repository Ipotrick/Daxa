#pragma once

#include <ostream>
#include <iostream>
#include <array>

#include "../DaxaCore.hpp"

#include "math.hpp"

#include "Vec.hpp"

namespace daxa {

	template<size_t M, size_t N, std::floating_point T>
	struct Mat {

		std::array<T, M * N>& linear()
		{
			return static_cast<std::array<T, M* N>>(values);
		}
		
		const std::array<T, M * N>& linear() const
		{
			return static_cast<std::array<T, M* N>>(values);
		}

		T* data()
		{
			return &values[0][0];
		}

		std::array<T, N>& operator[](u32 index)
		{
			return values[index];
		}

		const std::array<T, N>& operator[](u32 index) const
		{
			return values[index];
		}

		std::array<std::array<T, N>, M> values{ 0 };
	};

	template<size_t M, size_t N, std::floating_point T>
	inline constexpr Mat<M, N, T> operator+(const Mat<M, N, T>& a, const Mat<M, N, T>& b)
	{
		Mat<M, N, T> ret;
		auto& rLinear = ret.linear();
		auto& aLinear = a.linear();
		auto& bLinear = b.linear();
		for (u32 i = 0; i < M * N; i++) {
			rLinear[i] = aLinear[i] + bLinear[i];
		}
		return ret;
	}

	template<size_t M, size_t N, std::floating_point T>
	inline constexpr Mat<M, N, T>& operator+=(Mat<M, N, T>& a, const Mat<M, N, T>& b)
	{
		auto& aLinear = a.linear();
		auto& bLinear = b.linear();
		for (u32 i = 0; i < M * N; i++) {
			aLinear[i] = aLinear[i] + bLinear[i];
		}
		return a;
	}

	template<size_t M, size_t N, std::floating_point T>
	inline constexpr Mat<M, N, T> operator-(const Mat<M, N, T>& a, const Mat<M, N, T>& b)
	{
		Mat<M, N, T> ret;
		auto& rLinear = ret.linear();
		auto& aLinear = a.linear();
		auto& bLinear = b.linear();
		for (u32 i = 0; i < M * N; i++) {
			rLinear[i] = aLinear[i] - bLinear[i];
		}
		return ret;
	}

	template<size_t M, size_t N, std::floating_point T>
	inline constexpr Mat<M, N, T>& operator-=(Mat<M, N, T>& a, const Mat<M, N, T>& b)
	{
		auto& aLinear = a.linear();
		auto& bLinear = b.linear();
		for (u32 i = 0; i < M * N; i++) {
			aLinear[i] = aLinear[i] - bLinear[i];
		}
		return a;
	}

	template<size_t M, size_t N, size_t K, std::floating_point T>
	inline constexpr Mat<M, K, T> operator*(const Mat<M, N, T>& a, const Mat<N, K, T>& b)
	{
		Mat<M, K, T> ret;

		for (u32 y = 0; y < M; y++) {
			for (u32 x = 0; x < K; x++) {
				ret[y][x] = 0;
				for (u32 i = 0; i < N; i++) {
					ret[y][x] += a.values[y][i] * b.values[i][x];
				}
			}
		}
		return ret;
	}

	template<size_t M, size_t N, size_t K, std::floating_point T>
	inline constexpr Mat<M, K, T>& operator*=(Mat<M, N, T>& a, const Mat<N, K, T>& b)
	{
		return a = a * b;
	}

	template<size_t M, size_t N, std::floating_point T>
		requires requires() { M >= 2; M <= 4; N >= 2; N <= 4; }
	inline constexpr VecType<M,T>::type operator*(const Mat<M, N, T>& mat, typename VecType<N, T>::type vec)
	{
		typename VecType<M, T>::type ret;
		for (u32 i = 0; i < M; i++) {
			ret[i] = 0;
			for (u32 j = 0; j < N; j++) {
				ret[i] += mat[i][j] * vec[j];
			}
		}
		return ret;
	}
	
	template<size_t M, size_t N, std::floating_point T>
		requires requires() { M >= 2; M <= 4; N >= 2; N <= 4; }
	inline constexpr VecType<N, T>::type operator*(typename VecType<M, T>::type vec, const Mat<M, N, T>& mat)
	{
		typename VecType<N, T>::type ret;
		for (u32 i = 0; i < N; i++) {
			ret[i] = 0;
			for (u32 j = 0; j < M; j++) {
				ret += mat[i][j] * vec[j];
			}
		}
		return ret;
	}

	template<size_t M, size_t N, std::floating_point T>
	inline constexpr Mat<M, N, T> operator*(const Mat<M, N, T>& mat, T scalar)
	{
		Mat<M, N, T> ret;
		auto& retLinear = ret.linear();
		auto& linear = mat.linear();
		for (u32 i = 0; i < M * N; i++) {
			retLinear[i] = linear[i] * scalar;
		}
		return ret;
	}

	template<size_t M, size_t N, std::floating_point T>
	inline constexpr Mat<M, N, T>& operator*=(Mat<M, N, T>& mat, T scalar)
	{
		auto& linear = mat.linear();
		for (u32 i = 0; i < M * N; i++) {
			linear[i] *= scalar;
		}
		return mat;
	}

	template<size_t M, size_t N, std::floating_point T>
	inline constexpr std::ostream& operator<<(std::ostream& os, const Mat<M, N, T>& mat)
	{
		os << "(";
		for (u32 y = 0; y < M; y++) {
			os << '(';
			for (u32 x = 0; x < N; x++) {
				os << mat[y][x];
				if (x < N - 1) {
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
	inline constexpr T det2x2(const Mat<2, 2, T>& mat)
	{
		return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
	}

	template<size_t M, std::floating_point T>
	inline constexpr T det(const Mat<M,M, T>& mat)
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
				Mat<M - 1, M - 1, T> subMat;
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
	inline constexpr T cofactor(const Mat<M, M, T>& mat, u32 i, u32 j)
	{
		if constexpr (M == 1) {
			return mat[0][0];
		}
		else {
			u32 py{ 0 };
			Mat<M - 1, M - 1, T> subMat;
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
			const T sign = signs[(i + j) & 1];
			return sign * det(subMat);
		}
	}

	template<size_t M, size_t N, std::floating_point T>
	inline constexpr Mat<N, M, T> transpose(const Mat<M, N,T>& mat)
	{
		Mat<N, M, T> ret;
		for (u32 y = 0; y < N; y++) {
			for (u32 x = 0; x < M; x++) {
				ret[y][x] = mat[x][y];
			}
		}
		return ret;
	}

	template<size_t M, std::floating_point T>
	inline constexpr Mat<M, M, T> inverse(const Mat<M, M, T>& mat)
	{
		Mat<M, M, T> ret;
		const T invDeterminant = static_cast<T>(1.0) / det(mat);
		if constexpr (M == 2) {
			ret[0][0] = invDeterminant * mat[1][1];
			ret[0][1] = -invDeterminant * mat[0][1];
			ret[1][0] = -invDeterminant * mat[1][0];
			ret[1][1] = invDeterminant * mat[0][0];
		}
		else {
			for (u32 y = 0; y < M; y++) {
				for (u32 x = 0; x < M; x++) {
					const T cofac = cofactor(mat, y, x);
					ret[x][y] = invDeterminant * cofac;
				}
			}
		}
		return ret;
	}

	template<size_t K, size_t L, size_t M, size_t N, std::floating_point T>
	inline constexpr Mat<K, L, T> reform(const Mat<M,N,T>& mat)
	{
		Mat<K, L, T> ret;
		for (u32 y = 0; y < std::min(M, K); y++) {
			for (u32 x = 0; x < std::min(N, L); x++) {
				ret[y][x] = mat[y][x];
			}
		}
		return ret;
	}

	using Mat2x2 = Mat<2, 2, f32>;
	using Mat2x3 = Mat<2, 3, f32>;
	using Mat2x4 = Mat<2, 4, f32>;
	using Mat3x2 = Mat<3, 2, f32>;
	using Mat3x3 = Mat<3, 3, f32>;
	using Mat3x4 = Mat<3, 4, f32>;
	using Mat4x2 = Mat<4, 2, f32>;
	using Mat4x3 = Mat<4, 3, f32>;
	using Mat4x4 = Mat<4, 4, f32>;
}
