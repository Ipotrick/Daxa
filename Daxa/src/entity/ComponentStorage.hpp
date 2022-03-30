#pragma once

#include "../DaxaCore.hpp"

#include <vector>

#include "Entity.hpp"

namespace daxa {

    class IGenericComponentStorage {
    public:
        virtual ~IGenericComponentStorage() { }

        virtual void remove(EntityIndex index) = 0;
        virtual bool has(EntityIndex index) = 0;
    };

    template<typename FirstComponentType, typename ... RestComponentTypes> class EntityComponentView;

    template<typename ComponentT>
    class ComponentStorageSparseSet : public IGenericComponentStorage {
    public:
        virtual ~ComponentStorageSparseSet() { }

        virtual bool has(EntityIndex index) override final {
            return index < sparseIndices.size() && sparseIndices[index] != INVALID_ENTITY_INDEX;
        }

        virtual void remove(EntityIndex index) override final {
            DAXA_ASSERT_M(has(index), "tried to remove a component of an entity that does NOT posess to to be removed component!");
            EntityIndex denseIndex = sparseIndices[index];
            EntityIndex denseLastIndex = static_cast<EntityIndex>(denseIndices.size() - 1);
            EntityIndex sparseLastIndex = denseIndices[denseLastIndex];

            sparseIndices[index] = INVALID_ENTITY_INDEX;
            sparseIndices[sparseLastIndex] = denseIndex;
            denseIndices[denseIndex] = denseIndices.back();
            values[denseIndex] = std::move(values.back());
            denseIndices.pop_back();
            values.pop_back();
        }

        void add(EntityIndex index, ComponentT const& value) {
            if (sparseIndices.size() <= index) {
                sparseIndices.resize(index + 128, INVALID_ENTITY_INDEX);
            }

            DAXA_ASSERT_M(sparseIndices[index] == INVALID_ENTITY_INDEX, "tried to add component to entity that allready posesses the to be added component!");
            EntityIndex denseIndex = static_cast<EntityIndex>(denseIndices.size());
            sparseIndices[index] = denseIndex;
            denseIndices.push_back(index);
            values.push_back(std::move(value));
        }

        ComponentT& get(EntityIndex index) {
            DAXA_ASSERT_M(has(index), "tried to get a component of an entity that does NOT posess this component!");
            return values[sparseIndices[index]];
        }

        ComponentT const& get(EntityIndex index) const {
            DAXA_ASSERT_M(has(index), "tried to get a component of an entity that does NOT posess this component!");
            return values[sparseIndices[index]];
        }
    private:
        template<typename FirstComponentType, typename ... RestComponentTypes> friend class EntityComponentView;
        //template<typename FirstComponentType, typename ... RestComponentTypes> friend class EntityComponentView<FirstComponentType, RestComponentTypes...>::EntityComponentIterator;

        std::vector<EntityIndex> sparseIndices = {};
        std::vector<EntityIndex> denseIndices = {};
        std::vector<ComponentT> values = {};
    };
}