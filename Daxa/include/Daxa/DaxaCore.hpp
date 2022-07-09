#pragma once

#include <cinttypes>
#include <cassert>
#include <iostream>
#include <optional>
#include <string_view>
#include <memory>
#include <cmath>
#include <algorithm>

using u64 = uint64_t;
using i64 = int64_t;

using u32 = uint32_t;
using i32 = int32_t;

using u16 = uint16_t;
using i16 = int16_t;

using u8 = uint8_t;
using i8 = int8_t;

using f32 = float;
using f64 = double;

using uz = size_t;

#ifdef _DEBUG
#define DAXA_ASSERT(x) assert(x)
#else
#define DAXA_ASSERT(x)
#endif

#define DAXA_ALLWAYS_ASSERT(x) assert(x)

#ifdef _DEBUG
#define DAXA_ASSERT_M(x, message) \
if (!(x)) {\
	std::cerr << "[[DAXA ASSERTION FALIURE]] " << message << std::endl;\
	abort();\
}\
((void)0)
#else
#define DAXA_ASSERT_M(x, message) ((void)0)
#endif

#ifdef _DEBUG
#define DAXA_ASSERT_WARN_M(x, message) \
if (!(x)) {\
	std::cerr << "[[DAXA ASSERTION WARNING]] " << message << std::endl;\
}\
((void)0)
#else
#define DAXA_ASSERT_WARN_M(x, message) ((void)0)
#endif

namespace daxa {
	struct ResultErr {
		std::string message = "";
	};	

	template<typename T>
	class Result {
	public:
		Result(T&& value)
			: v{ std::move(value) }
			, m{ "" }
		{}

		Result(T const& value)
			: v{ value }
			, m{ "" }
		{}

		Result(std::optional<T>&& opt) 
			: v{ std::move(opt) }
			, m{ opt.has_value() ? "" : "default error message" }
		{ }

		Result(std::optional<T> const& opt) 
			: v{ opt }
			, m{ opt.has_value() ? "" : "default error message" }
		{ }

		Result(ResultErr const& err) 
			: v{ std::nullopt }
			, m{ err.message }
		{ }

		Result(std::string_view message)
			: v{ std::nullopt }
			, m{ message }
		{}

		static Result ok(T&& value) {
			return Result(std::move(value));
		}

		static Result err(std::string_view message = "default error message") {
			return Result(message);
		}

		bool isOk() const {
			return v.has_value();
		}

		bool isErr() const {
			return !v.has_value();
		}

		T const& value() const {
			return v.value();
		}

		T& value() {
			DAXA_ASSERT_M(v.has_value(), (m != "" ? m : "tried getting value of empty Result"));
			if (!v.has_value()) {
				std::cout << "message: " << m << std::endl;
			}
			return v.value();
		}

		std::string const& message() const {
			return m;
		}

		operator bool () const {
			return v.has_value();
		}

		bool operator!() const {
			return !v.has_value();
		}
	private:
		std::optional<T> v;
		std::string m;
	};

	template<typename T>
	std::ostream& operator<<(std::ostream& os, Result<T> const& result) {
		if (result.isOk()) {
			os << "[ResultOK]";
		} else {
			os << "[ResultErr] message: \"" << result.message() << "\"";
		}
		return os;
	}

	struct DeviceBackend;
}
