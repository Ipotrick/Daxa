#pragma once

#include <daxa/c/core.h>
#include <optional>
#include <string>

#if defined(DAXA_CORE_USER_DEFINED_H)
#include DAXA_CORE_USER_DEFINED_H
#endif

#if !defined(DAXA_VALIDATION)
#if defined(NDEBUG)
#define DAXA_VALIDATION 0
#else
#define DAXA_VALIDATION 1
#endif
#endif

#if !defined(DAXA_GPU_ID_VALIDATION)
#if DAXA_VALIDATION
#define DAXA_GPU_ID_VALIDATION 1
#else
#define DAXA_GPU_ID_VALIDATION 0
#endif
#endif

#if !defined(DAXA_THROW_M)
#include <iostream>
#include <stdexcept>

#define DAXA_THROW_M(e, m)                                        \
    std::cerr << "[[DAXA ASSERT FAILURE]]: " << (m) << std::endl; \
    throw std::runtime_error(e)
#endif

#if !defined(DAXA_DBG_ASSERT_TRUE_M)
#if DAXA_VALIDATION
#define DAXA_DBG_ASSERT_TRUE_M(x, m)                         \
    do                                                       \
    {                                                        \
        if (std::is_constant_evaluated())                    \
        {                                                    \
            /* how do we check this??? static_assert(x); */  \
        }                                                    \
        else if (!(x))                                       \
        {                                                    \
            DAXA_THROW_M("DAXA DEBUG ASSERTION FAILURE", m); \
        }                                                    \
    } while (false)
#else
#define DAXA_DBG_ASSERT_TRUE_M(x, m)
#endif
#endif

#if !defined(DAXA_REMOVE_DEPRECATED)
#define DAXA_REMOVE_DEPRECATED 1
#endif

namespace daxa
{
    /// @brief  A platform-dependent window resource.
    ///         On Windows, this is an `HWND`
    ///         On Linux X11, this is a `Window`
    ///         On Linux Wayland, this is a `wl_surface *`
    using NativeWindowHandle = void *;

    enum struct NativeWindowPlatform
    {
        UNKNOWN,
        WIN32_API,
        XLIB_API,
        WAYLAND_API,
        MAX_ENUM = 0x7fffffff,
    };
} // namespace daxa

namespace daxa
{
    template <typename T>
    struct Result
    {
        std::optional<T> v = {};
        std::string m = {};

        explicit Result(T && value)
            : v{std::move(value)}, m{""}
        {
        }

        explicit Result(T const & value)
            : v{value}, m{""}
        {
        }

        explicit Result(std::optional<T> && opt)
            : v{std::move(opt)}, m{opt.has_value() ? "" : "default error message"}
        {
        }

        explicit Result(std::optional<T> const & opt)
            : v{opt}, m{opt.has_value() ? "" : "default error message"}
        {
        }

        explicit Result(std::string_view message)
            : v{std::nullopt}, m{message}
        {
        }

        bool is_ok() const
        {
            return v.has_value();
        }

        bool is_err() const
        {
            return !v.has_value();
        }

        auto value() const -> T const &
        {
            return v.value();
        }

        auto value() -> T &
        {
            DAXA_DBG_ASSERT_TRUE_M(v.has_value(), (m != "" ? m : "tried getting value of empty Result"));
            return v.value();
        }

        auto message() const -> std::string const &
        {
            return m;
        }

        auto to_string() const -> std::string
        {
            if (v.has_value())
            {
                return "Result OK";
            }
            else
            {
                return std::string("Result Err: ") + m;
            }
        }

        operator bool() const
        {
            return v.has_value();
        }

        auto operator!() const -> bool
        {
            return !v.has_value();
        }
    };

    template <>
    struct Result<void>
    {
        bool v = {};
        std::string m = {};

        explicit Result(bool opt)
            : v{opt}, m{opt ? "" : "default error message"}
        {
        }

        explicit Result(std::string_view message)
            : v{false}, m{message}
        {
        }

        bool is_ok() const
        {
            return v;
        }

        bool is_err() const
        {
            return !v;
        }

        auto message() const -> std::string const &
        {
            return m;
        }

        auto to_string() const -> std::string
        {
            if (v)
            {
                return "Result OK";
            }
            else
            {
                return std::string("Result Err: ") + m;
            }
        }

        operator bool() const
        {
            return v;
        }

        auto operator!() const -> bool
        {
            return !v;
        }
    };
} // namespace daxa
