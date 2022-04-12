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

#include "Handle.hpp"
#include "Image.hpp"
#include "Buffer.hpp"
#include "Sampler.hpp"

namespace daxa {
	constexpr inline size_t MAX_BINDINGS_PER_SET = 32;
	char const* const MORE_THAN_MAX_BINDINGS_MESSAGE = "a binding set can only have up to 16 bindings";

	class BindingSet;

	namespace {
		struct BindingSetAllocatorBindingiSetPool {
			VkDescriptorPool 							pool 			= VK_NULL_HANDLE;
			size_t 										allocatedSets 	= 0;
			std::vector<std::shared_ptr<BindingSet>> 	zombies 		= {};
			std::mutex 									mut 			= {};
		};
	}

	struct BindingLayout {
		bool operator==(BindingLayout const& other) const {
			return 	descriptorType == other.descriptorType && 
					descriptorCount == other.descriptorCount && 
					stageFlags == other.stageFlags &&
					bindingFlag == other.bindingFlag &&
					immutableSamplers == other.immutableSamplers;
		}

		VkDescriptorType      		descriptorType 		= {};
		u32              			descriptorCount 	= {};
		VkShaderStageFlags    		stageFlags 			= {};
		VkDescriptorBindingFlags  	bindingFlag			= {};
		std::vector<SamplerHandle>	immutableSamplers 	= {};
	};

	struct BindingSetDescription {
		bool operator==(BindingSetDescription const& other) const {
			for (size_t i = 0; i < MAX_BINDINGS_PER_SET; i++) {
				if (layouts[i] != other.layouts[i]) {
					return false;
				}
			}
			return flags == other.flags;
		}

		std::array<BindingLayout, MAX_BINDINGS_PER_SET> layouts = {};
		VkDescriptorPoolCreateFlags						flags 	= {};
	};

	struct BindingSetDesciptionHasher {
		size_t operator()(BindingSetDescription const& description) const
		{
			size_t hash = 0x44fe7234f2;
			for (size_t i = 0; i < MAX_BINDINGS_PER_SET; i++) {
				hash ^= description.layouts[i].descriptorCount << 1;
				hash ^= static_cast<u32>(description.layouts[i].descriptorType) << 2;
				hash ^= description.layouts[i].stageFlags << 3;
				hash ^= description.layouts[i].bindingFlag << 4;
				hash <<= 1;
			}
			hash ^= description.flags;
			return hash;
		}
	};

	class BindingSetLayout {
	public:
		BindingSetLayout() = default;
		BindingSetLayout(std::shared_ptr<DeviceBackend> deviceBackend, BindingSetDescription const& description);
		BindingSetLayout(BindingSetLayout const&) 			= delete;
		BindingSetLayout operator=(BindingSetLayout const&) = delete;
		BindingSetLayout(BindingSetLayout&&) noexcept;
		BindingSetLayout& operator=(BindingSetLayout&&) noexcept;
		~BindingSetLayout();

		BindingSetDescription const& getDescription() const { return description; }
		VkDescriptorSetLayout const& getVkDescriptorSetLayout() const { return layout; }
		std::span<VkDescriptorSetLayoutBinding const> getVkDescriptorBindingLayouts() const { 
			return std::span<VkDescriptorSetLayoutBinding const>{ descriptorSetBindingLayouts.data(), descriptorSetBindingLayoutsCount }; 
		}
	private:
		friend class BindingSetLayoutCache;

		void cleanup();

		std::shared_ptr<DeviceBackend> 									deviceBackend 							= {};
		BindingSetDescription 											description 							= {};
		std::array<VkDescriptorSetLayoutBinding, MAX_BINDINGS_PER_SET> 	descriptorSetBindingLayouts 			= {};
		size_t 															descriptorSetBindingLayoutsCount 		= {};
		std::array<VkDescriptorBindingFlags, MAX_BINDINGS_PER_SET> 		descriptorSetBindingLayoutFlags 		= {};
		size_t 															descriptorSetBindingLayoutFlagCounts 	= {};
		std::vector<std::vector<VkSampler>>								vkImmutableSamplers						= {};
		VkDescriptorSetLayout 											layout									= {};
		size_t 															descriptorCount 						= {};
	};

	class BindingSet {
	public:
		BindingSet(std::shared_ptr<DeviceBackend> deviceBackend, VkDescriptorSet set, std::weak_ptr<BindingSetAllocatorBindingiSetPool> pool, std::shared_ptr<BindingSetLayout const> info);
		BindingSet(BindingSet const&)					= delete;
		BindingSet& operator=(BindingSet const&)		= delete;
		BindingSet(BindingSet&&) noexcept				= delete;
		BindingSet& operator=(BindingSet&&) noexcept 	= delete;

		void bindSamplers(u32 binding, std::span<SamplerHandle> samplers, u32 descriptorArrayOffset = 0);
		void bindSampler(u32 binding, SamplerHandle set, u32 dstArrayElement = 0);

		void bindBuffers(u32 binding, std::span<BufferHandle> buffers, u32 descriptorArrayOffset = 0);
		void bindBuffer(u32 binding, BufferHandle, u32 dstArrayElement = 0);

