#include "ComponentStorage.hpp"

#include <tuple>

namespace daxa {

    template<typename FirstComponentType, typename ... RestComponentTypes> 
    class EntityComponentView {
    public:
        EntityComponentView(std::vector<EntityVersion>* versions, ComponentStorageSparseSet<FirstComponentType>* first, ComponentStorageSparseSet<RestComponentTypes>* ... rest)
            : versions{ versions }
            , storages{ first, rest... }
        {
            DAXA_ASSERT_M(versions, "version and component storages need need to be valid pointers!");
            DAXA_ASSERT_M((first && (rest && ...)), "version and component storages need need to be valid pointers!");
        }

        bool handleValid(EntityHandle handle) const {
            return handle.version <= versions->size() && versions->at(handle.index) == handle.version;
        }

        template<typename SpecificType>
        void addComp(EntityHandle handle, SpecificType&& value) {
            DAXA_ASSERT_M(handleValid(handle), "invalid entity handle!");
            ComponentStorageSparseSet<SpecificType>* storage = std::get<ComponentStorageSparseSet<SpecificType>*>(storages);
            storage->add(handle.index, std::move(value));
        }

        void remComp(EntityHandle handle) {
            DAXA_ASSERT_M(handleValid(handle), "invalid entity handle!");
            ComponentStorageSparseSet<SpecificType>* storage = std::get<ComponentStorageSparseSet<SpecificType>*>(storages);
            storage->remove(handle.index);
        }

        bool hasComp(EntityHandle handle) {
            DAXA_ASSERT_M(handleValid(handle), "invalid entity handle!");
            ComponentStorageSparseSet<SpecificType>* storage = std::get<ComponentStorageSparseSet<SpecificType>*>(storages);
            return storage->has(handle.index);
        }

        template<typename FirstSpecificType, typename ... RestSpecificTypes>
        void addComps(EntityHandle handle) {
            (addComp<FirstSpecificType>(handle), ...);
        }

        template<typename FirstSpecificType, typename ... RestSpecificTypes>
        void remComps(EntityHandle handle) {
            (remComp<FirstSpecificType>(handle), ...);
        }

        template<typename FirstSpecificType, typename ... RestSpecificTypes>
        bool hasComps(EntityHandle handle) {
            return (hasComp<FirstSpecificType>(handle) && ...);
        }
        
        class EntityComponentIterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type   = EntityIndex;
            using value_type        = std::tuple<EntityHandle, FirstComponentType&, RestComponentTypes&...>;
            using pointer           = value_type*;  // or also value_type*
            using reference         = value_type&;  // or also value_type&

            EntityComponentIterator(EntityIndex denseIndex, std::vector<EntityVersion>* versions, std::tuple<ComponentStorageSparseSet<FirstComponentType>*, ComponentStorageSparseSet<RestComponentTypes>* ...>* storages)
                : denseIndex{ denseIndex }
                , versions{ versions }
                , storages{ storages }
            {}

            value_type operator*() const { 
                auto sparseIndex = std::get<0>(*storages)->denseIndices[denseIndex];
                return value_type{
                    EntityHandle{ sparseIndex, (*versions)[sparseIndex] },
                    std::get<ComponentStorageSparseSet<FirstComponentType>*>(*storages)->get(sparseIndex),
                    std::get<ComponentStorageSparseSet<RestComponentTypes>*>(*storages)->get(sparseIndex) ...
                };
            }

            EntityComponentIterator& operator++() {
                EntityIndex sparseIndex;
                do {
                    denseIndex += 1;
                    if (denseIndex >= std::get<0>(*storages)->denseIndices.size()) {
                        return *this;
                    }
                    sparseIndex = std::get<0>(*storages)->denseIndices[denseIndex];
                } while(!(std::get<ComponentStorageSparseSet<RestComponentTypes>*>(*storages)->has(sparseIndex) && ...));

                return *this;
            }

            bool operator == (const EntityComponentIterator& b) { return denseIndex == b.denseIndex; };
            bool operator != (const EntityComponentIterator& b) { return denseIndex != b.denseIndex; };  

        private:
            EntityIndex denseIndex = {};
            std::vector<EntityVersion>* versions = {};
            std::tuple<ComponentStorageSparseSet<FirstComponentType>*, ComponentStorageSparseSet<RestComponentTypes>* ...>* storages = {};
        };

        auto begin() {
            return EntityComponentIterator{ 0, versions, &storages };
        }

        auto end() {
            return EntityComponentIterator{ (EntityIndex)storage<FirstComponentType>().denseIndices.size(), versions, &storages };
        }

        template<typename SpecificType>
        ComponentStorageSparseSet<SpecificType>& storage() {
            return *std::get<ComponentStorageSparseSet<SpecificType>*>(storages);
        }

    private:
        std::vector<EntityVersion>* versions = {};
        std::tuple<ComponentStorageSparseSet<FirstComponentType>*, ComponentStorageSparseSet<RestComponentTypes>* ...> storages = {};
    };
}