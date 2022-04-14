#include "BindingSet.hpp"
#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"
#include "backend/BufferBackend.hpp"

namespace daxa {
	BindingSet::BindingSet(std::shared_ptr<DeviceBackend> deviceBackend, VkDescriptorSet set, std::weak_ptr<BindingSetAllocatorBindingiSetPool> pool, std::shared_ptr<BindingSetLayout const> layout)
		: deviceBackend{ std::move(deviceBackend) }
		, set { set }
		, pool{ pool }
		, layout{ std::move(layout) }
	{ }

	thread_local std::vector<VkDescriptorImageInfo> descImageInfoBuffer = {};

	void BindingSet::bindSamplers(u32 binding, std::span<SamplerHandle> samplers, u32 descriptorArrayOffset) {
		DAXA_ASSERT_M(usesOnGPU == 0 || (layout->getDescription().flags & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT), "can not update binding set while it is used on gpu without VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT");
		descImageInfoBuffer.reserve(samplers.size());

		for (auto& sampler : samplers) {
			DAXA_ASSERT_M(sampler, "invalid sampler handle");
			descImageInfoBuffer.push_back(VkDescriptorImageInfo{
				.sampler = sampler->getVkSampler(),
				.imageView = VK_NULL_HANDLE,
				.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			});
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

		vkUpdateDescriptorSets(deviceBackend->device.device, 1, &write, 0, nullptr);

		descImageInfoBuffer.clear();
	}

	void BindingSet::bindSampler(u32 binding, SamplerHandle sampler, u32 dstArrayElement) {
		bindSamplers(binding, { &sampler,1 }, dstArrayElement);
	}

	thread_local std::vector<VkDescriptorBufferInfo> descBufferInfoBuffer = {};

	void BindingSet::bindBuffers(u32 binding, std::span<BufferHandle> buffers, u32 descriptorArrayOffset) {
		DAXA_ASSERT_M(usesOnGPU == 0 || (layout->getDescription().flags & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT), "can not update binding set while it is used on gpu without VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT");
		for (auto& buffer : buffers) {
			DAXA_ASSERT_M(buffer, "invalid buffer handle");
			descBufferInfoBuffer.push_back(VkDescriptorBufferInfo{
				.buffer = (VkBuffer)buffer.getVkBuffer(),
				.offset = 0,									// TODO Unsure what to put here
				.range = buffer.getSize(),
			});
		}

		VkWriteDescriptorSet write{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = set,
			.dstBinding = binding,
			.dstArrayElement = descriptorArrayOffset,
			.descriptorCount = (u32)descBufferInfoBuffer.size(),
			.descriptorType = this->layout->getDescription().layouts[binding].descriptorType,
			.pBufferInfo = descBufferInfoBuffer.data(),
		};

		vkUpdateDescriptorSets(deviceBackend->device.device, 1, &write, 0, nullptr);

		descBufferInfoBuffer.clear();
	}

	void BindingSet::bindBuffer(u32 binding, BufferHandle buffer, u32 dstArrayElement) {
		bindBuffers(binding, { &buffer, 1 }, dstArrayElement);
	}

	void BindingSet::bindImages(u32 binding, std::span<std::pair<ImageViewHandle, VkImageLayout>> images, u32 descriptorArrayOffset) {
		DAXA_ASSERT_M(usesOnGPU == 0 || (layout->getDescription().flags & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT), "can not update binding set while it is used on gpu without VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT");
		VkDescriptorType imageDescriptorType = layout->getDescription().layouts[binding].descriptorType;
		bool bIsImage = imageDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || 
			imageDescriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || 
			imageDescriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		DAXA_ASSERT_M(bIsImage, "tried binding image to non image binding index");
		
		for (auto& [image, layout] : images) {
			DAXA_ASSERT_M(image, "invalid image handle");
			VkSampler sampler = VK_NULL_HANDLE;
			if (imageDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				DAXA_ASSERT_M(image->getSampler().valid(), "can not bind image without sampler to a combined image sampler binding");
				sampler = image->getSampler()->getVkSampler();
			}

			descImageInfoBuffer.push_back(VkDescriptorImageInfo{
				.sampler = sampler,
				.imageView = image->getVkImageView(),
				.imageLayout = layout,
			});
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

		vkUpdateDescriptorSets(deviceBackend->device.device, 1, &write, 0, nullptr);

		descImageInfoBuffer.clear();
	}

	void BindingSet::bindImage(u32 binding, ImageViewHandle image, VkImageLayout imgLayout, u32 dstArrayElement) {
		std::pair imgAndLayout = { image, imgLayout };
		bindImages(binding, { &imgAndLayout, 1 }, dstArrayElement);
	}
	
	BindingSetLayout::BindingSetLayout(std::shared_ptr<DeviceBackend> deviceBackend, BindingSetDescription const& description) 
		: deviceBackend{ std::move(deviceBackend) }
		, description{ description }
	{
		for (u32 binding = 0; binding < MAX_BINDINGS_PER_SET; binding++) {
			auto& layout = description.layouts[binding];
			if (layout.descriptorCount != 0) {
				vkImmutableSamplers.push_back({});
				auto& immutableSamplers = vkImmutableSamplers.back();
				immutableSamplers.reserve(layout.descriptorCount);
				for (auto& sampler : layout.immutableSamplers) {
					immutableSamplers.push_back(sampler->getVkSampler());
				}

				descriptorSetBindingLayouts[descriptorSetBindingLayoutsCount++] = VkDescriptorSetLayoutBinding{
					.binding = binding,
					.descriptorType = layout.descriptorType,
					.descriptorCount = layout.descriptorCount,
					.stageFlags = layout.stageFlags,
					.pImmutableSamplers = immutableSamplers.empty() ? nullptr : immutableSamplers.data(),
				};

				this->descriptorCount += layout.descriptorCount;

				descriptorSetBindingLayoutFlags[descriptorSetBindingLayoutFlagCounts++] = layout.bindingFlag;
			}
		}
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindAllSetLayoutBindingFlagsCI {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.pNext = nullptr,
			.bindingCount = (u32)descriptorSetBindingLayoutFlagCounts,
			.pBindingFlags = descriptorSetBindingLayoutFlags.data(),
		};
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &bindAllSetLayoutBindingFlagsCI,
			.flags = 0,
			.bindingCount = (u32)descriptorSetBindingLayoutsCount,
			.pBindings = descriptorSetBindingLayouts.data(),
		};
		if (description.flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT) {
			descriptorSetLayoutCI.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		}
		DAXA_CHECK_VK_RESULT(vkCreateDescriptorSetLayout(this->deviceBackend->device.device, &descriptorSetLayoutCI, nullptr, &layout));
	}
	void BindingSetLayout::cleanup() {
		if (deviceBackend) {
			vkDestroyDescriptorSetLayout(deviceBackend->device.device, layout, nullptr);
			deviceBackend = {};
		}
	}
	BindingSetLayout::~BindingSetLayout() {
		cleanup();
	}
	BindingSetLayout::BindingSetLayout(BindingSetLayout&& other) noexcept {
		this->deviceBackend = std::move(other.deviceBackend);
		this->description = std::move(other.description);
		this->layout = std::move(other.layout);
		this->descriptorCount = std::move(other.descriptorCount);
	}
	BindingSetLayout& BindingSetLayout::operator=(BindingSetLayout&& other) noexcept {
		cleanup();
		this->deviceBackend = std::move(other.deviceBackend);
		this->description = std::move(other.description);
		this->layout = std::move(other.layout);
		this->descriptorCount = std::move(other.descriptorCount);
		return *this;
	}
	
	BindingSetLayoutCache::BindingSetLayoutCache(std::shared_ptr<DeviceBackend> deviceBackend) 
		: deviceBackend{ std::move(deviceBackend) }
	{ 
		map[BIND_ALL_SET_DESCRIPTION] = std::make_shared<BindingSetLayout>();
		auto& layout = map[BIND_ALL_SET_DESCRIPTION];
		layout->description = BIND_ALL_SET_DESCRIPTION,
		layout->descriptorCount = 
			BIND_ALL_SAMPLER_SET_LAYOUT_BINDING.descriptorCount + 
			BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING.descriptorCount + 
			BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING.descriptorCount + 
			BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING.descriptorCount + 
			BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING.descriptorCount;
		layout->descriptorSetBindingLayouts = std::array<VkDescriptorSetLayoutBinding, 32>{
			BIND_ALL_SAMPLER_SET_LAYOUT_BINDING,
			BIND_ALL_COMBINED_IMAGE_SAMPLER_SET_LAYOUT_BINDING,
			BIND_ALL_SAMPLED_IMAGE_SET_LAYOUT_BINDING,
			BIND_ALL_STORAGE_IMAGE_SET_LAYOUT_BINDING,
			BIND_ALL_STORAGE_BUFFER_SET_LAYOUT_BINDING,
		};
		layout->descriptorSetBindingLayoutsCount = (u32)layout->descriptorSetBindingLayouts.size();
		layout->layout = this->deviceBackend->bindAllSetLayout;
	}
	
	std::shared_ptr<BindingSetLayout const> BindingSetLayoutCache::getLayoutShared(BindingSetDescription const& description) {
		if (!map.contains(description)) {
			map[description] = std::make_shared<BindingSetLayout>(deviceBackend, description);
		}
		return map[description];
	}
	
	BindingSetLayout const& BindingSetLayoutCache::getLayout(BindingSetDescription const& description) {
		if (!map.contains(description)) {
			map[description] = std::make_shared<BindingSetLayout>(deviceBackend, description);
		}
		return *map[description];
	}

	BindingSetAllocator::BindingSetAllocator(std::shared_ptr<DeviceBackend> deviceBackend, BindingSetAllocatorCreateInfo const& ci)
		: deviceBackend{ std::move(deviceBackend) }
		, setLayout{ std::move(ci.setLayout) }
		, setsPerPool{ ci.setPerPool }
	{
		if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
			this->debugName = ci.debugName;
		}

		initPoolSizes();
	}