		void bindImages(u32 binding, std::span<std::pair<ImageViewHandle, VkImageLayout>> images, u32 descriptorArrayOffset = 0);
		void bindImage(u32 binding, ImageViewHandle image, VkImageLayout imgLayout, u32 dstArrayElement = 0);

		VkDescriptorSet getVkDescriptorSet() const { return set; }
		BindingSetLayout const& getLayout() { return *layout; }
		std::shared_ptr<BindingSetLayout const> getInfoShared() { return layout; }

		std::string const& getDebugName() const { return debugName; }
	private:
		friend class Device;
		friend class CommandListBackend;
		friend class BindingSetAllocator;
		friend class CommandQueue;
		friend struct BindingSetHandleStaticFunctionOverride;

		void setDebugName(char const* debugName);

		std::shared_ptr<DeviceBackend> 						deviceBackend 	= {};
		VkDescriptorSet 									set 			= VK_NULL_HANDLE;
		std::shared_ptr<BindingSetLayout const>				layout 			= {};
		std::weak_ptr<BindingSetAllocatorBindingiSetPool> 	pool 			= {};
		u32 												usesOnGPU 		= 0;
		std::string 										debugName		= {};
	};

	class BindingSetLayoutCache {
	public:
		BindingSetLayoutCache(std::shared_ptr<DeviceBackend> deviceBackend);
		BindingSetLayoutCache(BindingSetLayoutCache const&) 					= delete;
		BindingSetLayoutCache& operator=(BindingSetLayoutCache const&) 		= delete;
		BindingSetLayoutCache(BindingSetLayoutCache&&) noexcept 				= delete;
		BindingSetLayoutCache& operator=(BindingSetLayoutCache&&) noexcept 	= delete;

		BindingSetLayout const& getLayout(BindingSetDescription const& description);
		std::shared_ptr<BindingSetLayout const> getLayoutShared(BindingSetDescription const& description);
	private:
		using MapT = std::unordered_map<BindingSetDescription, std::shared_ptr<BindingSetLayout>, BindingSetDesciptionHasher>;

		std::shared_ptr<DeviceBackend> 	deviceBackend 	= {};
		MapT							map 			= {};
	};

	struct BindingSetHandleStaticFunctionOverride {
		static void cleanup(std::shared_ptr<BindingSet>& value) {
			if (value && value.use_count() == 1) {
				auto pool = value->pool.lock();
				auto lock = std::unique_lock(pool->mut);
				pool->zombies.push_back(std::move(value));
			}
		}
	};

	class BindingSetHandle : public SharedHandle<BindingSet, BindingSetHandleStaticFunctionOverride>{};

	struct BindingSetAllocatorCreateInfo {
		std::shared_ptr<BindingSetLayout const> setLayout 	= {};
		size_t 									setPerPool 	= 64;
		char const* 							debugName 	= {};
	};

	/**
	* Must be externally Synchronized. given out BindingSets must not be externally synchorized.
	*/
	class BindingSetAllocator {
	public:
		BindingSetAllocator(std::shared_ptr<DeviceBackend> deviceBackend, BindingSetAllocatorCreateInfo const& ci);
		BindingSetAllocator() 											= default;
		BindingSetAllocator(BindingSetAllocator&&) noexcept 			= delete;
		BindingSetAllocator& operator=(BindingSetAllocator&&) noexcept 	= delete;
		BindingSetAllocator(BindingSetAllocator const&) 				= delete;
		BindingSetAllocator& operator=(BindingSetAllocator const&) 		= delete;
		~BindingSetAllocator();

		BindingSetHandle getSet(const char* debugName = {});
		std::string const& getDebugName() const { return debugName; }
	private:
		void initPoolSizes();
		BindingSetHandle getNewSet(std::shared_ptr<BindingSetAllocatorBindingiSetPool>& pool);
		std::shared_ptr<BindingSetAllocatorBindingiSetPool> getNewPool();

		std::shared_ptr<DeviceBackend> 										deviceBackend 	= {};
		size_t 																setsPerPool 	= 0;
		std::vector<VkDescriptorPoolSize> 									poolSizes 		= {};
		std::shared_ptr<BindingSetLayout const> 							setLayout 		= {};
		std::vector<std::shared_ptr<BindingSetAllocatorBindingiSetPool>> 	pools 			= {};
		std::string 														debugName 		= {};
		std::string 														poolNameBuffer	= {};
	};

	class BindingSetAllocatorHandle : public SharedHandle<BindingSetAllocator>{};

	inline static const BindingSetDescription BIND_ALL_SET_DESCRIPTION {
		.layouts = {
			BindingLayout{
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
				.descriptorCount = (1<<16),
				.stageFlags = VK_SHADER_STAGE_ALL,
			},
			BindingLayout{
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = (1<<16),
				.stageFlags = VK_SHADER_STAGE_ALL,
			},
			BindingLayout{
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.descriptorCount = (1<<16),
				.stageFlags = VK_SHADER_STAGE_ALL,
			},
			BindingLayout{
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = (1<<16),
				.stageFlags = VK_SHADER_STAGE_ALL,
			},
			BindingLayout{
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.descriptorCount = (1<<16),
				.stageFlags = VK_SHADER_STAGE_ALL,
			},
		},
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
	};
}