#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <filesystem>
#include <functional>
#include <set>
#include <unordered_map>


// If you want to use spirv reflect for reflection on descriptor sets in pipeline creation,
// set the following define to your include path of spirv_reflect like the following:
// #define VULKANHELPER_SPIRV_REFLECT_INCLUDE_PATH <spirv_reflect.h>
#define VULKANHELPER_SPIRV_REFLECT_INCLUDE_PATH <spirv_reflect.hpp>

#if defined(VULKANHELPER_SPIRV_REFLECT_INCLUDE_PATH)
#define VULKANHELPER_USE_SPIRV_REFLECT
#endif

#if defined(VULKANHELPER_IMPLEMENTATION)
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <iostream>
#include <fstream>
#include <algorithm>
#ifdef VULKANHELPER_USE_SPIRV_REFLECT
#include VULKANHELPER_SPIRV_REFLECT_INCLUDE_PATH
#endif
#endif

#if defined(_WIN32) || defined(_WIN64)
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#undef max
#undef min
#else
#error "CURRENTLY UNSUPPORTED PLATFORM"
#endif

namespace vkh {
	struct VertexDescription {
		std::vector<vk::VertexInputBindingDescription> bindings;
		std::vector<vk::VertexInputAttributeDescription> attributes;
		vk::PipelineVertexInputStateCreateFlags flags{};

		vk::PipelineVertexInputStateCreateInfo makePipelineVertexInputStateCreateInfo() const {
			return vk::PipelineVertexInputStateCreateInfo{
				.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size()),
				.pVertexBindingDescriptions = bindings.data(),
				.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size()),
				.pVertexAttributeDescriptions = attributes.data(),
			};
		}
	};

	class VertexDiscriptionBuilder {
	public:
		VertexDiscriptionBuilder& beginBinding(uint32_t stride, vk::VertexInputRate inputRate = vk::VertexInputRate::eVertex);
		VertexDiscriptionBuilder& addAttribute(vk::Format format);
		VertexDiscriptionBuilder& stageCreateFlags(vk::PipelineVertexInputStateCreateFlags flags);
		VertexDescription build();

	private:
		uint32_t stride{ 0 };
		uint32_t offset{ 0 };
		uint32_t location{ 0 };
		vk::PipelineVertexInputStateCreateFlags flags{};
		std::vector<vk::VertexInputBindingDescription> bindings;
		std::vector<vk::VertexInputAttributeDescription> attributes;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	VertexDiscriptionBuilder& VertexDiscriptionBuilder::beginBinding(uint32_t stride, vk::VertexInputRate inputRate) {
		offset = 0;
		location = 0;
		vk::VertexInputBindingDescription binding{
			.binding = static_cast<uint32_t>(bindings.size()),
			.stride = stride,
			.inputRate = inputRate,
		};
		bindings.push_back(binding);
		return *this;
	}

	VertexDiscriptionBuilder& VertexDiscriptionBuilder::addAttribute(vk::Format format) {
		vk::VertexInputAttributeDescription attribute{
			.location = location,
			.binding = static_cast<uint32_t>(bindings.size() - 1),
			.format = format,
			.offset = offset,
		};

		attributes.push_back(attribute);

		location += 1;

		switch (format) {
		case vk::Format::eR32G32B32A32Sfloat:
		case vk::Format::eR32G32B32A32Sint:
		case vk::Format::eR32G32B32A32Uint:
			offset += sizeof(uint32_t) * 4;
			break;
		case vk::Format::eR32G32B32Sfloat:
		case vk::Format::eR32G32B32Sint:
		case vk::Format::eR32G32B32Uint:
			offset += sizeof(uint32_t) * 3;
			break;
		case vk::Format::eR32G32Sfloat:
		case vk::Format::eR32G32Sint:
		case vk::Format::eR32G32Uint:
			offset += sizeof(uint32_t) * 2;
			break;
		case vk::Format::eR32Sfloat:
		case vk::Format::eR32Sint:
		case vk::Format::eR32Uint:
			offset += sizeof(uint32_t) * 1;
			break;
		default:
			assert(false);
		}

		return *this;
	}

	VertexDiscriptionBuilder& VertexDiscriptionBuilder::stageCreateFlags(vk::PipelineVertexInputStateCreateFlags flags) {
		this->flags = flags;
		return *this;
	}

	VertexDescription VertexDiscriptionBuilder::build() {
		assert(bindings.size() > 0);
		return VertexDescription{
			bindings,
			attributes,
			flags };
	}
#endif

	class DescriptorSetLayoutCache {
	public:
		DescriptorSetLayoutCache(vk::Device device);
		vk::DescriptorSetLayout getLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings);

	private:
		struct DescriptorLayoutHash {
			std::size_t operator()(const std::vector<vk::DescriptorSetLayoutBinding>& bindings) const;
		};

		vk::Device device;
		std::unordered_map<std::vector<vk::DescriptorSetLayoutBinding>, vk::UniqueDescriptorSetLayout, DescriptorLayoutHash> bindingsToLayout;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	DescriptorSetLayoutCache::DescriptorSetLayoutCache(vk::Device device)
		: device{ device } {
	}

	vk::DescriptorSetLayout DescriptorSetLayoutCache::getLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings) {
		auto iter = bindingsToLayout.find(bindings);
		if (iter != bindingsToLayout.end()) {
			return *iter->second;
		}
		else {
			auto allocateInfo = vk::DescriptorSetLayoutCreateInfo{
				.bindingCount = static_cast<uint32_t>(bindings.size()),
				.pBindings = bindings.data(),
			};
			return *(bindingsToLayout[bindings] = device.createDescriptorSetLayoutUnique(allocateInfo));
		}
	}
	std::size_t DescriptorSetLayoutCache::DescriptorLayoutHash::operator()(const std::vector<vk::DescriptorSetLayoutBinding>& bindings) const {
		size_t h{ 0 };
		for (const auto& binding : bindings) {
			h ^= static_cast<size_t>(binding.descriptorType);
			h ^= static_cast<size_t>(binding.descriptorType);
		}
		return h;
	}
