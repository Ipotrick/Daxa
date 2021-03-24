#pragma once

#include <iostream>

#include "../DaxaCore.hpp"

#include "math.hpp"

namespace daxa {

    template<std::floating_point T>
    struct TVec4 {
        constexpr static TVec4<T> From255(u8 r, u8 g, u8 b, u8 a = 255)
        {
            return TVec4{ static_cast<T>(r) / 255.0f, static_cast<T>(g) / 255.0f, static_cast<T>(b) / 255.0f, static_cast<T>(a) / 255.0f };
        }

        constexpr T const* data() const { return &x; }

        constexpr T& operator[](u32 index)
        {
            DAXA_ASSERT(index < 4);
            return (&x)[index];
        }

        constexpr T const operator[](u32 index) const
        {
            DAXA_ASSERT(index < 4);
            return (&x)[index];
        }

        T x{ 0.0f }; // x coordinate
        T y{ 0.0f }; // y coordinate
        T z{ 0.0f }; // z coordinate
        T w{ 0.0f }; // w coordinate
    };

    template<std::floating_point T>
    inline constexpr  TVec4<T> operator-(TVec4<T> const& vec)
    {
        return TVec4<T>(-vec.x, -vec.y, -vec.z, -vec.w);
    }

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

    #define DAXA_TVEC4_OPERATOR_IMPL(op)\
    template<std::floating_point T> \
    inline constexpr TVec4<T> operator##op##(TVec4<T> vec, T const scalar) \
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
    inline constexpr TVec4<T>& operator##op##=(TVec4<T>& vec, T const scalar) \
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
    inline T norm(TVec4<T> const& vec)
    {
        return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
    }

    template<std::floating_point T>
    inline T length(TVec4<T> const& vec)
    {
        return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
    }

    template<std::floating_point T>
    inline T dot(TVec4<T> const& vecA, TVec4<T> const& vecB)
    {
        return (vecA.x * vecB.x + vecA.y * vecB.y + vecA.z * vecB.z + vecA.w * vecB.w);
    }

    template<std::floating_point T>
    inline std::istream& operator>>(std::istream& is, TVec4<T>& vec)
    {
        is >> vec.x >> vec.y >> vec.z >> vec.w;
        return is;
    }

    template<std::floating_point T>
    inline std::ostream& operator<<(std::ostream& os, TVec4<T> const& vec)
    {
        os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ')';
        return os;
    }

}