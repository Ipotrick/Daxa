#pragma once

#include <ostream>
#include <iostream>
#include <array>

#include "../DaxaCore.hpp"

#include "math.hpp"

#include "Vec.hpp"

namespace daxa {

	template<size_t M, size_t N, std::floating_point T>
	requires requires{ M >= 2; M <= 4; N >= 2; N <= 4; }
	struct Mat {
		constexpr Mat() = default;
		constexpr Mat(T diagInit)
		{
			for (i32 i = 0; i < std::min(M, N); i++) {
				values[i][i] = diagInit;
			}
		}
		constexpr Mat(const std::array<typename VecType<N, T>::type, M>& arr) {
			for (i32 y = 0; y < M; y++) {
				for (i32 x = 0; x < N; x++) {
					values[y][x] = arr[y][x];
				}
			}
		}

		constexpr std::array<T, M * N>& linear()
		{
			return static_cast<std::array<T, M* N>>(values);
		}
		
		constexpr const std::array<T, M * N>& linear() const
		{
			return static_cast<std::array<T, M* N>>(values);
		}

		constexpr T* data()
		{
			return &values[0][0];
		}

		constexpr std::array<T, N>& operator[](u32 index)
		{
			return values[index];
		}

		constexpr const std::array<T, N>& operator[](u32 index) const
		{
			return values[index];
		}

		std::array<std::array<T, N>, M> values{ static_cast<T>(0.0) };
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
	inline constexpr typename VecType<M,T>::type operator*(const Mat<M, N, T>& mat, typename VecType<N, T>::type vec)
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
	inline constexpr typename VecType<N, T>::type operator*(typename VecType<M, T>::type vec, const Mat<M, N, T>& mat)
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
	inline constexpr Mat<K, L, T> reform(const Mat<M,N,T>& mat, T diagFill = static_cast<T>(1))
	{
		Mat<K, L, T> ret;
		for (u32 y = 0; y < K; y++) {
			for (u32 x = 0; x < L; x++) {
				if ((y < M) & (x < N)) {
					ret[y][x] = mat[y][x];
				}
				else if (y == x) {
					ret[y][x] = diagFill;
				}
				else {
					ret[y][x] = static_cast<T>(0);
				}
			}
		}
		return ret;
	}

	template<size_t M, size_t N, std::floating_point T>
	requires requires { N == M || N-1 == M;  }
	inline constexpr Mat<M, N, T> translate(Mat<M, N, T> mat, typename VecType<N-1, T>::type vec)
	{
		for (i32 i = 0; i < N - 1; i++) {
			mat[i][N - 1] += vec[i];
		}
		return mat;
	}

	template<size_t M, size_t N, std::floating_point T>
	requires requires { N == M || N - 1 == M;  }
	inline constexpr Mat<M, N, T> makeTranslate(typename VecType<N - 1, T>::type vec) 	{
		Mat<M, N, T> mat{static_cast<T>(1)};
		for (i32 i = 0; i < N - 1; i++) {
			mat[i][N - 1] += vec[i];
		}
		return mat;
	}

	template<size_t M, std::floating_point T>
	requires requires { M >= 3; }
	inline Mat<M ,M, T> makeRotaX(T rad)
	{
		Mat<M, M, T> res{ static_cast<T>(1) };
		const T c = std::cos(rad);
		const T s = std::sin(rad);
		res[1][1] =  c;
		res[1][2] = -s;
		res[2][1] =  s;
		res[2][2] =  c;
		return res;
	}

	template<size_t M, std::floating_point T>
	requires requires { M >= 3; }
	inline Mat<M, M, T> makeRotaY(T rad)
	{
		Mat<M, M, T> res{ static_cast<T>(1) };
		const T c = std::cos(rad);
		const T s = std::sin(rad);
		res[0][0] =  c;
		res[0][2] =  s;
		res[2][0] = -s;
		res[2][2] =  c;
		return res;
	}

	template<size_t M, std::floating_point T>
	requires requires { M >= 3; }
	inline Mat<M, M, T> makeRotaZ(T rad)
	{
		Mat<M, M, T> res{ static_cast<T>(1) };
		const T c = std::cos(rad);
		const T s = std::sin(rad);
		res[0][0] =  c;
		res[0][1] = -s;
		res[1][0] =  s;
		res[1][1] =  c;
		return res;
	}