#endif

	class GeneralDescriptorSetAllocator {
	public:
		GeneralDescriptorSetAllocator() = default;
		GeneralDescriptorSetAllocator(vk::Device device, std::vector<vk::DescriptorPoolSize> specificPoolSizes = {}, vk::DescriptorPoolCreateFlagBits poolCreateFlags = {}, uint32_t maxSetsPerPool = 500);
		void reset();
		vk::DescriptorSet allocate(vk::DescriptorSetLayout layout);
		std::vector<vk::DescriptorSet> allocate(vk::DescriptorSetLayout layout, uint32_t count);

		const vk::Device device;

	protected:
		GeneralDescriptorSetAllocator(vk::Device device, vk::DescriptorPoolCreateFlagBits poolFlags, uint32_t maxSetsPerPool);

		vk::UniqueDescriptorPool getUnusedPool();
		vk::UniqueDescriptorPool createPool();

		const vk::DescriptorPoolCreateFlagBits poolCreateFlags;
		const uint32_t maxSetsPerPool;

		vk::UniqueDescriptorPool currentPool;
		std::vector<vk::DescriptorPoolSize> descriptorPoolSizes;
		std::vector<vk::UniqueDescriptorPool> usedPools;
		std::vector<vk::UniqueDescriptorPool> unusedPools;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	GeneralDescriptorSetAllocator::GeneralDescriptorSetAllocator(vk::Device device, std::vector<vk::DescriptorPoolSize> specificPoolSizes, vk::DescriptorPoolCreateFlagBits poolCreateFlags, uint32_t maxSetsPerPool)
		: device{ device }, maxSetsPerPool{ maxSetsPerPool }, poolCreateFlags{ poolCreateFlags } {
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eSampler, .descriptorCount = 500u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = 4000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eSampledImage, .descriptorCount = 4000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformTexelBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eStorageTexelBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eStorageBuffer, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformBufferDynamic, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eStorageBufferDynamic, .descriptorCount = 1000u });
		this->descriptorPoolSizes.push_back(vk::DescriptorPoolSize{ .type = vk::DescriptorType::eInputAttachment, .descriptorCount = 500u });

		for (auto specificPoolSize : specificPoolSizes) {
			vk::DescriptorPoolSize* foundValue = nullptr;
			for (auto& poolSize : this->descriptorPoolSizes) {
				if (poolSize.type == specificPoolSize.type) {
					foundValue = &poolSize;
					break;
				}
			}
			if (foundValue) {
				foundValue->descriptorCount = specificPoolSize.descriptorCount;
			}
			else {
				this->descriptorPoolSizes.push_back(specificPoolSize);
			}
		}

		currentPool = createPool();
	}
	GeneralDescriptorSetAllocator::GeneralDescriptorSetAllocator(vk::Device device, vk::DescriptorPoolCreateFlagBits poolFlags, uint32_t maxSetsPerPool)
		: device{ device }, poolCreateFlags{ poolFlags }, maxSetsPerPool{ maxSetsPerPool } {
	}
	void GeneralDescriptorSetAllocator::reset() {
		if (currentPool) {
			device.resetDescriptorPool(*currentPool);
			unusedPools.push_back(std::move(currentPool));
			currentPool.release();
		}

		for (auto&& pool : usedPools) {
			device.resetDescriptorPool(*pool);
			unusedPools.push_back(std::move(pool));
		}
		usedPools.clear();
	}
	vk::DescriptorSet GeneralDescriptorSetAllocator::allocate(vk::DescriptorSetLayout layout) {
		auto result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = 1, .pSetLayouts = &layout });
		if (result.size() == 0) /* we need to reallocate with another pool */ {
			usedPools.push_back(std::move(currentPool));
			currentPool = getUnusedPool();

			result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = 1, .pSetLayouts = &layout });

			if (result.size() == 0) {
				// possible cause for this error is that the requested descriptor count exceeds the maximum descriptor pool size
				std::cerr << "error: discriptor allocator can not allocate this descriptor set layout!\n";
				assert(false);
			}
		}

		return std::move(result.front());
	}
	std::vector<vk::DescriptorSet> GeneralDescriptorSetAllocator::allocate(vk::DescriptorSetLayout layout, uint32_t count) {
		std::vector<vk::DescriptorSetLayout> layouts(count, layout);
		auto result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = count, .pSetLayouts = layouts.data() });
		if (result.size() == 0) /* we need to reallocate with another pool */ {
			usedPools.push_back(std::move(currentPool));
			currentPool = getUnusedPool();

			result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = count, .pSetLayouts = layouts.data() });

			if (result.size() == 0) {
				// possible cause for this error is that the requested descriptor count exceeds the maximum descriptor pool size
				std::cerr << "error: discriptor allocator can not allocate this descriptor set layout!\n";
				assert(false);
			}
		}

		return std::move(result);
	}
	vk::UniqueDescriptorPool GeneralDescriptorSetAllocator::getUnusedPool() {
		if (unusedPools.empty()) {
			return std::move(createPool());
		}
		else {
			auto pool = std::move(unusedPools.back());
			unusedPools.pop_back();
			return std::move(pool);
		}
	}
	vk::UniqueDescriptorPool GeneralDescriptorSetAllocator::createPool() {
		auto allocInfo = vk::DescriptorPoolCreateInfo{
			.flags = poolCreateFlags,
			.maxSets = maxSetsPerPool,
			.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
			.pPoolSizes = descriptorPoolSizes.data(),
		};

		return device.createDescriptorPoolUnique(allocInfo);
	}
#endif

	class DescriptorSetAllocator : public GeneralDescriptorSetAllocator {
	public:
		DescriptorSetAllocator(vk::Device device, const std::vector<vk::DescriptorSetLayoutBinding>& bindings, vk::DescriptorSetLayout layout, vk::DescriptorPoolCreateFlagBits poolFlags = {}, uint32_t maxSetsPerPool = 500);
		vk::DescriptorSet allocate();
		std::vector<vk::DescriptorSet> allocate(uint32_t count);

	private:
		using GeneralDescriptorSetAllocator::allocate;
		vk::DescriptorSetLayout layout;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	DescriptorSetAllocator::DescriptorSetAllocator(vk::Device device, const std::vector<vk::DescriptorSetLayoutBinding>& bindings, vk::DescriptorSetLayout layout, vk::DescriptorPoolCreateFlagBits poolFlags, uint32_t maxSetsPerPool)
		: GeneralDescriptorSetAllocator{ device, poolFlags, maxSetsPerPool }, layout{ layout } {
		const uint32_t descriptorPoolSizesCount = static_cast<uint32_t>(bindings.size());
		this->descriptorPoolSizes.resize(descriptorPoolSizesCount);

		for (uint32_t i = 0; i < descriptorPoolSizesCount; i++) {
			this->descriptorPoolSizes[i].type = bindings[i].descriptorType;
			this->descriptorPoolSizes[i].descriptorCount = bindings[i].descriptorCount * maxSetsPerPool;
		}

		currentPool = createPool();
	}
	vk::DescriptorSet DescriptorSetAllocator::allocate() {
		auto result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = 1, .pSetLayouts = &layout });
		if (result.size() == 0) /* we need to reallocate with another pool */ {
			usedPools.push_back(std::move(currentPool));
			currentPool = getUnusedPool();

			result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = 1, .pSetLayouts = &layout });
		}

		return std::move(result.front());
	}
	std::vector<vk::DescriptorSet> DescriptorSetAllocator::allocate(uint32_t count) {
		std::vector<vk::DescriptorSetLayout> layouts(count, layout);
		auto result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = count, .pSetLayouts = layouts.data() });
		if (result.size() == 0) /* we need to reallocate with another pool */ {
			usedPools.push_back(std::move(currentPool));
			currentPool = getUnusedPool();

			result = device.allocateDescriptorSets({ .descriptorPool = *currentPool, .descriptorSetCount = count, .pSetLayouts = layouts.data() });
		}

		return std::move(result);
	}
