#pragma once

#include <daxa/types.hpp>

#if !defined(DAXA_THREADSAFETY)
#define DAXA_THREADSAFETY 1
#endif

#if !defined(DAXA_VALIDATION)
#if defined(NDEBUG)
#define DAXA_VALIDATION 0
#else
#define DAXA_VALIDATION 1
#endif
#endif

#if DAXA_VALIDATION
#include <iostream>
#include <cstdlib>
#include <stdexcept>

#define DAXA_GPU_ID_VALIDATION 1

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

#if !defined(DAXA_GPU_ID_VALIDATION)
#define DAXA_GPU_ID_VALIDATION 0
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

    template <typename Derived>
    struct HandleWithCleanup
    {
        HandleWithCleanup(std::shared_ptr<void> impl) : impl(impl) {}

        HandleWithCleanup(HandleWithCleanup const &) = default;
        HandleWithCleanup(HandleWithCleanup &&) = default;
        HandleWithCleanup & operator=(HandleWithCleanup const & other)
        {
            static_cast<Derived *>(this)->cleanup();
            this->impl = other.impl;
            return *this;
        }
        HandleWithCleanup & operator=(HandleWithCleanup && other)
        {
            static_cast<Derived *>(this)->cleanup();
            std::swap(this->impl, other.impl);
            return *this;
        }
        ~HandleWithCleanup() { static_cast<Derived *>(this)->cleanup(); }

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
