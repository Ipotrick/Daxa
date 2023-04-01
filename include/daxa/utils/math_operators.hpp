#pragma once

#include <daxa/types.hpp>

namespace daxa::math_operators
{
    template <typename T, usize N>
    constexpr auto operator+(detail::GenericVector<T, N> const & a, detail::GenericVector<T, N> const & b) -> detail::GenericVector<T, N>
    {
        detail::GenericVector<T, N> result;
        for (usize i = 0; i < N; ++i)
            result[i] = a[i] + b[i];
        return result;
    }
    template <typename T, usize N>
    constexpr auto operator-(detail::GenericVector<T, N> const & a, detail::GenericVector<T, N> const & b) -> detail::GenericVector<T, N>
    {
        detail::GenericVector<T, N> result;
        for (usize i = 0; i < N; ++i)
            result[i] = a[i] - b[i];
        return result;
    }
    template <typename T, usize N>
    constexpr auto operator*(detail::GenericVector<T, N> const & a, detail::GenericVector<T, N> const & b) -> detail::GenericVector<T, N>
    {
        detail::GenericVector<T, N> result;
        for (usize i = 0; i < N; ++i)
            result[i] = a[i] * b[i];
        return result;
    }
    template <typename T, usize N>
    constexpr auto operator/(detail::GenericVector<T, N> const & a, detail::GenericVector<T, N> const & b) -> detail::GenericVector<T, N>
    {
        detail::GenericVector<T, N> result;
        for (usize i = 0; i < N; ++i)
            result[i] = a[i] / b[i];
        return result;
    }
    template <typename T, usize N>
    constexpr auto dot(detail::GenericVector<T, N> const & a, detail::GenericVector<T, N> const & b) -> T
    {
        T result = 0;
        for (usize i = 0; i < N; ++i)
            result += a[i] * b[i];
        return result;
    }

    template <typename T, usize N>
    constexpr auto operator+(detail::GenericVector<T, N> const & a, T b) -> detail::GenericVector<T, N>
    {
        detail::GenericVector<T, N> result;
        for (usize i = 0; i < N; ++i)
            result[i] = a[i] + b;
        return result;
    }
    template <typename T, usize N>
    constexpr auto operator-(detail::GenericVector<T, N> const & a, T b) -> detail::GenericVector<T, N>
    {
        detail::GenericVector<T, N> result;
        for (usize i = 0; i < N; ++i)
            result[i] = a[i] - b;
        return result;
    }
    template <typename T, usize N>
    constexpr auto operator*(detail::GenericVector<T, N> const & a, T b) -> detail::GenericVector<T, N>
    {
        detail::GenericVector<T, N> result;
        for (usize i = 0; i < N; ++i)
            result[i] = a[i] * b;
        return result;
    }
    template <typename T, usize N>
    constexpr auto operator/(detail::GenericVector<T, N> const & a, T b) -> detail::GenericVector<T, N>
    {
        detail::GenericVector<T, N> result;
        for (usize i = 0; i < N; ++i)
            result[i] = a[i] / b;
        return result;
    }

    template <typename T, usize M, usize N, usize P>
    constexpr auto operator*(detail::GenericMatrix<T, M, N> const & a, detail::GenericMatrix<T, N, P> const & b)
    {
        auto c = detail::GenericMatrix<T, M, P>{};
        for (usize i = 0; i < M; ++i)
        {
            for (usize j = 0; j < P; ++j)
            {
                c[i][j] = 0;
                for (usize k = 0; k < N; ++k)
                    c[i][j] += a[i][k] * b[k][j];
            }
        }
        return c;
    }
    template <typename T, usize M, usize N>
    constexpr auto operator*(detail::GenericMatrix<T, M, N> const & a, detail::GenericVector<T, N> const & v)
    {
        auto c = detail::GenericVector<T, N>{};
        for (usize i = 0; i < M; ++i)
        {
            c[i] = 0;
            for (usize k = 0; k < N; ++k)
                c[i] += a[i][k] * v[k];
        }
        return c;
    }
    template <typename T, usize N, usize P>
    constexpr auto operator*(detail::GenericVector<T, N> const & v, detail::GenericMatrix<T, N, P> const & b)
    {
        auto c = detail::GenericVector<std::remove_cv_t<T>, N>{};
        for (usize j = 0; j < P; ++j)
        {
            c[j] = 0;
            for (usize k = 0; k < N; ++k)
                c[j] += v[k] * b[k][j];
        }
        return c;
    }

    template <typename T, usize M, usize N>
    constexpr auto transpose(detail::GenericMatrix<T, M, N> const & x)
    {
        auto result = detail::GenericMatrix<T, M, N>{};
        for (usize mi = 0; mi < M; ++mi)
        {
            for (usize ni = 0; ni < N; ++ni)
                result[ni][mi] = x[mi][ni];
        }
        return result;
    }

    template <typename T, usize N>
    constexpr auto vec_from_span(std::span<T, N> const x) -> detail::GenericVector<std::remove_cv_t<T>, N>
    {
        auto result = detail::GenericVector<std::remove_cv_t<T>, N>{};
        for (usize i = 0; i < N; ++i)
            result[i] = x[i];
        return result;
    }

    template <typename T, usize M, usize N>
    constexpr auto mat_from_span(std::span<T, M * N> const x) -> detail::GenericMatrix<std::remove_cv_t<T>, M, N>
    {
        auto result = detail::GenericMatrix<std::remove_cv_t<T>, M, N>{};
        for (usize mi = 0; mi < M; ++mi)
        {
            for (usize ni = 0; ni < N; ++ni)
                result[mi][ni] = x[ni + mi * N];
        }
        return result;
    }
} // namespace daxa::math_operators
