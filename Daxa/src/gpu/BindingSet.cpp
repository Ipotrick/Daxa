#include "BindingSet.hpp"

namespace daxa {
	namespace gpu {
		BindingSet::BindingSet(VkDevice device, VkDescriptorSet set, std::weak_ptr<BindingSetAllocatorBindingiSetPool> pool, BindingSetDescription const* description)
			: device{ device }
			, set { set }
			, pool{ pool }
			, description{ description }
		{
			handles.resize(description->descriptorCount, std::monostate{});
		}

		thread_local std::vector<VkDescriptorImageInfo> descImageInfoBuffer = {};

		void BindingSet::bindSamplers(u32 binding, std::span<SamplerHandle> samplers, u32 descriptorArrayOffset) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not update binding set while it is used on gpu");
			descImageInfoBuffer.reserve(samplers.size());

			for (auto& sampler : samplers) {
				descImageInfoBuffer.push_back(VkDescriptorImageInfo{
					.sampler = sampler->getVkSampler(),
					.imageView = VK_NULL_HANDLE,
					.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				});

				// update the handles inside the set
				u32 bindingSetIndex = this->description->bindingToHandleVectorIndex[binding];
				handles[bindingSetIndex] = sampler;
			}

			VkWriteDescriptorSet write{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = set,
				.dstBinding = binding,
				.dstArrayElement = descriptorArrayOffset,
				.descriptorCount = (u32)descImageInfoBuffer.size(),
				.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo = descImageInfoBuffer.data()
			};

			vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

			descImageInfoBuffer.clear();
		}

		void BindingSet::bindSampler(u32 binding, SamplerHandle sampler, u32 dstArrayElement) {
			bindSamplers(binding, { &sampler,1 }, dstArrayElement);
		}

		thread_local std::vector<VkDescriptorBufferInfo> descBufferInfoBuffer = {};

		void BindingSet::bindBuffers(u32 binding, std::span<BufferHandle> buffers, u32 descriptorArrayOffset) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not update binding set, that is still in use on the gpu");
			for (auto& buffer : buffers) {
				descBufferInfoBuffer.push_back(VkDescriptorBufferInfo{
					.buffer = buffer->getVkBuffer(),
					.offset = 0,									// TODO Unsure what to put here
					.range = buffer->getSize(),
				});

				// update the handles inside the set
				u32 bindingSetIndex = description->bindingToHandleVectorIndex[binding];
				handles[bindingSetIndex] = buffer;
			}

			VkWriteDescriptorSet write{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = set,
				.dstBinding = binding,
				.dstArrayElement = descriptorArrayOffset,
				.descriptorCount = (u32)descBufferInfoBuffer.size(),
				.descriptorType = this->description->layoutBindings.bindings[binding].descriptorType,
				.pBufferInfo = descBufferInfoBuffer.data(),
			};

			vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

			descBufferInfoBuffer.clear();
		}

		void BindingSet::bindBuffer(u32 binding, BufferHandle image, u32 dstArrayElement) {
			bindBuffers(binding, { &image, 1 }, dstArrayElement);
		}

		void BindingSet::bindImages(u32 binding, std::span<std::pair<ImageHandle, VkImageLayout>> images, u32 descriptorArrayOffset) {
			DAXA_ASSERT_M(usesOnGPU == 0, "can not update binding set while it is used on gpu");
			VkDescriptorType imageDescriptorType = description->layoutBindings.bindings[binding].descriptorType;
			
			for (auto& [image, layout] : images) {
				VkSampler sampler = imageDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER 
					? image->getSampler()->getVkSampler() 
					: VK_NULL_HANDLE;

				descImageInfoBuffer.push_back(VkDescriptorImageInfo{
					.sampler = sampler,
					.imageView = image->getVkView(),
					.imageLayout = layout,
				});

				// update the handles inside the set
				u32 bindingSetIndex = description->bindingToHandleVectorIndex[binding];
				handles[bindingSetIndex] = image;
			}

			VkWriteDescriptorSet write{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = set,
				.dstBinding = binding,
				.dstArrayElement = descriptorArrayOffset,
				.descriptorCount = (u32)descImageInfoBuffer.size(),
				.descriptorType = imageDescriptorType,
				.pImageInfo = descImageInfoBuffer.data()
			};

			vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

			descImageInfoBuffer.clear();
		}

		void BindingSet::bindImage(u32 binding, ImageHandle image, VkImageLayout imgLayout, u32 dstArrayElement) {
			std::pair imgAndLayout = { image, imgLayout };
			bindImages(binding, { &imgAndLayout, 1 }, dstArrayElement);
		}

		BindingSetHandle::BindingSetHandle(std::shared_ptr<BindingSet>&& set) 
			: set { std::move(set) }
		{ }

		BindingSetHandle::~BindingSetHandle() {
			if (set && set.use_count() == 1) {
				size_t handlesSize = set->handles.size();
				set->handles.clear();
				set->handles.resize(handlesSize, std::monostate{});
				auto pool = set->pool.lock();
				auto lock = std::unique_lock(pool->mut);
				pool->zombies.push_back(std::move(set));
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
			return descriptions[bindingArray].get();
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

		BindingSetAllocator::BindingSetAllocator(VkDevice device, BindingSetDescription const* setDescription, size_t setsPerPool)
			: device{ device }
			, setDescription{ setDescription }
			, setsPerPool{ setsPerPool }
		{
			DAXA_ASSERT_M(setDescription, "setDescription was nullptr");
			initPoolSizes();
		}

		BindingSetAllocator::~BindingSetAllocator() {
			if (device) {
				for (auto& pool : pools) {
					auto lock = std::unique_lock(pool->mut);
					DAXA_ASSERT_M(pool->allocatedSets == pool->zombies.size(), "at the time of the descruction of a BindingSetAllocator, there were still living bindingsets left");
					vkResetDescriptorPool(device, pool->pool, 0);
					vkDestroyDescriptorPool(device, pool->pool, nullptr);
				}
			}
		}

		BindingSetHandle BindingSetAllocator::getSet() {
			std::optional<BindingSetHandle> handleOpt = std::nullopt;
			for (auto& pool : pools) {
				auto lock = std::unique_lock(pool->mut);
				if (!pool->zombies.empty()) {
					handleOpt = BindingSetHandle{ std::move(pool->zombies.back()) };
					pool->zombies.pop_back();
					break;
				}
				else if (pool->allocatedSets < setsPerPool) {
					handleOpt = getNewSet(pool);
				}
			}

			if (handleOpt.has_value()) {
				return std::move(handleOpt.value());
			}
			else {
				pools.push_back(std::move(getNewPool()));
				// dont need mutex lock, as we are the only ones that have a ptr to the pool:
				return getNewSet(pools.back());
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

		BindingSetHandle BindingSetAllocator::getNewSet(std::shared_ptr<BindingSetAllocatorBindingiSetPool>& pool) {
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

			return BindingSetHandle{ std::make_shared<BindingSet>(device, set, pool, setDescription) };
		}

		std::shared_ptr<BindingSetAllocatorBindingiSetPool> BindingSetAllocator::getNewPool() {
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
			auto ret = std::make_shared<BindingSetAllocatorBindingiSetPool>();
			ret->pool = pool;
			ret->allocatedSets = 0;
			ret->zombies = {};
			return std::move(ret);
		}
	}
}