#pragma once

#include <memory>

#define DAXA_DEFINE_TRIVIAL_MOVE(CLASS_NAME) \
CLASS_NAME::CLASS_NAME(CLASS_NAME&& rhs) noexcept  {\
	std::memcpy(this, &rhs, sizeof(CLASS_NAME));\
	std::memset(&rhs, 0, sizeof(CLASS_NAME));\
}\
CLASS_NAME& CLASS_NAME::operator=(CLASS_NAME&& rhs) noexcept {\
	std::memcpy(this, &rhs, sizeof(CLASS_NAME));\
	std::memset(&rhs, 0, sizeof(CLASS_NAME));\
	return *this;\
}
