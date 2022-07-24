#pragma once

#include <daxa/types.hpp>

#if !defined(NDEBUG)
#define DAXA_DEBUG
#endif

#if defined(DAXA_DEBUG)
#include <iostream>
#include <cstdlib>
#include <stdexcept>

#define DAXA_DBG_ASSERT_FAIL_STRING "[[DAXA ASSERT FAILURE]]"

#define DAXA_DBG_ASSERT_TRUE_M(x, m)                                              \
    [&] {                                                                         \
        if (!(x))                                                                 \
        {                                                                         \
            std::cerr << DAXA_DBG_ASSERT_FAIL_STRING << ": " << (m) << std::endl; \
            throw std::runtime_error("DAXA DEBUG ASSERTION FAILURE");             \
        }                                                                         \
    }()
#else

#define DAXA_DBG_ASSERT_TRUE_M(x, m)

#endif

#if defined(_WIN32)
// HACK TO NOT INCLUDE Windows.h, DECLARE HWND
typedef struct HWND__ * HWND;

namespace daxa
{
    using NativeWindowHandle = HWND;
} // namespace daxa
#endif

#if defined(__linux__)
#include <X11/Xlib.h>

namespace daxa
{
    using NativeWindowHandle = ::Window;
} // namespace daxa
#endif

namespace daxa
{

    struct Handle
    {
        Handle(std::shared_ptr<void> impl);

      protected:
        std::shared_ptr<void> impl = {};
    };

    struct ResultErr
    {
        std::string message = {};
    };

    template <typename T>
    struct Result
    {
        std::optional<T> v = {};
        std::string m = {};

      public:
        Result(T && value)
            : v{std::move(value)}, m{""}
        {
        }

        Result(T const & value)
            : v{value}, m{""}
        {
        }

        Result(std::optional<T> && opt)
            : v{std::move(opt)}, m{opt.has_value() ? "" : "default error message"}
        {
        }

        Result(std::optional<T> const & opt)
            : v{opt}, m{opt.has_value() ? "" : "default error message"}
        {
        }

        Result(ResultErr const & err)
            : v{std::nullopt}, m{err.message}
        {
        }

        Result(std::string_view message)
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

        operator bool() const
        {
            return v.has_value();
        }

        auto operator!() const -> bool
        {
            return !v.has_value();
        }
    };
} // namespace daxa
