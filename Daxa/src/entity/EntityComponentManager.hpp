#pragma once

#include "../DaxaCore.hpp"

#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "EntityComponentView.hpp"

namespace daxa {
    class EntityComponentManager {
    public:
        EntityComponentManager()
            : versions{ std::make_unique<std::vector<EntityVersion>>() }
        { }

        template<typename ... T>
        EntityComponentView<T...> view() {
            (assureComponentType<T>(),...);
            return EntityComponentView{ versions.get(), dynamic_cast<ComponentStorageSparseSet<T>*>(storages[typeid(T)].get())... };
        }

        EntityHandle createEntity() {
            EntityHandle ret = {};
            if (!freeList.empty()) {
                ret.index = freeList.back();
                freeList.pop_back();
                ret.version = versions->at(ret.index);
            } else {
                versions->push_back(0);
                ret.index = static_cast<EntityIndex>(versions->size() - 1);
                ret.version = 0;
            }
            return ret;
        }

        bool handleValid(EntityHandle entity) const {
            return entity.index < versions->size() && entity.version == versions->at(entity.index);
        }

        void destroyEntity(EntityHandle entity) {
            DAXA_ASSERT_M(handleValid(entity), "invalid entity handle!");
            versions->at(entity.index) += 1;
            freeList.push_back(entity.index);
        }
    private:
        template<typename T>
        void assureComponentType() {
            if (!storages.contains(typeid(T))) {
                storages[typeid(T)] = std::make_unique<ComponentStorageSparseSet<T>>();
            }
        }

        std::vector<EntityIndex> freeList = {};
        std::unique_ptr<std::vector<EntityVersion>> versions = {};
        std::unordered_map<std::type_index, std::unique_ptr<IGenericComponentStorage>> storages = {};
    };
}