#pragma once

#include <atomic>
#include <optional>
#include <concepts>

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

#if DAXA_THREADSAFETY
#define DAXA_ONLY_IF_THREADSAFETY(x) x
#define DAXA_ATOMIC_U64 std::atomic_uint64_t
#define DAXA_ATOMIC_FETCH_INC(x) x.fetch_add(1)
#define DAXA_ATOMIC_ADD_FETCH(x, v) x.fetch_add(v)
#define DAXA_ATOMIC_FETCH_DEC(x) x.fetch_sub(1)
#define DAXA_ATOMIC_FETCH(x) x.load()
#else
#define DAXA_ONLY_IF_THREADSAFETY(x)
#define DAXA_ATOMIC_U64 daxa::types::u64
#define DAXA_ATOMIC_FETCH_INC(x) (x++)
#define DAXA_ATOMIC_FETCH_DEC(x) (x--)
#define DAXA_ATOMIC_ADD_FETCH(x, v) (x += v)
#define DAXA_ATOMIC_FETCH(x) (x)
#endif

namespace daxa
{
    struct ManagedSharedState
    {
        virtual ~ManagedSharedState() = default;

        DAXA_ATOMIC_U64 weak_count = {};
        DAXA_ATOMIC_U64 strong_count = {};
        virtual auto managed_cleanup() -> bool;
    };

    struct ManagedWeakPtr
    {
        ManagedSharedState * object = {};

        template <typename T>
        auto as() -> T *
        {
#if DAXA_VALIDATION
            auto ret = dynamic_cast<T *>(object);
            u64 strong_count = DAXA_ATOMIC_FETCH(object->strong_count);
            DAXA_DBG_ASSERT_TRUE_M(strong_count > 0, "strong count must be greater then zero when a weak ptr is still alive!");
            DAXA_DBG_ASSERT_TRUE_M(ret != nullptr, "bad dynamic cast");
            return ret;
#else
            return reinterpret_cast<T *>(object);
#endif
        }
        template <typename T>
        auto as() const -> T const *
        {
#if DAXA_VALIDATION
            auto ret = dynamic_cast<T const *>(object);
            u64 strong_count = DAXA_ATOMIC_FETCH(object->strong_count);
            DAXA_DBG_ASSERT_TRUE_M(strong_count > 0, "strong count must be greater then zero when a weak ptr is still alive!");
            DAXA_DBG_ASSERT_TRUE_M(ret != nullptr, "bad dynamic cast");
            return ret;
#else
            return reinterpret_cast<T const *>(object);
#endif
        }

        ManagedWeakPtr() = default;
        ManagedWeakPtr(ManagedSharedState * ptr);
        ~ManagedWeakPtr();

        ManagedWeakPtr(const ManagedWeakPtr &);
        ManagedWeakPtr(ManagedWeakPtr &&);
        ManagedWeakPtr & operator=(const ManagedWeakPtr &);
        ManagedWeakPtr & operator=(ManagedWeakPtr &&);
    };

    struct ManagedPtr
    {
        ManagedSharedState * object = {};

        template <typename T>
        auto as() -> T *
        {
#if DAXA_VALIDATION
            auto ret = dynamic_cast<T *>(object);
            DAXA_DBG_ASSERT_TRUE_M(ret != nullptr, "bad dynamic cast");
            return ret;
#else
            return reinterpret_cast<T *>(object);
#endif
        }
        template <typename T>
        auto as() const -> T const *
        {
#if DAXA_VALIDATION
            auto ret = dynamic_cast<T const *>(object);
            DAXA_DBG_ASSERT_TRUE_M(ret != nullptr, "bad dynamic cast");
            return ret;
#else
            return reinterpret_cast<T const *>(object);
#endif
        }

        ManagedPtr() = default;
        ManagedPtr(ManagedSharedState * ptr);
        ~ManagedPtr();

        ManagedPtr(const ManagedPtr &);
        ManagedPtr(ManagedPtr &&);
        ManagedPtr & operator=(const ManagedPtr &);
        ManagedPtr & operator=(ManagedPtr &&);

        void cleanup();
        ManagedWeakPtr make_weak();
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
} // namespace daxa
