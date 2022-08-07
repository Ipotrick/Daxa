#pragma once

#include <daxa/types.hpp>

namespace daxa::math_operators
{
    template <typename VecT>
    constexpr auto operator+(detail::GenericVec2<VecT> const & a, detail::GenericVec2<VecT> const & b) -> detail::GenericVec2<VecT>
    {
        return detail::GenericVec2<VecT>{a.x + b.x, a.y + b.y};
    }
    template <typename VecT>
    constexpr auto operator+(detail::GenericVec3<VecT> const & a, detail::GenericVec3<VecT> const & b) -> detail::GenericVec3<VecT>
    {
        return detail::GenericVec3<VecT>{a.x + b.x, a.y + b.y, a.z + b.z};
    }
    template <typename VecT>
    constexpr auto operator+(detail::GenericVec4<VecT> const & a, detail::GenericVec4<VecT> const & b) -> detail::GenericVec4<VecT>
    {
        return detail::GenericVec4<VecT>{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
    }

    template <typename VecT>
    constexpr auto operator-(detail::GenericVec2<VecT> const & a, detail::GenericVec2<VecT> const & b) -> detail::GenericVec2<VecT>
    {
        return detail::GenericVec2<VecT>{a.x - b.x, a.y - b.y};
    }
    template <typename VecT>
    constexpr auto operator-(detail::GenericVec3<VecT> const & a, detail::GenericVec3<VecT> const & b) -> detail::GenericVec3<VecT>
    {
        return detail::GenericVec3<VecT>{a.x - b.x, a.y - b.y, a.z - b.z};
    }
    template <typename VecT>
    constexpr auto operator-(detail::GenericVec4<VecT> const & a, detail::GenericVec4<VecT> const & b) -> detail::GenericVec4<VecT>
    {
        return detail::GenericVec4<VecT>{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    }

    template <typename VecT>
    constexpr auto dot(detail::GenericVec2<VecT> const & a, detail::GenericVec2<VecT> const & b) -> detail::GenericVec2<VecT>
    {
        return a.x * b.x + a.y * b.y;
    }
    template <typename VecT>
    constexpr auto dot(detail::GenericVec3<VecT> const & a, detail::GenericVec3<VecT> const & b) -> detail::GenericVec3<VecT>
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
    template <typename VecT>
    constexpr auto dot(detail::GenericVec4<VecT> const & a, detail::GenericVec4<VecT> const & b) -> detail::GenericVec4<VecT>
    {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }
} // namespace daxa::math_operators
