#include "BindingSet.hpp"

#include "common.hpp"

namespace daxa {
	namespace gpu {

		DAXA_DEFINE_TRIVIAL_MOVE(BindingSet)

		BindingSet::BindingSet(VkDescriptorSet set, PoolInfo* poolInfo, BindingSetDescription const* description)
			: set{ set }
			, poolInfo{ poolInfo }
			, description{ description }
		{
			handles.resize(description->descriptorCount, std::monostate{});
		}

		//BindingSet::~BindingSet() {
		//	if (poolInfo) {
		//		poolInfo->zombies.push_back(set);
		//		this->set = {};
		//		this->poolInfo = {};
		//	}
		//}

		BindingSetHandle::BindingSetHandle(std::shared_ptr<BindingSet>&& set) 
			: set { std::move(set) }
		{ }

		BindingSetHandle::~BindingSetHandle() {
			if (set.use_count() == 1) {
				set->handles.clear();
				set->poolInfo->zombies.push_back(set);
			}
		}

		DAXA_DEFINE_TRIVIAL_MOVE(BindingSetAllocator)

		BindingSetAllocator::BindingSetAllocator(VkDevice device, BindingSetDescription const* setDescription, size_t setsPerPool)
			: device{ device }
			, setDescription{ setDescription }
			, setsPerPool{ setsPerPool }
		{
			DAXA_ASSERT_M(setDescription, "setDescription was nullptr");
			initPoolSizes();
		}

		BindingSetAllocator::~BindingSetAllocator() {
			for (auto& pool : pools) {
				DAXA_ASSERT_M(pool->allocatedSets == pool->zombies.size(), "at the time of the descruction of a BindingSetAllocator, there were still living bindingsets left");
				vkResetDescriptorPool(device, pool->pool, 0 /*TODO COULD BE WRONG*/);
				vkDestroyDescriptorPool(device, pool->pool, nullptr);
			}
		}

		BindingSetHandle BindingSetAllocator::getSet() {
			std::optional<BindingSetHandle> handleOpt = std::nullopt;
			for (auto& pool : pools) {
				if (!pool->zombies.empty()) {
					handleOpt = BindingSetHandle{ std::move(pool->zombies.back()) };
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
			for (int i = 0; i < setDescription->descriptorCount; i++) {
				poolSizes.push_back(VkDescriptorPoolSize{
					.type = setDescription->layoutBindings.bindings[i].descriptorType,
					.descriptorCount = (u32)(setsPerPool * setDescription->layoutBindings.bindings[i].descriptorCount),
				});
			}
		}

		BindingSetHandle BindingSetAllocator::getNewSet(PoolInfo* pool) {
			VkDescriptorSetAllocateInfo descriptorSetAI{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.pNext = nullptr,
				.descriptorPool = pool->pool,
				.descriptorSetCount = 1,
				.pSetLayouts = &setDescription->layout,
			};
			pool->allocatedSets += 1;
			VkDescriptorSet set;
			vkAllocateDescriptorSets(device, &descriptorSetAI, &set);

			return BindingSetHandle{ std::make_shared<BindingSet>(BindingSet{ set, pool, setDescription }) };
		}

		PoolInfo BindingSetAllocator::getNewPool() {
			VkDescriptorPoolCreateInfo descriptorPoolCI{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.maxSets = (u32)setsPerPool,
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