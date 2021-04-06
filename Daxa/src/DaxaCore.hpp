#pragma once

#include <cinttypes>
#include <cassert>

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
