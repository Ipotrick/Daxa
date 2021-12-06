#pragma once

#include "../../DaxaCore.hpp"

#include <vector>
#include <array>
#include <optional>
#include <variant>
#include <span>
#include <array>
#include <unordered_map>

#include "vulkan/vulkan.h"

#include "Image.hpp"
#include "Buffer.hpp"

namespace daxa {
	namespace gpu {

		constexpr inline size_t MAX_BINDINGS_PER_SET = 16;

		class BindingSet;

		struct PoolInfo {
			VkDescriptorPool pool;
			size_t allocatedSets = 0;
			std::vector<std::shared_ptr<BindingSet>> zombies;
		};

		struct BindingsArray {
			std::array<VkDescriptorSetLayoutBinding, MAX_BINDINGS_PER_SET> bindings = {};
			size_t size = 0;
			bool operator==(BindingsArray const& other) const {
				return std::memcmp(this, &other, sizeof(BindingsArray));
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

			using HandleVariants = std::variant<ImageHandle, BufferHandle, std::monostate>;

			VkDescriptorSet getVkDescriptorSet() const { return set; }
		private:
			friend class Device;
			friend class CommandList;
			friend class BindingSetAllocator;
			friend class BindingSetHandle;
			friend class Queue;

			BindingSet(VkDescriptorSet set, PoolInfo* poolInfo, BindingSetDescription const* description);

			VkDescriptorSet set = {};
			BindingSetDescription const* description = {};
			PoolInfo* poolInfo = {};
			bool bInUseOnGPU = false;

			std::vector<HandleVariants> handles = {};
		};

		class BindingSetDescriptionCache {
		public:
			BindingSetDescriptionCache() = default;
			~BindingSetDescriptionCache() {
				for (auto& [_, description] : descriptions) {
					vkDestroyDescriptorSetLayout(device, description->layout, nullptr);
				}
			}

			void init(VkDevice device) {
				this->device = device;
			}

			BindingSetDescription const* getSetDescription(std::span<VkDescriptorSetLayoutBinding> bindings) {
				DAXA_ASSERT_M(device, "BindingSetDescriptionCache was not initialized");
				DAXA_ASSERT_M(bindings.size() < MAX_BINDINGS_PER_SET, "a binding set can only have up to 16 bindings");
				BindingsArray bindingArray = {};
				bindingArray.size = bindings.size();
				for (int i = 0; i < bindings.size(); i++) {
					bindingArray.bindings[i] = bindings[i];
				}
				if (!descriptions.contains(bindingArray)) {
					descriptions[bindingArray] = std::make_unique<BindingSetDescription>(makeNewDescription(bindingArray));
				}
				return &*descriptions[bindingArray];
			}
		private:
			BindingSetDescription makeNewDescription(BindingsArray& bindingArray) {
				BindingSetDescription description = {};

				description.layoutBindings = bindingArray;

				VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.bindingCount = (u32)bindingArray.size,
					.pBindings = bindingArray.bindings.data(),
				};
				vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &description.layout);

				size_t nextHandleVectorIndex = 0;
				for (int i = 0; i < bindingArray.size; i++) {
					DAXA_ASSERT_M(bindingArray.bindings[i].binding < MAX_BINDINGS_PER_SET, "all bindings of a binding set must be smaller than 16");
					description.bindingToHandleVectorIndex[bindingArray.bindings[i].binding] = nextHandleVectorIndex;
					nextHandleVectorIndex += bindingArray.bindings[i].descriptorCount;
				}
				description.descriptorCount = nextHandleVectorIndex;

				return description;
			}

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

		class BindingSetAllocator {
		public:
			BindingSetAllocator(VkDevice device, BindingSetDescription const* setDescription, size_t setsPerPool = 64);
			BindingSetAllocator(BindingSetAllocator&&) noexcept;
			BindingSetAllocator& operator=(BindingSetAllocator&&) noexcept;
			BindingSetAllocator(BindingSetAllocator const&) = delete;
			BindingSetAllocator& operator=(BindingSetAllocator const&) = delete;
			~BindingSetAllocator();

			BindingSetHandle getSet();
		private:
			void initPoolSizes();

			BindingSetHandle getNewSet(PoolInfo* pool);

			PoolInfo getNewPool();

			size_t	setsPerPool;

			std::vector<VkDescriptorPoolSize> poolSizes;
			VkDevice device = VK_NULL_HANDLE;
			BindingSetDescription const* setDescription;

			std::vector<std::unique_ptr<PoolInfo>> pools;
		};

		// TODO: MAKE GENERIC BINDING SET ALLOCATOR THAT CAN ALLOCATE ALL BINDING SETS
	}
}