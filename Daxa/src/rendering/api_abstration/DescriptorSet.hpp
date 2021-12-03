#pragma once

#include "../../DaxaCore.hpp"

#include <vector>
#include <array>
#include <optional>

#include "vulkan/vulkan.h"

#include "DescriptorSetLayoutCache.hpp"

namespace daxa {
	namespace gpu {

		struct PoolInfo {
			VkDescriptorPool pool;
			size_t allocatedSets = 0;
			std::vector<VkDescriptorSet> zombies;
		};

		class DescriptorSet {
		public:
			PoolInfo* poolInfo = {};
			VkDescriptorSet set = {};
		};

		class DescriptorSetHandle {
		public:
			std::shared_ptr<DescriptorSet> set;
		};

		class DescriptorSetDescription {
		public:

			VkDescriptorSetLayoutBinding* getVkBindings() { return layoutBindings.data(); }
			size_t getBindingCount() { return size; }
			const VkDescriptorSetLayout layout;
		private:
			static inline constexpr size_t CAPACITY = 32;

			std::array<VkDescriptorSetLayoutBinding, CAPACITY> layoutBindings;
			size_t size = 0;
		};

		class DescriptorSetAllocator {
		public:
			DescriptorSetAllocator(VkDevice device, DescriptorSetDescription setDescription, size_t setsPerPool) 
				: device{ device }
				, setDescription{ setDescription }
				, setsPerPool{ setsPerPool }
			{
				initPoolSizes();
			}

			~DescriptorSetAllocator() {
				for (auto& pool : pools) {
					assert(pool->allocatedSets == pool->zombies.size());	// assert that all given out set handles have died
					vkResetDescriptorPool(device, pool->pool, 0 /*TODO COULD BE WRONG*/);
					vkDestroyDescriptorPool(device, pool->pool, nullptr);
				}
			}

			DescriptorSetHandle getSet() {
				std::optional<DescriptorSetHandle> handleOpt = std::nullopt;
				for (auto& pool : pools) {
					if (!pool->zombies.empty()) {
						handleOpt = DescriptorSetHandle{ .set = std::make_shared<DescriptorSet>(DescriptorSet{.poolInfo = &*pool, .set = pool->zombies.back() }) };
						pool->zombies.pop_back();
						break;
					}
					else if (pool->allocatedSets < setsPerPool) {
						handleOpt = getNewSet(&*pool);
					}
				}

				if (handleOpt.has_value()) {
					return std::move(handleOpt.value());
				}
				else {
					pools.push_back(std::make_unique<PoolInfo>(getNewPool()));
					return getNewSet(&*pools.back());
				}
			}
		private:
			void initPoolSizes() {
				for (int i = 0; i < setDescription.getBindingCount(); i++) {
					poolSizes.push_back(VkDescriptorPoolSize{
						.type = setDescription.getVkBindings()[i].descriptorType,
						.descriptorCount = (u32)(setsPerPool * setDescription.getVkBindings()[i].descriptorCount),
					});
				}
			}

			DescriptorSetHandle getNewSet(PoolInfo* pool) {
				VkDescriptorSetAllocateInfo descriptorSetAI{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
					.pNext = nullptr,
					.descriptorPool = pool->pool,
					.descriptorSetCount = 1,
					.pSetLayouts = &setDescription.layout,
				};
				pool->allocatedSets += 1;
				VkDescriptorSet set;
				vkAllocateDescriptorSets(device, &descriptorSetAI, &set);

				DescriptorSetHandle handle{ .set = std::make_shared<DescriptorSet>(DescriptorSet{.poolInfo = pool, .set = set }) };
				return handle;
			}

			PoolInfo getNewPool() {
				VkDescriptorPoolCreateInfo descriptorPoolCI{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.maxSets = setsPerPool,
					.poolSizeCount = (u32)poolSizes.size(),
					.pPoolSizes = poolSizes.data(),
				};

				VkDescriptorPool pool = VK_NULL_HANDLE;
				vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &pool);
				return PoolInfo{
					.pool = pool,
					.allocatedSets = {},
					.zombies = {},
				};
			}

			size_t	setsPerPool = 1024;

			std::vector<VkDescriptorPoolSize> poolSizes;
			VkDevice device = VK_NULL_HANDLE;
			DescriptorSetDescription setDescription;

			std::vector<std::unique_ptr<PoolInfo>> pools;
		};

		class GeneralDescriptorSetAllocator {
		public:
		private:
		};
	}
}