	BindingSetAllocator::~BindingSetAllocator() {
		if (deviceBackend->device.device) {
			for (auto& pool : pools) {
				auto lock = std::unique_lock(pool->mut);
				DAXA_ASSERT_M(pool->allocatedSets == pool->zombies.size(), "At the time of the descruction of a BindingSetAllocator, there were still living bindingsets left. It is very likely that this bug is caused by not calling waitIdle on the queue before destroying a BindingSetAllocator!");
				vkResetDescriptorPool(deviceBackend->device.device, pool->pool, 0);
				vkDestroyDescriptorPool(deviceBackend->device.device, pool->pool, nullptr);
			}
		}
	}
	
	void BindingSet::setDebugName(char const* debugName) {
		if (instance->pfnSetDebugUtilsObjectNameEXT != nullptr && debugName != nullptr) {
			this->debugName = debugName;

			VkDebugUtilsObjectNameInfoEXT imageNameInfo {
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET,
				.objectHandle = (uint64_t)this->set,
				.pObjectName = debugName,
			};
			instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &imageNameInfo);
		}
	}

	BindingSetHandle BindingSetAllocator::getSet(char const* debugName) {
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

		if (!handleOpt.has_value()) {
			pools.push_back(std::move(getNewPool()));
			// dont need mutex lock, as we are the only ones that have a ptr to the pool:
			handleOpt = getNewSet(pools.back());
		}

		if (
			instance->pfnSetDebugUtilsObjectNameEXT && 
			debugName != nullptr && 
			handleOpt.value()->getDebugName() != debugName
		) {
			handleOpt.value()->setDebugName(debugName);
		}

		return std::move(handleOpt.value());
	}

	void BindingSetAllocator::initPoolSizes() {
		for (auto const& binding : setLayout->getVkDescriptorBindingLayouts()) {
			poolSizes.push_back(VkDescriptorPoolSize{
				.type = binding.descriptorType,
				.descriptorCount = (u32)(setsPerPool * binding.descriptorCount),
			});
		}
	}

	BindingSetHandle BindingSetAllocator::getNewSet(std::shared_ptr<BindingSetAllocatorBindingiSetPool>& pool) {
		VkDescriptorSetAllocateInfo descriptorSetAI{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = pool->pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &setLayout->getVkDescriptorSetLayout(),
		};
		pool->allocatedSets += 1;
		VkDescriptorSet set;
		DAXA_CHECK_VK_RESULT(vkAllocateDescriptorSets(deviceBackend->device.device, &descriptorSetAI, &set));

		return BindingSetHandle{ std::make_shared<BindingSet>(deviceBackend, set, pool, setLayout) };
	}

	std::shared_ptr<BindingSetAllocatorBindingiSetPool> BindingSetAllocator::getNewPool() {
		VkDescriptorPoolCreateInfo descriptorPoolCI{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = setLayout->getDescription().flags,
			.maxSets = (u32)setsPerPool,
			.poolSizeCount = (u32)poolSizes.size(),
			.pPoolSizes = poolSizes.data(),
		};

		VkDescriptorPool pool = VK_NULL_HANDLE;
		DAXA_CHECK_VK_RESULT(vkCreateDescriptorPool(deviceBackend->device.device, &descriptorPoolCI, nullptr, &pool));
		auto ret = std::make_shared<BindingSetAllocatorBindingiSetPool>();
		ret->pool = pool;
		ret->allocatedSets = 0;
		ret->zombies = {};

		if (instance->pfnSetDebugUtilsObjectNameEXT && !debugName.empty()) {
			poolNameBuffer.clear();
			poolNameBuffer = debugName;
			poolNameBuffer += " pool nr ";
			poolNameBuffer += std::to_string(pools.size());
			
			VkDebugUtilsObjectNameInfoEXT imageNameInfo{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL,
				.objectHandle = (uint64_t)pool,
				.pObjectName = poolNameBuffer.c_str(),
			};
			instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &imageNameInfo);
		}

		return std::move(ret);
	}
}