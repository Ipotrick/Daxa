#pragma once

#include "../DaxaCore.hpp"

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
			VkDescriptorSetLayout getVkDescriptorSetLayout() const { return layout; }
			BindingsArray const& getBindingsArray() const { return layoutBindings; }
			std::array<u32, MAX_BINDINGS_PER_SET> const& getBindingToHandleVectorIndex() const { return bindingToHandleVectorIndex; }
			size_t getTotalDescriptorCount() const { descriptorCount; }
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
			BindingSet(VkDevice device, VkDescriptorSet set, std::weak_ptr<BindingSetAllocatorBindingiSetPool> pool, BindingSetDescription const* description);
			BindingSet(BindingSet const&)					= delete;
			BindingSet& operator=(BindingSet const&)		= delete;
			BindingSet(BindingSet&&) noexcept				= delete;
			BindingSet& operator=(BindingSet&&) noexcept 	= delete;

			void bindSamplers(u32 binding, std::span<SamplerHandle> samplers, u32 descriptorArrayOffset = 0);
			void bindSampler(u32 binding, SamplerHandle set, u32 dstArrayElement = 0);

			void bindBuffers(u32 binding, std::span<BufferHandle> buffers, u32 descriptorArrayOffset = 0);
			void bindBuffer(u32 binding, BufferHandle, u32 dstArrayElement = 0);

			void bindImages(u32 binding, std::span<std::pair<ImageHandle, VkImageLayout>> images, u32 descriptorArrayOffset = 0);
			void bindImage(u32 binding, ImageHandle image, VkImageLayout imgLayout, u32 dstArrayElement = 0);

			std::string const& getDebugName() const { return debugName; }

			VkDescriptorSet getVkDescriptorSet() const { return set; }
			VkDescriptorSetLayout getVkDescriptorSetLayout() const { return description->layout; }
		private:
			friend class Device;
			friend class CommandList;
			friend class BindingSetAllocator;
			friend class BindingSetHandle;
			friend class Queue;

			void setDebugName(char const* debugName);

			VkDevice device = VK_NULL_HANDLE;
			VkDescriptorSet set = VK_NULL_HANDLE;
			BindingSetDescription const* description = {};
			std::weak_ptr<BindingSetAllocatorBindingiSetPool> pool = {};
			u32 usesOnGPU = 0;

			using HandleVariants = std::variant<ImageHandle, BufferHandle, SamplerHandle, std::monostate>;
			std::vector<HandleVariants> handles = {};
			std::string debugName;
		};

		class BindingSetDescriptionCache {
		public:
			BindingSetDescriptionCache(VkDevice device);
			BindingSetDescriptionCache(BindingSetDescriptionCache const&) 					= delete;
			BindingSetDescriptionCache& operator=(BindingSetDescriptionCache const&) 		= delete;
			BindingSetDescriptionCache(BindingSetDescriptionCache&&) noexcept 				= delete;
			BindingSetDescriptionCache& operator=(BindingSetDescriptionCache&&) noexcept 	= delete;
			~BindingSetDescriptionCache();

			BindingSetDescription const* getSetDescription(std::span<VkDescriptorSetLayoutBinding> bindings);
		private:
			BindingSetDescription makeNewDescription(BindingsArray& bindingArray);

			VkDevice device = VK_NULL_HANDLE;
			std::unordered_map<BindingsArray, std::unique_ptr<BindingSetDescription>, BindingsArrayHasher> descriptions;
		};

		class BindingSetHandle {
		public:
			BindingSetHandle() = default;
			BindingSetHandle(BindingSetHandle&& other) noexcept = default;
			BindingSetHandle(BindingSetHandle const& other) = default;
			BindingSetHandle& operator=(BindingSetHandle&& other) noexcept;
			BindingSetHandle& operator=(BindingSetHandle const& other);
			~BindingSetHandle();

			BindingSet const& operator*() const { return *set; }
			BindingSet& operator*() { return *set; }
			BindingSet const* operator->() const { return &*set; }
			BindingSet* operator->() { return &*set; }

			operator bool() const { return set.operator bool(); }
			bool operator!() const { return !set.operator bool(); }

			bool valid() const { return set.operator bool(); }

			size_t getRefCount() const { return set.use_count(); }
		private:
			friend class BindingSetAllocator;
			friend class GeneralDescriptorSetAllocator;

			void cleanup();

			BindingSetHandle(std::shared_ptr<BindingSet>&&) noexcept;

			std::shared_ptr<BindingSet> set;
		};

		/**
		* Must be externally Synchronized. given out BindingSets must not be externally synchorized.
		*/
		class BindingSetAllocator {
		public:
			BindingSetAllocator(VkDevice device, BindingSetDescription const* setDescription, size_t setsPerPool, char const* debugName);
			BindingSetAllocator() 											= default;
			BindingSetAllocator(BindingSetAllocator&&) noexcept 			= delete;
			BindingSetAllocator& operator=(BindingSetAllocator&&) noexcept 	= delete;
			BindingSetAllocator(BindingSetAllocator const&) 				= delete;
			BindingSetAllocator& operator=(BindingSetAllocator const&) 		= delete;
			~BindingSetAllocator();

			BindingSetHandle getSet(const char* debugName = "");

			std::string const& getDebugName() const { return debugName; }
		private:

			void initPoolSizes();

			BindingSetHandle getNewSet(std::shared_ptr<BindingSetAllocatorBindingiSetPool>& pool);

			std::shared_ptr<BindingSetAllocatorBindingiSetPool> getNewPool();

			size_t setsPerPool = 0;

			std::vector<VkDescriptorPoolSize> poolSizes = {};
			VkDevice device 							= VK_NULL_HANDLE;
			BindingSetDescription const* setDescription = nullptr;

			std::vector<std::shared_ptr<BindingSetAllocatorBindingiSetPool>> pools;
			std::string debugName;
			std::string poolNameBuffer;
		};

		class BindingSetAllocatorHandle {
		public:
			BindingSetAllocatorHandle(std::shared_ptr<BindingSetAllocator> alloc) 
				: allocator{ alloc }
			{}
			BindingSetAllocatorHandle() = default;

			BindingSetAllocator const& operator*() const { return *allocator; }
			BindingSetAllocator& operator*() { return *allocator; }
			BindingSetAllocator const* operator->() const { return allocator.get(); }
			BindingSetAllocator* operator->() { return allocator.get(); }

			size_t getRefCount() const { return allocator.use_count(); }

			operator bool() const { return allocator.operator bool(); }
			bool operator!() const { return !allocator; }
			bool valid() const { return *this; }
		private:

			std::shared_ptr<BindingSetAllocator> allocator = {};
		};
	}
}