	template<size_t M, std::floating_point T>
	requires requires { M >= 3; }
	inline Mat<M, M, T> makeProjection(const T viewRadians, const T aspectRatio, const T near, const T far)
	{
		Mat<4, 4, T> ret{ static_cast<T>(1) };
		const T scale = static_cast<T>(1) / std::tan(viewRadians * static_cast<T>(0.5));
		ret[0][0] = scale / aspectRatio; // scale the x coordinates of the projected point 
		ret[1][1] = scale; // scale the y coordinates of the projected point 
		ret[2][2] = -far / (far - near); // used to remap z to [0,1] 
		ret[2][3] = -far * near / (far - near); // used to remap z [0,1] 
		ret[3][2] = static_cast<T>(-1); // set w = -z 
		ret[3][3] = static_cast<T>(0);
		return ret;
	}

	template<std::floating_point T>
	inline constexpr std::array<TVec3<T>, 3> getFPSViewAxis(TVec3<T> eye, T pitch, T yaw) 
	{
		T cosPitch = cos(pitch);
		T sinPitch = sin(pitch);
		T cosYaw = cos(yaw);
		T sinYaw = sin(yaw);

		TVec3<T> xaxis = { cosYaw, 0, -sinYaw };
		TVec3<T> yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
		TVec3<T> zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };
		return { xaxis, -yaxis, -zaxis };
	}

	template<std::floating_point T>
	inline constexpr std::array<TVec3<T>, 3> getLookAtAxis(TVec3<T> eye, TVec3<T> at, TVec3<T> up)
	{
		TVec3<T> zaxis = normalize(at - eye);
		TVec3<T> xaxis = normalize(cross(zaxis, up));
		TVec3<T> yaxis = cross(xaxis, zaxis);
		return { xaxis, -yaxis, -zaxis };
	}

	template<std::floating_point T>
	inline constexpr Mat<4, 4, T> makeView(std::array<TVec3<T>, 3> axis, TVec3<T> eye) 
	{
		axis[1] *= static_cast<T>(-1);
		axis[2] *= static_cast<T>(-1);
		return Mat<4, 4, T>{
			{
				TVec4<T>{axis[0].x, axis[0].y, axis[0].z, -dot(axis[0], eye)},
					TVec4<T>{axis[1].x, axis[1].y, axis[1].z, -dot(axis[1], eye)},
					TVec4<T>{axis[2].x, axis[2].y, axis[2].z, -dot(axis[2], eye)},
					TVec4<T>{ 0, 0, 0, 1 }
			} };
	}

	template<std::floating_point T>
	TVec3<T> rotate(TVec3<T> vec, TVec3<T> normal, T amount)
	{
		TVec3<T> vecInNormal = normal * (dot(normal, vec) / dot(normal,normal));
		TVec3<T> clean = vec - vecInNormal;
		TVec3<T> orth = cross(clean, normal);
		T x1 = std::cos(amount) / length(clean);
		T x2 = std::sin(amount) / length(orth);
		TVec3<T> abt = (clean * x1 + orth * x2) * length(clean);
		return abt + vecInNormal;
	}

	template<std::floating_point T>
	inline constexpr Mat<4, 4, T> makeView(TVec3<T> xaxis, TVec3<T> yaxis, TVec3<T> zaxis, TVec3<T> eye) 
	{
		makeView({ xaxis,yaxis,zaxis }, eye);
	}

	template<std::floating_point T>
	inline Mat<4, 4, T> makeLookAt(TVec3<T> eye, TVec3<T> at, TVec3<T> up) 
	{
		return makeView(getLookAtAxis(eye, at, up), eye);
	}

	template<std::floating_point T>
	inline Mat<4, 4, T> makeFPSView(TVec3<T> eye, T pitch, T yaw) 
	{
		return makeView(getFPSViewAxis(eye, pitch, yaw), eye);
	}


	template<size_t M, std::floating_point T>
	requires requires { M >= 3; }
	inline constexpr Mat<M, M, T> rotate(Mat<M,M,T> mat, T x, T y, T z)
	{	
		return makeRotaX<M, T>(x) * makeRotaY<M, T>(y) * makeRotaZ<M, T>(z) * mat;
	}

	template<size_t M, size_t N, std::floating_point T>
	inline constexpr bool hasNANS(const Mat<M, N, T>& mat) {
		auto& lin = mat.linear();
		for (u32 i = 0; i < M * N; i++) {
			if (std::isnan(lin[i])) {
				return true;
			}
		}
		return false;
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
	using Mat2x2f64 = Mat<2, 2, f64>;
	using Mat2x3f64 = Mat<2, 3, f64>;
	using Mat2x4f64 = Mat<2, 4, f64>;
	using Mat3x2f64 = Mat<3, 2, f64>;
	using Mat3x3f64 = Mat<3, 3, f64>;
	using Mat3x4f64 = Mat<3, 4, f64>;
	using Mat4x2f64 = Mat<4, 2, f64>;
	using Mat4x3f64 = Mat<4, 3, f64>;
	using Mat4x4f64 = Mat<4, 4, f64>;
}
