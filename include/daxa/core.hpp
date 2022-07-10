#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <array>
#include <filesystem>

#include <daxa/types.hpp>

#ifdef _Debug
#define DAXA_DEBUG
#endif

#ifdef DAXA_DEBUG
#include <iostream>
#include <cstdlib>

#define DAXA_DBG_ASSERT_FAIL_STRING "[[DAXA ASSERT FAILURE]]"

#define DAXA_DBG_ASSERT_TRUE_M(x, m)                                            \
    [&] {                                                                       \
        if (!x)                                                                 \
        {                                                                       \
            std::cerr << DAXA_DBG_ASSERT_FAIL_STRING << ": " << m << std::endl; \
            std::exit(-1);                                                      \
        }                                                                       \
    }()
#else

#define DAXA_DBG_ASSERT_TRUE_M(x, m)

#endif

namespace daxa
{

    struct Handle
    {
      protected:
        std::shared_ptr<void> backend = {};
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

        bool isOk() const
        {
            return v.has_value();
        }

        bool isErr() const
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
