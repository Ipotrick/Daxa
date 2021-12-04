#include "BindingSet.hpp"

#include "common.hpp"

namespace daxa {
	namespace gpu {

		DAXA_DEFINE_TRIVIAL_MOVE(BindingSet)

		BindingSet::BindingSet(VkDescriptorSet set, PoolInfo* poolInfo, BindingSetDescription& description)
			: set{ set }
			, poolInfo{ poolInfo }
		{
			handles.resize(description.descriptorCount, std::monostate{});
			size_t nextBindingHandleIndex = 0;
			for (int i = 0; i < description.size; i++) {
				bindingToHandleVectorIndex[i] = nextBindingHandleIndex;
				nextBindingHandleIndex += description.layoutBindings[i].descriptorCount;
			}
		}

		BindingSet::~BindingSet() {
			if (poolInfo) {
				poolInfo->zombies.push_back(set);
				this->set = {};
				this->poolInfo = {};
			}
		}

		BindingSetHandle::BindingSetHandle(BindingSet&& set) 
			: set { std::make_shared<BindingSet>(std::move(set)) }
		{ }

		DAXA_DEFINE_TRIVIAL_MOVE(BindingSetAllocator)

		BindingSetAllocator::BindingSetAllocator(VkDevice device, BindingSetDescription setDescription, size_t setsPerPool)
			: device{ device }
			, setDescription{ setDescription }
			, setsPerPool{ setsPerPool }
		{
			initPoolSizes();
		}

		BindingSetAllocator::~BindingSetAllocator() {
			for (auto& pool : pools) {
				assert(pool->allocatedSets == pool->zombies.size());	// assert that all given out set handles have died
				vkResetDescriptorPool(device, pool->pool, 0 /*TODO COULD BE WRONG*/);
				vkDestroyDescriptorPool(device, pool->pool, nullptr);
			}
		}

		BindingSetHandle BindingSetAllocator::getSet() {
			std::optional<BindingSetHandle> handleOpt = std::nullopt;
			for (auto& pool : pools) {
				if (!pool->zombies.empty()) {
					handleOpt = BindingSetHandle{ BindingSet{ pool->zombies.back(), &*pool, setDescription } };
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

		void BindingSetAllocator::initPoolSizes() {
			for (int i = 0; i < setDescription.size; i++) {
				poolSizes.push_back(VkDescriptorPoolSize{
					.type = setDescription.layoutBindings[i].descriptorType,
					.descriptorCount = (u32)(setsPerPool * setDescription.layoutBindings[i].descriptorCount),
					});
			}
		}

		BindingSetHandle BindingSetAllocator::getNewSet(PoolInfo* pool) {
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

			return BindingSetHandle{ BindingSet{ set, pool, setDescription } };
		}

		PoolInfo BindingSetAllocator::getNewPool() {
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
	}
}