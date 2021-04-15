#pragma once

#include <ostream>

#include "../DaxaCore.hpp"

#include "math.hpp"

#include "Vec2.hpp"

namespace daxa {

    template<std::floating_point T>
    struct TVec3 {
        constexpr TVec3() = default;
        constexpr TVec3(T x, T y, T z) : 
            x{ x },
            y{ y }, 
            z{ z }
        {}

        constexpr TVec3(TVec2<T> xy, T z) :
            x{ xy.x },
            y{ xy.y },
            z{ z }
        {}

        constexpr static TVec3<T> From255(u8 r, u8 g, u8 b)
        {
            return TVec3{ static_cast<T>(r) / static_cast<T>(255.0), static_cast<T>(g) / static_cast<T>(255.0), static_cast<T>(b) / static_cast<T>(255.0) };
        }

        constexpr T const* data() const { return &x; }

        constexpr T& operator[](u32 index)
        {
            return (&x)[index];
        }

        constexpr const T& operator[](u32 index) const
        {
            return (&x)[index];
        }

        T x{ static_cast<T>(0.0) }; // x coordinate
        T y{ static_cast<T>(0.0) }; // y coordinate
        T z{ static_cast<T>(0.0) }; // z coordinate
    };

    template<std::floating_point T>
    inline constexpr bool operator==(const TVec3<T>& a, const TVec3<T>& b)
    {
        return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    }

    template<std::floating_point T>
    inline constexpr bool operator!=(const TVec3<T>& a, const TVec3<T>& b)
    {
        return !operator==(a, b);
    }

    template<std::floating_point T>
    inline constexpr TVec3<T> operator-(TVec3<T> vec)
    {
        return TVec3<T>(-vec.x, -vec.y, -vec.z);
    }

#define DAXA_TVEC3_OPERATOR_IMPL(op)\
    template<std::floating_point T> \
    inline constexpr TVec3<T> operator##op##(TVec3<T> vec, T scalar) \
    { \
        return { \
            vec.x op scalar, \
            vec.y op scalar, \
            vec.z op scalar, \
        }; \
    } \
    template<std::floating_point T> \
    inline constexpr TVec3<T> operator##op##(TVec3<T> a, TVec3<T> b) \
    { \
        return { \
            a.x op b.x, \
            a.y op b.y, \
            a.z op b.z, \
        }; \
    } \
    template<std::floating_point T> \
    inline constexpr TVec3<T>& operator##op##=(TVec3<T>& vec, T scalar) \
    { \
        vec.x op##= scalar;\
        vec.y op##= scalar;\
        vec.z op##= scalar;\
        return vec; \
    } \
    template<std::floating_point T> \
    inline constexpr TVec3<T>& operator##op##=(TVec3<T>& a, TVec3<T> b) \
    { \
        a.x op##= b.x;\
        a.y op##= b.y;\
        a.z op##= b.z;\
        return a; \
    }

    DAXA_TVEC3_OPERATOR_IMPL(*)
    DAXA_TVEC3_OPERATOR_IMPL(/)
    DAXA_TVEC3_OPERATOR_IMPL(+)
    DAXA_TVEC3_OPERATOR_IMPL(-)

    template<std::floating_point T>
    inline bool hasNANS(const TVec3<T>& vec)
    {
        return std::isnan(vec.x) || std::isnan(vec.y) || std::isnan(vec.z);
    }

    template<std::floating_point T>
    inline TVec3<T> round(TVec3<T> vec)
    {
        return TVec3{ std::round(vec.x), std::round(vec.y), std::round(vec.z) };
    }

    template<std::floating_point T>
    inline TVec3<T> floor(TVec3<T> vec)
    {
        return TVec3{ std::floor(vec.x), std::floor(vec.y), std::floor(vec.z) };
    }

    template<std::floating_point T>
    inline TVec3<T> ceil(TVec3<T> vec)
    {
        return TVec3{ std::ceil(vec.x), std::ceil(vec.y), std::ceil(vec.z) };
    }

    template<std::floating_point T>
    inline constexpr TVec3<T> min(TVec3<T> a, TVec3<T> b)
    {
        return {
            std::min(a.x, b.x),
            std::min(a.y, b.y),
            std::min(a.z, b.z)
        };
    }

    template<std::floating_point T>
    inline constexpr TVec3<T> max(TVec3<T> a, TVec3<T> b)
    {
        return {
            std::max(a.x, b.x),
            std::max(a.y, b.y),
            std::max(a.z, b.z)
        };
    }

    template<std::floating_point T>
    inline T length(TVec3<T> vec)
    {
        return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    }

    template<std::floating_point T>
    inline TVec3<T> normalize(TVec3<T> vec)
    {
        const T n = length(vec);
        if (n != static_cast<T>(0.0))     {
            return TVec3<T>(vec.x / n,
                vec.y / n,
                vec.z / n);
        }
        return vec;
    }

    template<std::floating_point T>
    inline T distance(TVec3<T> vecA, TVec3<T> vecB)
    {
        return norm(vecA - vecB);
    }

    template<std::floating_point T>
    inline constexpr T dot(TVec3<T> vecA, TVec3<T> vecB)
    {
        return (vecA.x * vecB.x + vecA.y * vecB.y + vecA.z * vecB.z);
    }

    template<std::floating_point T>
    inline constexpr TVec3<T> cross(TVec3<T> vecA, TVec3<T> vecB)
    {
        return { vecA.y * vecB.z - vecA.z * vecB.y,
                    vecA.z * vecB.x - vecA.x * vecB.z,
                    vecA.x * vecB.y - vecA.y * vecB.x };
    }

    template<std::floating_point T>
    inline constexpr TVec3<T> reflect(TVec3<T> vec, TVec3<T> n)
    {
        return vec - (static_cast<T>(2.0) * dot(n, vec)) * n;
    }

    template<std::floating_point T>
    inline std::ostream& operator<<(std::ostream& os, TVec3<T> const& vec)
    {
        os << '(' << vec.x << ',' << vec.y << ',' << vec.z << ')';
        return os;
    }

    using Vec3 = TVec3<f32>;
    using Vec3f64 = TVec3<f64>;
}