#endif

	vk::DescriptorSet createDescriptorSet(const std::vector<vk::DescriptorSetLayoutBinding>& bindings, GeneralDescriptorSetAllocator& alloc, DescriptorSetLayoutCache& layoutCache);

#if defined(VULKANHELPER_IMPLEMENTATION)
	vk::DescriptorSet createDescriptorSet(const std::vector<vk::DescriptorSetLayoutBinding>& bindings, GeneralDescriptorSetAllocator& alloc, DescriptorSetLayoutCache& layoutCache) {
		return alloc.allocate(layoutCache.getLayout(bindings));
	}
#endif

	class DescriptorSetBuilder {
	public:
		DescriptorSetBuilder(GeneralDescriptorSetAllocator* alloc, DescriptorSetLayoutCache* layoutCache);
		DescriptorSetBuilder(GeneralDescriptorSetAllocator* alloc, vk::DescriptorSetLayout setLayout);

		DescriptorSetBuilder& addBufferBinding(const vk::DescriptorSetLayoutBinding& binding, const vk::DescriptorBufferInfo& bufferInfo);
		DescriptorSetBuilder& addImageBinding(const vk::DescriptorSetLayoutBinding& binding, const vk::DescriptorImageInfo& imageInfo);
		vk::DescriptorSet build();

	private:
		GeneralDescriptorSetAllocator* alloc;
		DescriptorSetLayoutCache* layoutCache;
		vk::DescriptorSetLayout setLayout;

		std::vector<std::pair<vk::DescriptorSetLayoutBinding, vk::DescriptorBufferInfo>> bufferBindings;
		std::vector<std::pair<vk::DescriptorSetLayoutBinding, vk::DescriptorImageInfo>> imageBindings;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	DescriptorSetBuilder::DescriptorSetBuilder(GeneralDescriptorSetAllocator* alloc, DescriptorSetLayoutCache* layoutCache)
		: alloc{ alloc }, layoutCache{ layoutCache } {
	}
	DescriptorSetBuilder::DescriptorSetBuilder(GeneralDescriptorSetAllocator* alloc, vk::DescriptorSetLayout setLayout)
		: alloc{ alloc }, setLayout{ setLayout } {
	}

	DescriptorSetBuilder& DescriptorSetBuilder::addBufferBinding(const vk::DescriptorSetLayoutBinding& binding, const vk::DescriptorBufferInfo& bufferInfo) {
		bufferBindings.emplace_back(binding, bufferInfo);
		return *this;
	}
	DescriptorSetBuilder& DescriptorSetBuilder::addImageBinding(const vk::DescriptorSetLayoutBinding& binding, const vk::DescriptorImageInfo& imageInfo) {
		imageBindings.emplace_back(binding, imageInfo);
		return *this;
	}
	vk::DescriptorSet DescriptorSetBuilder::build() {
		if (!setLayout) {
			std::vector<vk::DescriptorSetLayoutBinding> bindings;
			bindings.reserve(bufferBindings.size() + imageBindings.size());
			for (auto& [binding, bufferInfo] : bufferBindings) {
				bindings.emplace_back(binding);
			}
			for (auto& [binding, imageInfo] : imageBindings) {
				bindings.emplace_back(binding);
			}
			setLayout = layoutCache->getLayout(bindings);
		}

		vk::DescriptorSet set = alloc->allocate(setLayout);

		std::vector<vk::WriteDescriptorSet> writes;
		writes.reserve(bufferBindings.size() + imageBindings.size());
		for (auto& [binding, bufferInfo] : bufferBindings) {
			writes.push_back(
				vk::WriteDescriptorSet{
					.dstSet = set,
					.dstBinding = binding.binding,
					.descriptorCount = binding.descriptorCount,
					.descriptorType = binding.descriptorType,
					.pBufferInfo = &bufferInfo });
		}
		for (auto& [binding, imageInfo] : imageBindings) {
			writes.push_back(
				vk::WriteDescriptorSet{
					.dstSet = set,
					.dstBinding = binding.binding,
					.descriptorCount = binding.descriptorCount,
					.descriptorType = binding.descriptorType,
					.pImageInfo = &imageInfo });
		}

		alloc->device.updateDescriptorSets(writes, {});
		return set;
	}
#endif

	vk::DescriptorSetLayout createDescriptorLayout(vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> binding);
	vk::UniqueDescriptorSetLayout createDescriptorLayoutUnique(vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> binding);

#if defined(VULKANHELPER_IMPLEMENTATION)
	vk::DescriptorSetLayout createDescriptorLayout(vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> binding) {
		vk::DescriptorSetLayoutCreateInfo createInfo{
			.bindingCount = static_cast<uint32_t>(binding.size()),
			.pBindings = binding.data() };
		return device.createDescriptorSetLayout(createInfo);
	}

	vk::UniqueDescriptorSetLayout createDescriptorLayoutUnique(vk::Device device, std::vector<vk::DescriptorSetLayoutBinding> binding) {
		vk::DescriptorSetLayoutCreateInfo createInfo{
			.bindingCount = static_cast<uint32_t>(binding.size()),
			.pBindings = binding.data() };
		return device.createDescriptorSetLayoutUnique(createInfo);
	}
#endif

#if defined(VULKANHELPER_USE_SPIRV_REFLECT)
	std::unordered_map<uint32_t, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>>
		reflectSetBindings(const std::vector<uint32_t>& spv, vk::ShaderStageFlagBits shaderStage);

	std::vector<vk::DescriptorSetLayout> mergeReflectedSetBindings(
		std::vector<std::unordered_map<uint32_t, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>>> setMaps,
		vkh::DescriptorSetLayoutCache& layoutCache);

	std::vector<vk::PushConstantRange> reflectPushConstants(const std::vector<uint32_t>& spv, vk::ShaderStageFlagBits shaderStage);
	std::vector<vk::PushConstantRange> mergeReflectedPushConstants(const std::vector<std::vector<vk::PushConstantRange>>& pushConstantsRanges);

#endif

#if defined(VULKANHELPER_IMPLEMENTATION)
#if defined(VULKANHELPER_USE_SPIRV_REFLECT)
	std::vector<vk::PushConstantRange> mergeReflectedPushConstants(const std::vector<std::vector<vk::PushConstantRange>>& pushConstantsRanges) {
		std::vector<vk::PushConstantRange> ret;
		for (auto& ranges : pushConstantsRanges) {
			for (auto pushConstant : ranges) {
				if (auto iter = std::find(ret.begin(), ret.end(), pushConstant); 
					iter != ret.end()) 
				{
					iter->stageFlags |= pushConstant.stageFlags;
				}
				else {
					ret.push_back(pushConstant);
				}
			}
		}
		return std::move(ret);
	}
	std::vector<vk::PushConstantRange> reflectPushConstants(const std::vector<uint32_t>& spv, vk::ShaderStageFlagBits shaderStage) {
		SpvReflectShaderModule module = {};
		SpvReflectResult result = spvReflectCreateShaderModule(spv.size() * sizeof(uint32_t), spv.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t count = 0;
		result = spvReflectEnumeratePushConstantBlocks(&module, &count, NULL);

		std::vector<SpvReflectBlockVariable*> blocks(count);
		result = spvReflectEnumeratePushConstantBlocks(&module, &count, blocks.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<vk::PushConstantRange> ret;
		for (auto* block : blocks) {
			ret.push_back(vk::PushConstantRange{
				.stageFlags = shaderStage,
				.offset = block->offset,
				.size = block->size,
			});
		}
		return std::move(ret);
	}
	std::unordered_map<uint32_t, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>>
		reflectSetBindings(const std::vector<uint32_t>& spv, vk::ShaderStageFlagBits shaderStage) {
		SpvReflectShaderModule module = {};
		SpvReflectResult result = spvReflectCreateShaderModule(spv.size() * sizeof(uint32_t), spv.data(), &module);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		uint32_t count = 0;
		result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::vector<SpvReflectDescriptorSet*> sets(count);
		result = spvReflectEnumerateDescriptorSets(&module, &count, sets.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		std::unordered_map<uint32_t, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>> setMap;
		for (auto* set : sets) {
			std::vector<vk::DescriptorSetLayoutBinding> bindings;
			for (uint32_t i = 0; i < set->binding_count; i++) {
				auto* reflBinding = set->bindings[i];

				vk::DescriptorSetLayoutBinding binding{
					.binding = reflBinding->binding,
					.descriptorType = static_cast<vk::DescriptorType>(reflBinding->descriptor_type),
					.descriptorCount = reflBinding->count,
					.stageFlags = shaderStage };
				setMap[set->set][binding.binding] = binding;
			}
		}

		spvReflectDestroyShaderModule(&module);
		return std::move(setMap);
	}

	std::vector<vk::DescriptorSetLayout> mergeReflectedSetBindings(
		std::vector<std::unordered_map<uint32_t, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>>> setMaps,
		vkh::DescriptorSetLayoutCache& layoutCache) {

		std::vector<std::vector<std::optional<vk::DescriptorSetLayoutBinding>>> setLayoutBindings;

		for (auto& setMap : setMaps) {
			for (auto& [set, bindingMap] : setMap) {
				if (setLayoutBindings.size() <= set) {
					setLayoutBindings.resize(set + 1);
				}
				for (auto& [bindingIndex, binding] : bindingMap) {
					if (setLayoutBindings[set].size() <= bindingIndex) {
						setLayoutBindings[set].resize(bindingIndex + 1);
					}
					if (!setLayoutBindings[set][bindingIndex].has_value()) {
						setLayoutBindings[set][bindingIndex] = binding;
					}
					else {
						setLayoutBindings[set][bindingIndex]->stageFlags |= binding.stageFlags;
					}
				}
			}
		}

		std::vector<vk::DescriptorSetLayout> layouts;
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		for (auto& setBindings : setLayoutBindings) {
			bindings.clear();
			for (auto& bindingOpt : setBindings) {
				if (bindingOpt) {
					bindings.push_back(*bindingOpt);
				}
			}
			layouts.push_back(layoutCache.getLayout(bindings));
		}

		return layouts;
	}

#endif // #if defined(VULKANHELPER_USE_SPIRV_REFLECT)
#endif // #if defined(VULKANHELPER_IMPLEMENTATION)

	vk::PipelineRasterizationStateCreateInfo makeDefaultRasterisationStateCreateInfo(vk::PolygonMode polygonMode);

	vk::PipelineMultisampleStateCreateInfo makeDefaultMultisampleStateCreateInfo();

	vk::PipelineColorBlendAttachmentState makeDefaultColorBlendSAttachmentState();

	struct Pipeline {
		vk::UniquePipeline pipeline;
		vk::UniquePipelineLayout layout;
	};
	class GraphicsPipelineBuilder {
	public:
		GraphicsPipelineBuilder(vk::Device device, vk::RenderPass pass, vk::PipelineCache pipelineCache = nullptr);
		Pipeline build();

		GraphicsPipelineBuilder& setSubPass(uint32_t index);
		GraphicsPipelineBuilder& setViewport(const vk::Viewport& viewport);
		GraphicsPipelineBuilder& setScissor(const vk::Rect2D& scissor);
		GraphicsPipelineBuilder& setVertexInput(const vk::PipelineVertexInputStateCreateInfo& vertexInput);
		GraphicsPipelineBuilder& setInputAssembly(const vk::PipelineInputAssemblyStateCreateInfo& inputassembly);
		GraphicsPipelineBuilder& setRasterization(const vk::PipelineRasterizationStateCreateInfo& rasterization);
		GraphicsPipelineBuilder& setMultisampling(const vk::PipelineMultisampleStateCreateInfo& multisampling);
		GraphicsPipelineBuilder& setDepthStencil(const vk::PipelineDepthStencilStateCreateInfo& depthStencil);
		GraphicsPipelineBuilder& setColorBlend(const vk::PipelineColorBlendStateCreateInfo& colorBlend);
		GraphicsPipelineBuilder& addColorBlendAttachemnt(const vk::PipelineColorBlendAttachmentState& colorAttachmentBlend);
		GraphicsPipelineBuilder& addShaderStage(
			const std::vector<uint32_t>* spv,
			vk::ShaderStageFlagBits shaderStage,
			vk::PipelineShaderStageCreateFlags flags = {},
			vk::SpecializationInfo* pSpecializationInfo = {});
		GraphicsPipelineBuilder& addDynamicState(const vk::DynamicState& dynamicstates);
		GraphicsPipelineBuilder& addPushConstants(const vk::PushConstantRange& pushconstants);
		GraphicsPipelineBuilder& setDescriptorLayouts(const std::vector<vk::DescriptorSetLayout>& layouts);
#if defined(VULKANHELPER_USE_SPIRV_REFLECT)
		GraphicsPipelineBuilder& reflectSPVForDescriptors(DescriptorSetLayoutCache& layoutCache);
		GraphicsPipelineBuilder& reflectSPVForPushConstants();
#endif // VULKANHELPER_USE_SPIRV_REFLECT

	private:
#if defined(VULKANHELPER_USE_SPIRV_REFLECT)
		std::vector<std::pair<vk::ShaderStageFlagBits, const std::vector<uint32_t>*>> spvs;
#endif // VULKANHELPER_USE_SPIRV_REFLECT
		vk::Device device;
		vk::RenderPass renderPass;
		uint32_t subPass;
		vk::PipelineCache pipelineCache;
		std::optional<vk::Viewport> viewport;
		std::optional<vk::Rect2D> scissor;
		std::optional<vk::PipelineVertexInputStateCreateInfo> vertexInput;
		std::optional<vk::PipelineInputAssemblyStateCreateInfo> inputAssembly;
		std::optional<vk::PipelineRasterizationStateCreateInfo> rasterization;
		std::optional<vk::PipelineMultisampleStateCreateInfo> multisampling;
		std::optional<vk::PipelineDepthStencilStateCreateInfo> depthStencil;
		std::optional<vk::PipelineColorBlendStateCreateInfo> colorBlend;
		std::vector<vk::PipelineColorBlendAttachmentState> colorAttachmentBlends;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::vector<vk::DynamicState> dynamicStateEnable;
		std::vector<vk::PushConstantRange> pushConstants;
		std::vector<vk::DescriptorSetLayout> descLayouts;

		std::vector<vk::UniqueShaderModule> shaderModules;
	};
	class ComputePipelineBuilder {
	public:
		ComputePipelineBuilder(vk::Device device, vk::PipelineCache pipelineCache = nullptr);
		Pipeline build();

		ComputePipelineBuilder& setShaderStage(
			const std::vector<uint32_t>* spv,
			vk::PipelineShaderStageCreateFlags flags = {},
			vk::SpecializationInfo* pSpecializationInfo = {});
		ComputePipelineBuilder& addPushConstants(const vk::PushConstantRange& pushconstants);
		ComputePipelineBuilder& setDescriptorLayouts(const std::vector<vk::DescriptorSetLayout>& layouts);
#if defined(VULKANHELPER_USE_SPIRV_REFLECT)
		ComputePipelineBuilder& reflectSPVForDescriptors(DescriptorSetLayoutCache& layoutCache);
		ComputePipelineBuilder& reflectSPVForPushConstants();
#endif

	private:
		vk::Device device;
		vk::PipelineCache pipelineCache;
#if defined(VULKANHELPER_USE_SPIRV_REFLECT)
		const std::vector<uint32_t>* spv;
#endif
		vk::PipelineShaderStageCreateInfo shaderStage;
		std::vector<vk::PushConstantRange> pushConstants;
		std::vector<vk::DescriptorSetLayout> descLayouts;

		vk::UniqueShaderModule shaderModule;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
#if defined(VULKANHELPER_USE_SPIRV_REFLECT)
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::reflectSPVForDescriptors(DescriptorSetLayoutCache& layoutCache) {
		std::vector<std::unordered_map<uint32_t, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>>> setMaps;

		if (this->descLayouts.size() > 0) {
			std::cerr << "vulkan helper warning: there are diescriptor set layouts set before reflectSPV. All descriptor set layouts will be replaced by reflectSPV!\n";
		}

		for (auto [stage, spvPtr] : spvs) {
			setMaps.push_back(reflectSetBindings(*spvPtr, stage));
		}
		this->descLayouts = mergeReflectedSetBindings(setMaps, layoutCache);

		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::reflectSPVForPushConstants() {
		std::vector<std::vector<vk::PushConstantRange>> results; 
		if (this->descLayouts.size() > 0) {
			std::cerr << "vulkan helper warning: there are push constants ranges set before reflectSPV. All push constant ranges will be replaced by reflectSPVs!\n";
		}
		std::vector<std::vector<vk::PushConstantRange>> ranges;
		for (auto [stage, spvPtr] : spvs) {
			ranges.push_back(reflectPushConstants(*spvPtr, stage));
		}
		this->pushConstants = mergeReflectedPushConstants(ranges);
		return *this;
	}
#endif // VULKANHELPER_USE_SPIRV_REFLECT

	GraphicsPipelineBuilder::GraphicsPipelineBuilder(vk::Device device, vk::RenderPass pass, vk::PipelineCache pipelineCache)
		: device{ device }, renderPass{ pass }, pipelineCache{ pipelineCache } {
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setSubPass(uint32_t index) {
		this->subPass = index;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setViewport(const vk::Viewport& viewport) {
		this->viewport = viewport;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setScissor(const vk::Rect2D& scissor) {
		this->scissor = scissor;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setVertexInput(const vk::PipelineVertexInputStateCreateInfo& vertexInput) {
		this->vertexInput = vertexInput;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setInputAssembly(const vk::PipelineInputAssemblyStateCreateInfo& inputAssembly) {
		this->inputAssembly = inputAssembly;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setRasterization(const vk::PipelineRasterizationStateCreateInfo& rasterization) {
		this->rasterization = rasterization;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setMultisampling(const vk::PipelineMultisampleStateCreateInfo& multisampling) {
		this->multisampling = multisampling;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setDepthStencil(const vk::PipelineDepthStencilStateCreateInfo& depthStencil) {
		this->depthStencil = depthStencil;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setColorBlend(const vk::PipelineColorBlendStateCreateInfo& colorBlend) {
		this->colorBlend = colorBlend;
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addColorBlendAttachemnt(const vk::PipelineColorBlendAttachmentState& colorAttachmentBlend) {
		this->colorAttachmentBlends.push_back(colorAttachmentBlend);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addShaderStage(
		const std::vector<uint32_t>* spv,
		vk::ShaderStageFlagBits shaderStage,
		vk::PipelineShaderStageCreateFlags flags,
		vk::SpecializationInfo* pSpecializationInfo) {
		vk::ShaderModuleCreateInfo moduleCreateInfo{
			.codeSize = static_cast<uint32_t>(spv->size()) * sizeof(uint32_t),
			.pCode = spv->data() };
#if defined(VULKANHELPER_USE_SPIRV_REFLECT)
		this->spvs.emplace_back(shaderStage, spv);
#endif // VULKANHELPER_USE_SPIRV_REFLECT
		this->shaderModules.push_back(device.createShaderModuleUnique(moduleCreateInfo));
		vk::PipelineShaderStageCreateInfo pipelineStageCI{
			.stage = shaderStage,
			.module = this->shaderModules.back().get(),
			.pName = "main" };
		this->shaderStages.push_back(pipelineStageCI);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addDynamicState(const vk::DynamicState& dynamicstates) {
		this->dynamicStateEnable.push_back(dynamicstates);
		return *this;
	}
	GraphicsPipelineBuilder& GraphicsPipelineBuilder::addPushConstants(const vk::PushConstantRange& pushconstant) {
		this->pushConstants.push_back(pushconstant);
		return *this;
	}

	GraphicsPipelineBuilder& GraphicsPipelineBuilder::setDescriptorLayouts(const std::vector<vk::DescriptorSetLayout>& layouts) {
		this->descLayouts = layouts;
		return *this;
	}

	vk::PipelineRasterizationStateCreateInfo makeDefaultRasterisationStateCreateInfo(vk::PolygonMode polygonMode) {
		return vk::PipelineRasterizationStateCreateInfo{
			.polygonMode = polygonMode,
			.cullMode = vk::CullModeFlagBits::eNone,
			.frontFace = vk::FrontFace::eClockwise,
			.lineWidth = 1.0f,
		};
	}

	vk::PipelineMultisampleStateCreateInfo makeDefaultMultisampleStateCreateInfo() {
		return vk::PipelineMultisampleStateCreateInfo{ .minSampleShading = 1.0f };
	}

	vk::PipelineColorBlendAttachmentState makeDefaultColorBlendSAttachmentState() {
		return vk::PipelineColorBlendAttachmentState{
			.blendEnable = VK_FALSE,
			.colorWriteMask =
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA,
		};
	}

	Pipeline GraphicsPipelineBuilder::build() {
		if (!vertexInput) {
			std::cout << "error: vertexInput was not specified in pipeline builder!\n";
			exit(-1);
		}

		// set state create infos:
		vk::PipelineVertexInputStateCreateInfo pvertexInputCI = vertexInput.value();
		vk::PipelineInputAssemblyStateCreateInfo pinputAssemlyStateCI = inputAssembly.value_or(vk::PipelineInputAssemblyStateCreateInfo{ .topology = vk::PrimitiveTopology::eTriangleList });
		vk::PipelineRasterizationStateCreateInfo prasterizationStateCI = rasterization.value_or(vkh::makeDefaultRasterisationStateCreateInfo(vk::PolygonMode::eFill));
		vk::PipelineMultisampleStateCreateInfo multisamplerStateCI = multisampling.value_or(vkh::makeDefaultMultisampleStateCreateInfo());
		vk::PipelineDepthStencilStateCreateInfo pDepthStencilStateCI = depthStencil.value_or(vk::PipelineDepthStencilStateCreateInfo{});
		vk::Viewport pviewport = viewport.value_or(vk::Viewport{ .width = 1, .height = 1 });
		vk::Rect2D pscissor = scissor.value_or(vk::Rect2D{ .extent = {static_cast<uint32_t>(pviewport.width), static_cast<uint32_t>(pviewport.height)} });

		Pipeline pipeline;

		//build pipeline layout:
		vk::PipelineLayoutCreateInfo layoutCI{
			.setLayoutCount = static_cast<uint32_t>(descLayouts.size()),
			.pSetLayouts = descLayouts.data(),
			.pushConstantRangeCount = uint32_t(pushConstants.size()),
			.pPushConstantRanges = pushConstants.data(),
		};

		pipeline.layout = device.createPipelineLayoutUnique(layoutCI);

		vk::PipelineViewportStateCreateInfo viewportStateCI{
			.viewportCount = 1,
			.pViewports = &pviewport,
			.scissorCount = 1,
			.pScissors = &pscissor,
		};

		vk::PipelineColorBlendAttachmentState defaultColorBlendAttachmentStateCI = makeDefaultColorBlendSAttachmentState();
		vk::PipelineColorBlendStateCreateInfo colorBlendingSCI;
		if (this->colorBlend.has_value()) {
			colorBlendingSCI = this->colorBlend.value();
		}
		else if (this->colorAttachmentBlends.size() > 0) {
			colorBlendingSCI = vk::PipelineColorBlendStateCreateInfo{
				.logicOpEnable = VK_FALSE,
				.logicOp = vk::LogicOp::eCopy,
				.attachmentCount = static_cast<uint32_t>(this->colorAttachmentBlends.size()),
				.pAttachments = this->colorAttachmentBlends.data(),
			};
		}
		else {
			colorBlendingSCI = vk::PipelineColorBlendStateCreateInfo{
				.logicOpEnable = VK_FALSE,
				.logicOp = vk::LogicOp::eCopy,
				.attachmentCount = 1,
				.pAttachments = &defaultColorBlendAttachmentStateCI,
			};
		}

		// dynamic state setup:
		if (std::find(dynamicStateEnable.begin(), dynamicStateEnable.end(), vk::DynamicState::eViewport) == dynamicStateEnable.end()) {
			dynamicStateEnable.push_back(vk::DynamicState::eViewport);
		}
		if (std::find(dynamicStateEnable.begin(), dynamicStateEnable.end(), vk::DynamicState::eScissor) == dynamicStateEnable.end()) {
			dynamicStateEnable.push_back(vk::DynamicState::eScissor);
		}

		vk::PipelineDynamicStateCreateInfo dynamicStateCI{
			.dynamicStateCount = (uint32_t)dynamicStateEnable.size(),
			.pDynamicStates = dynamicStateEnable.data(),
		};

		//we now use all of the info structs we have been writing into into this one to create the pipeline
		vk::GraphicsPipelineCreateInfo pipelineCI{
			.stageCount = (uint32_t)shaderStages.size(),
			.pStages = shaderStages.data(),
			.pVertexInputState = &pvertexInputCI,
			.pInputAssemblyState = &pinputAssemlyStateCI,
			.pViewportState = &viewportStateCI,
			.pRasterizationState = &prasterizationStateCI,
			.pMultisampleState = &multisamplerStateCI,
			.pDepthStencilState = &pDepthStencilStateCI,
			.pColorBlendState = &colorBlendingSCI,
			.pDynamicState = &dynamicStateCI,
			.layout = pipeline.layout.get(),
			.renderPass = renderPass,
			.subpass = subPass,
		};

		auto ret = device.createGraphicsPipelineUnique(pipelineCache, pipelineCI);
		if (ret.result != vk::Result::eSuccess) {
			throw std::runtime_error("error: Failed to create graphics pipeline!");
		}
		pipeline.pipeline = std::move(ret.value);

		return pipeline;
	}

	ComputePipelineBuilder::ComputePipelineBuilder(vk::Device device, vk::PipelineCache pipelineCache)
		: device{ device }, pipelineCache{ pipelineCache } {
	}
	ComputePipelineBuilder& ComputePipelineBuilder::setShaderStage(
		const std::vector<uint32_t>* spv,
		vk::PipelineShaderStageCreateFlags flags,
		vk::SpecializationInfo* pSpecializationInfo) {
#if defined(VULKANHELPER_USE_SPIRV_REFLECT)
		this->spv = spv;
#endif
		vk::ShaderModuleCreateInfo moduleCreateInfo{
			.codeSize = static_cast<uint32_t>(spv->size()) * sizeof(uint32_t),
			.pCode = spv->data() };
		this->shaderModule = device.createShaderModuleUnique(moduleCreateInfo);
		vk::PipelineShaderStageCreateInfo pipelineStageCI{
			.stage = vk::ShaderStageFlagBits::eCompute,
			.module = this->shaderModule.get(),
			.pName = "main" };
		this->shaderStage = pipelineStageCI;
		return *this;
	}
	ComputePipelineBuilder& ComputePipelineBuilder::addPushConstants(const vk::PushConstantRange& pushconstant) {
		this->pushConstants.push_back(pushconstant);
		return *this;
	}

	ComputePipelineBuilder& ComputePipelineBuilder::setDescriptorLayouts(const std::vector<vk::DescriptorSetLayout>& layouts) {
		this->descLayouts = layouts;
		return *this;
	}

#if defined(VULKANHELPER_USE_SPIRV_REFLECT)
	ComputePipelineBuilder& ComputePipelineBuilder::reflectSPVForDescriptors(DescriptorSetLayoutCache& layoutCache) {

		if (this->descLayouts.size() > 0) {
			std::cerr << "vulkan helper warning: there are diescriptor set layouts set brefore reflectSPVForDescriptors. All descriptor set layouts will be replaced by reflectSPVForDescriptors!\n";
		}

		auto setMap = reflectSetBindings(*spv, vk::ShaderStageFlagBits::eCompute);
		std::vector vec{ setMap };
		this->descLayouts = mergeReflectedSetBindings(vec, layoutCache);

		const std::vector<uint32_t>* vertexShaderSPV{ nullptr };

		return *this;
	}
	ComputePipelineBuilder& ComputePipelineBuilder::reflectSPVForPushConstants() {
		std::vector<std::vector<vk::PushConstantRange>> results;
		if (this->descLayouts.size() > 0) {
			std::cerr << "vulkan helper warning: there are push constants ranges set before reflectSPV. All push constant ranges will be replaced by reflectSPVs!\n";
		}
		this->pushConstants = reflectPushConstants(*spv, vk::ShaderStageFlagBits::eCompute);
		return *this;
	}
#endif // #if defined(VULKANHELPER_USE_SPIRV_REFLECT)

	Pipeline ComputePipelineBuilder::build() {
		Pipeline pipeline;

		//build pipeline layout:
		vk::PipelineLayoutCreateInfo layoutCI{
			.setLayoutCount = static_cast<uint32_t>(descLayouts.size()),
			.pSetLayouts = descLayouts.data(),
			.pushConstantRangeCount = uint32_t(pushConstants.size()),
			.pPushConstantRanges = pushConstants.data(),
		};

		pipeline.layout = device.createPipelineLayoutUnique(layoutCI);

		//we now use all of the info structs we have been writing into into this one to create the pipeline
		vk::ComputePipelineCreateInfo pipelineCI{
			.stage = this->shaderStage,
			.layout = pipeline.layout.get(),
		};

		auto ret = device.createComputePipelineUnique(pipelineCache, pipelineCI);
		if (ret.result != vk::Result::eSuccess) {
			throw std::runtime_error("error: Failed to create compute pipeline!");
		}
		pipeline.pipeline = std::move(ret.value);

		return pipeline;
	}
#endif

	std::optional<vk::UniqueShaderModule> loadShaderModule(vk::Device device, std::filesystem::path filePath);

	vk::PipelineShaderStageCreateInfo makeShaderStageCreateInfo(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule);

#if defined(VULKANHELPER_IMPLEMENTATION)
	std::optional<vk::UniqueShaderModule> loadShaderModule(vk::Device device, std::filesystem::path filePath) {
		std::ifstream file{ filePath, std::ios::ate | std::ios::binary };

		if (!file.is_open()) {
			return {};
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
		file.seekg(0);
		file.read((char*)buffer.data(), fileSize);
		file.close();

		vk::ShaderModuleCreateInfo createInfo = {};

		createInfo.codeSize = buffer.size() * sizeof(uint32_t);
		createInfo.pCode = buffer.data();

		vk::ShaderModule shaderModule;
		return std::move(device.createShaderModuleUnique(createInfo));
	}

	vk::PipelineShaderStageCreateInfo makeShaderStageCreateInfo(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule) {
		vk::PipelineShaderStageCreateInfo info{};

		info.stage = stage;
		info.module = shaderModule;
		info.pName = "main";
		return info;
	}

#endif

	vk::FenceCreateInfo makeDefaultFenceCI();

	vk::AttachmentDescription makeDefaultColorAttackmentDescription();

#if defined(VULKANHELPER_IMPLEMENTATION)
	vk::FenceCreateInfo makeDefaultFenceCI() {
		vk::FenceCreateInfo info{};
		info.flags |= vk::FenceCreateFlagBits::eSignaled;
		return info;
	}

	vk::AttachmentDescription makeDefaultColorAttackmentDescription() {
		return vk::AttachmentDescription{
			.format = vk::Format::eUndefined,
			.samples = vk::SampleCountFlagBits::e1,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
			.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			.initialLayout = vk::ImageLayout::eUndefined,
			.finalLayout = vk::ImageLayout::ePresentSrcKHR,
		};
	}
#endif

	template <typename T>
	class Pool {
	public:
		Pool() = default;
		Pool(std::function<T(void)> creator, std::function<void(T)> destroyer, std::function<void(T)> resetter) : creator{ std::move(creator) }, destroyer{ std::move(destroyer) }, resetter{ std::move(resetter) } {}

		Pool(Pool&& other) {
			this->creator = std::move(other.creator);
			this->destroyer = std::move(other.destroyer);
			this->resetter = std::move(other.resetter);
			this->pool = std::move(other.pool);
			this->usedList = std::move(other.usedList);
		}

		Pool& operator=(Pool&& other) {
			if (&other == this)
				return *this;
			return *new (this) Pool(std::move(other));
		}

		~Pool() {
			for (auto& el : pool) {
				destroyer(el);
			}
			for (auto& el : usedList) {
				destroyer(el);
			}
			pool.clear();
		}

		void flush() {
			for (auto& el : usedList) {
				resetter(el);
			}
			pool.insert(pool.end(), usedList.begin(), usedList.end());
			usedList.clear();
		}

		T get() {
			if (pool.size() == 0) {
				pool.push_back(creator());
			}

			auto el = pool.back();
			pool.pop_back();
			usedList.push_back(el);
			return el;
		}

	private:
		std::function<T(void)> creator;
		std::function<void(T)> destroyer;
		std::function<void(T)> resetter;
		std::vector<T> pool;
		std::vector<T> usedList;
	};

	class CommandBufferAllocator {
	public:
		CommandBufferAllocator() = default;
		CommandBufferAllocator(vk::Device device, const vk::CommandPoolCreateInfo& poolCI, const vk::CommandBufferLevel& bufferLevel = vk::CommandBufferLevel::ePrimary);

		void flush();

		vk::CommandBuffer getElement();

		vk::CommandPool get();

		vk::CommandPool operator*();

	private:
		vk::Device device;
		vk::UniqueCommandPool pool;
		vk::CommandBufferLevel bufferLevel;
		std::vector<vk::CommandBuffer> unused;
		std::vector<vk::CommandBuffer> used;
	};

#if defined(VULKANHELPER_IMPLEMENTATION)
	CommandBufferAllocator::CommandBufferAllocator(vk::Device device, const vk::CommandPoolCreateInfo& createInfo, const vk::CommandBufferLevel& bufferLevel) : device{ device }, pool{ device.createCommandPoolUnique(createInfo) }, bufferLevel{ bufferLevel } {}
	void CommandBufferAllocator::flush() {
		device.resetCommandPool(*pool);
		unused.insert(unused.end(), used.begin(), used.end());
		used.clear();
	}

	vk::CommandBuffer CommandBufferAllocator::getElement() {
		if (unused.empty()) {
			unused.push_back(device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{ .commandPool = pool.get(), .level = bufferLevel, .commandBufferCount = 1 }).back());
		}
		auto buffer = unused.back();
		unused.pop_back();
		used.push_back(buffer);
		return buffer;
	}

	vk::CommandPool CommandBufferAllocator::get() {
		return pool.get();
	}

	vk::CommandPool CommandBufferAllocator::operator*() {
		return get();
	}

#endif
} // namespace vkh

namespace vkh_detail {
	static PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
	static PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;
} // namespace vkh_detail

#if defined(VULKANHELPER_IMPLEMENTATION)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
	return vkh_detail::pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const* pAllocator) {
	return vkh_detail::pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}
#endif

namespace vkh {
	vk::Instance createInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

	vk::DebugUtilsMessengerEXT createDebugMessenger(vk::Instance instance);

	vk::PhysicalDevice selectPhysicalDevice(vk::Instance instance, const std::function<std::size_t(vk::PhysicalDevice)>& rateDeviceSuitability);

	vk::Device createLogicalDevice(vk::PhysicalDevice physicalDevice, const std::set<std::size_t>& queueIndices, const std::vector<const char*>& extensions);

	std::uint32_t findMemoryTypeIndex(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask);

#if defined(VULKANHELPER_IMPLEMENTATION)
	vk::Instance createInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions) {
		vk::ApplicationInfo vulkanApplicationInfo{ .apiVersion = VK_API_VERSION_1_1 };
		return vk::createInstance({
			.pApplicationInfo = &vulkanApplicationInfo,
			.enabledLayerCount = static_cast<uint32_t>(layers.size()),
			.ppEnabledLayerNames = layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data(),
			});
	}

	vk::DebugUtilsMessengerEXT createDebugMessenger(vk::Instance instance) {
		vkh_detail::pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
		if (!vkh_detail::pfnVkCreateDebugUtilsMessengerEXT)
			throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function.");
		vkh_detail::pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
		if (!vkh_detail::pfnVkDestroyDebugUtilsMessengerEXT)
			throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function.");

		return instance.createDebugUtilsMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT{
			.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
			.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
								  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) -> VkBool32 {
				if (pCallbackData->messageIdNumber == 648835635) {
					// UNASSIGNED-khronos-Validation-debug-build-warning-message
					return VK_FALSE;
				}
				if (pCallbackData->messageIdNumber == 767975156) {
					// UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension
					return VK_FALSE;
				}
				std::string message =
					vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) + ": " +
					vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) + ":\n\ttmessage name   = <" +
					pCallbackData->pMessageIdName + ">\n\tmessage number = " +
					std::to_string(pCallbackData->messageIdNumber) + "\n\tmessage        = <" +
					pCallbackData->pMessage + ">\n";
				if (0 < pCallbackData->queueLabelCount) {
					message += "\tQueue Labels:\n";
					for (uint8_t i = 0; i < pCallbackData->queueLabelCount; i++)
						message += std::string("\t\tlabelName = <") + pCallbackData->pQueueLabels[i].pLabelName + ">\n";
				}
				if (0 < pCallbackData->cmdBufLabelCount) {
					message += "\tCommandBuffer Labels:\n";
					for (uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++)
						message += std::string("\t\tlabelName = <") + pCallbackData->pCmdBufLabels[i].pLabelName + ">\n";
				}
				if (0 < pCallbackData->objectCount) {
					message += "\tObjects:\n";
					for (uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
						message += std::string("\t\tlabelName = <") +
								   pCallbackData->pCmdBufLabels[i].pLabelName + ">\nObject " + std::to_string(i) + "\n\t\t\tobjectType   = " +
								   vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) + "\n\t\t\tobjectHandle = " +
								   std::to_string(pCallbackData->pObjects[i].objectHandle) + "\n";
						if (pCallbackData->pObjects[i].pObjectName)
							message += std::string("\t\t\tobjectName   = <") + pCallbackData->pObjects[i].pObjectName + ">\n";
					}
				}
				//MessageBox(nullptr, message.c_str(), "Vulkan Validation Error", MB_OK);
				return VK_TRUE;
			},
			});
	}

	vk::PhysicalDevice selectPhysicalDevice(vk::Instance instance, const std::function<std::size_t(vk::PhysicalDevice)>& rateDeviceSuitability) {
		auto devices = instance.enumeratePhysicalDevices();
		std::vector<std::size_t> devicesSuitability;
		devicesSuitability.reserve(devices.size());
		for (const auto& device : devices)
			devicesSuitability.push_back(rateDeviceSuitability(device));
		auto bestDeviceIter = std::max_element(devicesSuitability.begin(), devicesSuitability.end());
		if (*bestDeviceIter == 0)
			throw std::runtime_error("Failed to find a suitable physical device");
		return devices[std::distance(devicesSuitability.begin(), bestDeviceIter)];
	}

	vk::Device createLogicalDevice(vk::PhysicalDevice physicalDevice, const std::set<std::size_t>& queueIndices, const std::vector<const char*>& extensions) {
		float queuePriority = 0.0f;
		std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateinfos;
		for (auto index : queueIndices) {
			deviceQueueCreateinfos.push_back({
				.queueFamilyIndex = static_cast<std::uint32_t>(index),
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
				});
		}
		return physicalDevice.createDevice({
			.queueCreateInfoCount = static_cast<std::uint32_t>(deviceQueueCreateinfos.size()),
			.pQueueCreateInfos = deviceQueueCreateinfos.data(),
			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data(),
			});
	}

	std::uint32_t findMemoryTypeIndex(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask) {
		std::uint32_t type_index = std::uint32_t(~0);
		for (std::uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
			if (typeBits & 1 && (memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask) {
				type_index = i;
				break;
			}
			typeBits >>= 1;
		}
		if (type_index == std::uint32_t(~0))
			throw std::runtime_error("Unable to find suitable memory type index");
		return type_index;
	}
#endif
} // namespace vkh
