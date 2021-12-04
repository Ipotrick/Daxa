#pragma once

#include "../../DaxaCore.hpp"

#include <vector>
#include <array>
#include <optional>
#include <variant>
#include <span>

#include "vulkan/vulkan.h"

#include "Image.hpp"
#include "Buffer.hpp"
#include "DescriptorSetLayoutCache.hpp"

namespace daxa {
	namespace gpu {

		struct PoolInfo {
			VkDescriptorPool pool;
			size_t allocatedSets = 0;
			std::vector<VkDescriptorSet> zombies;
		};

		class BindingSetDescription {
		public:
		private:
			friend class BindingSet;
			friend class BindingSetAllocator;
			friend class Device;
			friend class CommandList;

			size_t descriptorCount = 0;
			VkDescriptorSetLayout layout;
			std::array<VkDescriptorSetLayoutBinding, 16> layoutBindings = {};
			size_t size = 0;
		};

		class BindingSet {
		public:
			BindingSet(BindingSet const&) = delete;
			BindingSet& operator=(BindingSet const&) = delete;
			BindingSet(BindingSet&&) noexcept;
			BindingSet& operator=(BindingSet&&) noexcept;
			~BindingSet();

			using HandleVariants = std::variant<ImageHandle, BufferHandle, std::monostate>;

			VkDescriptorSet getVkDescriptorSet() const { return set; }
		private:
			friend class CommandList;
			friend class BindingSetAllocator;

			BindingSet(VkDescriptorSet set, PoolInfo* poolInfo, BindingSetDescription& description);

			VkDescriptorSet set = {};
			PoolInfo* poolInfo = {};

			std::array<size_t, 16> bindingToHandleVectorIndex;

			// TODO: TRY TO FACTOR THESE INTO THREE VECTORS: SAMPLERS, IMAGES, BUFFERS
			std::vector<HandleVariants> handles;
		};

		class BindingSetHandle {
		public:
			BindingSet const& operator*() const { return *set; }
			BindingSet& operator*() { return *set; }
			BindingSet const* operator->() const { return &*set; }
			BindingSet* operator->() { return &*set; }
		private:
			friend class BindingSetAllocator;
			friend class GeneralDescriptorSetAllocator;

			BindingSetHandle(BindingSet&&);

			std::shared_ptr<BindingSet> set;
		};

		class BindingSetAllocator {
		public:
			BindingSetAllocator(VkDevice device, BindingSetDescription setDescription, size_t setsPerPool = 1024);
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
			BindingSetDescription setDescription;

			std::vector<std::unique_ptr<PoolInfo>> pools;
		};

		// TODO: MAKE GENERIC BINDING SET ALLOCATOR THAT CAN ALLOCATE ALL BINDING SETS
	}
}
