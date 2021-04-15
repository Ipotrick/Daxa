#pragma once

#include <ostream>

#include "../DaxaCore.hpp"

#include "math.hpp"

#include "Vec3.hpp"

namespace daxa {

    template<std::floating_point T>
    struct TVec4 {
        constexpr TVec4() = default;

        constexpr TVec4(T x, T y, T z, T w) :
            x{ x },
            y{ y },
            z{ z },
            w{ w }
        {}

        constexpr TVec4(TVec2<T> vec, T z, T w) :
            x{ vec.x },
            y{ vec.y },
            z{ z },
            w{ w }
        {} 
        
        constexpr TVec4(TVec3<T> vec, T w) :
            x{ vec.x },
            y{ vec.y },
            z{ vec.z },
            w{ w }
        {}

        constexpr static TVec4<T> From255(u8 r, u8 g, u8 b, u8 a = 255)
        {
            return TVec4{ static_cast<T>(r) / 255.0, static_cast<T>(g) / 255.0, static_cast<T>(b) / 255.0, static_cast<T>(a) / 255.0 };
        }

        constexpr T const* data() const { return &x; }

        constexpr T& operator[](u32 index)
        {
            return (&x)[index];
        }

        constexpr T const operator[](u32 index) const
        {
            return (&x)[index];
        }

        T x{ static_cast<T>(0.0) }; // x coordinate
        T y{ static_cast<T>(0.0) }; // y coordinate
        T z{ static_cast<T>(0.0) }; // z coordinate
        T w{ static_cast<T>(0.0) }; // w coordinate
    };

    template<std::floating_point T>
    bool operator==(const TVec4<T>& a, const TVec4<T>& b)
    {
        return a.x == b.x &&
            a.y == b.y &&
            a.z == b.z &&
            a.w == b.w;
    }

    template<std::floating_point T>
    bool operator!=(const TVec4<T>& a, const TVec4<T>& b)
    {
        return !operator==(a, b);
    }

    template<std::floating_point T>
    inline constexpr  TVec4<T> operator-(TVec4<T> const& vec)
    {
        return TVec4<T>(-vec.x, -vec.y, -vec.z, -vec.w);
    }

    #define DAXA_TVEC4_OPERATOR_IMPL(op)\
    template<std::floating_point T> \
    inline constexpr TVec4<T> operator##op##(TVec4<T> vec, T scalar) \
    { \
        return { \
            vec.x op scalar, \
            vec.y op scalar, \
            vec.z op scalar, \
            vec.w op scalar, \
        }; \
    } \
    template<std::floating_point T> \
    inline constexpr TVec4<T> operator##op##(T scalar, TVec4<T> vec) \
    { \
        return { \
            vec.x op scalar, \
            vec.y op scalar, \
            vec.z op scalar, \
            vec.w op scalar, \
        }; \
    } \
    template<std::floating_point T> \
    inline constexpr TVec4<T> operator##op##(TVec4<T> a, TVec4<T> b) \
    { \
        return { \
            a.x op b.x, \
            a.y op b.y, \
            a.z op b.z, \
            a.w op b.w, \
        }; \
    } \
    template<std::floating_point T> \
    inline constexpr TVec4<T>& operator##op##=(TVec4<T>& vec, T scalar) \
    { \
        vec.x op##= scalar;\
        vec.y op##= scalar;\
        vec.z op##= scalar;\
        vec.w op##= scalar;\
        return vec; \
    } \
    template<std::floating_point T> \
    inline constexpr TVec4<T>& operator##op##=(TVec4<T>& a, TVec4<T> b) \
    { \
        a.x op##= b.x;\
        a.y op##= b.y;\
        a.z op##= b.z;\
        a.w op##= b.w;\
        return a; \
    }

    DAXA_TVEC4_OPERATOR_IMPL(*)
    DAXA_TVEC4_OPERATOR_IMPL(/)
    DAXA_TVEC4_OPERATOR_IMPL(+)
    DAXA_TVEC4_OPERATOR_IMPL(-)

    template<std::floating_point T>
    inline bool hasNANS(const TVec4<T>& vec)
    {
        return std::isnan(vec.x) || std::isnan(vec.y) || std::isnan(vec.z) || std::isnan(vec.w);
    }

    template<std::floating_point T>
    inline TVec4<T> round(TVec4<T> vec)
    {
        return TVec4{ std::round(vec.x), std::round(vec.y), std::round(vec.z), std::round(vec.w) };
    }

    template<std::floating_point T>
    inline TVec4<T> floor(TVec4<T> vec)
    {
        return TVec4{ std::floor(vec.x), std::floor(vec.y), std::floor(vec.z), std::floor(vec.w) };
    }

    template<std::floating_point T>
    inline TVec4<T> ceil(TVec4<T> vec)
    {
        return TVec4{ std::ceil(vec.x), std::ceil(vec.y), std::ceil(vec.z), std::ceil(vec.w) };
    }

    template<std::floating_point T>
    inline constexpr TVec4<T> min(TVec4<T> a, TVec4<T> b)
    {
        return { 
            std::min(a.x, b.x),
            std::min(a.y, b.y),
            std::min(a.z, b.z),
            std::min(a.w, b.w)
        };
    }

    template<std::floating_point T>
    inline constexpr TVec4<T> max(TVec4<T> a, TVec4<T> b)
    {
        return {
            std::max(a.x, b.x),
            std::max(a.y, b.y),
            std::max(a.z, b.z),
            std::max(a.w, b.w)
        };
    }

    template<std::floating_point T>
    inline T length(TVec4<T> const& vec)
    {
        return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
    }

    template<std::floating_point T>
    inline T dot(TVec4<T> const& a, TVec4<T> const& b)
    {
        return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
    }

    template<std::floating_point T>
    inline std::ostream& operator<<(std::ostream& os, TVec4<T> const& vec)
    {
        os << '(' << vec.x << ',' << vec.y << ',' << vec.z << ',' << vec.w << ')';
        return os;
    }

    using Vec4 = TVec4<f32>;
    using Vec4f64 = TVec4<f64>;

}