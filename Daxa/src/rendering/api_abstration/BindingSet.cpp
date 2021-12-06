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

		BindingSetHandle::BindingSetHandle(std::shared_ptr<BindingSet>&& set) 
			: set { std::move(set) }
		{ }

		BindingSetHandle::~BindingSetHandle() {
			if (set.use_count() == 1) {
				set->handles.clear();
				set->poolInfo->zombies.push_back(set);
			}
		}

		BindingSetDescriptionCache::~BindingSetDescriptionCache() {
			for (auto& [_, description] : descriptions) {
				vkDestroyDescriptorSetLayout(device, description->layout, nullptr);
			}
		}
		
		BindingSetDescription const* BindingSetDescriptionCache::getSetDescription(std::span<VkDescriptorSetLayoutBinding> bindings) {
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

		BindingSetDescriptionCache::BindingSetDescriptionCache(VkDevice device)
			:device{device}
		{}

		BindingSetDescription BindingSetDescriptionCache::makeNewDescription(BindingsArray& bindingArray) {
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