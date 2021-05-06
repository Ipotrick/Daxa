#pragma once

#include <vector>
#include <memory>
#include <any>

#include "../DaxaCore.hpp"

#include "../util/UUID.hpp"

//namespace daxa {
//
//	struct EntityHandle {
//		u32 index{ ~0 };
//		u32 version{ ~0 };
//	};
//
//	struct EntityInfo {
//		u32 storageOffset{ ~0 };
//		u16 storageIndex{ ~0 };
//	};
//
//	struct ArchetypeStorage {
//		std::vector<u32> typeToComponentArray;
//
//		std::vector<std::any> componentArrays;
//	};
//
//	template<typename T>
//	static inline constexpr u32 typeIdOf() {
//		static_assert(false):
//	}
//
//	template<> static inline constexpr u32 typeIdOf<u32>() { return 0; }
//
//	struct ECS {
//		std::vector<EntityInfo> entityInfos;
//		std::vector<u32> entityInfoVersions;
//		std::vector<u32> freeEntityIndices;
//
//		std::vector<std::unique_ptr<ArchetypeStorage>> storages;
//
//		bool isValid(EntityHandle handle) const {
//			return handle.index < entityInfos.size() && entityInfoVersions[handle.index] == handle.version;
//		}
//
//		template<typename T>
//		void getComp(EntityHandle handle, const T& comp) {
//			assert(isValid(handle));
//
//			const auto& entityInfo = entityInfos[handle.index];
//			auto& storage = *storages[entityInfo.storageIndex];
//			const u32 storageTypeIndex = storage.typeToComponentArray[typeIdOf<T>()];
//			std::vector<T>& componentArray = std::any_cast<std::vector<T>>(storage.componentArrays[storageTypeIndex]);
//			return componentArray[entityInfo.storageOffset];
//		}
//	};
//}
