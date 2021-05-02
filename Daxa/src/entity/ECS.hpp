#pragma once

#include <vector>
#include <any>

#include "../DaxaCore.hpp"

namespace daxa {

	template<typename ... T>
	struct ECSArchetypeStorage {
		void* get(u32 typeIndex) {
			return &componentStorages[typeIndexMapping[typeIndex]];
		}

		std::array<u32, sizeof...(T)> typeIndexMapping;
		std::tuple<std::vector<T>, ...> componentStorages;
	};
}
