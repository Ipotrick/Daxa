#pragma once

#include <memory>

#define DAXA_DEFINE_TRIVIAL_MOVE(CLASS_NAME) \
CLASS_NAME::CLASS_NAME(CLASS_NAME&& other) noexcept  {\
	std::memcpy(this, &other, sizeof(CLASS_NAME));\
	std::memset(&other, 0, sizeof(CLASS_NAME));\
}\
CLASS_NAME& CLASS_NAME::operator=(CLASS_NAME&& other) noexcept {\
	if (this != &other) { \
		this->~CLASS_NAME(); \
		new	(this) CLASS_NAME{std::move(other)};\
	}\
	return *this;\
}
