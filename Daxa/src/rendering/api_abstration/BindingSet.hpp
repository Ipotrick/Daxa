#pragma once

#include "../../DaxaCore.hpp"

#include <vector>
#include <array>
#include <optional>
#include <variant>
#include <span>
#include <array>
#include <unordered_map>
#include <mutex>

#include "vulkan/vulkan.h"

#include "Image.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"

namespace daxa {
	namespace gpu {

		constexpr inline size_t MAX_BINDINGS_PER_SET = 16;

		class BindingSet;

		namespace {
			struct BindingSetAllocatorBindingiSetPool {
				VkDescriptorPool pool = VK_NULL_HANDLE;
				size_t allocatedSets = 0;
				std::vector<std::shared_ptr<BindingSet>> zombies = {};
				std::mutex mut = {};
			};
		}

		struct BindingsArray {
			std::array<VkDescriptorSetLayoutBinding, MAX_BINDINGS_PER_SET> bindings = {};
			size_t size = 0;
			bool operator==(BindingsArray const& other) const {	
				auto equal = std::memcmp(this, &other, sizeof(BindingsArray)) == 0;
				return equal;
			}
		};

		struct BindingsArrayHasher
		{
			size_t operator()(const BindingsArray& bindingArray) const
			{
				size_t hash = bindingArray.size;
				for (int i = 0; i < MAX_BINDINGS_PER_SET; i++) {
					hash ^= bindingArray.bindings[i].binding;
					hash ^= bindingArray.bindings[i].descriptorCount;
					hash ^= bindingArray.bindings[i].descriptorType;
					hash ^= bindingArray.bindings[i].stageFlags;
				}
				return hash;
			}
		};

		class BindingSetDescription {
		public:
		private:
			friend class BindingSet;
			friend class BindingSetAllocator;
			friend class Device;
			friend class CommandList;
			friend class BindingSetDescriptionCache;
			friend class GraphicsPipelineBuilder;

			VkDescriptorSetLayout layout = {};
			BindingsArray layoutBindings = {};
			std::array<u32, MAX_BINDINGS_PER_SET> bindingToHandleVectorIndex = {};	// used to index the vector for storing the handles of the descriptor sets
			size_t descriptorCount = {};
		};

		class BindingSet {
		public:
			BindingSet(BindingSet const&)				= delete;
			BindingSet& operator=(BindingSet const&)	= delete;
			BindingSet(BindingSet&&) noexcept;
			BindingSet& operator=(BindingSet&&) noexcept;

			using HandleVariants = std::variant<ImageHandle, BufferHandle, SamplerHandle, std::monostate>;

			VkDescriptorSet getVkDescriptorSet() const { return set; }
		private:
			friend class Device;
			friend class CommandList;
			friend class BindingSetAllocator;
			friend class BindingSetHandle;
			friend class Queue;

			BindingSet(VkDescriptorSet set, std::weak_ptr<BindingSetAllocatorBindingiSetPool> pool, BindingSetDescription const* description);

			VkDescriptorSet set = {};
			BindingSetDescription const* description = {};
			std::weak_ptr<BindingSetAllocatorBindingiSetPool> pool = {};
			bool bInUseOnGPU = false;

			std::vector<HandleVariants> handles = {};
		};

		class BindingSetDescriptionCache {
		public:
			BindingSetDescriptionCache(VkDevice device);
			BindingSetDescriptionCache(BindingSetDescriptionCache const&) = delete;
			BindingSetDescriptionCache& operator=(BindingSetDescriptionCache const&) = delete;
			BindingSetDescriptionCache(BindingSetDescriptionCache&&) noexcept = delete;
			BindingSetDescriptionCache& operator=(BindingSetDescriptionCache&&) noexcept = delete;
			~BindingSetDescriptionCache();

			BindingSetDescription const* getSetDescription(std::span<VkDescriptorSetLayoutBinding> bindings);
		private:
			BindingSetDescription makeNewDescription(BindingsArray& bindingArray);

			VkDevice device = VK_NULL_HANDLE;
			std::unordered_map<BindingsArray, std::unique_ptr<BindingSetDescription>, BindingsArrayHasher> descriptions;
		};

		class BindingSetHandle {
		public:
			BindingSetHandle(BindingSetHandle const&) = default;
			BindingSetHandle& operator=(BindingSetHandle const&) = default;
			BindingSetHandle(BindingSetHandle&&) noexcept = default;
			BindingSetHandle& operator=(BindingSetHandle&&) noexcept = default;
			~BindingSetHandle();

			BindingSet const& operator*() const { return *set; }
			BindingSet& operator*() { return *set; }
			BindingSet const* operator->() const { return &*set; }
			BindingSet* operator->() { return &*set; }
		private:
			friend class BindingSetAllocator;
			friend class GeneralDescriptorSetAllocator;

			BindingSetHandle(std::shared_ptr<BindingSet>&&);

			std::shared_ptr<BindingSet> set;
		};

		/**
		* Must be externally Synchronized. given out BindingSets must not be externally synchorized.
		*/
		class BindingSetAllocator {
		public:
			BindingSetAllocator() = default;
			BindingSetAllocator(VkDevice device, BindingSetDescription const* setDescription, size_t setsPerPool = 64);
			BindingSetAllocator(BindingSetAllocator&&) noexcept;
			BindingSetAllocator& operator=(BindingSetAllocator&&) noexcept;
			BindingSetAllocator(BindingSetAllocator const&) = delete;
			BindingSetAllocator& operator=(BindingSetAllocator const&) = delete;
			~BindingSetAllocator();

			BindingSetHandle getSet();
		private:
			void initPoolSizes();

			BindingSetHandle getNewSet(std::shared_ptr<BindingSetAllocatorBindingiSetPool>& pool);

			std::shared_ptr<BindingSetAllocatorBindingiSetPool> getNewPool();

			size_t	setsPerPool = 0;

			std::vector<VkDescriptorPoolSize> poolSizes = {};
			VkDevice device = VK_NULL_HANDLE;
			BindingSetDescription const* setDescription = nullptr;

			std::vector<std::shared_ptr<BindingSetAllocatorBindingiSetPool>> pools;
		};
	}
}