#pragma once

#include <atomic>
#include <optional>
#include <concepts>
#include <span>
#include <bit>

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
#define DAXA_DBG_ASSERT_TRUE_MS(x, STREAM)                                        \
    [&] {                                                                         \
        if (!(x))                                                                 \
        {                                                                         \
            std::cerr << DAXA_DBG_ASSERT_FAIL_STRING << ": " STREAM << std::endl; \
            throw std::runtime_error("DAXA DEBUG ASSERTION FAILURE");             \
        }                                                                         \
    }()
#else

#define DAXA_DBG_ASSERT_TRUE_M(x, m)
#define DAXA_DBG_ASSERT_TRUE_MS(x, m)

#endif

#if !defined(DAXA_GPU_ID_VALIDATION)
#define DAXA_GPU_ID_VALIDATION 0
#endif

namespace daxa
{
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
    };

    struct ManagedWeakPtr
    {
        ManagedSharedState * object = {};

        template <typename T>
        auto as() -> T *
        {
#if DAXA_VALIDATION
            DAXA_DBG_ASSERT_TRUE_M(object != nullptr, "can not dereference empty weak pointer!");
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
            DAXA_DBG_ASSERT_TRUE_M(object != nullptr, "can not dereference empty weak pointer!");
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
        explicit ManagedWeakPtr(ManagedSharedState * ptr);
        ~ManagedWeakPtr();

        ManagedWeakPtr(ManagedWeakPtr const &);
        ManagedWeakPtr(ManagedWeakPtr &&) noexcept;
        ManagedWeakPtr & operator=(ManagedWeakPtr const &);
        ManagedWeakPtr & operator=(ManagedWeakPtr &&) noexcept;
    };

    struct ManagedPtr
    {
        ManagedSharedState * object = {};

        template <typename T>
        auto as() -> T *
        {
#if DAXA_VALIDATION
            DAXA_DBG_ASSERT_TRUE_M(object != nullptr, "can not dereference empty weak pointer!");
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
            DAXA_DBG_ASSERT_TRUE_M(object != nullptr, "can not dereference empty weak pointer!");
            auto ret = dynamic_cast<T const *>(object);
            DAXA_DBG_ASSERT_TRUE_M(ret != nullptr, "bad dynamic cast");
            return ret;
#else
            return reinterpret_cast<T const *>(object);
#endif
        }

        ManagedPtr() = default;
        explicit ManagedPtr(ManagedSharedState * ptr);
        ~ManagedPtr();

        ManagedPtr(ManagedPtr const &);
        ManagedPtr(ManagedPtr &&) noexcept;
        ManagedPtr & operator=(ManagedPtr const &);
        ManagedPtr & operator=(ManagedPtr &&) noexcept;

        ManagedWeakPtr make_weak() const;

        auto is_valid() const -> bool;
        operator bool() const;

      private:
        void cleanup();
    };

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
