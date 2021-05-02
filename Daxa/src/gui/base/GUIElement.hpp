#pragma once

#include "../../DaxaCore.hpp"

#include <math.h>
#include <variant>

#include "GUIDrawContext.hpp"
#include "GUIDrawUtil.hpp"

namespace daxa {
	namespace gui {

		static inline constexpr u32 INVALID_ELEMENT_ID{ 0xFFFFFFFF };

		static inline constexpr Vec4 UNSET_COLOR{ NAN, NAN, NAN, NAN };

		class IElement {};

		template<typename T>
		concept CElement = std::is_base_of_v<IElement, T>;

		template<typename T>
		class ValueOrPtr {
		public:
			ValueOrPtr(const T& t) :value{ t } {}
			ValueOrPtr(T&& t) :value{ t } {}
			ValueOrPtr(T* const t) :value{ t } {}

			ValueOrPtr& operator=(const T& t) { value = t; return *this; }
			ValueOrPtr& operator=(T&& t) { value = t; return *this; }

			ValueOrPtr& operator=(T* t) { value = t; return *this; }

			operator T& () 		{
				if (T** t = std::get_if<T*>(&value)) {
					return **t;
				}
				else {
					return std::get<T>(value);
				}
			}
		private:
			std::variant<T, T*> value;
		};
	}
}
